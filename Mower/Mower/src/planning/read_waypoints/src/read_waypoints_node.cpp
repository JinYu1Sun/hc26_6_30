#include "util/LocalPath.h"
#include "util/Position.h"
#include <fstream>
#include <vector>
#include <geometry_msgs/Point.h>
#include <geometry_msgs/PointStamped.h>
#include <geometry_msgs/PoseArray.h>
#include <iostream>
#include <math.h>
#include <nav_msgs/Path.h>
#include <ros/ros.h>
#include <sstream>
#include <std_msgs/Bool.h>
#include <std_msgs/String.h>
#include <std_msgs/UInt8.h>
#include "util/Position.h"
#include "util/LocalPath.h"
#include <deque>
#include <cstdio>
#include <cmath>
#include "read_waypoints/TurnCompleted.h"
#define DEBUG_LOGGING // 定义DEBUG_LOGGING以启用调试日志输出


// std::string csv_path_pre_ = "/home/oem/Mower/waypoints/";
std::string csv_path_pre_ = "/home/nvidia/Mower/waypoints/";
std::ofstream csv_file;
std::deque<std::pair<double, double>> waypoint_buffer; 
std::string current_csv_path = csv_path_pre_ + "dropped_waypoints.csv";
util::LocalPath planned_path_; 
std::string current_name_;
bool use_multi_map_ = false;  // 新增：标识是否使用多地图模式

std::string getLogTime()
{
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    
    // 将时间格式化为字符串
    std::tm local_time;
#ifdef _WIN32
    localtime_s(&local_time, &now_time);  // Windows
#else
    localtime_r(&now_time, &local_time);  // POSIX
#endif

    // 使用 ostringstream 来格式化时间
    std::ostringstream oss;
    oss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
    std::string result = "[" + oss.str() + "] ";
    return result;  // 返回格式化后的时间字符串
}

//TODO
void initCsvLogger(const std::string &file_path)
{
    current_csv_path = file_path;  // 更新当前文件路径

    csv_file.open(current_csv_path, std::ios::app);  // 追加写入
    if (!csv_file.is_open())
    {
        ROS_ERROR("Failed to open CSV file: %s", current_csv_path.c_str());
    }
    else
    {
        ROS_INFO("Opened CSV file: %s", current_csv_path.c_str());
    }
}
//TODO
void saveDroppedWaypoint(double x, double y)
{
    waypoint_buffer.emplace_back(x, y);

    if (waypoint_buffer.size() > 100) {
        waypoint_buffer.erase(waypoint_buffer.begin(), waypoint_buffer.begin() + 80);
    }

    csv_file.close();
    csv_file.open(current_csv_path, std::ios::out | std::ios::trunc);  // 这里路径已经被initCsvLogger更新了，实际文件名是1.csv或者multi1.csv
    if (!csv_file.is_open()) {
        ROS_ERROR("Failed to open CSV file for writing: %s", current_csv_path.c_str());
        return;
    }

    for (const auto& point : waypoint_buffer) {
        csv_file << point.first << "," << point.second << "\n";
    }
    csv_file.flush();
}




class WaypointManager
{
private:
  // ROS节点句柄
  ros::NodeHandle nh_;

  // 订阅器
  ros::Subscriber sub_trajectory_;
  ros::Subscriber sub_avoid_trajectory_;
  ros::Subscriber sub_avoid_status_;
  ros::Subscriber sub_goal_point_;
  ros::Subscriber sub_position_;
  ros::Subscriber sub_signal_;

  ros::ServiceClient turn_completed_cli_;

  // 发布器
  ros::Publisher control_path_pub_;
  ros::Publisher avoid_detection_path_pub_;
  ros::Publisher mower_num_pub_;
  ros::Publisher test_waypoint_pub_;
  ros::Publisher pub_signal_;

  #ifdef DEBUG_LOGGING
  ros::Publisher global_trajectory_pub_;
  ros::Publisher avoid_trajectory_pub_;
  ros::Publisher local_trajectory_pub_;
  #endif

  // 路径数据
  util::LocalPath global_path_; 
  util::LocalPath global_path_copy_; // 一份最原始的全局轨迹
  util::LocalPath avoid_path_;
  util::LocalPath current_path_segment_; // 当前发布的路径段
  util::LocalPath recovery_path_;        // 路径恢复用的路径
  geometry_msgs::PoseArray pose_array_msg_;

  // 状态标志
  int min_pos_ = 0;                // 全局路径当前最近点索引
  int avoid_min_pos_ = 0;          // 避障路径当前最近点索引
  int recovery_min_pos_ = 0;       // 恢复路径当前最近点索引

  geometry_msgs::PointStamped current_position_; // 当前位置
  geometry_msgs::Point goal_point_; // 避障的终点
  // 避障状态
  std::mutex avoid_status_mutex_; // 保护避障状态的互斥量
  std_msgs::UInt8 avoid_status_;
  int pre_avoid_status_ = 0;
  bool avoid_status_changed_ = false;       // 避障状态从1变为0，避障完成的标志
  bool need_recovery_path_ = false; // 需要进行路径恢复

  // 初始化标志
  bool first_loop_ = true;
  bool avoid_first_loop_ = false;

  // // 避障路径终点
  // double avoid_end_pos_x_ = 0.0;
  // double avoid_end_pos_y_ = 0.0;

