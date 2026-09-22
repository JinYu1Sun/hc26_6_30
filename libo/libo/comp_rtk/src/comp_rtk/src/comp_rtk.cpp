#include <iostream>
#include <thread>
#include <cmath>
#include <queue>
#include <fstream>
#include <ros/ros.h>
#include "util/GpsPosition.h"//////
#include "util/Position.h"
#include "util/err.h"
//#include "serial_reader/GpsInfo.h"
#include <chrono>
#include <mutex>
#include <Eigen/Dense>
#include <yaml-cpp/yaml.h>

#include <nav_msgs/Odometry.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_datatypes.h>

//#include "fusion/global_fusion.h"
#include "LocalCartesian.hpp"
//#include "comp_rtk/err.h"
#define math_pi 3.14159265358979323846264338327950288419716939937510
constexpr auto DEG2RAD = math_pi / 180.0;

class FusionNode
{
public:
    FusionNode() : nh_("~")
    {
        std::string node_name = ros::this_node::getName();
        std::string dual_topic, ant_topic, fusion_topic, puberr_topic;

        ParameterInit();

        nh_.param<std::string>(node_name + "/dual_topic", dual_topic, "/nanobot/gpsposition");
        nh_.param<std::string>(node_name + "/ant_topic", ant_topic, "/nanobot/gpsposition1");
        nh_.param<std::string>(node_name + "/fusion_topic", fusion_topic, "/Mower/position");
        nh_.param<std::string>(node_name + "/puberr_topic", puberr_topic, "/nanobot/err");

        subdual = nh_.subscribe(dual_topic, 1, &FusionNode::dualCallBack, this);
        subslam = nh_.subscribe(fusion_topic, 1, &FusionNode::slamCallBack, this);
        subant = nh_.subscribe(ant_topic, 1, &FusionNode::oneantCallBack, this);
        puberr = nh_.advertise<util::err>(puberr_topic, 10);/////////////////////////
    }

private:
    void dualCallBack(const util::GpsPositionConstPtr& msg)
    {
        std::unique_lock<std::mutex> lock(dual_mutex_);
        dual_queue_.push(msg);
    }

    void slamCallBack(const util::PositionConstPtr& msg)
    {
        std::unique_lock<std::mutex> lock(slam_mutex_);
        slam_queue_.push(msg);
    }

