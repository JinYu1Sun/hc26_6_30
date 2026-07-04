#include <fstream>
#include <geometry_msgs/Point.h>
#include <geometry_msgs/PointStamped.h>
#include <geometry_msgs/PoseArray.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Polygon.h>
#include <iterator>
#include <math.h>
#include <ros/ros.h>
#include <sstream>
#include <std_msgs/String.h>
#include <util/LocalPath.h>
#include <util/MapPath.h>
#include <util/Position.h>
#include <vector>
#include <cstdio>  // 添加这个头文件以支持std::remove函数
#include <yaml-cpp/yaml.h>  // TODO: 多地图路径拼接功能 - 添加YAML文件读取支持 cnk 0805
using namespace std;

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

// std::string csv_path_pre_ = "/home/oem/Mower/waypoints/";
std::string csv_path_pre_ = "/home/nvidia/Mower/waypoints/";
// 函数声明
void wplCallback(const geometry_msgs::PoseArray &msg);
vector<vector<double>> LinearInterpolate(vector<double> startPnt, vector<double> endPnt, double step_size);
vector<vector<double>> velocityAssign(double step_size, double amax, double vmax, vector<vector<double>> traj);
void assembleMultiMapPaths();
void reset();

#define PI 3.1415926

ros::Publisher pub_path;
ros::Publisher pub_state;
ros::Publisher planning_ready_pub;  // 向location_map发送地图路径准备就绪信号
ros::Publisher new_waypoints_pub;   // 新增：发布拼接后的路径
ros::Publisher map_hull_pub;

vector<vector<double>> routine_;
vector<vector<double>> routine;
vector<vector<double>> traj_;
// vector<vector<double>> traj_guide_;
// vector<vector<double>> traj_global_;

// 新增：用于检测waypoints数据更新的变量
ros::Time last_waypoints_time_;
int last_waypoints_count_ = 0;
bool waypoints_processed_ = false;

// 新增：存储上一次路径的起始点，用于判断是否是新路径
// geometry_msgs::Point last_start_point_;
bool has_last_start_point_ = false;

// 新增：多地图收集超时判断相关变量
ros::Time last_map_received_time_;
double map_collection_timeout_ = 2.0;  // 5秒超时
bool collection_timeout_started_ = false;

double vmax = 1.4;
double vmin = 0.1;
double amax = 0.1;
double step_size = 0.25;
bool RS_curve_gen_flag = 0;
bool veh_pose_fisrt_sub = 0;
bool channel_path_gen_flag = 0;
std::atomic<bool> repeat_mode_{false};

// 添加地图名称相关变量
std::string current_map_name_ = "default";  // 当前地图名称

// TODO: 多地图路径拼接功能 - 多地图模式标志和路径收集 cnk 0805
bool multi_map_mode_ = false;
std::vector<geometry_msgs::PoseArray> collected_map_paths_;  // 收集的各地图路径
bool is_collecting_paths_ = false;
int expected_map_count_ = 0;
int received_map_count_ = 0;

// TODO: 多地图路径拼接功能 - 地图文件读取相关数据结构 cnk 0805
struct MapHull {
    double name;
    std::vector<geometry_msgs::Point32> brd_points; // 边界点
    std::vector<std::vector<geometry_msgs::Point32>> holes;
    std::vector<geometry_msgs::Point32> connection_path;  // 连接路径
};

std::string map_files_path_ = csv_path_pre_ + "../src/location_map/map_files/";
std::string current_map_file_;

// TODO: 多地图路径拼接功能 - YAML文件读取函数 cnk 0805
bool load_collected_link_paths(std::vector<std::vector<geometry_msgs::Pose>> &link_paths)
{
    link_paths.clear();
    
    std::string full_path = map_files_path_ + current_map_file_;
    try {
      YAML::Node yaml_data = YAML::LoadFile(full_path);
      
      if (!yaml_data["Polygons"] || !yaml_data["Polygons"]["polygon"]) {
        ROS_ERROR("Invalid YAML file format: %s", full_path.c_str());
        return false;
      }
      
      for (const auto& polygon : yaml_data["Polygons"]["polygon"]) {
        std::vector<geometry_msgs::Pose> connect_path;
        int map_name = polygon["name"].as<int>();
        
        // 读取连接路径
        if (polygon["path"]) {
          for (const auto& path_point : polygon["path"]) {
            geometry_msgs::Pose point;
            point.position.x = path_point["x"].as<double>();
            point.position.y = path_point["y"].as<double>();
            point.position.z = path_point["z"].as<double>();
            connect_path.push_back(point);
          }
        }
        else
        {
          if (map_name == 1)
          {
            std::cout << getLogTime() << "地图间没有连接路径" << std::endl;
            return false;
          }
        }      
        link_paths.push_back(connect_path);
      }
      std::cout << getLogTime() << "连接路径加载完成！" << std::endl;
      return true; 
    } catch (const std::exception& e) {
        ROS_ERROR("Error loading map file %s: %s", full_path.c_str(), e.what());
        return false;
    }
}

vector<vector<double>> LinearInterpolate(vector<double> startPnt,
                                         vector<double> endPnt,
                                         double step_size) {
  double dx = endPnt[0] - startPnt[0];
  double dy = endPnt[1] - startPnt[1];
  double yaw = atan2(dy, dx);
  if (yaw != startPnt[2]) {
    startPnt[2] = yaw;
  }
  if (step_size <= 0) {
    cout << "the step_size (sampling density) is eqaul or less than zero! "
            "return {}"
         << endl;
    return {};
  }

  vector<vector<double>> traj_interpolate;
  double dist = sqrt(dx * dx + dy * dy);
  double dis = step_size;
  vector<double> unit_vec = {dx / (dist + 1e-5), dy / (dist + 1e-5)};
  traj_interpolate.push_back({startPnt[0], startPnt[1], startPnt[2]});
  while (dis < dist) {
    double tempX = startPnt[0] + unit_vec[0] * dis;
    double tempY = startPnt[1] + unit_vec[1] * dis;
    double tempYaw = startPnt[2];
    traj_interpolate.push_back({tempX, tempY, tempYaw});

    dis += step_size;
  }

  int size = traj_interpolate.size();
  if (dist > 0 && (traj_interpolate[size - 1][0] != endPnt[0] ||
                   traj_interpolate[size - 1][1] != endPnt[1])) {
    traj_interpolate.push_back({endPnt[0], endPnt[1], endPnt[2]});
  } else if (traj_interpolate[size - 1][0] == endPnt[0] &&
             traj_interpolate[size - 1][1] == endPnt[1]) {
    traj_interpolate[size - 1][2] = endPnt[2];
  }
  return traj_interpolate;
}