  // 断点复割模式
  bool repeat_mode_ = false;
  std_msgs::String mower_num_;

public:
  WaypointManager() : nh_() {
    // 初始化避障状态
    avoid_status_.data = 0;

    // 初始化订阅器
    sub_trajectory_ = nh_.subscribe("/lawn_mower/global_trajectory", 1, &WaypointManager::trajectoryCallback, this);
    sub_avoid_trajectory_ = nh_.subscribe("/lawn_mower/avoid_traj", 1, &WaypointManager::avoidTrajectoryCallback, this);
    sub_avoid_status_ = nh_.subscribe("/lawn_mower/avoid_state", 1, &WaypointManager::avoidStatusCallback, this);
    sub_goal_point_ = nh_.subscribe("/lawn_mower/target_point", 1, &WaypointManager::goalPointCallback, this);
    sub_position_ = nh_.subscribe("/Mower/position", 1, &WaypointManager::positionCallback, this);
    sub_signal_ =nh_.subscribe("/signal", 1, &WaypointManager::signalCallback, this);

    turn_completed_cli_ = nh_.serviceClient<read_waypoints::TurnCompleted>("/turn_completed");

    // 初始化发布器
    control_path_pub_ = nh_.advertise<util::LocalPath>("/lawn_mower/global_path", 1); // 发布控制需要的局部路径
    avoid_detection_path_pub_ = nh_.advertise<util::LocalPath>("/lawn_mower/avoid_detection_path", 1); // 发布避障检测路径
    mower_num_pub_ = nh_.advertise<std_msgs::String>("/lawn_mower/machineNo", 1);
    // 读取并发布最终的全局轨迹(包括拼接路径)，由UI点阅显示
    test_waypoint_pub_ = nh_.advertise<geometry_msgs::PoseArray>("/newwaypoints_list", 1, true);
    pub_signal_ = nh_.advertise<std_msgs::String>("/signal", 1);  // 发布任务执行完成的信号，其他规划节点订阅后用于重置状态
  #ifdef DEBUG_LOGGING
    global_trajectory_pub_ = nh_.advertise<nav_msgs::Path>("/debug/global_trajectory", 1, true);
    avoid_trajectory_pub_ = nh_.advertise<nav_msgs::Path>("/debug/avoid_trajectory", 1, true);
    local_trajectory_pub_ = nh_.advertise<nav_msgs::Path>("/debug/local_trajectory", 1, true);
  #endif

    // 读取割草机编号
    readMowerID();
  }

  // 读取割草机编号
  void readMowerID() {
    std::ifstream input_file(
        csv_path_pre_ + "MowerNumber.txt");
    if (!input_file) {
      ROS_ERROR("Could not read the mower SN!");
      return;
    }

    std::ostringstream buffer;
    buffer << input_file.rdbuf();
    std::string file_contents = buffer.str();
    ROS_INFO("Mower SN: %s", file_contents.c_str());

    mower_num_.data = file_contents;
    mower_num_pub_.publish(mower_num_);
  }

