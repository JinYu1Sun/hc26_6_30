#include <iostream>
#include <ros/ros.h>
#include <geometry_msgs/TransformStamped.h>
#include <geometry_msgs/Point.h>
#include <std_msgs/UInt8.h>
#include <std_msgs/Bool.h>
#include <tf/transform_broadcaster.h>
#include <mutex>
#include <atomic>
#include "util/Position.h"
#include "util/LocalPath.h"
#include "read_waypoints/TurnCompleted.h"

class MoveSimulation
{
private:
    ros::NodeHandle nh_;
    ros::Subscriber sub_local_path_;
    ros::Subscriber sub_avoid_status_;
    ros::Subscriber sub_stop_car_;
    tf::TransformBroadcaster tf_broadcaster_;
    ros::Publisher pub_position_;
    ros::Publisher pub_turn_success_;
    ros::ServiceServer srv_turn_success_;

    std_msgs::UInt8 avoid_status_;
    std_msgs::UInt8 pre_avoid_status_;

    std::atomic<bool> slow_down_;
    std::atomic<bool> start_avoid_plan_; // 用来中断正常运动
    std::atomic<bool> avoid_plan_success_; // 用来更新避障路线
    std::atomic<bool> quit_avoid_; // 停障时障碍物小消失，退出停障恢复正常工作状态
    std::atomic<bool> has_new_path_;
    std::atomic<bool> stop_car_; // 停止小车运动

    std::mutex path_mutex_;
    util::LocalPath current_local_path_;

    std::mutex turn_success_mutex_;
    read_waypoints::TurnCompleted turn_success_msg_;

public:
    MoveSimulation(ros::NodeHandle &nh) : nh_(nh), slow_down_(false), start_avoid_plan_(false), avoid_plan_success_(false), quit_avoid_(false), has_new_path_(false), stop_car_(false)
    {
        sub_local_path_ = nh.subscribe("/lawn_mower/global_path", 1, &MoveSimulation::local_path_call_back, this);
        sub_avoid_status_ = nh_.subscribe("/lawn_mower/avoid_state", 1, &MoveSimulation::avoidStatusCallback, this);
        sub_stop_car_ = nh_.subscribe("/mower/stop_car2", 1, &MoveSimulation::stopCarCallback, this);

        pub_position_ = nh.advertise<util::Position>("/Mower/position", 1);
        pub_turn_success_ = nh.advertise<std_msgs::Bool>("/turn_completed", 1);

        srv_turn_success_ = nh.advertiseService("/turn_completed", &MoveSimulation::turn_success_callback, this);
        avoid_status_.data = 0;
        pre_avoid_status_.data = 0;
        current_local_path_.point_num = 0;
    }

    void local_path_call_back(const util::LocalPath::ConstPtr &msg)
    {
        std::lock_guard<std::mutex> lock(path_mutex_);
        if (avoid_plan_success_.load())
        {
            current_local_path_ = *msg;
            has_new_path_.store(true);
            ROS_INFO("有避障数据");
            // avoid_plan_success_.store(false);
            return;
        }
        if (msg->x.size() == 0)
        {
            ROS_INFO("无路径数据");
            has_new_path_.store(false);
            return;
        }
        if (current_local_path_ != *msg)
        {
            current_local_path_ = *msg;
            has_new_path_.store(true);
            // ROS_INFO("有新数据");
        }
        else
        {
            has_new_path_.store(false);
            // ROS_INFO("无新数据");
        }
    }

    void avoidStatusCallback(const std_msgs::UInt8 &avoid_status_msgs)
    {
        avoid_status_ = avoid_status_msgs;
        if (pre_avoid_status_.data == 2 && avoid_status_.data == 3 )
        {
            start_avoid_plan_.store(true);
            // ROS_INFO("进入避障规划状态");
        }
        if (pre_avoid_status_.data == 3 && avoid_status_.data == 3)
        {
            start_avoid_plan_.store(true);
        }
        if (pre_avoid_status_.data == 1 && avoid_status_.data == 3)
        {
            start_avoid_plan_.store(true);
        }
        if (pre_avoid_status_.data == 3 && avoid_status_.data == 1)
        {
            ros::Duration(1).sleep();
            avoid_plan_success_.store(true);
            // ROS_INFO("避障规划完成，进入避障状态");
        }
        if (pre_avoid_status_.data == 3 && avoid_status_.data == 0)
        {
            quit_avoid_.store(true);
            ROS_INFO("障碍物消失，退出停障状态");
        }
        if (avoid_status_.data == 2)
            slow_down_ = true;
        else
            slow_down_ = false;
        pre_avoid_status_ = avoid_status_;
        // ROS_INFO("avoid_status_: %d", avoid_status_.data);
    }

    bool turn_success_callback(read_waypoints::TurnCompleted::Request &req, read_waypoints::TurnCompleted::Response &res)
    {
        std::lock_guard<std::mutex> lock(turn_success_mutex_);
        if (req.x == turn_success_msg_.request.x && req.y == turn_success_msg_.request.y)
        {
            if (turn_success_msg_.response.turn_finish)
            {
                res.turn_finish = true;
                turn_success_msg_.response.turn_finish = false;
                ROS_INFO("转弯完成");
                return true;
            }
            else
            {
                res.turn_finish = false;
                ROS_INFO("转弯未完成");
                return true;
            }
        }
    }