vector<vector<double>> velocityAssign(double step_size, double amax,
                                      double vmax,
                                      vector<vector<double>> traj) {

  double dis = traj.size() * step_size;
  if (dis <= 0 || amax <= 0 || vmax <= 0) {
    cout << "distance and amax and vmax all should be greater than 0!" << endl;
    return traj;
  }
  traj[0][4] = 0.0;
  traj[traj.size() - 1][4] = 0.0;

  double vmid = sqrt(dis * amax);
  if (vmid > vmax) {
    double s1 = vmax * vmax / (2.0 * amax);
    double s2 = dis - s1;
    for (int i = 0; i < traj.size(); i++) {
      double si = i * step_size;
      if (si > 0 && si < s1)
        traj[i][4] = sqrt(2 * amax * si);
      if (s1 <= si && si < s2)
        traj[i][4] = vmax;
      if (si >= s2 && si < dis)
        traj[i][4] = sqrt(2 * amax * (dis - si));
    }
  }
  if (vmid <= vmax) {

    for (int i = 0; i < traj.size(); i++) {
      double si = i * step_size;
      if (si > 0 && si <= 0.5 * dis)
        traj[i][4] = sqrt(2 * amax * si);
      if (si > 0.5 * dis && si < dis)
        traj[i][4] = sqrt(2 * amax * (dis - si));
    }
  }

  return traj;
}

// 地图拼接处生成可通行边界
geometry_msgs::Polygon createRoadPolygon(
    const geometry_msgs::Pose& point1,
    const geometry_msgs::Pose& point2,
    double road_width)
{
    geometry_msgs::Polygon polygon;

    double x1 = point1.position.x;
    double y1 = point1.position.y;

    double x2 = point2.position.x;
    double y2 = point2.position.y;

    // 线段方向
    double dx = x2 - x1;
    double dy = y2 - y1;

    double length = std::hypot(dx, dy);

    if (length < 1e-6)
    {
        return polygon;
    }

    // 单位方向向量
    dx /= length;
    dy /= length;

    // 单位法向量（左侧）
    double nx = -dy;
    double ny = dx;

    double half_width = road_width * 0.5;

    geometry_msgs::Point32 p1;
    geometry_msgs::Point32 p2;
    geometry_msgs::Point32 p3;
    geometry_msgs::Point32 p4;

    // point1 左侧
    p1.x = x1 + nx * half_width;
    p1.y = y1 + ny * half_width;
    p1.z = 666.0;

    // point2 左侧
    p2.x = x2 + nx * half_width;
    p2.y = y2 + ny * half_width;
    p2.z = 666.0;

    // point2 右侧
    p3.x = x2 - nx * half_width;
    p3.y = y2 - ny * half_width;
    p3.z = 666.0;

    // point1 右侧
    p4.x = x1 - nx * half_width;
    p4.y = y1 - ny * half_width;
    p4.z = 666.0;

    polygon.points.push_back(p1);
    polygon.points.push_back(p2);
    polygon.points.push_back(p3);
    polygon.points.push_back(p4);

    return polygon;
}

