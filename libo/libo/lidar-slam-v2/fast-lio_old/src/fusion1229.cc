#include <iostream>
#include <queue>
#include <thread>
#include <cmath>
#include <chrono>
#include <mutex>
#include <fstream>
#include <Eigen/Dense>
#include <yaml-cpp/yaml.h>
#include <ros/ros.h>
#include <nav_msgs/Odometry.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_datatypes.h>
#include "util/GpsPosition.h"
#include "util/LIOPose.h"
#include "util/Position.h"
#include "fusion/global_fusion.h"
#include <std_msgs/Bool.h>
#include "LocalCartesian.hpp"
#include <cstdlib>
#include <unistd.h>
#include <limits.h>

constexpr auto DEG2RAD = M_PI / 180.0;
int nn = 0;

#define DEBUG

// =========== [辅助函数] ============================================
struct Vector
{
    double x;
    double y;
};

bool isValidarray(const double *arr, const int size)
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i] == 0.0)
            return false;
    }
    return true;
}

// 计算两个点之间的欧氏距离
double computeDistance(const double *p1, const double *p2)
{
    return std::sqrt((p1[0] - p2[0]) * (p1[0] - p2[0]) + (p1[1] - p2[1]) * (p1[1] - p2[1]));
}

// ===================================================================

class FusionNode
{
public:
    FusionNode() : nh_("~")
    {
        std::string node_name = ros::this_node::getName();
        std::string gps_topic, lio_slam_topic, fusion_topic, android_topic;
        ParameterInit();
        nh_.param<bool>(node_name + "/location_mode", location_mode_, false);
        nh_.param<std::string>(node_name + "/map_path", map_path_, "./my_map.mp");
        // +++++++++++
        nh_.param<std::string>(node_name + "/last_pub_pos_path", last_pub_pos_path, "/home/nvidia/libo/lidar-slam-v2/src/fast-lio/Pos/last_pub_pos.txt");
        nh_.param<std::string>(node_name + "/gps_topic", gps_topic, "/nanobot/gpsposition");
        nh_.param<std::string>(node_name + "/lio_slam_topic", lio_slam_topic, "/Mower/lio_slam");
        nh_.param<std::string>(node_name + "/fusion_topic", fusion_topic, "/Mower/position");
        // +++++++++++
        nh_.param<std::string>(node_name + "/android_topic", android_topic, "/signal");
        nh_.param<int>(node_name + "/map_index", map_index_, 0);
        nh_.param<int>(node_name + "/pub_fusion", pub_fusion, 0);
        if (location_mode_)
            ReadMap();
        // +++++++++++
        subGps_ = nh_.subscribe(gps_topic, 5, &FusionNode::GPSCallBack, this); // 0822
        subLio_ = nh_.subscribe(lio_slam_topic, 1, &FusionNode::LioCallBack, this);

        pubPosition_ = nh_.advertise<util::Position>(fusion_topic, 1);
        pub_fusion_Reboot_ = nh_.advertise<std_msgs::Bool>("/Mower/reboot", 1);
        pub_gps_Reboot_ = nh_.advertise<std_msgs::Bool>("/nanobot/reboot", 1);
#ifdef DEBUG
        pubDebugGPS_ = nh_.advertise<util::Position>("/Mower/debug/gps", 1);
        pubDebugLIO_ = nh_.advertise<util::Position>("/Mower/debug/lio", 1);
        pubDebugFusion_ = nh_.advertise<util::Position>("/Mower/fusion", 1);
#endif
    }

private:
    void GPSCallBack(const util::GpsPositionConstPtr &msg)
    {
        if (msg == nullptr)
            return;
        {
            std::unique_lock<std::mutex> lock(gps_mutex_);
            gps_queue_.push(msg);
        }

        double time_diff = msg->header.stamp.toSec() - lio_stamp_;

        if (pub_fusion == 2) //! reboot && (system_init_ && time_diff > 1) ||
        {
            double location_gps[3];
            memset(location_gps, 0, sizeof(double) * 3);
            double latitude = msg->latitude;
            double longitude = msg->longitude;
            double altitude = msg->height;
            // +++++++++++
            double gauss_yaw = msg->azimuth - 9000;
            if (gauss_yaw < 0)
            {
                gauss_yaw += 36000;
            }
            gauss_yaw *= 0.01;

            GPS2Local(latitude, longitude, altitude, gauss_yaw, location_gps);

            util::Position gps_position;
            gps_position.header.stamp = msg->header.stamp;
            gps_position.header.frame_id = "base_link";
            gps_position.position_x = location_gps[0];
            gps_position.position_y = location_gps[1];
            gps_position.position_z = 0;
            gps_position.roll = 0;
            gps_position.pitch = 0;
            gps_position.yaw = location_gps[2];
            gps_position.position_state = 3;
            pubPosition_.publish(gps_position);
        }
    }

