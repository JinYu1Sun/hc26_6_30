/* ============================================================================
 * Copyright (c) 2024 ShangZhi.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS' AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 * ============================================================================
 */
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
#include "fast_lio/LIOPose.h"
#include "fast_lio/Position.h"
#include "fusion/global_fusion.h"
#include "LocalCartesian.hpp"

constexpr auto DEG2RAD = M_PI / 180.0;
int cnt = 0;

#define DEBUG

// =========== [辅助验证函数] ============================================
struct Vector
{
    double x;
    double y;
};

Vector calculateDirectionVector(const double *a, const double *b)
{
    double deltaX = b[0] - a[0];
    double deltaY = b[1] - a[1];
    double magnitude = std::sqrt(deltaX * deltaX + deltaY * deltaY);
    return {deltaX / magnitude, deltaY / magnitude};
}

double dotProduct(const Vector &v1, const Vector &v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

bool areDirectionsConsistent(const Vector &v1, const Vector &v2)
{
    double dot = dotProduct(v1, v2);
    // cos30
    return dot > 0.7;
}

double directionsConsistent(const Vector &v1, const Vector &v2)
{
    double dot = dotProduct(v1, v2);
    return dot;
}

// =======================================================================

class FusionNode
{
public:
    FusionNode() : nh_("~")
    {
        std::string node_name = ros::this_node::getName();
        std::string gps_topic, lio_slam_topic, fusion_topic;

        ParameterInit();

        nh_.param<bool>(node_name + "/location_mode", location_mode_, false);
        nh_.param<std::string>(node_name + "/map_path", map_path_, "./my_map.mp");
        nh_.param<std::string>(node_name + "/gps_topic", gps_topic, "/nanobot/gpsposition");
        nh_.param<std::string>(node_name + "/lio_slam_topic", lio_slam_topic, "/Mower/lio_slam");
        nh_.param<std::string>(node_name + "/fusion_topic", fusion_topic, "/Mower/position");
        nh_.param<int>(node_name + "/init_threshold", fail_threshold_, 30);
        nh_.param<int>(node_name + "/map_index", map_index_, 0);

        if (location_mode_)
            ReadMap();

        subGps_ = nh_.subscribe(gps_topic, 1, &FusionNode::GPSCallBack, this);
        subLio_ = nh_.subscribe(lio_slam_topic, 1, &FusionNode::LioCallBack, this);
        pubPosition_ = nh_.advertise<fast_lio::Position>(fusion_topic, 10);

#ifdef DEBUG
        pubDebugGPS_ = nh_.advertise<fast_lio::Position>("/Mower/debug/gps", 10);
        pubDebugLIO_ = nh_.advertise<fast_lio::Position>("/Mower/debug/lio", 10);
        // pubDebugFusion_ = nh_.advertise<fast_lio::Position>("/Mower/debug/lio_fusion", 10);
#endif
    }

private:
    void GPSCallBack(const util::GpsPositionConstPtr &msg)
    {
        std::unique_lock<std::mutex> lock(gps_mutex_);
        gps_queue_.push(msg);
    }

    void LioCallBack(const fast_lio::LIOPoseConstPtr &msg)
    {
        double lio_stamp = msg->header.stamp.toSec();
        Eigen::Vector3d lio_t(-msg->position_x, -msg->position_y, 1.0);
        Eigen::Quaterniond lio_q;
        lio_q.w() = msg->q_w;
        lio_q.x() = msg->q_x;
        lio_q.y() = msg->q_y;
        lio_q.z() = msg->q_z;
        lio_state_ = msg->lio_state;
        double xyy_lio[3];
        LIOPrecess(lio_t, lio_q, xyy_lio);

        cnt++;
        if (cnt == INT_MAX)
            cnt = 0;
        if (cnt % 3 == 0 && lio_state_)
            globalEstimator.InputOdom(lio_stamp, lio_t, lio_q);

        double latitude = 0.;
        double longitude = 0.;
        double altitude = 0.;
        double gps_flag = 0.;
        double gauss_yaw = 0.;

        gps_mutex_.lock();
        while (!gps_queue_.empty())
        {
            util::GpsPositionConstPtr gps_msg = gps_queue_.front();
            double gps_stamp = gps_msg->header.stamp.toSec();
            if (gps_stamp >= lio_stamp - 0.02 && gps_stamp <= lio_stamp + 0.02)
            {
                latitude = gps_msg->latitude;
                longitude = gps_msg->longitude;
                altitude = gps_msg->height;
                gps_flag = gps_msg->gps_flag;
                gauss_yaw = gps_msg->azimuth;
                gps_queue_.pop();
                break;
            }
            else if (gps_stamp < lio_stamp - 0.02)
                gps_queue_.pop();
            else if (gps_stamp > lio_stamp + 0.02)
                break;
        }
        gps_mutex_.unlock();

        if (latitude == 0. || longitude == 0. || altitude == 0.)
            return;

        if (!system_init_)
        {
            if (gps_flag != 4)
            {
                fail_cnt_++;
                if (fail_cnt_ >= fail_threshold_)
                {
                    ROS_WARN("<---- GPS position have some problems! ---->");
                    fail_cnt_ = 0;
                }
                return;
            }
            else
            {
                if (!location_mode_)
                    SaveMap(latitude, longitude, altitude, gauss_yaw);

                globalEstimator.InputGPS(lio_stamp, latitude, longitude, altitude, gauss_yaw, 1);
                system_init_ = true;
                restore_flag_ = true;
            }
        }
        else
        {
            if (gps_flag == 4)
            {
                if (previous_gps_flag_ != 4) // 检测到从非4状态转换到4状态
                {
                    restore_flag_ = false;
                    stable_counter_ = 0; // 重置稳定计数器
                }
                else if (!restore_flag_) // 在稳定期内
                {
                    if (stable_counter_ >= 200)
                        restore_flag_ = true;
                    else
                        stable_counter_++;
                }

                // 当稳定期结束后（restore_flag_为true），才执行globalEstimator.InputGPS
                if (restore_flag_)
                {
                    // double xyy_t[3];
                    // GPS2Local(latitude, longitude, altitude, gauss_yaw, xyy_t);
                    // memcpy(last_location, xyy_t, sizeof(xyy_t));
                    // Vector direction1 = calculateDirectionVector(xyy_t, last_location);
                    // Vector direction2 = calculateDirectionVector(xyy_lio, last_lio);
                    // double delta_yaw = fabs(xyy_lio[2] - gauss_yaw);
                    // if (!areDirectionsConsistent(direction1, direction2) && delta_yaw < 0.8)
                        globalEstimator.InputGPS(lio_stamp, latitude, longitude, altitude, gauss_yaw, 1);
                    // if (!collect_flag_)
                    // {
                    //     collect_nums_++;
                    //     if (collect_nums_ == 400)
                    //         collect_flag_ = true;
                    // }
                }
            }
            else
            {
                stable_counter_ = 0; // 非4状态重置稳定计数器
                restore_flag_ = false;
            }
        }
        previous_gps_flag_ = gps_flag; // 更新前一个GPS状态

        double xyy_p[3];
        GetPosition(xyy_p);
 
        // 发布位置
        fast_lio::Position fusion_position;
        fusion_position.header.stamp = msg->header.stamp;
        fusion_position.header.frame_id = "base_link";
        fusion_position.position_x = xyy_p[0];
        fusion_position.position_y = xyy_p[1];
        fusion_position.position_z = 0;
        fusion_position.roll = 0;
        fusion_position.pitch = 0;
        fusion_position.yaw = xyy_p[2];
        fusion_position.position_state = 2;
        pubPosition_.publish(fusion_position);

        // Tf.
        static tf::TransformBroadcaster base_broadcaster;
        tf::Quaternion q;
        q.setRPY(0., 0., xyy_p[2]);
        base_broadcaster.sendTransform(tf::StampedTransform(tf::Transform(q, tf::Vector3(xyy_p[0], xyy_p[1], 0.0)), msg->header.stamp, "odom", "base_link"));
        static tf::TransformBroadcaster map_broadcaster;
        map_broadcaster.sendTransform(tf::StampedTransform(tf::Transform(tf::Quaternion(0, 0, 0, 1), tf::Vector3(0.0, 0.0, 0.0)), msg->header.stamp, "map", "odom"));

        memcpy(last_lio, xyy_lio, sizeof(xyy_lio));

#ifdef DEBUG
        double xyy_g[3];
        GPS2Local(latitude, longitude, altitude, gauss_yaw, xyy_g);
        fast_lio::Position gps2xyz_msg;
        gps2xyz_msg.header.frame_id = "map";
        gps2xyz_msg.header.stamp = msg->header.stamp;
        gps2xyz_msg.position_x = xyy_g[0];
        gps2xyz_msg.position_y = xyy_g[1];
        gps2xyz_msg.position_z = 0;
        gps2xyz_msg.position_state = gps_flag;
        gps2xyz_msg.yaw = xyy_g[2];
        pubDebugGPS_.publish(gps2xyz_msg);

        double rol_t = 0, pitch_t = 0, yaw_t = 0;
        tf::Quaternion q_t(lio_q.x(), lio_q.y(), lio_q.z(), lio_q.w());
        tf::Matrix3x3(q_t).getRPY(rol_t, pitch_t, yaw_t);
        yaw_t = ConvertYaw(yaw_t + M_PI / 2);
        fast_lio::Position LIO_msg;
        LIO_msg.header.frame_id = "map";
        LIO_msg.header.stamp = msg->header.stamp;
        LIO_msg.position_x = lio_t.x();
        LIO_msg.position_y = lio_t.y();
        LIO_msg.position_z = 0;
        LIO_msg.yaw = yaw_t;
        pubDebugLIO_.publish(LIO_msg);

        // save position to txt file.
        std::ofstream foutA("./gps.txt", std::ios::app);
        foutA.setf(std::ios::fixed, std::ios::floatfield);
        foutA.precision(5);
        foutA << xyy_g[0] << " "
              << xyy_g[1] << " "
              << xyy_g[2] << std::endl;
        foutA.close();

        std::ofstream foutB("./lio.txt", std::ios::app);
        foutB.setf(std::ios::fixed, std::ios::floatfield);
        foutB.precision(5);
        foutB << lio_t.x() << " "
              << lio_t.y() << " "
              << yaw_t << std::endl;
        foutB.close();

        std::ofstream foutC("./fusion.txt", std::ios::app);
        foutC.setf(std::ios::fixed, std::ios::floatfield);
        foutC.precision(5);
        foutC << xyy_p[0] << " "
              << xyy_p[1] << " "
              << xyy_p[2] << std::endl;
        foutC.close();
#endif
    }

private:
    void ParameterInit()
    {
        fail_cnt_ = 0;
        origin_gauss_yaw_ = 0.;
        position_state_ = -1;
        map_index_ = 0;
        stable_counter_ = 0;
        previous_gps_flag_ = 0;
        // TODO:
        collect_nums_ = 0;

        start_time_ = std::chrono::steady_clock::time_point();
        origin_latitude_ = 0.;
        origin_longitude_ = 0.;
        origin_height_ = 0.;
        gps_init_ = false;
        system_init_ = false;
        location_mode_ = false;
        lio_state_ = false;
        restore_flag_ = false;
        // TODO:
        collect_flag_ = false;

        // TODO: test.
        for (int i = 0; i < 3; ++i)
        {
            last_lio[i] = 1e-10;
            last_location[i] = 1e-10;
        }
    }

    bool SaveMap(double latitude, double longitude, double height, double gauss_yaw)
    {
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
        globalEstimator.location_mode = true;
        globalEstimator.origin_latitude = origin_latitude_;
        globalEstimator.origin_longitude = origin_longitude_;
        globalEstimator.origin_altitude = origin_height_;
        globalEstimator.origin_gauss_yaw = origin_gauss_yaw_;

        if (origin_gauss_yaw_ == 0 || origin_latitude_ == 0 || origin_height_ == 0 || origin_longitude_ == 0)
        {
            std::cerr << "<--- Read map error! --->" << std::endl;
            return false;
        }
#ifdef DEBUG
        std::cout << "<--- Read map: " << origin_latitude_ << " " << origin_longitude_ << " " << origin_height_ << " " << origin_gauss_yaw_ << std::endl;
#endif
        return true;
    }

    void GPS2Local(double latitude, double longitude, double height, double gauss_yaw, double *xyy)
    {
        if (!gps_init_)
        {
            if (!location_mode_)
            {
                geoConverter.Reset(latitude, longitude, height);
                // origin_gauss_yaw_ = gauss_yaw * 0.01 - 90;
                origin_gauss_yaw_ = gauss_yaw * 0.01;
            }
            else
            {
                geoConverter.Reset(origin_latitude_, origin_longitude_, origin_height_);
                // origin_gauss_yaw_ = origin_gauss_yaw_ * 0.01 - 90;
                origin_gauss_yaw_ = origin_gauss_yaw_ * 0.01;
            }
            gps_init_ = true;
        }
        double xyz[3];
        geoConverter.Forward(latitude, longitude, height, xyz[0], xyz[1], xyz[2]);
        xyy[0] = xyz[0] * cos(origin_gauss_yaw_ * DEG2RAD) + xyz[1] * sin(origin_gauss_yaw_ * DEG2RAD);
        xyy[1] = -xyz[0] * sin(origin_gauss_yaw_ * DEG2RAD) + xyz[1] * cos(origin_gauss_yaw_ * DEG2RAD);
        double yaw = (gauss_yaw * 0.01 - origin_gauss_yaw_) * DEG2RAD;
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
        lio_t.x() -= 0.64 * cos(yaw_t);
        lio_t.y() -= 0.64 * sin(yaw_t);
        lio_t.z() = 1.0;
        xyy_lio[0] = lio_t.x();
        xyy_lio[1] = lio_t.y();
        xyy_lio[2] = yaw_t;
    }

    void GetPosition(double *xyy_gps)
    {
        Eigen::Vector3d global_t;
        Eigen::Quaterniond global_q;
        globalEstimator.GetGlobalOdom(global_t, global_q);
        double roll, pitch, yaw;
        tf::Quaternion tf_q(global_q.x(), global_q.y(), global_q.z(), global_q.w());
        tf::Matrix3x3(tf_q).getRPY(roll, pitch, yaw);
        yaw += M_PI / 2;
        yaw = ConvertYaw(yaw);
        xyy_gps[0] = global_t.x();
        xyy_gps[1] = global_t.y();
        xyy_gps[2] = yaw;
        position_state_ = 2;
    }

private:
    ros::NodeHandle nh_;
    ros::Publisher pubPosition_;
    ros::Subscriber subGps_, subLio_;
    bool location_mode_;
    bool gps_init_;
    bool system_init_;
    bool lio_state_;
    int fail_threshold_;
    int fail_cnt_;
    int position_state_;
    int map_index_;
    int stable_counter_;
    int previous_gps_flag_;
    int collect_nums_;

    bool timer_flag_;
    bool input_flag_;
    bool restore_flag_;
    bool collect_flag_;
    std::chrono::steady_clock::time_point start_time_;

    double origin_gauss_yaw_, origin_latitude_, origin_longitude_, origin_height_;
    std::string map_path_;
    std::queue<util::GpsPositionConstPtr> gps_queue_;
    std::mutex gps_mutex_;
    // TODO: 
    double last_lio[3];
    double last_location[3];

    GlobalOptimization globalEstimator;
    GeographicLib::LocalCartesian geoConverter;

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