// TODO: 多地图路径拼接功能 - 拼接多地图路径的函数 cnk 0805
void assembleMultiMapPaths() {
    if (collected_map_paths_.empty()) {
        ROS_WARN("No map paths collected for assembly");
        return;
    }
    
    ROS_INFO("*** STARTING PATH ASSEMBLY ***");
    ROS_INFO("Assembling %zu map paths by direct concatenation (no connection paths)", collected_map_paths_.size());
    
    // 统计收集的路径点总数
    int total_collected_points = 0;
    for (int i = 0; i < collected_map_paths_.size(); i++) {
        int points = collected_map_paths_[i].poses.size();
        total_collected_points += points;
        ROS_INFO("Map %d collected: %d waypoints", i + 1, points);
    }
    ROS_INFO("Total collected waypoints: %d", total_collected_points);

    std::vector<geometry_msgs::Pose> collected_link_paths; // 连接路径的起终点
    std::vector<std::vector<geometry_msgs::Pose>> connect_paths; // 录制的连接路径
    bool have_connect_paths = load_collected_link_paths(connect_paths);
    
    geometry_msgs::PoseArray final_path;
    final_path.header.frame_id = "world";
    final_path.header.stamp = ros::Time::now();
    
    if (!have_connect_paths)
    {
      for (int i = 0; i < collected_map_paths_.size(); i++)
      {
        ROS_INFO("x_1 %f, y_1 %f", collected_map_paths_[i].poses[0].position.x, 
          collected_map_paths_[i].poses[0].position.y);
        ROS_INFO("x_end1 %f, y_end1 %f", collected_map_paths_[i].poses[collected_map_paths_[i].poses.size() - 1].position.x, 
          collected_map_paths_[i].poses[collected_map_paths_[i].poses.size() - 1].position.y);
        ROS_INFO("x_2 %f, y_2 %f", collected_map_paths_[i].poses[1].position.x, 
          collected_map_paths_[i].poses[1].position.y);
        ROS_INFO("x_end2 %f, y_end2 %f", collected_map_paths_[i].poses[collected_map_paths_[i].poses.size() - 2].position.x, 
          collected_map_paths_[i].poses[collected_map_paths_[i].poses.size() - 2].position.y);
        // 添加地图内的路径
        ROS_INFO("Adding map %d path with %zu points", i + 1, collected_map_paths_[i].poses.size());
        
        for (int j = 0; j < collected_map_paths_[i].poses.size(); j++)
        {
          // 去除起点，只保留第一个地图的起点
          // 如果没有连接路径，起点终点都是第一个地图的起点其由于起点终点在地图外，又会自动生成一个起点和终点，所以要删除两个
          if (i != 0 && (j == 0 || j == 1))
            continue;
          // 去除终点，
          if (j == collected_map_paths_[i].poses.size() - 1 || (i != 0 && j == collected_map_paths_[i].poses.size() - 2))
            continue;
          final_path.poses.push_back(collected_map_paths_[i].poses[j]);
  
          // 第一个地图弓型终点，第二个地图弓型起点，第二个地图弓型终点，第三个地图弓型起点......
          if (i == 0 && j == collected_map_paths_[i].poses.size() - 2)
            collected_link_paths.push_back(collected_map_paths_[i].poses[j]);
          if (i != 0 && (j == 2 || j == collected_map_paths_[i].poses.size() - 3))
          {
            collected_link_paths.push_back(collected_map_paths_[i].poses[j]);
          }
        }
      }
      // 最后回到起点，调试测试的时候可以让它最后回到起点，免得自己找回去还不一定在地图内
      // final_path.poses.push_back(collected_map_paths_[0].poses[0]);
      // collected_link_paths.push_back(collected_map_paths_[0].poses[0]);
    }
    else
    {
      for (int i = 0; i < collected_map_paths_.size(); i++)
      {
        ROS_INFO("Adding map %d path with %zu points", i + 1, collected_map_paths_[i].poses.size());
        for (int j = 0; j < collected_map_paths_[i].poses.size(); j++)
          final_path.poses.push_back(collected_map_paths_[i].poses[j]);

        for (int j = 0; j < connect_paths[i].size(); j++)
        {
          final_path.poses.push_back(connect_paths[i][j]);
          if (j == 0 || j == connect_paths[i].size() - 1)
            collected_link_paths.push_back(connect_paths[i][j]);
        }
      }
      final_path.poses.pop_back(); // 删除最后一个地图的最后一个点（因为这个点是最后一个地图的起点）
    }
    ROS_INFO("Final assembled path has %zu total points", final_path.poses.size());
		if (collected_link_paths.size() > 1)
		{
			// 发布连接路径的边界区域
			for (int i = 0; i <= collected_link_paths.size() - 2; i+=2)
			{
				geometry_msgs::Pose point1 = collected_link_paths[i];
				geometry_msgs::Pose point2 = collected_link_paths[i + 1];
				
				geometry_msgs::Polygon link_polygon = createRoadPolygon(point1, point2, 2.5);
				map_hull_pub.publish(link_polygon);
				// std::cout << "point1: " << point1.position.x << " " << point1.position.y << " " << std::endl;
				// std::cout << "point2: " << point2.position.x << " " << point2.position.y << " " << std::endl;
				ros::Duration(0.1).sleep();
			}
			std::cout << getLogTime() << "发布地图间的连接区域边界" << std::endl;
		}
    
    // 发布拼接后的路径到 /Newwaypoints_list 话题
    if (repeat_mode_.load())
    {
      std::cout << getLogTime() << "断点复割，无需全局规划" << std::endl;
      return;
    }
    final_path.header.stamp = ros::Time::now();
    final_path.header.frame_id = "world";
    new_waypoints_pub.publish(final_path);
		std::cout << getLogTime() << "多地图路径已发布，路径点数: " << final_path.poses.size() << std::endl;
    
    // 停止收集模式，设置为处理拼接后的完整路径
    is_collecting_paths_ = false;
    
    // 重置状态，准备处理完整路径
    collected_map_paths_.clear();
    received_map_count_ = 0;
    
    // 现在直接处理拼接后的完整路径，不调用wplCallback避免覆盖数据
    ROS_INFO("Starting 5D processing of assembled path with %zu points", final_path.poses.size());
    
    // 直接进行5维路径处理（复制wplCallback中的主要逻辑）
    routine_.clear();
    routine.clear();
    traj_.clear();
    
    int num = final_path.poses.size();
    if (num <= 1) {
      ROS_WARN("Assembled path too short: %d points", num);
      return;
    }
    
    // 转换为内部格式并进行处理
    for (int i = 0; i < num; i++) {
      double pntx = final_path.poses[i].position.x;
      double pnty = final_path.poses[i].position.y;
      routine_.push_back({pntx, pnty});
    }
    
    ROS_INFO("Converted assembled path to internal format, proceeding with 5D processing...");
    // 继续使用原有的处理逻辑...

  // 生成航向角信息,储存在routine中
  int routine_id = 0;
  for (int i = 0; i < routine_.size(); i++) {
    if (i < routine_.size() - 1) {
      if (abs(routine_[i][0] - routine_[i + 1][0]) < 0.05 &&
          abs(routine_[i][1] - routine_[i + 1][1]) < 0.05)
        continue;
      else {
        double tempx = routine_[i][0];
        double tempy = routine_[i][1];
        double tempyaw = atan2(routine_[i + 1][1] - routine_[i][1],
                               routine_[i + 1][0] - routine_[i][0]);

        routine.push_back({tempx, tempy, tempyaw});
        ++routine_id;
      }
    } else {
      routine.push_back(
          {routine_[i][0], routine_[i][1], routine[routine_id - 1][2]});
    }
  }

  // 调用LinearInterpolate针对两点之间进行线性插值
  vector<double> traj_pnt = {0.0, 0.0, 0.0, 0.0,
                             0.0}; // define the format of the trajectory point;
  int cnt = 0;
  for (int i = 0; i < routine.size() - 1; i++) {
    double s_x = routine[i][0];
    double s_y = routine[i][1];
    double s_yaw = routine[i][2];
    double g_x = routine[i + 1][0];
    double g_y = routine[i + 1][1];
    double g_yaw = routine[i + 1][2];

    vector<vector<double>> traj_interpolate =
        LinearInterpolate({s_x, s_y, s_yaw}, {g_x, g_y, g_yaw}, step_size);

    for (int j = 0; j < traj_interpolate.size() - 1; j++) {
      traj_pnt[0] = traj_interpolate[j][0];
      traj_pnt[1] = traj_interpolate[j][1];
      traj_pnt[2] = traj_interpolate[j][2];

      if (j == 0) {
        traj_pnt[3] = 2;
      } else {
        traj_pnt[3] = 1;
      }
      traj_.push_back(traj_pnt);
    }
  }
  // 调整第一个点和最后一个点的值
  traj_[0][3] = 1;
  if (traj_[traj_.size() - 1][0] != routine[routine.size() - 1][0] &&
      traj_[traj_.size() - 1][1] != routine[routine.size() - 1][1]) {
    traj_.push_back({routine[routine.size() - 1][0],
                     routine[routine.size() - 1][1],
                     routine[routine.size() - 1][2], 1, 0});
  }

  /*将traj_中路径点的yaw限制在-pi到pi的范围之内*/
  for (int i = 0; i < traj_.size(); i++) {
    if (traj_[i][2] > PI) {
      traj_[i][2] = traj_[i][2] - 2 * PI;
    } else if (traj_[i][2] < -PI) {
      traj_[i][2] = traj_[i][2] + 2 * PI;
    } else {
      traj_[i][2] = traj_[i][2];
    }
  }

  /*find the gear changing points and save the index*/
  vector<double> gear_change_pnt_ind;
  for (int i = 0; i < traj_.size(); i++) {
    if (traj_[i][3] == 2) {
      gear_change_pnt_ind.push_back(i);
    }
  }

  /*assign the velocity to the path section by section*/
  int num_gcp = gear_change_pnt_ind.size();
  if (num_gcp == 0) {
    traj_ = velocityAssign(step_size, amax, vmax, traj_);
  }
  if (num_gcp >= 1) {
    ROS_WARN("the have %.2d turn point", num_gcp);
    for (int i = 0; i < gear_change_pnt_ind.size(); i++) {
      if (i == 0) {
        vector<vector<double>> subtraj(traj_.begin(),
                                       traj_.begin() + gear_change_pnt_ind[i]);
        subtraj = velocityAssign(step_size, amax, vmax, subtraj);
        for (int j = 0; j < subtraj.size(); j++) {
          traj_[j][4] = subtraj[j][4];
        }
      }
      if (i >= 1 && i <= num_gcp - 1) {
        vector<vector<double>> subtraj(traj_.begin() +
                                           gear_change_pnt_ind[i - 1],
                                       traj_.begin() + gear_change_pnt_ind[i]);
        subtraj = velocityAssign(step_size, amax, vmax, subtraj);
        for (int j = 0; j < subtraj.size(); j++) {
          traj_[j + gear_change_pnt_ind[i - 1]][4] = subtraj[j][4];
        }
      }
    }
    if (gear_change_pnt_ind[num_gcp - 1] != traj_.size() - 1) {
      vector<vector<double>> subtraj(
          traj_.begin() + gear_change_pnt_ind[num_gcp - 1], traj_.end() - 1);
      subtraj = velocityAssign(step_size, amax, vmax, subtraj);
      for (int j = 0; j < subtraj.size(); j++) {
        traj_[j + gear_change_pnt_ind[num_gcp - 1]][4] = subtraj[j][4];
      }
    }
  }
  for (int i = 0; i < traj_.size(); i++) // TODO
  {
    if (traj_[i][4] < 0.2 && traj_[i][4] > vmin) {
      if (i != traj_.size() - 1) {
        traj_[i][4] = vmin;
      }
    }
  }

  /* ofstream outFile1;
  std::string csv_path1;
  csv_path1 = csv_path_pre_ + current_map_name_ + "test.csv";
  if(std::remove(csv_path1.c_str())==0)
  {
    ROS_INFO("CSV has been removed");
  }
  outFile1.open(csv_path1, std::ios::out);
  if (!outFile1) {
    cerr << "can not open the file: " << csv_path1 << endl;
    return;
  }
  for (int i = 0; i < traj_.size(); i++) {
    for (int j = 0; j < traj_[i].size(); j++) {
      outFile1 << traj_[i][j] << ",";
    }
    outFile1 << endl;
  }
  outFile1.close(); */

  RS_curve_gen_flag = 1;
  veh_pose_fisrt_sub = 1;
  
  cout << "The main part of the global trajectory have been generated!" << endl;
  cout << "waiting for the path from vehicle position to the first point of "
          "the globtal trajectory..."
       << endl;
       
  // 新增：标记waypoints已处理，并在多地图模式下发送反馈
  waypoints_processed_ = true;
  
  // 注意：多地图模式的反馈已经在收集阶段发送，这里只处理单地图模式
  // 如果是单地图模式或非收集模式，发送处理完成信号给location_map
  if (!multi_map_mode_) {
    // 单地图模式下发送反馈
    ROS_INFO("Single map mode: waypoints processed");
  }
  
  // 发送所有地图处理完成的信号
  std_msgs::String ready_msg;
  ready_msg.data = "all_maps_processed";
  planning_ready_pub.publish(ready_msg);
  ROS_INFO("Sent all_maps_processed signal to location_map");
  ROS_INFO("Multi-map path assembly and processing completed successfully!");
}