    void LioCallBack(const util::LIOPoseConstPtr &msg)
    {
        // Is_rate_ok();
        nn++;
        std::cout << "nn = " << nn << std::endl;

        if (!Is_LIO_USABLE(msg) || pub_fusion == 2)
        {
            return;
        }

        double lio_x = -msg->position_y;
        double lio_y = msg->position_x;

        Eigen::Vector3d lio_t(lio_x, lio_y, 1.0);
        Eigen::Quaterniond lio_q;

        lio_q.w() = msg->q_w;
        lio_q.x() = msg->q_x;
        lio_q.y() = msg->q_y;
        lio_q.z() = msg->q_z;
        bool lio_state_ = msg->lio_state;

        double xyy_lio[3];
        memset(xyy_lio, 0, sizeof(double) * 3);
        lio_raw_x_ = 0;
        lio_raw_y_ = 0;
        lio_raw_yaw_ = 0;
        LIOPrecess(lio_t, lio_q, xyy_lio);

        // Publish LIO position.
        if (!reboot && pub_fusion == 3 && lio_state_)
        {
            util::Position lio_position;
            lio_position.header.stamp = msg->header.stamp;
            lio_position.header.frame_id = "base_link";
            lio_position.position_x = xyy_lio[0];
            lio_position.position_y = xyy_lio[1];
            lio_position.position_z = 0;
            lio_position.roll = 0;
            lio_position.pitch = 0;
            lio_position.yaw = xyy_lio[2];
            lio_position.position_state = 4;
            pubPosition_.publish(lio_position);
            publish_count++;
            return;
        }

        // LIO速度判断
        double lio_speed = 0.0;
        double lio_pos_yaw = 0.0;
        // lio_speed_flag_：当前帧的LIO状态，lio_drift_detected_：当前帧或漂移期设置的LIO的状态
        bool lio_speed_flag_ = lio_state_ ? Is_LIO_GOOD(lio_t, last_lio_t_, lio_stamp_, &lio_speed, lio_raw_yaw_, &lio_pos_yaw) : false;

        lio_drift_detected_ = lio_drift_count_ > 0 ? true : lio_drift_detected_;
        lio_drift_count_ = lio_drift_count_ > 0 ? --lio_drift_count_ : lio_drift_count_;

        if (system_init_)
        {
            last_lio_t_ = lio_t;
            last_lio_stamp_ = lio_stamp_;
        }

        // if (nn > 50)
        // {
        //     lio_state_ = false;
        //     lio_speed_flag_ = false;
        //     lio_failed_cnt_ = 20;
        // }

        // LIO数据输入
        bool inputLIO_flag_ = false;
        if (lio_state_ && lio_speed_flag_)
        {
            if (system_init_ && !lio_drift_detected_)
            {
                globalEstimator_.InputOdom(lio_stamp_, lio_t, lio_q);
                inputLIO_flag_ = true;
            }
            lio_failed_cnt_ = 0;
        }
        else
        {
            lio_failed_cnt_++;
            if (lio_failed_cnt_ >= 20) // 可能导致当RTK数据较差时只能重启节点但由于无法得到RTK定位而无法得到定位结果，仅重启/lasermapping？？？
            {
                ROS_WARN("LIO data are not good, restart the node /lasermapping !");
                if (std::system("restart_ros_nodes.sh /laserMapping -- bash -c 'source /home/nvidia/libo/lidar-slam-v2/devel/setup.bash && rosrun fast_lio fastlio_mapping'") != 0)
                {
                    ROS_ERROR("Failed to restart nodes /lasermapping.");
                }
                globalEstimator_.clear_data = true;
                system_init_ = false;
                gps_init_ = false;
                // 重置lio相关状态！！！
                memset(xy_offset_, 0, sizeof(double) * 3);
                lio_drift_detected_ = false;
                lio_drift_count_ = 0;
                lio_failed_cnt_ = 0;
                sync_failed_cnt_ = 0;
                fusion_failed_cnt_ = 0;
                return;
            }

            // if (lio_failed_cnt_ >= 20) // 可能导致当RTK数据较差时只能重启节点但由于无法得到RTK定位而无法得到定位结果，仅重启/lasermapping？？？
            // {
            //     lio_bad_flag_ = true;
            // }

            // if (location_mode_ && lio_failed_cnt_ >= 20)
            // {
            //     reboot = true;
            //     ROS_WARN("LIO raw data are not good, restart the node!");
            //     return;
            // }
        }

        lio_drift_count_ = lio_failed_cnt_ >= 3 ? 30 : lio_drift_count_;
        //------------------------------LIO数据处理----------------------------------//

        //------------------------------GPS数据处理----------------------------------//
        double latitude = 0., longitude = 0., altitude = 0., gps_flag = 0., gauss_yaw = 0., gps_stamp = 0.;
        ros::Time gps_msg_stamp;

        gps_mutex_.lock();
        while (!gps_queue_.empty())
        {
            util::GpsPositionConstPtr gps_msg = gps_queue_.front();
            if (!heading_flag_)
            {
                heading_flag_ = gps_msg->positionStatus;
            }
            gps_msg_stamp = gps_msg->header.stamp;
            gps_stamp = gps_msg->header.stamp.toSec();

            printf("now = %15.4f\n", ros::Time::now().toSec());
            printf("lio_stamp = %15.4f\n", lio_stamp_);
            printf("gps_stamp = %12.4f\n", gps_stamp);
            printf("gps_stamp -lio_stamp = %12.4f\n", gps_stamp - lio_stamp_);
            if (gps_stamp >= lio_stamp_ - 0.05 && gps_stamp <= lio_stamp_ + 0.05)
            {
                latitude = gps_msg->latitude;
                longitude = gps_msg->longitude;
                altitude = gps_msg->height;
                gps_flag = gps_msg->gps_flag;
                gauss_yaw = gps_msg->azimuth - 9000;
                if (gauss_yaw < 0)
                {
                    gauss_yaw += 36000;
                }
                gauss_yaw *= 0.01;
                sync_failed_cnt_ > 0 ? sync_failed_cnt_ -= 2 : sync_failed_cnt_;
                gps_queue_.pop();
                break;
            }
            else if (gps_stamp < lio_stamp_ - 0.05)
            {
                sync_failed_cnt_++;
                gps_queue_.pop();
            }
            else if (gps_stamp > lio_stamp_ + 0.05)
            {
                sync_failed_cnt_++;
                break;
            }
        }
        gps_mutex_.unlock();

        std::cout << "sync_failed_cnt_ = " << sync_failed_cnt_ << std::endl;

        if (sync_failed_cnt_ >= 50)
        {
            // reboot = true;
            // ROS_WARN("---sync failed, restart the node!");

            // std_msgs::Bool gps_reboot_msg;
            // gps_reboot_msg.data = true;
            // pub_gps_Reboot_.publish(gps_reboot_msg);
            // return;

            ROS_WARN("---sync failed, restart the node /lasermapping !");
            if (std::system("restart_ros_nodes.sh /laserMapping -- bash -c 'source /home/nvidia/libo/lidar-slam-v2/devel/setup.bash && rosrun fast_lio fastlio_mapping'") != 0)
            {
                ROS_ERROR("Failed to restart nodes /lasermapping.");
            }
            globalEstimator_.clear_data = true;
            system_init_ = false;
            gps_init_ = false;
            // 重置lio相关状态！！！
            memset(xy_offset_, 0, sizeof(double) * 3);
            lio_drift_detected_ = false;
            lio_drift_count_ = 0;
            lio_failed_cnt_ = 0;
            sync_failed_cnt_ = 0;
            fusion_failed_cnt_ = 0;
            return;
        }

        // GPS状态检测和速度判断
        bool gps_good_flag = false;
        double gps_speed = 0.0;
        double gps_pos_yaw = 0.0;
        double location_gps[3];
        memset(location_gps, 0, sizeof(double) * 3);
        // gps_stamp = gps_flag > 0 ? lio_stamp_ : 0;
        // gps_good_flag：当前帧或漂移期设置的GPS的状态，不能代表当前帧的GPS状态
        gps_good_flag = Is_GPS_GOOD(latitude, longitude, altitude, gps_flag, gauss_yaw, gps_stamp, location_gps, &gps_speed, lio_speed, &gps_pos_yaw);

        // gps有数据且不是漂移期时，更新状态计数
        if (gps_good_flag && latitude != 0. && longitude != 0. && altitude != 0. && gps_flag == 4) // gps 无数据时gps_failed_cnt_不更新，无法切为融合
        {
            gps_stable_cnt_ = gps_stable_cnt_ >= 50 ? 50 : ++gps_stable_cnt_;
            gps_failed_cnt_ = 0;
        }
        else
        {
            gps_failed_cnt_++;
            if (!(latitude != 0. && longitude != 0. && altitude != 0. && gps_flag == 4))
            {
                gps_stable_cnt_ = gps_stable_cnt_ >= 25 ? gps_stable_cnt_ - 5 : gps_stable_cnt_;
            }
        }

        // +++++++++++调整参数
        if (gps_failed_cnt_ >= 5 && gps_drift_count_ == 0) // gps连续5帧异常
        {
            gps_drift_count_ = 30; // 漂移期为30帧
            // gps_failed_cnt_ = 0;     // 重置异常计数
            gps_failed_flag_ = true; // 置GPS连续异常不可用标志，pub数据赋值时不使用gps，使用融合定位数据
        }
        //------------------------------GPS数据处理----------------------------------//

        // 长时间无RTK信号，加载上次保存的定位数据作为当前位置
        // if (location_mode_ && no_rtk_cnt_ == 40) //
        // {
        //     double last_pub_pos[3] = {0.0, 0.0, 0.0};
        //     if (LoadFusionPositionFromFile(last_pub_pos))
        //     {
        //         ROS_INFO("load last published position: %f, %f, %f", last_pub_pos[0], last_pub_pos[1], last_pub_pos[2]);
        //         no_rtk_cnt_ = 0;
        //         geoConverter_.Reverse(last_pub_pos[0], last_pub_pos[1], 0.0, latitude, longitude, altitude);
        //         gps_flag = 4;
        //         gauss_yaw = last_pub_pos[2];
        //         gps_good_flag = true;
        //     }
        //     else
        //     {
        //         ROS_WARN("Failed to load last published position, using default values.");
        //     }
        // }

        // 初始化及GPS数据输入
        bool inputGPS_flag_ = false;

        if (gps_good_flag && !system_init_ && gps_stable_cnt_ >= 50)
        {
            if (!location_mode_)
                SaveMap(latitude, longitude, altitude, gauss_yaw);
            system_init_ = true;
            gps_init_ = false;
            GPS2Local(latitude, longitude, altitude, gauss_yaw, location_gps);
            gps_failed_cnt_ = 0;
            gps_stable_cnt_ = 50;
            gps_failed_flag_ = false;
            no_rtk_cnt_ = 0;
        }
        else
        {
            if (!system_init_)
            {
                no_rtk_cnt_++;
                gps_drift_count_ = 0; //???
                lio_drift_count_ = 0;
                lio_drift_detected_ = false;
                std::cout << "gps_good_flag = " << gps_good_flag << std::endl;
                std::cout << "gps_stable_cnt_ = " << gps_stable_cnt_ << std::endl;
                ROS_WARN("GPS data is not good, please check the GPS module!");
                return;
            }
        }

        if (gps_good_flag && system_init_)
        {
            globalEstimator_.InputGPS(lio_stamp_, latitude, longitude, altitude, gauss_yaw, 0.1);
            inputGPS_flag_ = true;
        }

        // 提取融合定位结果
        double fusion_pos_[3];
        memset(fusion_pos_, 0, sizeof(double) * 3);
        if (inputLIO_flag_)
        {
            GetPosition(fusion_pos_);
            fusion_pos_[2] = inputGPS_flag_ ? location_gps[2] : xyy_lio[2]; // 融合定位的航向
        }

        double pos_state = -1;
        // gps恢复稳定可用时置gps为正常可用，pub赋值时使用gps
        gps_failed_flag_ = gps_stable_cnt_ >= gps_stable_threshold_ ? false : gps_failed_flag_;

        double xyy_p[3];
        memset(xyy_p, 0, sizeof(double) * 3);
        // gps连续异常不到5帧时，不发布定位且不使用融合（gps_failed_flag_为false）；
        // gps连续异常不可用时，gps_failed_flag_为true，gps_stable_cnt_肯定小于阈值，使用融合定位数据
        // gps连续稳定可用，gps_stable_cnt_大于阈值，gps_failed_flag_被置为false，使用gps数据
        // +++++++++++调整参数
        if (gps_failed_cnt_ < 5 && (!gps_failed_flag_ || (gps_failed_flag_ && gps_stable_cnt_ >= gps_stable_threshold_)))
        {
            if (inputGPS_flag_) // gps无数据时无法切换为融合定位
            {
                memcpy(xyy_p, location_gps, sizeof(double) * 3);
                pos_state = 1;
                no_pos_cnt_ = 0;
            }
            else
            {
                ROS_WARN("inputGPS_flag_ = flase, no publish position!");
                return;
            }
        }
        else if (lio_failed_cnt_ < 5 && gps_init_)
        {
            ROS_WARN("gps_failed_cnt_ >= 5 or gps is not stable!");
            gps_failed_cnt_ = 0;
            gps_failed_flag_ = true;
            if (inputLIO_flag_)
            {
                memcpy(xyy_p, fusion_pos_, sizeof(double) * 3);
                pos_state = 2;
                no_pos_cnt_ = 0;
            }
            else
            {
                return;
            }
        }
        else
        {
            // reboot = true;
            // ROS_WARN("GPS and LIO data are not good, stop the car!");
            // return;

            ROS_WARN("fusion data are not good, restart the node /lasermapping !");
            if (std::system("restart_ros_nodes.sh /laserMapping -- bash -c 'source /home/nvidia/libo/lidar-slam-v2/devel/setup.bash && rosrun fast_lio fastlio_mapping'") != 0)
            {
                ROS_ERROR("Failed to restart nodes /lasermapping.");
            }
            globalEstimator_.clear_data = true;
            system_init_ = false;
            gps_init_ = false;
            // 重置lio相关状态！！！
            lio_drift_detected_ = false;
            lio_drift_count_ = 0;
            lio_failed_cnt_ = 0;
            sync_failed_cnt_ = 0;
            fusion_failed_cnt_ = 0;
            return;
        }

        // Publish position.
        if (!reboot && pub_fusion == 1)
        {
            double fusion_time = ros::Time::now().toSec(); // msg->header.stamp.toSec();

            double time_diff = last_fusion_time_ != 0 ? fusion_time - last_fusion_time_ : 0;
            double fusion_speed = 0;
            // +++++++++++调整前后帧时间间隔的有效判断
            if (0 < time_diff && time_diff < 0.15 && isValidarray(fusion_pos_, 3) && isValidarray(last_fusion_pos_, 3))
            {
                fusion_speed = computeDistance(fusion_pos_, last_fusion_pos_) / time_diff;
                std::cout << "fusion_speed = " << fusion_speed << std::endl;
                std::cout << "time_diff = " << time_diff << std::endl;
                last_fusion_time_ = fusion_time;
                memcpy(last_fusion_pos_, fusion_pos_, sizeof(double) * 3);

                if (fusion_failed_cnt_ >= 20)
                {
                    // reboot = true;
                    // ROS_WARN("fusion data are not good, stop the car!");
                    // return;

                    ROS_WARN("fusion data are not good, restart the node /lasermapping !");
                    if (std::system("restart_ros_nodes.sh /laserMapping -- bash -c 'source /home/nvidia/libo/lidar-slam-v2/devel/setup.bash && rosrun fast_lio fastlio_mapping'") != 0)
                    {
                        ROS_ERROR("Failed to restart nodes /lasermapping.");
                    }
                    globalEstimator_.clear_data = true;
                    system_init_ = false;
                    gps_init_ = false;
                    // 重置lio相关状态！！！
                    memset(xy_offset_, 0, sizeof(double) * 3);
                    lio_drift_detected_ = false;
                    lio_drift_count_ = 0;
                    lio_failed_cnt_ = 0;
                    sync_failed_cnt_ = 0;
                    fusion_failed_cnt_ = 0;
                    return;
                }
                // +++++++++++调整速度阈值
                if (fusion_speed > 3)
                {
                    fusion_failed_cnt_++;
                    return;
                }
                else
                {
                    fusion_failed_cnt_ = 0;
                }
            }
            last_fusion_time_ = fusion_time;
            memcpy(last_fusion_pos_, fusion_pos_, sizeof(double) * 3);

            util::Position fusion_position;
            fusion_position.header.stamp = msg->header.stamp;
            fusion_position.header.frame_id = "base_link";
            fusion_position.position_x = xyy_p[0];
            fusion_position.position_y = xyy_p[1];
            fusion_position.position_z = 0;
            fusion_position.roll = 0;
            fusion_position.pitch = 0;
            fusion_position.yaw = xyy_p[2];
            if (heading_flag_)
            {
                fusion_position.position_state = pos_state;
            }
            else
            {
                fusion_position.position_state = 9;
            }
            pubPosition_.publish(fusion_position);
            // 保存最后一个 fusion_position 数据到文件
            // SaveFusionPositionToFile(xyy_p);

            publish_count++;
        }

#ifdef DEBUG

        double xyy_g[3];
        memset(xyy_g, 0, sizeof(double) * 3);
        if (gps_good_flag)
        {
            GPS2Local(latitude, longitude, altitude, gauss_yaw, xyy_g);
        }
        util::Position gps2xyz_msg;
        gps2xyz_msg.header.frame_id = "map";
        gps2xyz_msg.header.stamp = gps_msg_stamp;
        gps2xyz_msg.position_x = xyy_g[0];
        gps2xyz_msg.position_y = xyy_g[1];
        gps2xyz_msg.position_z = gps_speed;
        gps2xyz_msg.position_state = gps_good_flag;
        gps2xyz_msg.pitch = gps_pos_yaw * (M_PI / 180.0);
        gps2xyz_msg.yaw = xyy_g[2];
        pubDebugGPS_.publish(gps2xyz_msg);

        if (lio_state_)
        {
            util::Position LIO_msg;
            LIO_msg.header.frame_id = "map";
            LIO_msg.header.stamp = msg->header.stamp;
            LIO_msg.position_x = xyy_lio[0];
            LIO_msg.position_y = xyy_lio[1];
            LIO_msg.position_z = lio_speed;
            LIO_msg.position_state = lio_drift_detected_;
            LIO_msg.pitch = lio_pos_yaw;
            LIO_msg.yaw = xyy_lio[2];
            pubDebugLIO_.publish(LIO_msg);

            util::Position fusion_msg;
            fusion_msg.header.frame_id = "map";
            fusion_msg.header.stamp = msg->header.stamp;
            fusion_msg.position_x = fusion_pos_[0];
            fusion_msg.position_y = fusion_pos_[1];
            fusion_msg.position_z = 0;
            fusion_msg.yaw = fusion_pos_[2];
            fusion_msg.position_state = inputGPS_flag_ ? 1 : 0;
            pubDebugFusion_.publish(fusion_msg);
        }

#endif
    }

