#include <fstream>
#include <istream>
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <std_msgs/Bool.h>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <iostream>
#include <filesystem>
#include "util/GpsPosition.h"
#include "util/Position.h"
#include "util/LIOPose.h"
#include <mutex>
#include <chrono>
namespace fs = std::filesystem;

std::mutex fusion_mutex; // 定义全局互斥锁
std::mutex publish_mutex;
std::mutex lio_mutex;
std::mutex gps_mutex;
std::mutex position_mutex;

int publish_count = 0;
double publish_window_start = 0.0;
int publish_error_cnt_ = 0;
bool published_flag = false;

double last_lio_slam_time = 0; // 初始化接收到/Mower/lio_slam 消息的时间
double last_position_time = 0; // 初始化接收到/Mower/position 消息的时间

// 分割字符串函数
void splitString(const std::string &str, char delimiter, std::string &part1, std::string &part2)
{
    size_t pos = str.find(delimiter);
    if (pos != std::string::npos)
    {
        part1 = str.substr(0, pos);  // 分割符前的部分
        part2 = str.substr(pos + 1); // 分割符后的部分
    }
    else
    {
        part1 = str; // 如果没有分割符，全部赋值给 part1
        part2 = "";  // part2 为空
    }
}

namespace SUP
{
    bool findNode(const std::string &node_name)
    {
        std::vector<std::string> nodes;
        ros::master::getNodes(nodes);
        return std::find(nodes.begin(), nodes.end(), node_name) != nodes.end();
    }

    bool killNode(const std::string &node_name)
    {
        std::string kill_command = "rosnode kill " + node_name;
        if (std::system(kill_command.c_str()) != 0)
        {
            ROS_ERROR("Failed to kill the node: %s", node_name.c_str());
            return false;
        }
        return true;
    }

} // namespace SUP