/* 
输入：路径点
单地图模式：接收路径点 → 线性插值 → 速度规划 → 档位控制 → 发布轨迹
输出：
  1.traj_{x, y, yaw, gear, speed}和轨迹的csv文件
  2.RS_curve_gen_flag = 1  RS曲线生成标志
  3.veh_pose_fisrt_sub = 1 车辆位置首次订阅标志
  4.waypoints_processed_ = true 输入的路径点已经处理过的标志
多地图模式：激活多地图模式 → 收集各地图路径 → 超时检查 → 路径拼接 → 统一处理 → 发布轨迹
输出：
  1.collected_map_paths_  收集到的所有路径点
  2.waypoints_processed_ = true 输入的路径点已经处理过的标志
  3.通过话题"/planning_ready"发布一个收集完成的信号
*/
void wplCallback(const geometry_msgs::PoseArray &msg) {
  // 新增：检测是否为新的waypoints数据
  ros::Time current_time = ros::Time::now();
  int current_waypoints_count = msg.poses.size();
  
  // 比较起点距离第二个点近还是倒数第二个点近
  geometry_msgs::PoseArray current_path; // 当前路径数据
  if (current_waypoints_count > 3)
  {
    const auto& start_point = msg.poses[0].position;
    const auto& second_point = msg.poses[1].position;
    const auto& second_last_point = msg.poses[current_waypoints_count - 2].position;
    ROS_INFO("start_point: (%.2f, %.2f), second_point: (%.2f, %.2f), second_last_point: (%.2f, %.2f)", start_point.x, start_point.y, second_point.x, second_point.y, second_last_point.x, second_last_point.y);
    
    double dist_to_second = sqrt(pow(start_point.x - second_point.x, 2) + 
                                 pow(start_point.y - second_point.y, 2));
    double dist_to_second_last = sqrt(pow(start_point.x - second_last_point.x, 2) + 
                                      pow(start_point.y - second_last_point.y, 2));
    
    if (dist_to_second < dist_to_second_last) 
    {
      current_path = msg; // 不需要调整路径顺序
    } 
    else
    {
      // 路径顺序是倒的，进行调整
      geometry_msgs::PoseArray reversed_path;
      reversed_path.header = msg.header;
      reversed_path.poses.push_back(msg.poses[0]); // 添加起点
      // 中间点倒序
      for (int i = current_waypoints_count - 2; i >= 1; i--) {
        reversed_path.poses.push_back(msg.poses[i]);
      }
      reversed_path.poses.push_back(msg.poses[current_waypoints_count - 1]); // 添加终点
      current_path = reversed_path;
      std::cout << getLogTime() << "路径顺序已调整（倒序）" << std::endl;
    }
  }
  
  // 打印每次收到的路径点信息
  ROS_INFO("=== Received waypoints callback ===");
  ROS_INFO("Current waypoints count: %d", current_waypoints_count);
  ROS_INFO("Last waypoints count: %d", last_waypoints_count_);
  ROS_INFO("Multi-map mode: %s, Collecting: %s", 
           multi_map_mode_ ? "YES" : "NO", 
           is_collecting_paths_ ? "YES" : "NO");
           
  // 打印第一个和最后一个点的坐标来区分路径
  /* if (current_waypoints_count > 0) {
    const auto& first_point = current_path.poses[0].position;
    const auto& last_point = current_path.poses[current_waypoints_count - 1].position;
    ROS_INFO("First point: (%.6f, %.6f, %.6f)", first_point.x, first_point.y, first_point.z);
    ROS_INFO("Last point:  (%.6f, %.6f, %.6f)", last_point.x, last_point.y, last_point.z);

    const auto& first2_point = current_path.poses[1].position;
    const auto& last2_point = current_path.poses[current_waypoints_count - 2].position;
    ROS_INFO("First2 point: (%.6f, %.6f, %.6f)", first2_point.x, first2_point.y, first2_point.z);
    ROS_INFO("Last2 point:  (%.6f, %.6f, %.6f)", last2_point.x, last2_point.y, last2_point.z);
  } */
  
  // 检查时间戳和数据点数量是否有变化
  bool is_new_data = false;
  if (current_waypoints_count != last_waypoints_count_ || 
      (current_time - last_waypoints_time_).toSec() > 1.0) {  // 超过1秒认为是新数据
    is_new_data = true;
    last_waypoints_time_ = current_time;
    last_waypoints_count_ = current_waypoints_count;
    waypoints_processed_ = false;
  }
  
  // 新增：检查起始点是否不同（如果有数据的话）
  // if (current_waypoints_count > 0) {
  //   const auto& current_start_point = current_path.poses[0].position;
    
  //   if (!has_last_start_point_) {
  //     // 第一次收到数据
  //     is_new_data = true;
  //     last_start_point_ = current_start_point;
  //     has_last_start_point_ = true;
  //     ROS_INFO("First waypoints data received, treating as new");
  //   } else {
  //     // 比较起始点坐标（使用小的容差）
  //     double distance = sqrt(pow(current_start_point.x - last_start_point_.x, 2) + 
  //                           pow(current_start_point.y - last_start_point_.y, 2));
  //     if (distance > 0.1) {  // 起始点距离超过0.1米认为是新路径
  //       is_new_data = true;
  //       last_start_point_ = current_start_point;
  //       ROS_INFO("Start point changed (distance: %.3f), treating as new data", distance);
  //     }
  //   }
  // }
  
  // 在多地图收集模式下，总是认为是新数据
  if (multi_map_mode_ && is_collecting_paths_) {
    is_new_data = true;
    waypoints_processed_ = false;
    ROS_INFO("Multi-map collection mode: treating as new data regardless of count");
  }
  
  // 如果不是新数据且已经处理过，直接返回
  if (!is_new_data && waypoints_processed_) {
    ROS_DEBUG("Ignoring duplicate waypoints data with %d points", current_waypoints_count);
    return;
  }
  
  ROS_INFO("Processing new waypoints data with %d points", current_waypoints_count);
  
  // TODO: 多地图路径拼接功能 - 多地图模式下的路径收集处理 cnk 0805
  if (multi_map_mode_ && is_collecting_paths_)
  {
    // 在多地图模式下，收集各个地图的路径
    collected_map_paths_.push_back(current_path);
    received_map_count_++;
    
    // 更新最后收到地图的时间
    last_map_received_time_ = ros::Time::now();
    collection_timeout_started_ = true;
    
    ROS_INFO("*** MULTI-MAP COLLECTION ***");
    ROS_INFO("Collected path from map %d with %zu waypoints", 
             received_map_count_, current_path.poses.size());
    ROS_INFO("Total maps collected so far: %d", received_map_count_);
    ROS_INFO("Collection timeout timer reset");
    
    // 打印当前地图的第一个和最后一个点
    if (current_path.poses.size() > 0) {
      const auto& first_point = current_path.poses[0].position;
      const auto& last_point = current_path.poses[current_path.poses.size() - 1].position;
      ROS_INFO("Map %d - First: (%.6f, %.6f), Last: (%.6f, %.6f)", 
               received_map_count_, first_point.x, first_point.y, last_point.x, last_point.y);
    }
    
    // 打印已收集的所有地图路径统计
    ROS_INFO("--- Summary of collected maps ---");
    int total_points = 0;
    for (int i = 0; i < collected_map_paths_.size(); i++) {
      int points = collected_map_paths_[i].poses.size();
      total_points += points;
      if (points > 0) {
        const auto& first = collected_map_paths_[i].poses[0].position;
        const auto& last = collected_map_paths_[i].poses[points-1].position;
        ROS_INFO("  Map %d: %d points, First:(%.3f,%.3f) Last:(%.3f,%.3f)", 
                 i + 1, points, first.x, first.y, last.x, last.y);
      }
    }
    ROS_INFO("Total waypoints collected: %d points", total_points);
             
    // 标记当前waypoints已处理，并发送反馈信号
    waypoints_processed_ = true;
    std_msgs::String ready_msg;
    ready_msg.data = "waypoints_received";
    planning_ready_pub.publish(ready_msg);
    ROS_INFO("Sent waypoints_received signal to location_map for map %d", received_map_count_);
    // 超时检查将在main函数的主循环中进行
    ROS_INFO("Waiting for next map or timeout (%.1f seconds)...", map_collection_timeout_);
    return;
  }
	// 单地图模式下发布路径给UI显示
  if (repeat_mode_.load())
  {
    std::cout << getLogTime() << "断点复割，无需全局规划" << std::endl;
    return;
  }
  current_path.poses.pop_back(); // 删除最后一个点
	new_waypoints_pub.publish(current_path);
	std::cout << getLogTime() << "单地图路径已发布, 路径点数" << current_waypoints_count << std::endl;
  
  routine_.clear(); // 不带航向角信息
  routine.clear();  // 带航向角信息
  traj_.clear();    // 最终轨迹
  
  for (int i = 0; i < current_path.poses.size(); i++) {
    double pntx = current_path.poses[i].position.x;
    double pnty = current_path.poses[i].position.y;

    routine_.push_back({pntx, pnty});
  }

  // 生成航向角信息,储存在routine中
  int routine_id = 0;
  for (int i = 0; i < routine_.size(); i++) {
    if (i < routine_.size() - 1) {
      if (abs(routine_[i][0] - routine_[i + 1][0]) < 0.05 &&
          abs(routine_[i][1] - routine_[i + 1][1]) < 0.05)
        continue;
      else {
        double tempx = routine_[i][0];
        double tempy = routine_[i][1];
        double tempyaw = atan2(routine_[i + 1][1] - routine_[i][1],
                               routine_[i + 1][0] - routine_[i][0]);

        routine.push_back({tempx, tempy, tempyaw});
        ++routine_id;
      }
    } else {
      routine.push_back(
          {routine_[i][0], routine_[i][1], routine[routine_id - 1][2]});
    }
  }

  // 调用LinearInterpolate针对两点之间进行线性插值
  vector<double> traj_pnt = {0.0, 0.0, 0.0, 0.0,
                             0.0}; // define the format of the trajectory point;
  for (int i = 0; i < routine.size() - 1; i++) {
    double s_x = routine[i][0];
    double s_y = routine[i][1];
    double s_yaw = routine[i][2];
    double g_x = routine[i + 1][0];
    double g_y = routine[i + 1][1];
    double g_yaw = routine[i + 1][2];

    vector<vector<double>> traj_interpolate =
        LinearInterpolate({s_x, s_y, s_yaw}, {g_x, g_y, g_yaw}, step_size);

    for (int j = 0; j < traj_interpolate.size() - 1; j++) {
      traj_pnt[0] = traj_interpolate[j][0];
      traj_pnt[1] = traj_interpolate[j][1];
      traj_pnt[2] = traj_interpolate[j][2];

      // 将每段插值的第一个插值点标为2，每段插值的第一个点通常是上一段与下一段航向不连续的接点，车辆可能需要原地转向或减速到零再调整朝向
      if (j == 0) {
        traj_pnt[3] = 2;
      } else {
        traj_pnt[3] = 1;
      }
      traj_.push_back(traj_pnt);
    }
  }
  // 调整第一个点和最后一个点的值
  traj_[0][3] = 1; // 把轨迹起点设置为正常档位
  if (traj_[traj_.size() - 1][0] != routine[routine.size() - 1][0] &&
      traj_[traj_.size() - 1][1] != routine[routine.size() - 1][1]) {
    traj_.push_back({routine[routine.size() - 1][0],
                     routine[routine.size() - 1][1],
                     routine[routine.size() - 1][2], 1, 0});
  } // 轨迹终点设置为正常行驶档位，速度设置为0

  // 将traj_中路径点的yaw限制在-pi到pi的范围之内
  for (int i = 0; i < traj_.size(); i++) {
    if (traj_[i][2] > PI) {
      traj_[i][2] = traj_[i][2] - 2 * PI;
    } else if (traj_[i][2] < -PI) {
      traj_[i][2] = traj_[i][2] + 2 * PI;
    } else {
      traj_[i][2] = traj_[i][2];
    }
  }

  // 找到档位为2的点并记录id
  vector<double> gear_change_pnt_ind;
  for (int i = 0; i < traj_.size(); i++) {
    if (traj_[i][3] == 2) {
      gear_change_pnt_ind.push_back(i);
    }
  }

  /*assign the velocity to the path section by section*/
  int num_gcp = gear_change_pnt_ind.size();
  if (num_gcp == 0) {
    traj_ = velocityAssign(step_size, amax, vmax, traj_);
  }
  if (num_gcp >= 1) {
    ROS_WARN("the have %.2d turn point", num_gcp);
    for (int i = 0; i < gear_change_pnt_ind.size(); i++) {
      if (i == 0) {
        vector<vector<double>> subtraj(traj_.begin(),
                                       traj_.begin() + gear_change_pnt_ind[i]);
        subtraj = velocityAssign(step_size, amax, vmax, subtraj);
        for (int j = 0; j < subtraj.size(); j++) {
          traj_[j][4] = subtraj[j][4];
        }
      }
      if (i >= 1 && i <= num_gcp - 1) {
        vector<vector<double>> subtraj(traj_.begin() +
                                           gear_change_pnt_ind[i - 1],
                                       traj_.begin() + gear_change_pnt_ind[i]);
        subtraj = velocityAssign(step_size, amax, vmax, subtraj);
        for (int j = 0; j < subtraj.size(); j++) {
          traj_[j + gear_change_pnt_ind[i - 1]][4] = subtraj[j][4];
        }
      }
    }
    if (gear_change_pnt_ind[num_gcp - 1] != traj_.size() - 1) {
      vector<vector<double>> subtraj(
          traj_.begin() + gear_change_pnt_ind[num_gcp - 1], traj_.end() - 1);
      subtraj = velocityAssign(step_size, amax, vmax, subtraj);
      for (int j = 0; j < subtraj.size(); j++) {
        traj_[j + gear_change_pnt_ind[num_gcp - 1]][4] = subtraj[j][4];
      }
    }
  }
  // (vmin, 0.2) 区间是低速点，会调整为最小速度，不清楚意义在哪里？**********************
  for (int i = 0; i < traj_.size(); i++) // TODO
  {
    if (traj_[i][4] < 0.2 && traj_[i][4] > vmin) {
      if (i != traj_.size() - 1) {
        traj_[i][4] = vmin;
      }
    }
  }

  RS_curve_gen_flag = 1;
  veh_pose_fisrt_sub = 1;
  
  ROS_INFO("%s已收到全覆盖弓型轨迹并生成了全局轨迹，等待小车位置确认全局轨迹起点......", getLogTime().c_str());
       
  // 新增：标记waypoints已处理，并在多地图模式下发送反馈
  waypoints_processed_ = true;
  
  // 注意：多地图模式的反馈已经在收集阶段发送，这里只处理单地图模式
  // 如果是单地图模式或非收集模式，发送处理完成信号给location_map
  if (!multi_map_mode_) {
    // 单地图模式下发送反馈
    ROS_INFO("Single map mode: waypoints processed");
  }
}