  void generateStraightLinePathToTarget(double target_x, double target_y)
  {
    planned_path_ = util::LocalPath();  // 清空旧路径

    const double start_x = current_position_.point.x;
    const double start_y = current_position_.point.y;

    ROS_INFO("Current position: (%.2f, %.2f), Target: (%.2f, %.2f)", start_x, start_y, target_x, target_y);

    // 计算总距离和方向
    const double dx = target_x - start_x;
    const double dy = target_y - start_y;
    const double distance = std::hypot(dx, dy);

    if (distance < 1e-3) {
        ROS_WARN("Target too close to current position, skipping path generation.");
        return;
    }

    // 计算航向角（弧度），OX=0, OY=π/2
    const double heading_rad = std::atan2(dy, dx);

    // 每0.25米一个点
    const double step = 0.25;
    const int num_points = std::max(2, static_cast<int>(std::ceil(distance / step)) + 1);

    ROS_INFO("Distance: %.2f, Num points: %d, Heading: %.3f rad (%.1f deg)", 
             distance, num_points, heading_rad, heading_rad * 180.0 / M_PI);

    planned_path_.x.resize(num_points);
    planned_path_.y.resize(num_points);
    planned_path_.speed.resize(num_points);
    planned_path_.heading.resize(num_points);
    planned_path_.gear.resize(num_points);

    // 生成路径点
    for (int i = 0; i < num_points; ++i) {
        double ratio = static_cast<double>(i) / (num_points - 1);
        planned_path_.x[i] = start_x + dx * ratio;
        planned_path_.y[i] = start_y + dy * ratio;
        planned_path_.heading[i] = heading_rad;  // 存储为弧度

        // gear设置：第一个点和最后一个点为2（R档），其他点为1（D档）
        if (i == 0 || i == num_points - 1) {
            planned_path_.gear[i] = 2;  // R档
        } else {
            planned_path_.gear[i] = 1;  // D档
        }

        // speed设置
        if (i == 0 || i == num_points - 1) {
            // 第一个点和最后一个点为0
            planned_path_.speed[i] = 0.0;
        } else if (i == 1 || i == 2 || i == num_points - 2 || i == num_points - 3) {
            // 第二个、第三个与倒数第二个、第三个为0.25
            planned_path_.speed[i] = 0.25;
        } else {
            // 其他点按照梯形方式进行速度分配，最大为1.0 m/s
            int middle_start = 3;
            int middle_end = num_points - 4;
            
            if (middle_end >= middle_start) {
                int middle_points = middle_end - middle_start + 1;
                int mid_point = middle_start + middle_points / 2;
                
                double accel_ratio;
                if (i <= mid_point) {
                    // 加速段：从0.25到1.0
                    accel_ratio = static_cast<double>(i - middle_start + 1) / (mid_point - middle_start + 1);
                } else {
                    // 减速段：从1.0到0.25
                    accel_ratio = static_cast<double>(middle_end - i + 1) / (middle_end - mid_point + 1);
                }
                accel_ratio = std::clamp(accel_ratio, 0.0, 1.0);
                planned_path_.speed[i] = 0.25 + accel_ratio * 0.75; // 从0.25到1.0
            } else {
                planned_path_.speed[i] = 0.25;
            }
        }
    }

    // 设置路径属性
    planned_path_.point_num = static_cast<uint8_t>(num_points);
    planned_path_.pathtype = 0;  // 临时路径类型
    planned_path_.header.stamp = ros::Time::now();
    planned_path_.header.frame_id = "map";

    // 打印前几个点的信息用于调试
    ROS_INFO("First 5 points:");
    for (int i = 0; i < std::min(5, num_points); ++i) {
        ROS_INFO("Point %d: (%.2f, %.2f), speed: %.2f, gear: %d, heading: %.3f rad", 
                 i, planned_path_.x[i], planned_path_.y[i], planned_path_.speed[i], 
                 planned_path_.gear[i], planned_path_.heading[i]);
    }
    
    if (num_points > 5) {
        ROS_INFO("Last 5 points:");
        for (int i = std::max(0, num_points - 5); i < num_points; ++i) {
            ROS_INFO("Point %d: (%.2f, %.2f), speed: %.2f, gear: %d, heading: %.3f rad", 
                     i, planned_path_.x[i], planned_path_.y[i], planned_path_.speed[i], 
                     planned_path_.gear[i], planned_path_.heading[i]);
        }
    }

    ROS_INFO("Generated straight path with %d points from (%.2f, %.2f) to (%.2f, %.2f), heading: %.3f rad",
             num_points, start_x, start_y, target_x, target_y, heading_rad);
  }
//TODO
  std::pair<double, double> readLastCsvPoint(const std::string &file_path)
  {
    std::ifstream file(file_path);
    std::string line;
    std::pair<double, double> result(-1, -1);

    if (!file.is_open()) {
        ROS_ERROR("Cannot open CSV file: %s", file_path.c_str());
        return result;
    }

    // 读取最后一行
    while (file.good()) {
        std::string temp;
        std::getline(file, temp);
        if (!temp.empty()) {
            line = temp;
        }
    }
    file.close();
    if (line.empty()) {
        ROS_WARN("CSV file is empty: %s", file_path.c_str());
        return result;
    }
    std::stringstream ss(line);
    std::string x_str, y_str;
    if (std::getline(ss, x_str, ',') && std::getline(ss, y_str)) {
        result.first = std::stod(x_str);
        result.second = std::stod(y_str);
    }
    ROS_INFO("Restar waypoint: (%f, %f)", result.first, result.second);
    return result;
  }
  void publishWaypointListFromGlobalPath() {
    geometry_msgs::PoseArray pose_array;
    pose_array.header.stamp = ros::Time::now();
    pose_array.header.frame_id = "map";  // 根据你的坐标系改

    for (size_t i = 0; i < global_path_.x.size(); ++i) {
        geometry_msgs::Pose pose;
        pose.position.x = global_path_.x[i];
        pose.position.y = global_path_.y[i];
        pose.position.z = 0.0;  // 默认
        double yaw = global_path_.heading[i];
        // heading是以弧度表示的，需要转换为四元数
        pose.orientation.x = 0.0;
        pose.orientation.y = 0.0;
        pose.orientation.z = sin(yaw / 2.0);
        pose.orientation.w = cos(yaw / 2.0);
        if (global_path_.gear[i] == 2)
          pose_array.poses.push_back(pose);
        if (i == global_path_.x.size() - 1)
          pose_array.poses.push_back(pose);
    }

    test_waypoint_pub_.publish(pose_array);
    ROS_INFO("Published %lu waypoints to /test_waypoint_list", pose_array.poses.size());
  }