    bool Is_LIO_GOOD(const Eigen::Vector3d &lio_t, const Eigen::Vector3d &last_lio_t_, const double lio_stamp_, double *lio_speed, double lio_yaw, double *lio_pos_yaw)
    {
        if (last_lio_stamp_ == 0)
        {
            lio_drift_detected_ = false;
            return true;
        }

        double time_diff = lio_stamp_ - last_lio_stamp_;
        // +++++++++++调整前后帧时间间隔的有效判断
        if (time_diff <= 0.05 || time_diff > 0.15)
        {
            ROS_WARN("Invalid time difference: %f", time_diff);
            lio_drift_detected_ = false;
            return true;
        }
        else
        {
            double distance = (lio_t - last_lio_t_).norm();
            *lio_speed = distance / time_diff;

            // 如果速度超过阈值，标记为漂移
            if (*lio_speed >= lio_speed_threshold_)
            {
                lio_drift_detected_ = true;
                // lio_drift_count_ = *lio_speed > 2 ? 30 : 10; // 设置漂移计数器为 30
                return false;
            }
            // +++++++++++调整参数
            if (location_mode_ && *lio_speed > 0.7)
            {
                // 计算前后两帧的位置航向角
                double delta_x = lio_t.x() - last_lio_t_.x();
                double delta_y = lio_t.y() - last_lio_t_.y();
                *lio_pos_yaw = atan2(delta_y, delta_x); // 计算航向角（弧度）

                // 将计算的航向角转换到 [-π, π] 范围
                *lio_pos_yaw = ConvertYaw(*lio_pos_yaw);
                while (*lio_pos_yaw >= 2 * M_PI)
                {
                    *lio_pos_yaw -= 2 * M_PI;
                }

                // 对比计算的航向角与 LIO 消息中的航向角
                double yaw_diff = fabs(*lio_pos_yaw - lio_yaw);
                if (yaw_diff > M_PI)
                {
                    yaw_diff = 2 * M_PI - yaw_diff;
                }

                // 如果航向角差值过大，标记为漂移
                // +++++++++++调整参数
                if (yaw_diff > 0.7)
                {
                    // lio_drift_detected_ = true;
                    // lio_drift_count_ = yaw_diff > 0.87 ? 30 : 10;
                    ROS_WARN("LIO drift detected due to yaw difference: *lio_pos_yaw = %.2f, lio_yaw = %.2f, yaw_diff = %.2f", *lio_pos_yaw, lio_yaw, yaw_diff);
                    // return false;
                }
            }
            lio_drift_detected_ = false;
            return true;
        }
    }

private:
    void ParameterInit()
    {
        // +++++++++++调整参数
        fail_cnt_ = 0;
        origin_gauss_yaw_ = 0.;
        map_index_ = 0;
        origin_latitude_ = 0.;
        origin_longitude_ = 0.;
        origin_height_ = 0.;
        gps_init_ = false;
        system_init_ = false;
        location_mode_ = false;
        memset(xy_offset_, 0, sizeof(double) * 3);
        memset(last_gps_, 0, sizeof(double) * 3);
        last_gps_stamp_ = 0.0;
        gps_drift_count_ = 0;
        last_lio_t_ = Eigen::Vector3d(0, 0, 0);
        last_lio_stamp_ = 0.0;
        pub_fusion = 0;
        lio_speed_threshold_ = 2;
        lio_stamp_ = 0;
        lio_drift_count_ = 0;
        lio_drift_detected_ = false;
        lio_raw_x_ = 0.0;
        lio_raw_y_ = 0.0;
        lio_raw_yaw_ = 0.0;
        no_pos_cnt_ = 0;
        reboot = false;
        gps_failed_cnt_ = 5;
        lio_failed_cnt_ = 0;
        last_fusion_time_ = 0.0;
        memset(last_fusion_pos_, 0, sizeof(double) * 3);
        fusion_failed_cnt_ = 0;

        pub_overtime_cnt_ = 0;

        gps_stable_cnt_ = 45;
        gps_stable_threshold_ = 50; // GPS稳定阈值
        gps_failed_flag_ = false;

        publish_window_start = 0;
        publish_count = 0;

        publish_error_cnt_ = 0;

        heading_flag_ = 0;

        sync_failed_cnt_ = 0;

        lio_bad_flag_ = false;

        reboot_pub_flag_ = false;

        map_saved_flag_ = false;
    }
    // TODO
    bool Is_GPS_GOOD(double latitude, double longitude, double altitude, double gps_flag, double gauss_yaw, double gps_stamp, double *location_gps, double *gps_speed, double lio_speed, double *gps_pos_yaw)
    {
        if (gps_drift_count_ > 0)
        {
            gps_drift_count_--;
            return false;
        }
        // 检查 GPS 数据是否有效
        if (latitude != 0. && longitude != 0. && altitude != 0. && gps_flag == 4)
        {
            GPS2Local(latitude, longitude, altitude, gauss_yaw, location_gps);
            if (last_gps_stamp_ && gps_stamp)
            {
                double gps_stamp_diff = gps_stamp - last_gps_stamp_;
                // 检查时间间隔和位置有效性
                // +++++++++++调整前后帧时间间隔的有效判断
                if ((0.08 < gps_stamp_diff && gps_stamp_diff < 0.22) && isValidarray(location_gps, 3) && isValidarray(last_gps_, 3))
                {
                    *gps_speed = computeDistance(location_gps, last_gps_);
                    *gps_speed = fabs(*gps_speed / gps_stamp_diff);
                    // std::cout << "GPS Speed: " << *gps_speed << std::endl;

                    // 检测 GPS 漂移
                    // +++++++++++调整GPS速度阈值
                    if (*gps_speed > 2)
                    {
                        ROS_WARN("GPS drift detected due to high speed: %.2f m/s", *gps_speed);
                        return false;
                    }

                    // // LIO速度小于0.2m/s，则认为车辆为静止状态，如果GPS速度较大，认为GPS异常
                    // if (0 < lio_speed && lio_speed < 0.15 && *gps_speed > 1.5)
                    // {
                    //     ROS_WARN("GPS speed is too high while LIO speed is low: LIO speed = %.2f, GPS speed = %.2f", lio_speed, *gps_speed);
                    //     return false;
                    // }

                    // GPS和LIO的速度同时大于0.4，则认为车辆在运动，此时若航向角差值过大，认为GPS异常
                    // +++++++++++调整参数
                    if (location_mode_ && *gps_speed > 0.5) //&& lio_speed > 0.7)
                    {
                        // 计算前后两帧的位置航向角
                        double delta_x = location_gps[0] - last_gps_[0];
                        double delta_y = location_gps[1] - last_gps_[1];
                        *gps_pos_yaw = atan2(delta_y, delta_x) * (180.0 / M_PI); // 转换为角度

                        // 将计算的航向角转换到 [0, 360) 范围
                        if (*gps_pos_yaw < 0)
                        {
                            *gps_pos_yaw += 360.0;
                        }
                        while (*gps_pos_yaw >= 360.0)
                        {
                            *gps_pos_yaw -= 360.0;
                        }

                        // 对比计算的航向角与真实的 location_gps[2]
                        double gps_yaw = location_gps[2] * (180 / M_PI);
                        gps_yaw = gps_yaw < 0 ? gps_yaw + 360 : gps_yaw;

                        double yaw_diff = fabs(*gps_pos_yaw - gps_yaw);
                        if (yaw_diff > 180.0)
                        {
                            yaw_diff = 360.0 - yaw_diff; // 处理周期性角度差
                        }

                        // 如果航向角差值过大，认为 GPS 漂移
                        // +++++++++++调整参数
                        if (yaw_diff > 40.0)
                        {
                            ROS_WARN("GPS drift detected due to yaw difference: *gps_pos_yaw = %.2f, gauss_yaw = %.2f, yaw_diff = %.2f", *gps_pos_yaw, gauss_yaw, yaw_diff);
                            return false;
                        }
                    }
                }
            }

            // 如果没有检测到漂移，更新 GPS 数据

            last_gps_stamp_ = gps_stamp;
            memcpy(last_gps_, location_gps, sizeof(double) * 3);
            return true;
        }

        // GPS 数据无效
        ROS_WARN("GPS data is not valid.");
        return false;
    }