// 和wplCallback()函数类似，只不过输入换成了录制的点，输出同样是traj_轨迹
void ChannelPathSubCallback(const util::MapPath &msg) {
  vector<double> traj_pnt = {0.0, 0.0, 0.0, 0.0, 0.0};
  traj_.clear();
  std::cout << "now in callback  mapPath" << std::endl;
  // 接收录制的点，存入traj_全局变量中，需要给定gear以及velocity
  for (int i = 0; i < msg.path_points.size(); i++) {
    traj_pnt[0] = msg.path_points[i].x;
    traj_pnt[1] = msg.path_points[i].y;
    traj_pnt[2] = msg.path_points[i].z;
    traj_.push_back(traj_pnt);
  }

  /*the recorded path need to decide gear value */
  for (int i = 0; i < traj_.size(); i++) {
    if (traj_.size() <= 2) {
      ROS_WARN("the recorded path is too short!"); // ERROR_CODE?2024-12-16
      return;
    }
    // 在录制路径，采用原地转向的策略，只要当前点和前一个点（第0个点除外）角度不同，则当前点就设为转向点
    if (i < 2)
      traj_[i][3] = 1;
    if (i >= 2) {
      if (traj_[i][2] != traj_[i - 1][2]) { // 不等号是不是写得过于绝对了**********************
        traj_[i][3] = 2;
      } else {
        traj_[i][3] = 1;
      }
    }
  }
  /*find the gear changing points and save the index*/
  vector<double> gear_change_pnt_ind;
  /* for (int i = 0; i < traj_.size(); i++) {
    if (i > 0 && i < traj_.size() - 1) {
      if (traj_[i][3] == 2)
        gear_change_pnt_ind.push_back(i);
    }  
  } */
  // **********************优化写法**********************
  for (int i = 1; i < traj_.size() - 1; ++i){
    if (traj_[i][3] == 2)
      gear_change_pnt_ind.push_back(i);
  }

  /*assign the velocity to the path section by section */
  int num_gcp = gear_change_pnt_ind.size();
  if (num_gcp == 0)
    traj_ = velocityAssign(step_size, amax, vmax, traj_);

  if (num_gcp > 0) {
    for (int i = 0; i < gear_change_pnt_ind.size(); i++) {
      if (i == 0) {
        vector<vector<double>> subtraj(traj_.begin(),
                                       traj_.begin() + gear_change_pnt_ind[i]);
        subtraj = velocityAssign(step_size, amax, vmax, subtraj);
        for (int j = 0; j < subtraj.size(); j++) {
          traj_[j][4] = subtraj[j][4];
        }
      }
      if (i >= 1 && i <= num_gcp - 1) {
        vector<vector<double>> subtraj(traj_.begin() +
                                           gear_change_pnt_ind[i - 1],
                                       traj_.begin() + gear_change_pnt_ind[i]);
        subtraj = velocityAssign(step_size, amax, vmax, subtraj);
        for (int j = 0; j < subtraj.size(); j++) {
          traj_[j + gear_change_pnt_ind[i - 1]][4] = subtraj[j][4];
        }
      }
    }
    if (gear_change_pnt_ind[num_gcp - 1] != traj_.size() - 1) {
      vector<vector<double>> subtraj(
          traj_.begin() + gear_change_pnt_ind[num_gcp - 1], traj_.end() - 1);
      subtraj = velocityAssign(step_size, amax, vmax, subtraj);
      for (int j = 0; j < subtraj.size(); j++) {
        traj_[j + gear_change_pnt_ind[num_gcp - 1]][4] = subtraj[j][4];
      }
    }
  }
  for (int i = 0; i < traj_.size(); i++) {
    if (traj_[i][4] <= 0.2 && traj_[i][4] > vmin) {
      if (i != traj_.size() - 1) {
        traj_[i][4] = vmin;
      }
    }
  }

  channel_path_gen_flag = 1;
  veh_pose_fisrt_sub = 1;
}

