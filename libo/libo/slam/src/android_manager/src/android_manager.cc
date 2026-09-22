#include <istream>
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <cstdlib>
#include <string>

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

void androidCallback(const std_msgs::String::ConstPtr &msg)
{
    bool find_node1 = SUP::findNode("/fusion");
    bool find_node2 = SUP::findNode("/laserMapping");

    // 启动节点“/fusion”
    std::string mode = msg->data;

    // 如果APP重启，那么需要关闭所有节点
    if (mode == "app_start")
    {
        if (find_node1)
            SUP::killNode("/fusion");
        if (find_node2)
            SUP::killNode("/laserMapping");
    }

    std::string launch_command;
    if (mode == "m_mode" && !find_node1)
        launch_command = "roslaunch fast_lio create_mode.launch > /dev/null 2>&1 &";
    else if (mode == "p_mode")
    {
        if (!find_node1 && !find_node2)
            launch_command = "roslaunch fast_lio location_mode.launch > /dev/null 2>&1 &";
        else
        {
            if (find_node1)
                SUP::killNode("/fusion");
            if (find_node2)
                SUP::killNode("/laserMapping");
            launch_command = "roslaunch fast_lio location_mode.launch > /dev/null 2>&1 &";
        }
    }
    else
        ROS_ERROR("Invalid mode: %s", mode.c_str());

    if (std::system(launch_command.c_str()) != 0)
    {
        ROS_ERROR("Failed to launch the node: %s", mode.c_str());
    }
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "android_node_manager");
    ros::NodeHandle nh;

    std::string android_topic;
    std::string node_name = ros::this_node::getName();
    nh.param<std::string>(node_name + "/android_topic", android_topic, "/signal");
    ros::Subscriber sub = nh.subscribe(android_topic, 1, androidCallback);
    ros::spin();
    return 0;
}