    bool Is_LIO_USABLE(const util::LIOPoseConstPtr &msg)
    {
        // 检查是否需要重启
        if (reboot && !reboot_pub_flag_)
        {
            // 发布 reboot 话题
            std_msgs::Bool fusion_reboot_msg;
            fusion_reboot_msg.data = true;
            pub_fusion_Reboot_.publish(fusion_reboot_msg);

            // std_msgs::Bool gps_reboot_msg;
            // gps_reboot_msg.data = true;
            // pub_gps_Reboot_.publish(gps_reboot_msg);

            ROS_WARN("Reboot triggered, publishing reboot message and shutting down the node.");

            reboot_pub_flag_ = true;
            return false;
        }

        // 检查 LIO 消息是否为空
        if (!msg)
        {
            ROS_ERROR("Received null LIO message.");
            return false;
        }

        // 检查 LIO 时间戳是否有效
        lio_stamp_ = msg->header.stamp.toSec();
        if (lio_stamp_ <= 0)
        {
            ROS_WARN("Invalid LIO timestamp: %f", lio_stamp_);
            return false;
        }

        return true; // 检查通过
    }

    bool SaveMap(double latitude, double longitude, double height, double gauss_yaw)
    {
        if (map_saved_flag_)
        {
            return true;
        }

        std::ofstream fcout(map_path_, std::ios::binary);
        if (!fcout.is_open())
        {
            std::cerr << "Error(Save map): Unable to open map file." << std::endl;
            return false;
        }

        char map_data[36];
        memset(map_data, 0, 36);
        std::memcpy(map_data, &map_index_, 4);
        std::memcpy(map_data + 4, &latitude, 8);
        std::memcpy(map_data + 12, &longitude, 8);
        std::memcpy(map_data + 20, &height, 8);
        std::memcpy(map_data + 28, &gauss_yaw, 8);
        fcout.write(map_data, 36);
        fcout.close();

        map_saved_flag_ = true;
        return true;
    }