/* 
输入：
  1.首次订阅到小车实时位置
  2.wplCallback()函数输出的轨迹traj_
输出：
  pub_path.publish(global_path);最终的全局轨迹，主要对变档前的速度了调整，删除了一些冗余的点
  veh_pose_fisrt_sub = 0;
  RS_curve_gen_flag = 0;
  channel_path_gen_flag = 0;
*/
void vehPoseFisrtSubCallback(const util::Position &Loc_msg) {
  vector<vector<double>> traj_guide_;   // 小车位置到弓型轨迹起点的插值轨迹
  vector<vector<double>> traj_global_;  // 全局轨迹

  // traj_guide_.clear();
  // traj_global_.clear();
  bool validFlag = 1;

  if (traj_.empty()) {
	  ROS_INFO("%s等待全覆盖弓型轨迹......", getLogTime().c_str());
    return;
  }

  if (veh_pose_fisrt_sub && (RS_curve_gen_flag || channel_path_gen_flag)) 
  {
    double veh_pose_x = Loc_msg.position_x;
    double veh_pose_y = Loc_msg.position_y;
    double veh_pose_yaw = Loc_msg.yaw; // 本地特别定义，与标准消息格式不同

    // double distance =
    //     sqrt((veh_pose_x - traj_[0][0]) * (veh_pose_x - traj_[0][0]) +
    //          (veh_pose_y - traj_[0][1]) * (veh_pose_y - traj_[0][1]));
    // if (distance - 1.0 < 1e-3 || abs(traj_[0][2] - veh_pose_yaw) < PI / 2.0)
    // {
    //     traj_guide_ = {};
    //     cout << "distance is too small ,no neccesary to generate a path!" <<
    //     endl; validFlag = 0;
    // }

    vector<vector<double>> traj_guide =
        LinearInterpolate({veh_pose_x, veh_pose_y, veh_pose_yaw},
                          {traj_[0][0], traj_[0][1], traj_[0][2]}, step_size);
    if (traj_guide.empty() || traj_guide.size() < 3) {
      cout << "no link path is generated! OR Too FEW points" << endl;
      validFlag = 0;
    }

    // 小车位置和轨迹起点相距太远，中间再插入一条轨迹
    if (validFlag) {
      vector<double> traj_pnt2 = {
          0.0, 0.0, 0.0, 0.0,
          0.0}; // define the format of the trajectory point;
      for (int i = 0; i < traj_guide.size() - 1; i++) {
        traj_pnt2[0] = traj_guide[i][0];
        traj_pnt2[1] = traj_guide[i][1];
        traj_pnt2[2] = traj_guide[i][2];

        // 如果插值轨迹起点的角度不等于小车起点的角度，则将轨迹起点的档位设置为2，表示小车到达该轨迹起点之前不能加速，要先调整好角度
        if (i == 0 && traj_guide[i][2] != veh_pose_yaw)
          traj_pnt2[3] = 2;
        else
          traj_pnt2[3] = 1;

        traj_guide_.push_back(traj_pnt2);
      }
      // std::cout << "traj_guide[traj_guide.size() - 1][0] " <<
      // traj_guide[traj_guide.size() - 1][0] << std::endl; std::cout <<
      // "traj_guide[traj_guide.size() - 1][1] " << traj_guide[traj_guide.size()
      // - 1][1] << std::endl;

      // 如果插值轨迹的倒数第二个点和原轨迹的起点角度不同，则将原轨迹起点的档位设置为2，表示小车到达该轨迹起点之前不能加速，要先调整好角度
      if (traj_guide[traj_guide.size() - 2][2] != traj_[0][2]) // 不等号是不是写得过于绝对了,影响运动的流畅性********************************************
        traj_[0][3] = 2;

      /*调整角度值-pi到pi之间*/
      for (int i = 0; i < traj_guide_.size(); i++) {
        if (traj_guide_[i][2] > PI) {
          traj_guide_[i][2] = traj_guide_[i][2] - 2 * PI;
        } else if (traj_guide_[i][2] < -PI) {
          traj_guide_[i][2] = traj_guide_[i][2] + 2 * PI;
        } else {
          traj_guide_[i][2] = traj_guide_[i][2];
        }
      }

      /*assign the velocity to the path*/
      traj_guide_ = velocityAssign(step_size, amax, vmax, traj_guide_);

      for (int i = 0; i < traj_guide_.size(); i++) {
        if (traj_guide_[i][4] <= 0.2) {
          traj_guide_[i][4] = vmin;
        }
      }
    }
    /********************************************/
    else {
      traj_guide_ = {};
    }

    traj_global_.insert(traj_global_.end(), traj_guide_.begin(),
                        traj_guide_.end());
    traj_global_.insert(traj_global_.end(), traj_.begin(), traj_.end());
    
    //  查找traj_global_中所有变档点(gear=2)的位置
    vector<int> global_gear_change_indices;
    for (int i = 0; i < traj_global_.size(); i++) {
      if (traj_global_[i][3] == 2) {
        global_gear_change_indices.push_back(i);
      }
    }
    for (int i = 0; i < global_gear_change_indices.size(); i++) {
      // 如果两个变档点之间的点数小于2，则把前一个变档点设为普通点
      if (i < global_gear_change_indices.size() - 1) {
        int curr_idx = global_gear_change_indices[i];
        int next_idx = global_gear_change_indices[i + 1];
        // if (next_idx - curr_idx < 3) {
        //   traj_global_[curr_idx][3] = 1; // 把前一个变档点设为普通点
        //   continue; // 跳过当前变档点的速度调整
        // }
        // 把变档点前5个点速度调整为0.25
        for (int j = max(1, curr_idx - 5); j < curr_idx; j++) {
          if (traj_global_[j][3] == 1) // 只调整普通点的速度，变档点的速度保持不变
            traj_global_[j][4] = 0.25;
        }
        // 把变档点速度设置为0
        traj_global_[curr_idx][4] = 0;
      }
    }
    
    // 在最终发布前确认一下第一个点的档位值
    traj_global_[0][3] = 2;
    traj_global_[0][4] = 0;

    /*填充全局路径traj_，发布*/
    /*注意：每次发布路径，我们从新定义一个消息类型*/
    util::LocalPath global_path;
    for (int i = 0; i < traj_global_.size(); i++) {
      global_path.header.stamp = ros::Time::now();
      global_path.header.frame_id = "world";
      global_path.pathtype = 0;
      global_path.x.push_back(traj_global_[i][0]);
      global_path.y.push_back(traj_global_[i][1]);
      global_path.heading.push_back(traj_global_[i][2]);
      global_path.gear.push_back(traj_global_[i][3]);
      global_path.speed.push_back(traj_global_[i][4]);
    }
    pub_path.publish(global_path);
    veh_pose_fisrt_sub = 0;
    RS_curve_gen_flag = 0;
    channel_path_gen_flag = 0;

	  ROS_INFO("%s起点->弓型全覆盖轨迹->终点的全局轨迹生成完毕！", getLogTime().c_str());

    ofstream outFile1;
    std::string csv_path1;
    csv_path1 = csv_path_pre_ + current_map_name_ + ".csv";
    if(std::remove(csv_path1.c_str())==0)
    {
      ROS_INFO("CSV has been removed: %s", csv_path1.c_str());
    }
    outFile1.open(csv_path1, std::ios::out);
    if (!outFile1) {
      cerr << "can not open the file: " << csv_path1 << endl;
      return;
    }
    for (int i = 0; i < global_path.x.size(); i++) {
      outFile1 << global_path.x[i] << "," << global_path.y[i] << "," << global_path.heading[i] << "," 
      << static_cast<int>(global_path.gear[i]) << "," << global_path.speed[i] << endl;
    }
    outFile1.close();
    ROS_INFO("CSV file has been written: %s", csv_path1.c_str());
  } else {
    return;
  }
}