  void readWaypointsAndPublish() {
    std::string csv_file_path;
    csv_file_path = csv_path_pre_ + current_name_ + ".csv";
    
    std::ifstream file(csv_file_path);
    
    if (!file.is_open()) {
      ROS_ERROR("Could't open the csv file: %s", csv_file_path.c_str());
      return;
    }

    ROS_INFO("Reading waypoints from: %s", csv_file_path.c_str());
    clearLocalPath(global_path_);

    std::string line;
    while (std::getline(file, line)) {
      std::stringstream ss(line);
      std::string token;

      if (std::getline(ss, token, ','))
        global_path_.x.push_back(std::stod(token));

      if (std::getline(ss, token, ','))
        global_path_.y.push_back(std::stod(token));

      if (std::getline(ss, token, ','))
        global_path_.heading.push_back(std::stod(token));

      if (std::getline(ss, token, ','))
        global_path_.gear.push_back(std::stoi(token));

      if (std::getline(ss, token, ','))
        global_path_.speed.push_back(std::stod(token));
    }
    first_loop_ = true;
    global_path_copy_ = global_path_;
    ROS_INFO("Read csv global size: %zu", global_path_.x.size());
  }
  // 计算当前点与路径点的关系，判断是否应该丢弃该点，如果当前点与路径点过近且无需转向，则丢弃该点
  bool shouldDropWaypoint(double path_x, double path_y, double path_heading, int gear, const geometry_msgs::PointStamped &current_position) {
    float theta = current_position.point.z;
    float cos_val = cosf(theta);
    float sin_val = sinf(theta);

    double dx = path_x - current_position.point.x;
    double dy = path_y - current_position.point.y;

    // 转换到车辆坐标系
    double local_x = dx * cos_val + dy * sin_val;
    double local_y = dx * (-sin_val) + dy * cos_val;

    // 计算航向误差
    double error_heading = theta - path_heading;
    if (error_heading > M_PI) {
      error_heading = error_heading - 2 * M_PI;
    } else if (error_heading < -M_PI) {
      error_heading = error_heading + 2 * M_PI;
    }

    ROS_INFO("local_x: %.4f, local_y: %.4f, gear: %ld,  error_heading: %.4f", local_x, local_y, gear, error_heading);
    // 当前点在车的前方1.5m或在后方3m外或横向偏差超过0.6m触发恢复
    if (local_x > 2.5 || local_x < -3 || fabs(local_y) > 2.6)
    {
      need_recovery_path_ = true;
      std::cout << getLogTime() << "已偏离原来割草路线" << std::endl;
      return false;
    }
    if (local_x <= 0.05 && gear == 1 && std::fabs(error_heading) < M_PI) 
    {
      ROS_INFO("gear=1 points is droped, %.4f, %.4f", path_x, path_y);
      return true;
    }
    // 转弯点处理，如果是个转向点，应该会先通过pathRecoveryCallback()通知转向是否完成
    if (gear == 2)
    {
      read_waypoints::TurnCompleted turn_completed_cli;
      turn_completed_cli.request.x = path_x;
      turn_completed_cli.request.y = path_y;
      if (turn_completed_cli_.call(turn_completed_cli))
      {
        if (turn_completed_cli.response.turn_finish)
        {
          ROS_INFO("gear=2 points is droped, %.4f, %.4f", path_x, path_y);
          if (min_pos_ == 0) {
            ROS_WARN("First gear=2 point is dropped");
          }
          std::cout << getLogTime() << "转向完成!" << std::endl;
          return true;
        }
        else
        {
          std::cout << getLogTime() << "还未完成转向!" << std::endl;
        }
      }
      else
      {
        std::cout << getLogTime() << "还未完成转向!" << std::endl;
      }
    }
    ROS_INFO("%ld points is not droped, %.4f, %.4f, %.4f, %d, %.4f, %.4f, %.4f", min_pos_, path_x, path_y, path_heading, gear, 
      current_position.point.x, current_position.point.y, current_position.point.z);
    return false;
  }

  // 清空路径数据
  void clearLocalPath(util::LocalPath &path) {
    path.point_num = 0;
    path.x.clear();
    path.y.clear();
    path.speed.clear();
    path.heading.clear();
    path.gear.clear();
    path.header.stamp = ros::Time(0);
    path.header.frame_id = "";
  }

  // 查找最近路径点
  int findNearestWaypoint(const util::LocalPath &path, const geometry_msgs::PointStamped &current_position) {
    if (path.x.empty()) {
      return -1;
    }

    double min_distance = INFINITY;
    int nearest_index = 0;

    // for (int i = 0; i < path.x.size(); i++)
    for (int i = 0; i < path.x.size(); i++) {
      double dx = path.x[i] - current_position.point.x;
      double dy = path.y[i] - current_position.point.y;
      double distance = sqrt(dx * dx + dy * dy);

      if (distance < min_distance) {
        min_distance = distance;
        nearest_index = i;
      }
    }

    return nearest_index;
  }
  // 查找距离避障终点的最近路径点
  int findNearestWaypoint2(const util::LocalPath &path) {
    if (path.x.empty()) {
      return -1;
    }

    double min_distance = INFINITY;
    int nearest_index = 0;

    // 这里搜索起点使用min_pos_可以避免搜索到已走的路径上去
    for (int i = min_pos_; i < path.x.size(); i++) {
      double dx = path.x[i] - goal_point_.x;
      double dy = path.y[i] - goal_point_.y;
      double distance = sqrt(dx * dx + dy * dy);

      if (distance < min_distance) {
        min_distance = distance;
        nearest_index = i;
      }
    }

    ROS_WARN("the goal_point is : %f, %f", goal_point_.x, goal_point_.y);
    ROS_WARN("the nearest index is : %d, the nearest point is : %f,%f",
             nearest_index, path.x[nearest_index], path.y[nearest_index]);

    return nearest_index;
  }