    bool ReadMap()
    {
        char data[36];
        memset(data, 0, 36);

        std::ifstream file(map_path_, std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Error(Read map): Unable to open file." << std::endl;
            return false;
        }

        file.read(data, 36);
        file.close();
        std::memcpy(&map_index_, data, 4);
        std::memcpy(&origin_latitude_, data + 4, 8);
        std::memcpy(&origin_longitude_, data + 12, 8);
        std::memcpy(&origin_height_, data + 20, 8);
        std::memcpy(&origin_gauss_yaw_, data + 28, 8);

        // 初始化参数.
        globalEstimator_.location_mode = true;
        globalEstimator_.origin_latitude = origin_latitude_;
        globalEstimator_.origin_longitude = origin_longitude_;
        globalEstimator_.origin_altitude = origin_height_;
        globalEstimator_.origin_gauss_yaw = origin_gauss_yaw_;

        if (origin_gauss_yaw_ == 0 || origin_latitude_ == 0 || origin_height_ == 0 || origin_longitude_ == 0)
        {
            std::cerr << "<--- Read map error! --->" << std::endl;
            return false;
        }
        return true;
    }

    void GPS2Local(double latitude, double longitude, double height, double gauss_yaw, double *xyy)
    {
        if (!gps_init_)
        {
            if (!location_mode_)
            {
                geoConverter_.Reset(latitude, longitude, height);
                origin_gauss_yaw_ = gauss_yaw;
            }
            else
            {
                geoConverter_.Reset(origin_latitude_, origin_longitude_, origin_height_);
                origin_gauss_yaw_ = origin_gauss_yaw_;
                double u_xyz[3];
                geoConverter_.Forward(latitude, longitude, height, u_xyz[0], u_xyz[1], u_xyz[2]);
                if (system_init_)
                {
                    xy_offset_[0] = u_xyz[0] * cos(origin_gauss_yaw_ * DEG2RAD) + u_xyz[1] * sin(origin_gauss_yaw_ * DEG2RAD);
                    xy_offset_[1] = -u_xyz[0] * sin(origin_gauss_yaw_ * DEG2RAD) + u_xyz[1] * cos(origin_gauss_yaw_ * DEG2RAD);
                    xy_offset_[2] = gauss_yaw - origin_gauss_yaw_;
                }
            }
            gps_init_ = true;
        }
        double xyz[3];
        geoConverter_.Forward(latitude, longitude, height, xyz[0], xyz[1], xyz[2]);
        xyy[0] = xyz[0] * cos(origin_gauss_yaw_ * DEG2RAD) + xyz[1] * sin(origin_gauss_yaw_ * DEG2RAD);
        xyy[1] = -xyz[0] * sin(origin_gauss_yaw_ * DEG2RAD) + xyz[1] * cos(origin_gauss_yaw_ * DEG2RAD);
        double yaw = (gauss_yaw - origin_gauss_yaw_) * DEG2RAD;
        yaw = ConvertYaw(yaw + M_PI / 2);
        xyy[2] = yaw;
    }