void SignalCallBack(const std_msgs::String &string) {
  const std::string &data = string.data;

  // 如果有 '/'，提取 prefix 和 name
  std::string prefix, name;
  std::size_t slash_pos = data.find('/');
  if (slash_pos != std::string::npos) {
      prefix = data.substr(0, slash_pos);
      name = data.substr(slash_pos + 1);
  }
  
  // TODO: 多地图路径拼接功能 - 检测多地图模式信号 cnk 0805
  if (data == "multi_map") {
      multi_map_mode_ = true;
      is_collecting_paths_ = true;
      repeat_mode_.store(false);
      collected_map_paths_.clear();
      received_map_count_ = 0;
      ROS_INFO("Multi-map mode activated, starting path collection");
  }
  else if (prefix == "use_map") {
      current_map_name_ = name;
      current_map_file_ = name + ".yaml";
  }
  // 如果是 delete_name/map01
  else if (prefix == "delete_name") {
      std::string csv_path = csv_path_pre_ + name + ".csv";
      
      if (std::remove(csv_path.c_str()) == 0) {
          ROS_INFO("Deleted CSV file: %s", csv_path.c_str());
      } else {
          ROS_ERROR("Failed to delete CSV file: %s", csv_path.c_str());
      }
  }
  else if (data == "test_map")
  {
    repeat_mode_.store(true);
    std::cout << getLogTime() << "断点复割，不发布新规划的路径" << std::endl;
  }
  else if (data == "stop")
  {
    reset();
    std::cout << getLogTime() << "割草任务停止" << std::endl;
  }
  else if (data == "mowing_finished")
  {
    reset();
    std::cout << getLogTime() << "割草任务完成" << std::endl;
  }
  else if (data == "reset") {
    reset();
  }
}

