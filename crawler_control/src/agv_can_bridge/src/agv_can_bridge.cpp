// 履带割草机can通信

#include "ros/ros.h"

#include "stdint.h"

#include "can_msgs/Frame.h"

#include "mower_msgs/VehicleCmd.h"

#include <unordered_map>

ros::Subscriber vehicle_cmd_sub;

ros::Publisher can0_send_pub;

std::unordered_map<int, double> recv_can0_timeouts; // key: CAN ID; value: timeout

void vehicle_cmd_callback(const mower_msgs::VehicleCmd::ConstPtr &msg)
{
    std::cout<<"dhaoidhsaisdsoda"<<std::endl;
    int linear_vel = static_cast<int>(msg->drive_value);
    int angular_vel = static_cast<int>(msg->turn_value);
    std::cout<<"dhaoidhsaioda"<<std::endl;
    can_msgs::Frame can_msg;
    
    // 设置CAN帧头
    can_msg.header.stamp = ros::Time::now();
    can_msg.id = 0x00000005;
    can_msg.dlc = 8;
    can_msg.is_extended = true;
    can_msg.is_rtr = false;
    can_msg.is_error = false;
    
    
    // 线性速度（前4字节）
    int32_t linear_vel_int = static_cast<int32_t>(linear_vel);
    can_msg.data[0] = (linear_vel >> 24) & 0xFF;
    can_msg.data[1] = (linear_vel >> 16) & 0xFF;
    can_msg.data[2] = (linear_vel >> 8) & 0xFF;
    can_msg.data[3] = linear_vel & 0xFF;

    // 角速度（后4字节）
    int32_t angular_vel_int = static_cast<int32_t>(angular_vel);
    can_msg.data[4] = (angular_vel >> 24) & 0xFF;
    can_msg.data[5] = (angular_vel >> 16) & 0xFF;
    can_msg.data[6] = (angular_vel >> 8) & 0xFF;
    can_msg.data[7] = angular_vel & 0xFF;

    // 发送CAN帧
    //sendCanFrame(can_id, data);
    //g_can_pub.publish(can_msg);
    
    ROS_INFO("Sent velocity command: linear=0x%08X angular=0x%08X", 
             (unsigned int)linear_vel_int, (unsigned int)angular_vel_int);
    can0_send_pub.publish(can_msg);
}

int main(int argc, char *argv[])
{
    ros::init(argc, argv, "agv_can_bridge");
    ros::NodeHandle nh;
    ROS_INFO_STREAM("chassis start");
    can0_send_pub = nh.advertise<can_msgs::Frame>("/sent_messages", 1);
    vehicle_cmd_sub = nh.subscribe("/vehicle/cmd",  2, vehicle_cmd_callback);
    ros::Rate loop_rate(50); // 50Hz

    while (ros::ok())
    {
        ros::spinOnce();
        

        loop_rate.sleep();
    }
    return 0;
}