  // 提取控制所需的局部路径段、全局路径和避障检测的局部路径段
  void extractPathSegment(const util::LocalPath &source, util::LocalPath &target1, util::LocalPath &target2,
                          int start_index, bool flag = false)
  {
    
    int waypoint_size = source.x.size();
    if (waypoint_size < 1 || start_index < 0 || start_index >= waypoint_size) {
      ROS_ERROR("Invalid start index or empty source path: start_index=%d, path_size=%d",
      start_index, waypoint_size);
      return;
    }
      
    clearLocalPath(target1); // 清空目标路径
    target1.header.stamp = ros::Time::now();
    target1.header.frame_id = "map";
    target1.pathtype = source.pathtype;
    target2 = target1; // 复制头信息和路径类型

    if (!flag)
    {
      // target1用于控制，提取到第一个转向点，target2用于避障检测，提取50个点
      target1.point_num = 0;
      for (int i = start_index; i < source.x.size(); i++)
      {
        target1.x.emplace_back(source.x[i]);
        target1.y.emplace_back(source.y[i]);
        target1.heading.emplace_back(source.heading[i]);
        target1.gear.emplace_back(source.gear[i]);
        target1.speed.emplace_back(source.speed[i]);
        target1.point_num++;
        if (source.gear[i] == 2)
          break; // 遇到转向点就停止提取
      }
      for (int i = start_index; i < std::min(start_index + 50, (int)source.x.size()); i++)
      {
        target2.x.emplace_back(source.x[i]);
        target2.y.emplace_back(source.y[i]);
        target2.heading.emplace_back(source.heading[i]);
        target2.gear.emplace_back(source.gear[i]);
        target2.speed.emplace_back(source.speed[i]);
        target2.point_num++;
      }
    }
    else
    {
      // 提取全部
      target1.point_num = 0;
      for (int i = start_index; i < source.x.size(); i++)
      {
        target1.x.emplace_back(source.x[i]);
        target1.y.emplace_back(source.y[i]);
        target1.heading.emplace_back(source.heading[i]);
        target1.gear.emplace_back(source.gear[i]);
        target1.speed.emplace_back(source.speed[i]);
        target1.point_num++;
      }
    }
    std::cout << getLogTime() << "控制路径段: 起点索引=" << start_index << ", 提取点数=" << (int)target1.point_num << std::endl;
    std::cout << getLogTime() << "避障检测路径段: 起点索引=" << start_index << ", 提取点数=" << (int)target2.point_num << std::endl;
  }

  void processGlobalPath(const geometry_msgs::PointStamped &current_position)
  {
    if (global_path_.x.size() < 1)
      return;
    
    if (avoid_status_changed_ || need_recovery_path_)
    {
      util::LocalPath tmp_path, null_path; // 新的全局路径，空路径在这里仅用于占位
      int min_pos = 0;
      if (need_recovery_path_)
      {
        need_recovery_path_ = false;
        min_pos = findNearestWaypoint(global_path_copy_, current_position);  // 在全局路径中查找与当前车位置最近的路径点索引
        extractPathSegment(global_path_copy_, tmp_path, null_path, min_pos, true);
        std::cout << getLogTime() << "恢复到最近的割草点上，最近割草路径点索引: " << min_pos << std::endl;
      }
      else if (avoid_status_changed_)
      {
        avoid_status_changed_ = false;
        min_pos = findNearestWaypoint2(global_path_); // 在全局路径中查找与该点最近的路径点索引，这里的全局路径是避障前的全局路径，不是原始的
        extractPathSegment(global_path_, tmp_path, null_path, min_pos, true);
        std::cout << getLogTime() << "避障结束，继续割草轨迹，最近割草路径点索引: " << min_pos << std::endl;
      }

      tmp_path.gear.front() = 2;  // 起点设置为停止点，gear=2
      tmp_path.speed.front() = 0;
      // 低速起步
      for (int i = 0; i < std::min(4, (int)tmp_path.speed.size()); ++i)
      {
        if (tmp_path.speed[i] != 0)
          tmp_path.speed[i] = 0.25;
      }
      global_path_ = tmp_path; // 更新新的全局路径(有了新起点，同时能够减小后续的计算量)
      min_pos_ = 0; // 更新了全局路径之后，min_pos_需要重置为0，重新开始
    }
    else if (first_loop_)
    {
      first_loop_ = false;
      min_pos_ = findNearestWaypoint(global_path_, current_position);
      std::cout << getLogTime() << "进入割草跟踪轨迹模式，最近点索引为：" << min_pos_ << std::endl;
      // 断点复割模式，低速起步
      for (int i = 0; i < std::min(4, (int)global_path_.speed.size()); ++i)
      {
        if (global_path_.speed[i] != 0)
          global_path_.speed[i] = 0.25;
      }
    }
    // 正常更新：如果当前点已经通过，则丢弃
    else if (shouldDropWaypoint(
                 global_path_.x[min_pos_], global_path_.y[min_pos_],
                 global_path_.heading[min_pos_], global_path_.gear[min_pos_], current_position))
    {
      saveDroppedWaypoint(global_path_.x[min_pos_], global_path_.y[min_pos_]);
      if (min_pos_ < static_cast<int>(global_path_.x.size()) - 1) {
        min_pos_++;
        // std::cout << getLogTime() << "新的最近点索引: " << min_pos_ << std::endl;
      }
      else {
        reset();
        std_msgs::String signal_msg;
        signal_msg.data = "mowing_finished";
        pub_signal_.publish(signal_msg);
        std::cout << getLogTime() << "已到达全局轨迹终点，当前割草任务完成" << std::endl;
        return;
      }
    }

    if (!need_recovery_path_)
    {
      util::LocalPath control_path_segment, avoid_detection_path_segment;
      extractPathSegment(global_path_, control_path_segment, avoid_detection_path_segment, min_pos_);
      control_path_pub_.publish(control_path_segment);
      avoid_detection_path_pub_.publish(avoid_detection_path_segment);
    #ifdef DEBUG_LOGGING
      // 发布当前路径段用于调试
      nav_msgs::Path debug_path;
      debug_path.header = control_path_segment.header;
      for (size_t i = 0; i < control_path_segment.x.size(); ++i) {
        geometry_msgs::PoseStamped pose;
        pose.header = control_path_segment.header;
        pose.pose.position.x = control_path_segment.x[i];
        pose.pose.position.y = control_path_segment.y[i];
        double yaw = control_path_segment.heading[i];
        pose.pose.orientation.z = sin(yaw / 2.0);
        pose.pose.orientation.w = cos(yaw / 2.0);
        debug_path.poses.push_back(pose);
      }
      local_trajectory_pub_.publish(debug_path);
    #endif
    }
  }

