#include<iostream>
#include<ros/ros.h>
#include<fstream>
#include<sstream>
#include "util/Position.h"

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");
    ros::init(argc, argv, "area_simulation");
    ros::NodeHandle nh;

    ros::Publisher position_pub = nh.advertise<util::Position>("/Mower/position", 10);

    // 读取csv文件
    std::ifstream file("/home/oem/Mower/src/move_simulation/test2.csv");
    std::string line;
    while (std::getline(file, line) && ros::ok())
    {
        if (line.empty()) continue; // 跳过空行

        std::stringstream ss(line);
        std::string token;
        util::Position position_msg;

        if (std::getline(ss, token, ','))
            position_msg.position_x = std::stod(token);

        if (std::getline(ss, token, ','))
            position_msg.position_y = std::stod(token);
        
        if (std::getline(ss, token, ','))
            position_msg.position_z = std::stod(token);
        
        ROS_INFO("Position: (%f, %f, %f)", position_msg.position_x, position_msg.position_y, position_msg.position_z);
        position_msg.position_state = 1;
        position_pub.publish(position_msg);
        ros::Duration(0.2).sleep();
    }

    file.close();
    ROS_INFO("Simulation finished.");

    return 0;
}