    double inline ConvertYaw(double yaw)
    {
        if (yaw > M_PI)
            yaw -= M_PI * 2;
        if (yaw < -M_PI)
            yaw += M_PI * 2;
        return yaw;
    }

    void LIOPrecess(Eigen::Vector3d &lio_t, Eigen::Quaterniond lio_q, double *xyy_lio)
    {
        double rol_t = 0, pitch_t = 0, yaw_t = 0;
        tf::Quaternion q_t(lio_q.x(), lio_q.y(), lio_q.z(), lio_q.w());
        tf::Matrix3x3(q_t).getRPY(rol_t, pitch_t, yaw_t);
        yaw_t = ConvertYaw(yaw_t + M_PI / 2);
        lio_t.x() -= 0.74 * cos(yaw_t);
        lio_t.y() = lio_t.y() - 0.74 * sin(yaw_t) + 0.74;
        lio_t.z() = 1.0;
        lio_raw_x_ = lio_t.x();
        lio_raw_y_ = lio_t.y();
        lio_raw_yaw_ = yaw_t;

        xyy_lio[0] = lio_t.x() * cos(xy_offset_[2] * DEG2RAD) - (lio_t.y()) * sin(xy_offset_[2] * DEG2RAD) + xy_offset_[0];
        xyy_lio[1] = lio_t.x() * sin(xy_offset_[2] * DEG2RAD) + (lio_t.y()) * cos(xy_offset_[2] * DEG2RAD) + xy_offset_[1];
        xyy_lio[2] = ConvertYaw(xy_offset_[2] * DEG2RAD + yaw_t);
    }