  // 处理避障路径
  void processAvoidPath(const geometry_msgs::PointStamped &current_position)
  {
    if (avoid_path_.x.size() < 1)
    {
      return;
    }

    // 首次进入避障模式(订阅到避障路径时)
    if (avoid_first_loop_) {
      avoid_min_pos_ = 0;
      avoid_first_loop_ = false;
      std::cout << getLogTime() << "避障轨迹已生成，首次进入避障模式" << std::endl;
    }
    std::cout << "避障轨迹最近点索引为：" << avoid_min_pos_ << std::endl;
    // 正常更新：如果当前点已经通过，则丢弃
    if (shouldDropWaypoint(avoid_path_.x[avoid_min_pos_],
                           avoid_path_.y[avoid_min_pos_],
                           avoid_path_.heading[avoid_min_pos_],
                           avoid_path_.gear[avoid_min_pos_], current_position)) {
      if (avoid_min_pos_ < static_cast<int>(avoid_path_.x.size()) - 1) {
        avoid_min_pos_++;
        std::cout << getLogTime() << "新的最近点索引: " << avoid_min_pos_ << std::endl;
      }
    }

    // 提取并发布路径段
    util::LocalPath control_path_segment, avoid_detection_path_segment;
    extractPathSegment(avoid_path_, control_path_segment, avoid_detection_path_segment, avoid_min_pos_);
    std::cout << getLogTime() << "当前避障路径段长度为：" << avoid_detection_path_segment.x.size() << std::endl;
    control_path_pub_.publish(control_path_segment);
    avoid_detection_path_pub_.publish(avoid_detection_path_segment);
  }

  // 回调函数：订阅全局轨迹
  void trajectoryCallback(const util::LocalPath &trajectory_msgs) {
    if (repeat_mode_)
    {
      std::cout << getLogTime() << "测试模式，使用本地轨迹，不接收规划的轨迹" << std::endl;
      return;
    }
	  ROS_INFO("%s接收到全局轨迹，轨迹点数为%d", getLogTime().c_str(), trajectory_msgs.x.size());
    global_path_ = trajectory_msgs;
    global_path_copy_ = trajectory_msgs;
    first_loop_ = true;

  #ifdef DEBUG_LOGGING
    // 发布全局轨迹用于调试
    nav_msgs::Path debug_path;
    debug_path.header = trajectory_msgs.header;
    for (size_t i = 0; i < trajectory_msgs.x.size(); ++i) {
      geometry_msgs::PoseStamped pose;
      pose.header = trajectory_msgs.header;
      pose.pose.position.x = trajectory_msgs.x[i];
      pose.pose.position.y = trajectory_msgs.y[i];
      double yaw = trajectory_msgs.heading[i];
      pose.pose.orientation.z = sin(yaw / 2.0);
      pose.pose.orientation.w = cos(yaw / 2.0);
      debug_path.poses.push_back(pose);
    }
    global_trajectory_pub_.publish(debug_path);
  #endif
  }

  // 回调函数：避障轨迹
  void avoidTrajectoryCallback(const util::LocalPath &trajectory_msgs) {
	ROS_INFO("%s接收到避障轨迹，轨迹点数为%d", getLogTime().c_str(), trajectory_msgs.x.size());
    avoid_path_ = trajectory_msgs;
    avoid_first_loop_ = true;
  #ifdef DEBUG_LOGGING

    // 发布全局轨迹用于调试
    nav_msgs::Path debug_path;
    debug_path.header.frame_id = "map";
    for (size_t i = 0; i < trajectory_msgs.x.size(); ++i) {
      geometry_msgs::PoseStamped pose;
      pose.header = trajectory_msgs.header;
      pose.pose.position.x = trajectory_msgs.x[i];
      pose.pose.position.y = trajectory_msgs.y[i];
      double yaw = trajectory_msgs.heading[i];
      pose.pose.orientation.z = sin(yaw / 2.0);
      pose.pose.orientation.w = cos(yaw / 2.0);
      debug_path.poses.push_back(pose);
      std::cout << "avoid_path_: " << trajectory_msgs.x[i] << " , " << trajectory_msgs.y[i] << ", " 
      << yaw << ", "  << static_cast<int>(trajectory_msgs.gear[i]) << std::endl;
    }
    avoid_trajectory_pub_.publish(debug_path);
  #endif
  }