    void oneantCallBack(const util::GpsPositionConstPtr& msg)
    {
        double ant_latitude = 0.;
        double ant_longitude = 0.;
        double ant_altitude = 0.;
        double ant_gps_flag = 0.;
        double ant_gauss_yaw = 0.;
        double dual_latitude = 0.;
        double dual_longitude = 0.;
        double dual_altitude = 0.;
        double dual_gps_flag = 0.;
        double dual_gauss_yaw = 0.;
        double dual_enu[3] = { 0 }, ant_enu[3] = { 0 };
        bool antdual_flag = false;
        double position_x = 0., position_y = 0., position_z = 0., slam_yaw = 0., yaw_temp = 0.;
        double err_rtkenu[3] = { 0 }, err_rtkyaw = 0.;
        double err_slamenu[3] = { 0 }, err_slamyaw = 0.;
        //    dual  
        //     ant
        //    slam
        double ant_stamp = msg->header.stamp.toSec();
        ant_latitude = msg->latitude; ant_longitude = msg->longitude; ant_altitude = msg->height; ant_gauss_yaw = msg->azimuth / 100;
        //--------------------dual ant---------------------
        dual_mutex_.lock();
        while (!dual_queue_.empty())
        {
            util::GpsPositionConstPtr dual_msg = dual_queue_.front();
            double dual_stamp = dual_msg->header.stamp.toSec();
            std::cout << "err_dual stamp = " << dual_stamp - ant_stamp << std::endl;
            //std::cout << "ant stamp = " << ant_stamp << std::endl;
            //std::cout << "dual stamp = " << dual_stamp << std::endl;
            printf("ant stamp = %12.4f\n", ant_stamp);
            printf("dual stamp = %12.4f\n", dual_stamp);

            if (dual_stamp >= ant_stamp - 0.2 && dual_stamp <= ant_stamp + 0.2)
            {
                dual_latitude = dual_msg->latitude;
                dual_longitude = dual_msg->longitude;
                dual_altitude = dual_msg->height;
                dual_gps_flag = dual_msg->gps_flag;
                dual_gauss_yaw = dual_msg->azimuth / 100;
                dual_queue_.pop();
                std::cout << "gps ok : pop " << std::endl;
                break;
            }
            else if (dual_stamp < ant_stamp - 0.2) {
                dual_queue_.pop(); 
                std::cout << "gps < : pop " << std::endl;
            }
            else if (dual_stamp > ant_stamp + 0.2) {
                std::cout << "gps > : no   pop " << std::endl;
                break;
            }
        }
        dual_mutex_.unlock();
        std::cout << "dual latitude ================= " << dual_latitude << dual_longitude << dual_altitude << std::endl;
        std::cout << "ant latitude = " << ant_latitude << ant_longitude << ant_altitude << std::endl;
        //--------------------dual ant---------------------
        if (isValue(ant_latitude, ant_longitude, ant_altitude) && isValue(dual_latitude, dual_longitude, dual_altitude)) {
            antdual_flag = true;

            if (!previous_antdual_flag) // 检测到从非4状态转换到4状态
            {
                restore_flag = false;
                stable_count = 0; // 重置稳定计数器
            }
            if (!restore_flag) {
                if (stable_count >= 5) {
                    restore_flag = true;
                }
                else {
                    stable_count++;
                }
            }
            if (restore_flag) {
                if (yaw_valflag == false && dual_gauss_yaw != 0) {
                    origin_gauss_yaw = dual_gauss_yaw;
                    yaw_valflag = true;
                }

                //---------------------------slam----------------------
                
                slam_mutex_.lock();
                while (!slam_queue_.empty())
                {
                    util::PositionConstPtr slam_msg = slam_queue_.front();
                    double slam_stamp = slam_msg->header.stamp.toSec();
                    std::cout << "err_stamp slammmmm = " << slam_stamp - ant_stamp << std::endl;
                    printf("ant stamp = %12.4f\n", ant_stamp);
                    printf("slam stamp = %12.4f\n", slam_stamp);

                    if (yaw_valflag == true) {
                        yaw_temp = origin_gauss_yaw - 90;
                        if (yaw_temp < 0) {
                            yaw_temp += 360;
                        }
                        //yaw_temp=360-yaw_temp;
                        yaw_temp*= DEG2RAD;
                        std::cout<<"yaw_temp = "<<yaw_temp<<std::endl;
                        std::cout<<"origin_gauss_yaw = "<<origin_gauss_yaw<<std::endl;
                    }
                    else {
                        ROS_WARN("<---- SLAM: origin_gauss_yaw have some problems! ---->");
                    }
                    if (slam_stamp >= ant_stamp - 0.2 && slam_stamp <= ant_stamp + 0.2)
                    {
                        position_x = slam_msg->position_x * cos(yaw_temp) - slam_msg->position_y * sin(yaw_temp);//注：fusion中pub时position_x = xyy_p[1];position_y = xyy_p[0];反了
                        position_y = slam_msg->position_y * cos(yaw_temp) + slam_msg->position_x * sin(yaw_temp);
                        position_z = slam_msg->position_z;
                        slam_yaw = slam_msg->yaw * DEG2RAD;
                        slam_valflag = true;
                        slam_queue_.pop();
                        std::cout << "slam ok : pop " << std::endl;
                        break;
                    }
                    else if (slam_stamp < ant_stamp - 0.2) {
                        slam_queue_.pop();
                        std::cout << "slam < : pop " << std::endl;
                    }
                    else if (slam_stamp > ant_stamp + 0.2) {
                        break;
                        std::cout << "slam > : no pop " << std::endl;
                    }
                }
                slam_mutex_.unlock();
                //std::cout << "slam position_x = " << position_x << std::endl;
                //---------------------------slam-----------------------

                //--------------------dual ant--------------------
                if (dual_valflag == false) {
                    geoConverter_dual.Reset(dual_latitude, dual_longitude, dual_altitude);
                    dual_valflag = true;
                }
                geoConverter_dual.Forward(dual_latitude, dual_longitude, dual_altitude, dual_enu[0], dual_enu[1], dual_enu[2]);
                //std::cout << "dual_enu = " << dual_enu[0] << dual_enu[1] << dual_enu[2] << std::endl;
                //std::cout << "dual valflag = " << dual_valflag << std::endl;
                //--------------------dual ant--------------------

                //--------------------one ant------------------------

                
                if (ant_valflag == false) {
                    geoConverter_ant.Reset(ant_latitude, ant_longitude, ant_altitude);
                    ant_valflag = true;
                }
                geoConverter_ant.Forward(ant_latitude, ant_longitude, ant_altitude, ant_enu[0], ant_enu[1], ant_enu[2]);
                //std::cout << "ant_enu = " << ant_enu[0] << ant_enu[1] << ant_enu[2] << std::endl;
                //std::cout << "ant valflag = " << ant_valflag << std::endl;
                //--------------------one ant------------------------

                
                if (ant_valflag == true && dual_valflag == true) {
                    for (int i = 0; i < 3; i++) {
                        err_rtkenu[i] = ant_enu[i] - dual_enu[i];
                    }
                    err_rtkyaw = ant_gauss_yaw - dual_gauss_yaw;
                }
                
                if (isValue(position_x, position_y, position_z + 1)) {
                    if (ant_valflag == true && slam_valflag == true) {
                        err_slamenu[0] = ant_enu[0] - position_x;
                        err_slamenu[1] = ant_enu[1] - position_y;
                        //err_slamenu[2] = ant_enu[2] - position_z;
                        err_slamyaw = ant_gauss_yaw - slam_yaw;
                    }
                }
            }
        }
        else {
            ROS_WARN("<---- 1ant or dual  have some problems!---->");
            //std::cout << "ant latitude = " << ant_latitude << std::endl;
            //std::cout << "ant longitude = " << ant_longitude << std::endl;
            //std::cout << "dual latitude = " << dual_latitude << std::endl;
            //std::cout << "dual longitude = " << dual_longitude << std::endl;
            stable_count = 0; // 非4状态重置稳定计数器
            restore_flag = false;
        }
        previous_antdual_flag = antdual_flag;

        // 发布位置
        util::err err_;
        err_.header.stamp = msg->header.stamp;
        err_.ant_e = ant_enu[0];
        err_.ant_n = ant_enu[1];
        err_.ant_u = ant_enu[2];
        err_.ant_yaw = ant_gauss_yaw;
        err_.dual_e = dual_enu[0];
        err_.dual_n = dual_enu[1];
        err_.dual_u = dual_enu[2];
        err_.dual_yaw = dual_gauss_yaw;
        err_.slam_x = position_x;
        err_.slam_y = position_y;
        err_.err_e = err_rtkenu[0];
        err_.err_n = err_rtkenu[1];
        err_.err_u = err_rtkenu[2];
        err_.err_yaw = err_rtkyaw;
        err_.err_slame = err_slamenu[0];
        err_.err_slamn = err_slamenu[1];
        err_.err_slamu = err_slamenu[2];
        err_.err_slamyaw = err_slamyaw;

        puberr.publish(err_);
    }
private:
    void ParameterInit()
    {
        slam_valflag = false;
        dual_valflag = false;
        ant_valflag = false;
        yaw_valflag = false;

        restore_flag = false;
        stable_count = 0;
        previous_antdual_flag = false;
    }