    void stopCarCallback(const std_msgs::Bool &stop_car_msg)
    {
        stop_car_.store(stop_car_msg.data);
        if (stop_car_.load())
        {
            ROS_INFO("收到停止小车运动信号");
        }
        else
        {
            ROS_INFO("收到恢复小车运动信号");
        }
    }

    void process()
    {
        util::LocalPath current_local_path;
        {
            std::lock_guard<std::mutex> lock(path_mutex_);
            current_local_path = current_local_path_;
            current_local_path_.point_num = 0; // 避免重复处理同一路径
        }
        double time_out = 0.1;
        if (has_new_path_.load())
        {
            for (int i = 0; i < current_local_path.point_num; ++i)
            {
                if (!has_new_path_.load())
                    break;
                if (start_avoid_plan_.load())
                {
                    ROS_INFO("正在规划避障路线，停止运动");
                    // start_avoid_plan_.store(false);
                    util::Position position_msg;
                    position_msg.position_x = current_local_path.x[std::max(0, i-1)];
                    position_msg.position_y = current_local_path.y[std::max(0, i-1)];
                    position_msg.yaw = current_local_path.heading[std::max(0, i-1)];
                    position_msg.position_state = 1;
                    while (ros::ok() && !avoid_plan_success_.load())
                    {
                        pub_position_.publish(position_msg);

                        if (avoid_plan_success_.load() || quit_avoid_.load())
                            break;
                        ros::Duration(0.1).sleep();
                        // ROS_INFO("pos: %.4f, %.4f, %.4f", position_msg.position_x, position_msg.position_y, position_msg.yaw);
                    }
                    if (avoid_plan_success_.load() || quit_avoid_.load())
                    {
                        ROS_INFO("避障路线规划完成，执行避障运动 或 障碍物消失，退出停障状态");
                        avoid_plan_success_.store(false);
                        start_avoid_plan_.store(false);
                        quit_avoid_.store(false);
                        break;
                    }
                    return;
                }
                if (slow_down_.load())
                {
                    ROS_INFO("减速");
                    time_out = 0.3;
                }
                if (stop_car_.load())
                {
                    ROS_INFO("小车运动已停止");
                    util::Position position_msg;
                    position_msg.position_x = current_local_path.x[std::max(0, i-1)];
                    position_msg.position_y = current_local_path.y[std::max(0, i-1)];
                    position_msg.yaw = current_local_path.heading[std::max(0, i-1)];
                    position_msg.position_state = 1;
                    while (ros::ok() && stop_car_.load())
                    {
                        pub_position_.publish(position_msg);
                        ros::Duration(0.1).sleep();
                        // ROS_INFO("pos: %.4f, %.4f, %.4f", position_msg.position_x, position_msg.position_y, position_msg.yaw);
                    }
                    ROS_INFO("小车运动恢复");
                }
                // 模拟小车位置更新
                util::Position position_msg;
                position_msg.position_x = current_local_path.x[i];
                position_msg.position_y = current_local_path.y[i];
                position_msg.yaw = current_local_path.heading[i];
                position_msg.position_state = 1;
                if (current_local_path.gear[i] == 2) // 模拟转向点，发布转向成功信号
                {
                    std::lock_guard<std::mutex> lock(turn_success_mutex_);
                    turn_success_msg_.request.x = position_msg.position_x;
                    turn_success_msg_.request.y = position_msg.position_y;
                    turn_success_msg_.response.turn_finish = false;

                    int turn_count = 2;
                    while (ros::ok() && turn_count > 0)
                    {
                        pub_position_.publish(position_msg);
                        // ROS_INFO("pos: %.4f, %.4f, %.4f", position_msg.position_x, position_msg.position_y, position_msg.yaw);
                        ros::Duration(0.1).sleep(); // 模拟小车转向时间
                        turn_count--;
                    }
                    turn_success_msg_.response.turn_finish = true;
                    ros::Duration(0.6).sleep();
                    // ROS_INFO("pos: %.4f, %.4f, %.4f", position_msg.position_x, position_msg.position_y, position_msg.yaw);
                    ROS_INFO("小车已转向完成");
                }
                pub_position_.publish(position_msg);
                ROS_INFO("pos: %.4f, %.4f, %.4f", position_msg.position_x, position_msg.position_y, position_msg.yaw);

                geometry_msgs::TransformStamped tf_msg;
                tf_msg.header.stamp = ros::Time::now();
                tf_msg.header.frame_id = "map";
                tf_msg.child_frame_id = "base_link";

                tf_msg.transform.translation.x = current_local_path.x[i];
                tf_msg.transform.translation.y = current_local_path.y[i];
                tf_msg.transform.translation.z = 0.0;
                geometry_msgs::Quaternion odom_quat = tf::createQuaternionMsgFromYaw(current_local_path.heading[i]);
                tf_msg.transform.rotation = odom_quat;
                tf_broadcaster_.sendTransform(tf_msg);
                
                ros::Duration(time_out).sleep(); // 模拟小车移动的时间
            }
        }
    }
};


int main(int argc, char **argv)
{
    setlocale(LC_ALL,"");
    ros::init(argc, argv, "move_simulation");
    ros::NodeHandle nh;
    MoveSimulation move_simulation(nh);

    ros::AsyncSpinner spinner(2); // 两个线程
    spinner.start();

    ros::Rate rate(10);
    while (ros::ok())
    {
        move_simulation.process();
        rate.sleep();
    }
    

    ros::waitForShutdown();
    return 0;
}