#include <ros/ros.h>
#include "obstacle_avoidance/a_star_planner.h"
std::string getVersion();  


int main(int argc, char** argv) {
    setlocale(LC_ALL,"");
    ros::init(argc, argv, "obstacle_avoidance_node");
    ros::NodeHandle nh;
    std::string version = getVersion();
    ROS_INFO("Version: %s", version.c_str());

    
    ROS_INFO("Starting A* Obstacle Avoidance Node");
    
    // 创建规划器实例
    obstacle_avoidance::AStarPlanner planner;
    
    // 进入ROS主循环：使用异步 spinner 以允许回调在后台线程执行
    ros::AsyncSpinner spinner(2); // 两个线程足够处理 pose 回调和其他回调
    spinner.start();
    ros::waitForShutdown();
    
    return 0;
} 