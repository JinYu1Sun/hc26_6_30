#ifndef _LOCATIONMAP_H_
#define _LOCATIONMAP_H_
#include <bitset>
#include <ros/ros.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <std_msgs/Bool.h>
#include <geometry_msgs/Point.h>
#include <geometry_msgs/PointStamped.h>
#include <geometry_msgs/Point32.h>
#include <geometry_msgs/Polygon.h>
#include <geometry_msgs/PolygonStamped.h>
#include <sensor_msgs/NavSatFix.h>
#include <nav_msgs/Odometry.h>
#include <nav_msgs/Path.h>
#include <std_msgs/String.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <yaml-cpp/yaml.h>
#include "util/Vslam.h"
#include "util/MapPath.h"
#include "util/PolygonWithHolesStamped.h"
#include "util/Position.h"
#include "util/VehicleInfoTest.h"

class LocationMap
{
public:
    LocationMap();
    ~LocationMap();
    
    void publishMapList();  // 发布所有可用地图的列表
    bool loadMap(const std::string& map_file);  // 加载指定的地图文件数据到yaml_hulls变量中

private:
    ros::NodeHandle *nh;
    ros::Publisher name_pub;        // 发布地图中包含的边界名称列表
    // ros::Publisher map_info_pub;
    ros::Publisher map_hull_pub;    // 发布边界多边形，由避障节点订阅来设置避障规划用的栅格地图
    ros::Publisher map_hole_pub;    // 发布录制的洞多边形，暂时无人订阅，可能是Android端需要用到
    ros::Publisher map_path_pub;    // 发布录制的路线，暂时无人订阅，可能是Android端需要用到
    ros::Publisher map_point_pub;   // 发布录制的停车位点，暂时无人订阅，可能是Android端需要用到
    ros::Publisher planning_hull_pub;   // 发布边界多边形和洞，由全覆盖节点订阅来规划全覆盖路径
    ros::Publisher planning_path_pub;   // 暂时无用********************
    ros::Publisher planning_point_pub;  // 发布规划的起点和终点，由全覆盖节点订阅来规划全覆盖路径
    ros::Publisher vehicle_info_pub;    // 暂时无用********************
    ros::Publisher file_pub;            // 发布本地保存的地图文件名列表

    ros::Subscriber localization_sub;
    ros::Subscriber get_signal_sub;
    ros::Subscriber planing_status_sub;
    ros::Subscriber planning_ready_sub;  // 新增：接收global_planning准备就绪信号

    // bool first_recv_gps;
    // bool signal_brd;   // 开始录制边界的信号
    // bool signal_path;  // 开始录制路径的信号
    // bool signal_point; // 开始录制停车位的信号
    // bool signal_hole;  // 开始录制障碍物的信号
    // bool signal_work;  // 开始割草的信号
    // bool signal_save;
    // bool signal_delete; // 删除地图
    // bool p_mode;
    // bool s_area;
    bool planning_status; // 规划状态

    double polygon_thr;
    double path_thr;

    bool single_map;    // 单地图模式
    bool multi_map;     // 多地图模式
    // int use_map; // 暂时未发现用途***********************************
    int delete_name;
    std::string base_file;

    std::string android_signal_topic,
        localization_topic,
        planning_status_topic,
        planning_path_topic,
        planning_hull_topic,
        planning_point_topic,
        vehicle_info_topic,
        // map_info_topic,
        map_hull_topic,
        map_hole_topic,
        map_path_topic,
        map_point_topic,
        hull_name_topic,
        filename,
        map_name_topic,
        planning_ready_topic;  // 新增：planning准备就绪话题

    int hull_tag; // 边界的编号
    geometry_msgs::Point32 point_stop;  // 暂存录制的停车点
    geometry_msgs::Point32 lastPoint;

public:
    struct map_hull
    {
        int name;
        geometry_msgs::Polygon polygon_hole;    // 录制洞的时候用它来临时存一下
        util::PolygonWithHoles polygon_with_holes;  // 这里面存边界和洞
        bool iscomplete;    // 该区域是否规划过
        double xmin;
        double xmax;
        double ymin;
        double ymax;
        // TODO: 多地图路径拼接功能 - 添加连接路径字段 cnk 0805
        std::vector<geometry_msgs::Point32> connection_path;  // 存储从YAML文件读取的连接路径
    };
    struct map_path
    {
        int name;
        bool iscomplete;
        util::MapPath link_path;
    };
    struct Point2d
    {
        double x, y;
    };
    void GetSignalCallback(const std_msgs::StringConstPtr &string);
    void StatusCallback(const util::PositionConstPtr &position_msg);
    void PlanningStatusCallback(const std_msgs::BoolConstPtr &plan_status);
    void PlanningReadyCallback(const std_msgs::StringConstPtr &ready_msg);  // 新增：处理planning准备就绪信号
    bool ComputeDistance(const geometry_msgs::Point32 &point, const double &thresh_distance);
    bool isPointInPolygon(const geometry_msgs::Point32 &point, LocationMap::map_hull &polygon_hull);
    void getBoundingBox(LocationMap::map_hull &polygon_hull);
    double perpendicularDistance(const LocationMap::Point2d &point, const LocationMap::Point2d &lineStart, const LocationMap::Point2d &lineEnd);
    void handleHullPoint(const geometry_msgs::Point32 &point);
    void handleHolePoint(const geometry_msgs::Point32 &point);
    void handleStopPoint(const geometry_msgs::Point32 &point);
    void handlePathPoint(const geometry_msgs::Point32 &point);
    void handleDeleteSignal();
    void handleSaveSignal();
    void handleWorkSignal(const geometry_msgs::Point32 &point);
    void reset();
    std::vector<LocationMap::Point2d> douglasPeucker(const std::vector<LocationMap::Point2d> &points, double epsilon);
    std::string get_current_path();

    std::vector<LocationMap::map_hull> hulls;
    std::vector<LocationMap::map_path> paths;

    // 存储当前地图的数据，包含边界点、洞、边界之间的连接路径(边界和洞都由四个点构成，边界看做地图中的一个工作区域，洞看做地图中的静态障碍物)
    std::vector<LocationMap::map_hull> yaml_hulls;
    // std::vector<LocationMap::map_path> yaml_paths;
    LocationMap::map_hull tmp_hull; // 暂存当前正在录制的地图数据
    LocationMap::map_hull last_hull;
    LocationMap::map_path tmp_path;
    
    // TODO: 多地图路径拼接功能 - 静态成员变量声明 cnk 0805
    static int current_map_index_;
    static bool is_multi_map_processing_;
    static geometry_msgs::Point32 current_start_point_;
    static geometry_msgs::Point32 current_end_point_;
    
    // TODO: 多地图路径拼接功能 - 函数声明 cnk 0805
    void processNextMapInSequence();

    enum BitFields {
    signal_brd = 0,
    signal_hole = 1,
    signal_path = 2,
    signal_point = 3,
    signal_work = 4,
    signal_delete = 5,
    signal_save = 6
};
// extern std::bitset<7> signal;
static std::bitset<7> signal; 

};

#endif // _LOCATIONMAP_H_