void androidCallback(const std_msgs::String::ConstPtr &msg, const std::string &map_path) // void androidCallback(const std_msgs::String::ConstPtr &msg)
{
    bool find_node1 = SUP::findNode("/fusion");
    bool find_node2 = SUP::findNode("/laserMapping");

    // 建图完成后，拿到安卓发送的该地图名称

    std::string data = msg->data;
    std::cout << "Received data: " << data << std::endl;
    std::string map_mode, map_name;
    std::string new_path;

    splitString(data, '/', map_mode, map_name); // 按 '/' 分割字符串

    // 打印分割结果
    std::cout << "Part 1: " << map_mode << std::endl;
    std::cout << "Part 2: " << map_name << std::endl;

    if (!map_name.empty())
    {
        // 获得以地图名称命名的文件的路径//使用C++中的std::filesystem库
        fs::path filePath(map_path);
        // 获取父路径
        fs::path parentPath = filePath.parent_path();
        // 将fs::path类型的变量转换为std::string，结合地图名称，得到以地图名称命名的文件的路径
        new_path = parentPath.string();
        std::cout << "Parent path: " << new_path << std::endl;
        std::string mp = ".mp";
        new_path.append("/");
        new_path.append(map_name);
        new_path.append(mp);
    }

    // 判断第一个字符串是否为 "save_map"
    if (map_mode == "save_map")
    {
        std::cout << "The first part is 'save_map'. Map name: " << map_name << std::endl;

        if (map_name.empty())
        {
            ROS_ERROR("Map_name is empty.");
            return;
        }

        // 从原建图模式下保存原点坐标的文件中提取坐标数据
        char pos_data[36];
        memset(pos_data, 0, 36);
        std::ifstream file(map_path);
        std::cout << "map_path = " << map_path << std::endl;
        if (!file.is_open())
        {
            std::cerr << "Error(Save map): Unable to open file." << std::endl;
            return;
        }
        file.read(pos_data, 36);
        file.close();

        // 拿到地图坐标文件后将建图模式原点坐标输入到以名字命名的文件中
        std::ofstream fcout(new_path);
        if (!fcout.is_open())
        {
            std::cerr << "Error(Save map): Unable to open map file." << std::endl;
            return;
        }

        fcout.write(pos_data, 36);
        fcout.close();
    }
    else if (map_mode == "use_map")
    {
        std::cout << "The first part is 'use_map'. Map name: " << map_name << std::endl;

        if (map_name.empty())
        {
            ROS_WARN("Map_name is empty.");
            return;
        }

        // 从原建图模式下保存原点坐标的文件中提取坐标数据
        char pos_data[36];
        memset(pos_data, 0, 36);
        std::ifstream file(new_path);
        if (!file.is_open())
        {
            std::cerr << "Error(Use map): Unable to open file." << std::endl;
            return;
        }
        file.read(pos_data, 36);
        file.close();

        // 将坐标数据写入到原建图模式下保存原点坐标的文件中
        std::ofstream fcout(map_path);
        if (!fcout.is_open())
        {
            std::cerr << "Error(Save map): Unable to open map file." << std::endl;
            return;
        }
        fcout.write(pos_data, 36);
        fcout.close();

        std::lock_guard<std::mutex> lock(publish_mutex);
        if (std::system("restart_ros_nodes.sh /fusion /laserMapping -- bash -c 'source /home/nvidia/libo/lidar-slam-v2/devel/setup.bash && roslaunch fast_lio location_mode.launch'") != 0)
        {
            ROS_ERROR("Failed to launch the node: %s", map_mode.c_str());
        }

        published_flag = false;     // 重置标志
        publish_count = 0;          // 重置计数器
        publish_window_start = 0.0; // 重置窗口开始时间
        publish_error_cnt_ = 0;     // 重置错误计数器
    }
    else
    {
        if (map_mode == "app_start")
        {
            // if (find_node1)
            //     SUP::killNode("/fusion");
            // if (find_node2)
            //     SUP::killNode("/laserMapping");
        }
        else if (map_mode == "m_mode")
        {
            if (std::system("restart_ros_nodes.sh /fusion /laserMapping -- bash -c 'source /home/nvidia/libo/lidar-slam-v2/devel/setup.bash && roslaunch fast_lio create_mode.launch'") != 0)
            {
                ROS_ERROR("Failed to launch the node: %s", map_mode.c_str());
            }
            std::cout << " create_mode " << std::endl;

            std::lock_guard<std::mutex> lock(publish_mutex); // 加锁
            published_flag = false;                          // 重置标志
            publish_count = 0;                               // 重置计数器
            publish_window_start = 0.0;                      // 重置窗口开始时间
            publish_error_cnt_ = 0;                          // 重置错误计数器
        }
        else if (map_mode == "p_mode")
        {
            std::lock_guard<std::mutex> lock(publish_mutex); // 加锁
            published_flag = false;                          // 重置标志
            publish_count = 0;                               // 重置计数器
            publish_window_start = 0.0;                      // 重置窗口开始时间
            publish_error_cnt_ = 0;                          // 重置错误计数器
        }
        else if (map_mode == "true")
        {
            std::lock_guard<std::mutex> lock(publish_mutex);
            if (std::system("restart_ros_nodes.sh /fusion /laserMapping -- bash -c 'source /home/nvidia/libo/lidar-slam-v2/devel/setup.bash && roslaunch fast_lio location_mode.launch'") != 0)
            {
                ROS_ERROR("Failed to launch the node: %s", map_mode.c_str());
            }

            published_flag = false;     // 重置标志
            publish_count = 0;          // 重置计数器
            publish_window_start = 0.0; // 重置窗口开始时间
            publish_error_cnt_ = 0;     // 重置错误计数器
        }
        else
        {
            ROS_ERROR("Invalid mode: %s", map_mode.c_str());
        }
    }
}

void RebootCallBack(const std_msgs::Bool::ConstPtr &msg)
{
    if (msg->data)
    {
        std::cout << "Reboot signal received, restarting nodes..." << std::endl;
        std::lock_guard<std::mutex> lock(publish_mutex);
        if (std::system("restart_ros_nodes.sh /fusion /laserMapping -- bash -c 'source /home/nvidia/libo/lidar-slam-v2/devel/setup.bash && roslaunch fast_lio location_mode.launch'") != 0)
        {
            ROS_ERROR("Failed to launch the node: %s", "/fusion /laserMapping");
        }

        published_flag = false;     // 重置标志
        publish_count = 0;          // 重置计数器
        publish_window_start = 0.0; // 重置窗口开始时间
        publish_error_cnt_ = 0;     // 重置错误计数器
    }
}

void FusionCallBack(const util::PositionConstPtr &msg)
{
    std::lock_guard<std::mutex> lock(fusion_mutex); // 加锁

    if (msg->position_state == 1 || msg->position_state == 2)
    {
        publish_count++;
    }
    if (!published_flag && publish_count >= 50)
    {
        published_flag = true;
        publish_count = 1;
    }
}

