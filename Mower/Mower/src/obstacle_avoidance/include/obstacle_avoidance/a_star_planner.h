#ifndef A_STAR_PLANNER_H
#define A_STAR_PLANNER_H

#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Point.h>
#include <geometry_msgs/PolygonStamped.h>
#include <nav_msgs/Path.h>
#include <visualization_msgs/MarkerArray.h>
#include <std_msgs/Bool.h>
#include <vector>
#include <queue>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <string>
#include <cmath>
#include <array>
#include <limits>
#include <mutex>
#include <atomic>
#include <optional>
#include "std_msgs/UInt8.h"
#include <std_msgs/String.h>
#include <opencv2/opencv.hpp>

// 添加自定义消息头文件
#include <util/Position.h>
#include <util/Obstacle.h>
#include <util/ObstacleList.h>
#include <util/LocalPath.h>
#include <obstacle_avoidance/Point2D.h>
// #include <obstacle_avoidance/PathPoint.h>
// #include <obstacle_avoidance/Obstacle.h>
// #include <obstacle_avoidance/ObstacleArray.h>

namespace obstacle_avoidance {

// 图中节点的定义
struct Node {
    int x, y;
    double g_cost;       // 从起点到当前节点的代价
    double h_cost;       // 从当前节点到目标的启发式代价估计
    double f_cost;       // f = g + h，总代价
    int turn_count;      // 转向次数
    int direction;       // 0=上, 1=右, 2=下, 3=左
    Node* parent;
    
    Node(int x, int y) 
        : x(x), y(y), g_cost(std::numeric_limits<double>::max()), 
          h_cost(0), f_cost(std::numeric_limits<double>::max()), 
          turn_count(0), direction(-1), parent(nullptr) {}
    
    bool operator==(const Node& other) const {
        return x == other.x && y == other.y;
    }
};

// 用于unordered_map的哈希函数
struct NodeHash {
    std::size_t operator()(const Node* node) const {
        return std::hash<int>()(node->x) ^ std::hash<int>()(node->y);
    }
};

// 用于unordered_map的比较函数
struct NodeEqual {
    bool operator()(const Node* lhs, const Node* rhs) const {
        return lhs->x == rhs->x && lhs->y == rhs->y;
    }
};

// 用于优先队列的比较函数
struct NodeCompare {
    bool operator()(const Node* lhs, const Node* rhs) const {
        if (lhs->f_cost == rhs->f_cost) {
            return lhs->h_cost > rhs->h_cost;
        }
        return lhs->f_cost > rhs->f_cost;
    }
};

// 自定义的障碍物结构体改为使用消息类型
typedef util::Obstacle ObstacleMsg;

class AStarPlanner {
public:
    AStarPlanner();
    ~AStarPlanner();

private:
    // ROS节点句柄
    ros::NodeHandle nh_;
    ros::NodeHandle private_nh_;
    
    // 参数
    double resolution_;          // 栅格地图分辨率（用户指定，不再计算）
    double safety_distance_;     // 安全距离
    double planning_distance_threshold_; // 当与碰撞点距离小于此值时触发规划
    double slowdown_threshold_;    // 当与碰撞点距离小于此值时触发减速
    double step_size_;                  // 两个路径点间隔，用来计算间隔多少个路径点出发规划，参数来源于global_planning节点
    double avoidance_completion_dis_threshold_, avoidance_completion_ang_threshold_; // 当距离目标点距离小于此值时认为避障完成
    std::string map_file_;       // 边界地图文件路径
    int map_width_;              // 地图宽度(像素)，现在根据边界和分辨率自动计算
    int map_height_;             // 地图高度(像素)，现在根据边界和分辨率自动计算
    double inflation_distance_;  // 膨胀半径
    int callback_counter;       // 每次避障规划路径的次数
    bool boundary_check_enabled_; // TODO: 2025/01/28 - 边界检测使能开关
    double boundary_shrink_distance_; // TODO: 2025/01/28 - 边界内缩距离
    int required_memrgency_frames_; // 需要连续检测的帧数,用来控制停障的时间
    bool cutter__runing_;   // 刀盘是否运行

    // 避障状态跟踪
    std_msgs::UInt8 is_avoiding_;      // 是否正在进行避障
    std::mutex avoiding_mutex_; // 保护避障状态的互斥量
    geometry_msgs::Point avoidance_goal_; // 当前避障的目标点，由栅格坐标转换而来
    bool avoidfinish_flag_; // 避障完成标志
    std::mutex avoidfinish_mutex_; // 保护避障完成标志的互斥量
    
    
    // 地图边界信息
    double map_min_x_;           // 地图边界x最小值
    double map_max_x_;           // 地图边界x最大值
    double map_min_y_;           // 地图边界y最小值
    double map_max_y_;           // 地图边界y最大值
    
    // 地图数据
    std::vector<std::vector<geometry_msgs::Point>> map_boundarys_; // 保存所有的地图边界
    std::vector<geometry_msgs::Point> last_map_boundary_;   // 上一个地图边界

    std::vector<std::vector<bool>> grid_map_; // 栅格地图
    
