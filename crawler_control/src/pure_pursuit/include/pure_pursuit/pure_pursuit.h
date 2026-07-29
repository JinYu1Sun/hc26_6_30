#pragma once
#include <iostream>
#include <mutex>
#include <deque>
#include <ros/ros.h>
#include <atomic>
#include <vector>
#include <cmath>
#include <limits>
#include "std_msgs/Bool.h"
#include "std_msgs/UInt8.h"
#include "std_msgs/String.h"
#include "util/Position.h"
#include "util/LocalPath.h"
#include "pure_pursuit/TurnCompleted.h"
#include "mower_msgs/VehicleCmd.h"


struct waypoint
{
	float global_x; // 全局坐标系下的x坐标
	float global_y;
	float global_yaw;
	float local_x;	// 小车坐标系下的x坐标
	float local_y;
	float local_yaw;
	int gear;
	float speed;
	bool operator==(const waypoint& other) const
	{
		return global_x == other.global_x && global_y == other.global_y && global_yaw == other.global_yaw && 
		local_x == other.local_x && local_y == other.local_y && local_yaw == other.local_yaw &&
		gear == other.gear && speed == other.speed;
	}
};

typedef struct
{
	double linear;
	double angular;
}Twist;

enum class RunStateValue : uint8_t {
	Stop,
	Follow,
	Turn,
	Reverse
};

class PurePursuit
{
public:
	PurePursuit(const ros::NodeHandle &nh);
private:
	ros::NodeHandle nh_;
	std::atomic<RunStateValue> running_state_;

	util::Position car_position_;    //  小车实时位置
	ros::Time last_pos_time_;        // 小车实时位置最新时间
	
	std::mutex local_path_mutex_;
	util::LocalPath last_local_path_;   // 最新局部路径
	ros::Time last_local_path_time_;
	
	std::mutex local_waypoints_mutex_;
	std::vector<waypoint> local_waypoints_; // 局部路径转换到小车坐标系下

	std::mutex turn_completed_mutex_;
	std::deque<waypoint> turn_completed_points_;  // 小车完成转弯的点
	uint turn_count_; // 转弯的次数
	
	float min_lookahead_distance_;
	waypoint lookahead_waypoint_;	// 预瞄点
	waypoint last_lookahead_waypoint_;	//
	float lookahead_distance_;	// 预瞄距离
	float last_lookahead_distance_;   // 期望速度
	ros::Subscriber sub_position_;
	ros::Subscriber sub_local_path_;
	ros::Subscriber sub_stop_signal_;
	ros::Subscriber sub_avoid_state_;
	ros::Subscriber sub_signal_;

	ros::Publisher pub_command_;    // 发送控制指令

	ros::ServiceServer turn_completed_srv_;

	float v_min_;   // 最小速度
	float w_max_;   // 最大角速度
	float regulated_min_radius_; // 用于限制和速度从而调节角速度，防止角速度过大
	std::atomic<bool> low_speed_flag_;   // 低速模式
	float low_speed_;   // 低速模式下的速度

	std::mutex avoid_state_mutex_;
	uint8_t avoid_state_;   // 避障状态

	std::atomic<bool> stop_car_; // 是否订阅到停车信号


	std::mutex mover_mutex_;
	int mover_bool_cfg_; // 割盘开关控制
	int mower_height_cfg_; // 割盘高度控制

	void positionCallback(const util::PositionConstPtr &msg_carposition);
	void localPathCallback(const util::LocalPathConstPtr &msg_planning_result);
	void stopSignalCallback(const std_msgs::BoolConstPtr &msg_stop_signal);
	void avoidstateCallback(const std_msgs::UInt8ConstPtr &avoid_msg);
	void signalCallback(const std_msgs::StringConstPtr &signal_msg);

	bool turnCompletedCallback(pure_pursuit::TurnCompleted::Request &req, pure_pursuit::TurnCompleted::Response &res);

	int findNearestPoint(const std::vector<waypoint> &local_waypoints);   // 找最近点索引
	waypoint calculateLookaheadPoint(const std::vector<waypoint> &local_waypoints, const int &nearest_waypoint_index, const float &v_expect); // 计算预瞄点
	Twist calculate_PurePursuit(const float &v_expect, const waypoint &lookahead_point);  // 纯跟踪计算和速度

	float calculate_rotate_vel(const float &yaw);
	void publishCommand(const Twist &cmd);

	void state_machine_run();
};