    void GetPosition(double *xyy_gps)
    {
        Eigen::Vector3d global_t;
        Eigen::Quaterniond global_q;
        globalEstimator_.GetGlobalOdom(global_t, global_q);
        double roll, pitch, yaw;
        tf::Quaternion tf_q(global_q.x(), global_q.y(), global_q.z(), global_q.w());
        tf::Matrix3x3(tf_q).getRPY(roll, pitch, yaw);
        yaw = ConvertYaw(yaw + M_PI / 2);
        yaw = ConvertYaw(yaw + xy_offset_[2] * DEG2RAD);
        xyy_gps[0] = global_t.x();
        xyy_gps[1] = global_t.y();
        xyy_gps[2] = yaw;
    }

    void Is_rate_ok()
    {
        publish_window_start = (publish_window_start == 0) ? ros::Time::now().toSec() : publish_window_start;
        double current_time = ros::Time::now().toSec();
        double window_duration = current_time - publish_window_start;
        // printf("current_time = %14.4f\n", current_time);
        // printf("publish_window_start = %14.4f\n", publish_window_start);
        // std::cout << "Duration: " << window_duration << std::endl;

        if (system_init_ && heading_flag_ && window_duration >= 0.95) // 检查 1 秒内的发布频率
        {
            double publish_frequency = publish_count / window_duration;
            if (publish_frequency < 8)
            {
                publish_error_cnt_++;
                reboot = publish_error_cnt_ >= 20 ? true : reboot;
                if (reboot)
                {
                    ROS_WARN("Publish frequency is too low: %.2f Hz, restarting the node!", publish_frequency);
                }
            }
            else
            {
                publish_error_cnt_ = 0;
            }
            publish_window_start = current_time;
            publish_count = 0; // 重置计数器
        }
    }