  void goalPointCallback(const geometry_msgs::Point &goal_point_msgs) {
    goal_point_.x = goal_point_msgs.x;
    goal_point_.y = goal_point_msgs.y;
  }
  // 回调函数：避障状态
  void avoidStatusCallback(const std_msgs::UInt8 &avoid_status_msgs) {
    std::lock_guard<std::mutex> lock(avoid_status_mutex_);
    avoid_status_ = avoid_status_msgs;

    // 检测避障状态从1变为0
    if (pre_avoid_status_ == 1 && avoid_status_.data == 0) {
      std::cout << getLogTime() << "避障状态改变为割草状态，准备继续割草轨迹" << std::endl;
      avoid_status_changed_ = true;
    }

    pre_avoid_status_ = avoid_status_.data;
  }

  // 回调函数：位置更新
  void positionCallback(const util::Position &loc_msg)
  {
    // 更新当前位置
    current_position_.point.x = loc_msg.position_x;
    current_position_.point.y = loc_msg.position_y;
    current_position_.point.z = loc_msg.yaw;
    geometry_msgs::PointStamped current_position = current_position_;

    std_msgs::UInt8 avoid_status_copy;
    {
      std::lock_guard<std::mutex> lock(avoid_status_mutex_);
      avoid_status_copy = avoid_status_;
    }
    // std::cout << getLogTime() << "当前点: " << current_position.point.x << " "
    //          << current_position.point.y << " " << current_position.point.z
    //          << std::endl;
    // 根据当前模式处理不同路径
    if (avoid_status_copy.data == 1) {
      processAvoidPath(current_position);
    } else {
      processGlobalPath(current_position);
    }
  }

  /*
  单地图正常工作流程：
  1.收到use_map/map_name信号，进入单地图模式，设置已通过的路径点scv文件为map_name1.scv
  2.收到test_map信号，开始测试
    2.1 先读取map_name1.csv文件的最后一个点，生成当前位置到该点的路径planned_path_
    2.2 再读取map_name.csv文件的所有点，获得割草要走的全局路径global_path_
    2.3 在global_path_中找到与最后一个已通过的路径点最近的点，截取该点之后的路径更新global_path_，丢弃之前的点
    2.4 拼接：planned_path_ = planned_path_ + global_path_，发布planned_path_
  */
  void signalCallback(const std_msgs::String &string)
  {
    const std::string &data = string.data;

    // 如果有 '/'，提取 prefix 和 name
    std::string prefix, name;
    std::size_t slash_pos = data.find('/');
    if (slash_pos != std::string::npos) {
      prefix = data.substr(0, slash_pos);
      name = data.substr(slash_pos + 1);
    }
    
    if (prefix == "use_map") {
      repeat_mode_ = false;
      use_multi_map_ = false;  // 重置多地图标志
      current_name_ = name;
      std::string csv_path = csv_path_pre_ + name + "1.csv";
      initCsvLogger(csv_path); // 更新文件路径
      ROS_INFO("use_map signal received, using map: %s, CSV file: %s", name.c_str(), csv_path.c_str());
    }
    // 新增：处理 multi_map 信号
    else if (prefix == "multi_map") {
      use_multi_map_ = true;
      ROS_INFO("multi_map signal received, will use multi-map CSV: %s", name.c_str());
    }
    // 如果是 delete_name/map01
    else if (prefix == "delete_name") {
      current_name_ = name;
      std::string csv_path = csv_path_pre_ + name + "1.csv";
      if (std::remove(csv_path.c_str()) == 0) {
          ROS_INFO("Deleted CSV file: %s", csv_path.c_str());
      } else {
          ROS_ERROR("Failed to delete CSV file: %s", csv_path.c_str());
      }
    }
    /* 
    这个分支是在没有订阅到全局路径时用于测试的，进入测试模式后会杀死全局路径发布节点
    先读取后缀为1.csv文件(已通过的点)的最后一个点，再拼接新的全局路径
    */
    else if (data == "test_map") {
      repeat_mode_ = true;
        
      // 根据是否使用多地图模式选择不同的CSV文件路径
      std::string csv_path;
      csv_path = csv_path_pre_ + current_name_ + "1.csv";
      ROS_INFO("Using map CSV file: %s", csv_path.c_str());
    
      // 先读取后缀为1.csv文件(已通过的点)的最后一个点
      std::pair<double, double> last_point = readLastCsvPoint(csv_path);
        
      if (last_point.first != -1 && last_point.second != -1) {  // 这里用-1是因为上面读取函数中把-1设为默认值了
        ROS_INFO("Last point from CSV file: (%.2f, %.2f)", last_point.first, last_point.second);
        ROS_INFO("Generating straight line path to last point...");
        generateStraightLinePathToTarget(last_point.first, last_point.second);  // 生成当前位置到最后一个已通过的点的路径，保存到planned_path_
        
        ROS_INFO("Reading waypoints from CSV file...");
        readWaypointsAndPublish();  // 读取另一个csv文件并更新 global_path_
        
        ROS_INFO("Before insertion - global_path_ size: %zu", global_path_.x.size());
        ROS_INFO("planned_path_ size: %zu", planned_path_.x.size());

        // 下面的处理获取当前车的位置到第一个CSV终点再拼接第二个CSV
        // 找到与第一个CSV终点最近的点，并截取该点之后的路径保存到 global_path_，丢弃之前的点
        int matched_index = -1;
        double min_dist = std::numeric_limits<double>::max();
        for (size_t i = 0; i < global_path_.x.size(); ++i) 
        {
          double dx = global_path_.x[i] - last_point.first;
          double dy = global_path_.y[i] - last_point.second;
          double dist = std::hypot(dx, dy);
          if (dist < min_dist) {
              min_dist = dist;
              matched_index = i;
          }
        }
        if (matched_index != -1) 
        {
          ROS_INFO("Matched point found at index %d (dist=%.3f), trimming global_path_", matched_index, min_dist);

          global_path_.x = std::vector<float>(global_path_.x.begin() + matched_index, global_path_.x.end());
          global_path_.y = std::vector<float>(global_path_.y.begin() + matched_index, global_path_.y.end());
          global_path_.speed = std::vector<float>(global_path_.speed.begin() + matched_index, global_path_.speed.end());
          global_path_.heading = std::vector<float>(global_path_.heading.begin() + matched_index, global_path_.heading.end());
          global_path_.gear = std::vector<int8_t>(global_path_.gear.begin() + matched_index, global_path_.gear.end());
        }
        else {
          ROS_WARN("No matching point found in global_path_ for last point from CSV.");
        }

        //TODO:global_path_要截取last_point之后的点        
        // 简单拼接 planned_path_ 到 global_path_ 前面
        global_path_.x.insert(global_path_.x.begin(), planned_path_.x.begin(), planned_path_.x.end());
        global_path_.y.insert(global_path_.y.begin(), planned_path_.y.begin(), planned_path_.y.end());
        global_path_.speed.insert(global_path_.speed.begin(), planned_path_.speed.begin(), planned_path_.speed.end());
        global_path_.heading.insert(global_path_.heading.begin(), planned_path_.heading.begin(), planned_path_.heading.end());
        global_path_.gear.insert(global_path_.gear.begin(), planned_path_.gear.begin(), planned_path_.gear.end());
        global_path_.point_num = global_path_.x.size();
        // 重置处理状态，确保从拼接后的路径开始
        min_pos_ = 0;
        first_loop_ = true;
        ROS_INFO("Reset min_pos_ to 0 and first_loop_ to true for test mode");
        // std::system("rosnode kill /global_path");
        // std::system("rosnode kill /mower/global_path");
        publishWaypointListFromGlobalPath();
        
        // 重置多地图标志（仅在使用后重置）
        if (use_multi_map_) {
          use_multi_map_ = false;
          ROS_INFO("Multi-map mode used and reset");
        }
      }
      else {
        ROS_ERROR("Failed to read last point from CSV file: %s", csv_path.c_str());
      }
    }
    else if (data == "pause")
    {
      std::cout << getLogTime() << "收到割草任务暂停信号，控制层会直接处理" << std::endl;
    }
    else if (data == "continue")
    {
      std::cout << getLogTime() << "收到割草任务继续信号，控制层会直接处理" << std::endl;
    }
    else if (data == "stop") {
      reset();
      std::cout << getLogTime() << "割草任务停止" << std::endl;
    }
    else if (data == "reset") {
      reset();
    }
  }