    // 路径和障碍物数据
    nav_msgs::Path path_;                     // 当前小车正在执行的路径
    std::vector<ObstacleMsg> obstacles_;      // 使用自定义障碍物消息
    std::mutex obstacles_mutex_; // 保护障碍物数据的互斥量
    util::LocalPath path_points_; // 自定义路径点
    util::Position current_pose_;
    std::mutex current_pose_mutex_; // 保护 current_pose_ 的互斥量
    util::LocalPath avoidance_path_points_;  // 存储避障路径
    geometry_msgs::Point end_point_; // 避障目标点
    util::Position backup_start_position;

    

    
    // ROS订阅者
    ros::Subscriber path_points_sub_;   // 自定义路径点订阅
    ros::Subscriber obstacles_sub_;
    ros::Subscriber pose_sub_;
    ros::Subscriber boundary_sub_;
    ros::Subscriber command_;
    ros::Subscriber signal_sub_;
    // ROS发布者
    ros::Publisher local_path_pub_;
    ros::Publisher avoidstate_pub;
    ros::Publisher pub_stopflag1;  // 边界停车信号发布器
    ros::Publisher pub_stopflag2;  // 避障过程中紧急停车信号发布器
    ros::Publisher goal_point_pub_;
    ros::Publisher grid_map_pub_; // 用于调试的占用栅格地图发布器
    ros::Publisher obstacle_markers_pub_; // 用于调试的障碍物多边形发布器


    
    // 回调函数
    void pathPointsCallback(const  util::LocalPath ::ConstPtr& msg);
    void obstaclesCallback(const util::ObstacleList::ConstPtr& msg);
    void poseCallback(const util::Position::ConstPtr& msg);
    void boundaryCallback(const geometry_msgs::Polygon::ConstPtr& msg);
    void commandCallback(const std_msgs::String::ConstPtr &string);
    void signalCallback(const std_msgs::String::ConstPtr& msg); 

    // 地图操作
    bool loadMapFromFile(const std::string& filename);
    bool isBoundaryChanged(const std::vector<geometry_msgs::Point>& new_boundary);  // 是否需要更新地图边界
    void calculateMapDimensions();  // 计算地图尺寸
    void generateGridMap();
    void boundaryOpenCVImg(cv::Mat &map_img, cv::Mat &dist, const std::vector<geometry_msgs::Point> &map_boundary);
    void publishDebugMap();
    
    // A*算法核心函数
    std::vector<Node*> findPath(int start_x, int start_y, int goal_x, int goal_y);
    double calculateHeuristic(const Node& current, const Node& goal);
    bool isCollision(int x, int y);
    bool isValidPoint(int x, int y);
    std::vector<Node*> reconstructPath(Node* goal);
    void publishPath(const std::vector<Node*>& path);
    
    // 路径处理
    bool checkPathCollision();
    bool checkAvoidanceCompletion(const util::Position& local_pose); // 检查避障是否完成
    void checkReverseDistance(); // 检查倒车距离
    std::pair<int, int> findCollisionPoint();
    std::pair<int, int> findSafeStartPoint(int collision_idx);
    std::pair<int, int> findSafeGoalPoint(int collision_idx);
    void planAndPublishPath();
    


    // 自定义路径解析
    nav_msgs::Path convertPathPointsToPath(const util::LocalPath& path_points);
    
    // 辅助函数
    void clearPath(std::vector<Node*>& path);
    std::pair<int, int> worldToGrid(double x, double y);
    std::pair<double, double> gridToWorld(int x, int y);
    bool isPositionInBoundary(double x, double y); // TODO: 2025/01/28 - 边界检测函数
    bool isPositionInBoundary(double x, double y, const std::vector<geometry_msgs::Point> &boundary, bool &current_frame_in_connect_path); //边界检测函数重载，增加当前帧是否在连接路径上的输出
    double pointToLineSegmentDistance(double px, double py, double x1, double y1, double x2, double y2); // TODO: 2025/01/28 - 点到线段距离计算
    void clearLocalPath(util::LocalPath& path);
    void reset();
    
    // 判断障碍物碰撞
    bool intersectsWithObstacle(double px, double py, const ObstacleMsg& obstacle);
    bool isPositionStable(double threshold,double threshold_yaw);
    bool is_stable_ = false; // 是否稳定
    bool is_fisrt_avoid_= true; // 第一次避障标志，开始寻找避障路线时会将其置为false，直到避障完成才会重新置为true
    std::deque<util::Position> last_positions_; // 非正常状态和避障状态下的位置历史记录
    int goal_x, goal_y,start_x, start_y;
    
    // cnk0822: 新增 - 保存避障终点信息，用于倒车后重新规划
    bool has_saved_goal_; // 是否已保存终点
    
    // 倒车相关状态
    geometry_msgs::Point reverse_start_position_; // 倒车开始时的位置
    bool is_reversing_; // 是否正在倒车
    
    // 多帧障碍物检测相关
    int emergency_detection_frames_; // 紧急状态检测帧数计数器
    std::atomic<bool> emergency_confirmed_; // 紧急状态是否已确认
    
    // 状态超时保护
    ros::Time state3_start_time_; // 状态3开始时间
    ros::Time state4_start_time_; // 状态4开始时间
    static constexpr double STATE3_TIMEOUT_SECONDS = 10.0; // 状态3超时时间（秒）
    static constexpr double STATE4_TIMEOUT_SECONDS = 20.0; // 状态4超时时间（秒）
};

} // namespace obstacle_avoidanc
#endif // A_STAR_PLANNER_H