    double inline ConvertYaw(double yaw)
    {
        if (yaw > M_PI)
            yaw -= M_PI * 2;
        if (yaw < -M_PI)
            yaw += M_PI * 2;
        return yaw;
    }

    inline double ComputeDistance(double* xyy1, double* xyy2)
    {
        return std::sqrt(std::pow(xyy1[0] - xyy2[0], 2) + std::pow(xyy1[1] - xyy2[1], 2));
    }
    inline bool isValue(double latitude, double longitude, double height)
    {
        return (latitude != 0. && longitude != 0. && height != 0.);
    }

private:
    ros::NodeHandle nh_;
    ros::Publisher puberr;
    ros::Subscriber subdual, subant, subslam;

    double origin_latitude_, origin_longitude_, origin_height_;
    std::queue<util::GpsPositionConstPtr> dual_queue_;
    std::mutex dual_mutex_;
    std::queue < util::PositionConstPtr> slam_queue_;////
    std::mutex slam_mutex_;
    bool slam_valflag;
    bool dual_valflag;
    bool ant_valflag;
    bool yaw_valflag;
    double origin_gauss_yaw;
    bool restore_flag;
    int stable_count;
    bool previous_antdual_flag;
        

    GeographicLib::LocalCartesian geoConverter_ant;
    GeographicLib::LocalCartesian geoConverter_dual;
};


int main(int argc, char** argv)
{
    ros::init(argc, argv, "comp_rtk");
    FusionNode node;
    ros::spin();
    return 0;
}


