void GpsCallBack(const util::GpsPositionConstPtr &msg)
{
    if (!published_flag)
    {
        return;
    }

    double current_time = ros::Time::now().toSec();
    double window_duration = 0.0;

    {
        std::lock_guard<std::mutex> lock(gps_mutex);
        publish_window_start = (publish_window_start == 0) ? current_time : publish_window_start;
        window_duration = current_time - publish_window_start;
    }
    // printf("current_time = %14.4f\n", current_time);
    // printf("publish_window_start = %14.4f\n", publish_window_start);
    // std::cout << "Duration: " << window_duration << std::endl;

    if (published_flag && window_duration >= 0.95) // 检查 1 秒内的发布频率
    {
        double publish_frequency;
        {
            std::lock_guard<std::mutex> lock(gps_mutex);
            publish_frequency = publish_count / window_duration;
        }

        std::cout << "Publish frequency: " << publish_frequency << " Hz" << std::endl;

        if (publish_frequency < 7)
        {
            {
                std::lock_guard<std::mutex> lock(gps_mutex);
                publish_error_cnt_++;
                if (publish_error_cnt_ >= 10)
                {
                    std::system("restart_ros_nodes.sh /fusion /laserMapping -- bash -c 'source /home/nvidia/libo/lidar-slam-v2/devel/setup.bash && roslaunch fast_lio location_mode.launch'");

                    published_flag = false;     // 重置标志
                    publish_count = 0;          // 重置计数器
                    publish_window_start = 0.0; // 重置窗口开始时间
                    publish_error_cnt_ = 0;     // 重置错误计数器
                    last_position_time = 0;     // 重置 /Mower/position 的时间
                    last_lio_slam_time = 0;     // 重置 /Mower/lio_slam 的时间
                    ROS_WARN("Publish frequency is too low: %.2f Hz, restarting the node!", publish_frequency);
                    return;
                }
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

void LioSlamCallback(const util::LIOPoseConstPtr &msg)
{
    std::lock_guard<std::mutex> lock(lio_mutex);
    last_lio_slam_time = msg->header.stamp.toSec(); // 更新 /Mower/lio_slam 的最后接收时间

    // 检查 /Mower/position 的时间差
    double time_since_last_position = last_lio_slam_time - last_position_time;
    if (last_lio_slam_time > 0 && last_position_time > 0 && time_since_last_position > 10.0)
    {
        ROS_WARN("No /Mower/position data for 10 seconds, restarting nodes...");
        if (std::system("restart_ros_nodes.sh /fusion /laserMapping -- bash -c 'source /home/nvidia/libo/lidar-slam-v2/devel/setup.bash && roslaunch fast_lio location_mode.launch'") != 0)
        {
            ROS_ERROR("Failed to restart nodes.");
        }
        last_position_time = 0; // 重置 /Mower/position 的时间
        last_lio_slam_time = 0; // 重置 /Mower/lio_slam 的时间

        published_flag = false;     // 重置标志
        publish_count = 0;          // 重置计数器
        publish_window_start = 0.0; // 重置窗口开始时间
        publish_error_cnt_ = 0;     // 重置错误计数器
    }
}

void PositionCallback(const util::PositionConstPtr &msg)
{
    std::lock_guard<std::mutex> lock(position_mutex);
    last_position_time = msg->header.stamp.toSec(); // 更新 /Mower/position 的最后接收时间
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "android_node_manager");
    ros::NodeHandle nh;

    std::string map_name;
    std::string android_topic, gps_topic, fusion_topic, reboot_topic;
    std::string map_path;
    std::string node_name = ros::this_node::getName();

    nh.param<std::string>(node_name + "/android_topic", android_topic, "/signal");
    nh.param<std::string>(node_name + "/gps_topic", gps_topic, "/nanobot/gpsposition");
    nh.param<std::string>(node_name + "/fusion_topic", fusion_topic, "/Mower/position");
    nh.param<std::string>(node_name + "/reboot_topic", reboot_topic, "/Mower/reboot");
    nh.param<std::string>(node_name + "/map_path", map_path, "./my_map.mp");
    std::cout << "map_path = " << map_path << std::endl;
    // ros::Subscriber sub = nh.subscribe(android_topic, 1, androidCallback);
    auto callback = boost::bind(androidCallback, _1, map_path);
    ros::Subscriber sub = nh.subscribe<std_msgs::String>(android_topic, 1, callback);

    ros::Subscriber subGps_ = nh.subscribe(gps_topic, 5, GpsCallBack);
    ros::Subscriber subFusion_ = nh.subscribe(fusion_topic, 5, FusionCallBack);
    ros::Subscriber subReboot_ = nh.subscribe<std_msgs::Bool>(reboot_topic, 1, RebootCallBack);
    ros::Subscriber subLioSlam = nh.subscribe("/Mower/lio_slam", 10, LioSlamCallback);
    ros::Subscriber subPosition = nh.subscribe("/Mower/position", 10, PositionCallback);

    ros::spin();

    return 0;
}