    void SaveFusionPositionToFile(const double *fusion_pos)
    {
        std::ofstream file(last_pub_pos_path, std::ios::out);
        if (!file.is_open())
        {
            ROS_ERROR("Failed to open file for saving fusion position: %s", last_pub_pos_path.c_str());
            return;
        }
        file << fusion_pos[0] << " " << fusion_pos[1] << " " << fusion_pos[2] << std::endl; // x, y, yaw
        file.close();
    }
    bool LoadFusionPositionFromFile(double *fusion_pos)
    {
        std::ifstream file(last_pub_pos_path, std::ios::in);
        if (!file.is_open())
        {
            // ROS_WARN("No saved fusion position file found: %s", last_pub_pos_path.c_str());
            return false;
        }
        file >> fusion_pos[0] >> fusion_pos[1] >> fusion_pos[2]; // x, y, yaw
        file.close();
        ROS_INFO("Loaded fusion position from file: %s", last_pub_pos_path.c_str());
        return true;
    }

private:
    ros::NodeHandle nh_;
    ros::Publisher pubPosition_, pub_fusion_Reboot_, pub_gps_Reboot_;
    ros::Subscriber subGps_, subLio_, subAndroid_;
    bool location_mode_;
    bool gps_init_;
    bool system_init_;
    int fail_cnt_;
    int map_index_;

    double origin_gauss_yaw_, origin_latitude_, origin_longitude_, origin_height_;
    std::string map_path_;
    std::string last_pub_pos_path;
    std::queue<util::GpsPositionConstPtr>
        gps_queue_;
    std::mutex gps_mutex_;

    double xy_offset_[3];
    double last_gps_[3];

    GlobalOptimization globalEstimator_;
    GeographicLib::LocalCartesian geoConverter_;

    double last_gps_stamp_;
    int gps_drift_count_;

    double last_lio_stamp_;
    Eigen::Vector3d last_lio_t_;

    int pub_fusion;

    double lio_speed_threshold_;
    double lio_stamp_;
    int lio_drift_count_;
    bool lio_drift_detected_;

    double lio_raw_x_;
    double lio_raw_y_;
    double lio_raw_yaw_;

    int no_pos_cnt_;
    bool reboot;

    int gps_failed_cnt_; // gps连续异常计数
    int lio_failed_cnt_;

    double last_fusion_time_;
    double last_fusion_pos_[3];

    int fusion_failed_cnt_;

    int pub_overtime_cnt_;

    int gps_stable_cnt_;
    int gps_stable_threshold_;
    bool gps_failed_flag_;

    double publish_window_start;
    int publish_count;

    int no_rtk_cnt_;
    int publish_error_cnt_;

    int heading_flag_;

    int sync_failed_cnt_;

    bool lio_bad_flag_;

    bool reboot_pub_flag_;

    bool map_saved_flag_;

#ifdef DEBUG
    ros::Publisher pubDebugGPS_, pubDebugLIO_, pubDebugFusion_;
#endif
};

int main(int argc, char **argv)
{
    ros::init(argc, argv, "Fusion");
    FusionNode node;
    ros::spin();
    return 0;
}