  void reset()
  {
    // 清空所有路径数据
    clearLocalPath(global_path_);
    clearLocalPath(global_path_copy_);
    clearLocalPath(avoid_path_);
    clearLocalPath(current_path_segment_);
    clearLocalPath(recovery_path_);
    clearLocalPath(planned_path_);
    
    // 重置pose_array_msg_
    pose_array_msg_.poses.clear();
    pose_array_msg_.header.stamp = ros::Time(0);
    pose_array_msg_.header.frame_id = "";
    
    // 重置所有位置索引
    min_pos_ = 0;
    avoid_min_pos_ = 0;
    recovery_min_pos_ = 0;
    
    // 重置所有标志位
    avoid_status_changed_ = false;
    need_recovery_path_ = false;
    first_loop_ = true;
    avoid_first_loop_ = false;
    repeat_mode_ = false;
    use_multi_map_ = false;
    
    // 重置位置和目标点
    current_position_ = geometry_msgs::PointStamped();
    goal_point_ = geometry_msgs::Point();
    
    // 重置避障状态
    {
      std::lock_guard<std::mutex> lock(avoid_status_mutex_);
      avoid_status_.data = 0;
      pre_avoid_status_ = 0;
    }
    
    // 重置字符串变量
    current_name_ = "";
    current_csv_path = csv_path_pre_ + "dropped_waypoints.csv";
    mower_num_.data = "";
    
    // 清空waypoint缓冲区
    waypoint_buffer.clear();
    
    // 关闭CSV文件
    if (csv_file.is_open()) {
        csv_file.close();
    }
    
    // 重新读取割草机编号
    readMowerID();
    
    std::cout << getLogTime() << "局部规划器重置成功！" << std::endl;
  }

  // 运行
  void run() {
    ros::Rate loop_rate(10);
    while (ros::ok()) {
      ros::spinOnce();
      loop_rate.sleep();
    }
  }
};

int main(int argc, char **argv) {
	setlocale(LC_ALL,"");
  ros::init(argc, argv, "read_waypoints");

  WaypointManager waypoint_manager;
  waypoint_manager.run();

  return 0;
}