void reset()
{
  geometry_msgs::Polygon empty_hull;
  map_hull_pub.publish(empty_hull);
  // 清空所有路径数据
  routine_.clear();
  routine.clear();
  traj_.clear();
  // 重置多地图相关状态
  multi_map_mode_ = false;
  is_collecting_paths_ = false;
  collected_map_paths_.clear();
  received_map_count_ = 0;
  expected_map_count_ = 0;
  collection_timeout_started_ = false;
  // 重置waypoints处理状态
  waypoints_processed_ = false;
  has_last_start_point_ = false;
  last_waypoints_count_ = 0;
  // 重置时间相关变量
  last_waypoints_time_ = ros::Time(0);
  last_map_received_time_ = ros::Time(0);
  // 重置起始点相关
  // last_start_point_ = geometry_msgs::Point();
  // 重置所有标志位到初始状态
  vmax = 1.0;
  vmin = 0.1;
  amax = 0.1;
  step_size = 0.25;
  RS_curve_gen_flag = 0;
  veh_pose_fisrt_sub = 0;
  channel_path_gen_flag = 0;
  repeat_mode_.store(false);
  // 重置地图名称
  current_map_name_ = "default";
  map_collection_timeout_ = 2.0;
  std::cout << getLogTime() << "全局规划器重置成功！" << std::endl;
}

int main(int argc, char **argv) {
	setlocale(LC_ALL,"");
	// ros节点初始化
	ros::init(argc, argv, "global_path");
	// 创建消息发送节点句柄
	ros::NodeHandle n;
	// 创建publisher，发布消息
	pub_path = n.advertise<util::LocalPath>("/lawn_mower/global_trajectory", 1); // 最终输出的全局路径，由read_watepoints节点订阅
	planning_ready_pub = n.advertise<std_msgs::String>("/planning_ready", 1);  // 新增：发布反馈信号
	new_waypoints_pub = n.advertise<geometry_msgs::PoseArray>("/newwaypoints_list", 1,true);  // 发布路径给UI显示
	map_hull_pub = n.advertise<geometry_msgs::Polygon>("/send_hull_info", 1, true);  // 发布边界多边形，由避障节点订阅来设置避障规划用的栅格地图
	// pub_state = n.advertise<mower_msgs::PlaningOK>("/mower/planing_ok", 1);
	// 创建subscriber，接受消息
	ros::Subscriber wpl_sub = n.subscribe("/waypoint_list", 1, wplCallback);
	ros::Subscriber veh_pose_sub = n.subscribe("/Mower/position", 1, vehPoseFisrtSubCallback);
	ros::Subscriber channel_path_sub = n.subscribe("/planning_path_info", 1, ChannelPathSubCallback);
	ros::Subscriber sub_signal = n.subscribe("/signal", 1, SignalCallBack);
	// mower_msgs::PlaningOK planning_ok;
	// planning_ok.is_planing_ok = true;
	ros::Rate loop_rate(10);
	ROS_INFO("Global path planning node started");

	/*该节点中所有操作都是在回调函数中完成，所以主程序中只含有spin()*/
	while (ros::ok()) {
		// pub_state.publish(planning_ok);
		ros::spinOnce();
		loop_rate.sleep();

		// 超时检查：只在多地图收集模式下进行，每5秒处理一次多地图拼接逻辑，避免频繁检查导致性能问题
		if (multi_map_mode_ && is_collecting_paths_ && collection_timeout_started_) {
			double elapsed_time = (ros::Time::now() - last_map_received_time_).toSec();
			if (elapsed_time > map_collection_timeout_) {
			ROS_WARN("Map collection timeout (%.1f seconds)! Received %d maps. Starting assembly.", 
						elapsed_time, received_map_count_);
			assembleMultiMapPaths();
			collection_timeout_started_ = false; // 重置超时标志
			}
		}
	}

	return 0;
}