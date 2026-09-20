#include "obstacle_avoidance/a_star_planner.h"
#include <nav_msgs/OccupancyGrid.h>
#include <visualization_msgs/Marker.h>
#include <geometry_msgs/Polygon.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <tf2/utils.h>
#define DEBUG_LOGGING // 定义DEBUG_LOGGING以启用调试日志输出

#define VERSION "1.0"
int avoid_counter = -1;

namespace obstacle_avoidance {

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

AStarPlanner::AStarPlanner() : private_nh_("~"), map_width_(0), map_height_(0)
{
    // 加载参数
    private_nh_.param<double>("resolution", resolution_, 0.25);                                         // 栅格大小
    private_nh_.param<double>("safety_distance", safety_distance_, 0.0);                                // 这里再a把顶点做一个扩展，可以不要，因为inflation_distance已经扩展了
    private_nh_.param<double>("inflation_distance", inflation_distance_, 0.9);                           // 障碍物膨胀半径
    private_nh_.param<double>("planning_distance_threshold", planning_distance_threshold_, 2.0);        // 触发避障规划的距离阈值
    private_nh_.param<double>("slowdown_threshold", slowdown_threshold_, 3.0);                         // 减速阈值
    private_nh_.param<double>("step_size", step_size_, 0.15);                                           // 路径点间隔
    private_nh_.param<double>("avoidance_completion_dis_threshold", avoidance_completion_dis_threshold_, 0.25); // 避障完成距离阈值
    private_nh_.param<double>("avoidance_completion_ang_threshold", avoidance_completion_ang_threshold_, 0.2); // 避障完成角度阈值
    private_nh_.param<bool>("boundary_check_enabled", boundary_check_enabled_, false); // TODO: 2025/01/28 - 边界检测使能开关，默认关闭
    private_nh_.param<double>("boundary_shrink_distance", boundary_shrink_distance_, 0.0); // TODO: 2025/01/28 - 边界内缩距离
    private_nh_.param<int>("required_emergency_frames", required_memrgency_frames_, 50); // 需要障碍物连续检测的帧数,用来控制停障的时间

    cutter_runing_ = true; // 刀盘默认是打开的
    // clear_obstacles_time_ = ros::Time::now();
    
    // 初始化避障状态
    is_avoiding_.data = 0;
    avoidfinish_flag_ = false;
    callback_counter = 0;
    backup_success_.store(false);
    
    // 初始化多帧检测相关变量
    emergency_detection_frames_ = 0;
    emergency_confirmed_.store(false);

    map_boundarys_.clear();

    // 初始化地图边界
    map_min_x_ = map_min_y_ = std::numeric_limits<double>::max();
    map_max_x_ = map_max_y_ = std::numeric_limits<double>::lowest();

    ROS_INFO("地图分辨率: %.2fm, 安全距离: %.2fm, 障碍物膨胀半径: %.2fm, 触发避障规划的距离阈值: %.2fm, 减速阈值: %.2fm, 路径点间隔: %.2fm",
            resolution_, safety_distance_, inflation_distance_, planning_distance_threshold_, slowdown_threshold_, step_size_);
    ROS_INFO("需要障碍物连续检测的帧数: %d, 避障完成距离阈值: %.2fm, 避障完成角度阈值: %.2f弧度", 
            required_memrgency_frames_, avoidance_completion_dis_threshold_, avoidance_completion_ang_threshold_);
    ROS_INFO("边界检测是否打开: %s, 边界内缩距离: %.2f m", boundary_check_enabled_ ? "true" : "false", boundary_shrink_distance_);

    local_path_pub_ = nh_.advertise<util::LocalPath>("/lawn_mower/avoid_traj", 1, true);
    avoidstate_pub = nh_.advertise<std_msgs::UInt8>("/lawn_mower/avoid_state", 1);
    out_of_bound_pub_ = nh_.advertise<std_msgs::Bool>("/lawn_mower/out_of_bounds", 1);   // 超出边界发布出界信号，由控制节点订阅进行减速
    pub_stopflag2 = nh_.advertise<std_msgs::Bool>("/mower/stop_car2", 1);   // 避障过程中紧急停车信号发布器
    goal_point_pub_ = nh_.advertise<geometry_msgs::Point>("/lawn_mower/target_point", 1);
    
#ifdef DEBUG_LOGGING
    grid_map_pub_ = nh_.advertise<nav_msgs::OccupancyGrid>("/lawn_mower/grid_map", 1, true);
    obstacle_markers_pub_ = nh_.advertise<visualization_msgs::MarkerArray>("/lawn_mower/obstacle_polygon", 1, true);
    view_marker_pub_ = nh_.advertise<visualization_msgs::Marker>("/lawn_mower/view_polygon", 1, true);
#endif
    
    avoidstate_pub.publish(is_avoiding_);

    // 订阅话题 - 只使用自定义路径点
    path_points_sub_ = nh_.subscribe("/lawn_mower/avoid_detection_path", 1, &AStarPlanner::pathPointsCallback, this);
    obstacles_sub_ = nh_.subscribe("/mower_perception/obstacles", 1, &AStarPlanner::obstaclesCallback, this);
    pose_sub_ = nh_.subscribe("/Mower/position", 1, &AStarPlanner::poseCallback, this);

    // 添加订阅边界多边形的话题
    boundary_sub_ = nh_.subscribe("/send_hull_info", 10, &AStarPlanner::boundaryCallback, this);

    // cnk0811: 添加signal话题订阅
    signal_sub_ = nh_.subscribe("/signal", 1, &AStarPlanner::signalCallback, this);


    ROS_INFO("Waiting for boundary information from /send_hull_info topic...");
}

AStarPlanner::~AStarPlanner()
{
    // 释放资源
}
bool AStarPlanner::isBoundaryChanged(const std::vector<geometry_msgs::Point> &new_boundary)
{
    if (new_boundary.size() != last_map_boundary_.size())
        return true;

    if (new_boundary.empty() || last_map_boundary_.empty())
        return false; // 避免空数组情况

    // 只比较同长度的有效点，避免固定 2 个点导致越界
    const size_t compare_count = std::min(new_boundary.size(), last_map_boundary_.size());
    for (size_t i = 0; i < compare_count; ++i)
    {
        if (fabs(new_boundary[i].x - last_map_boundary_[i].x) > 1e-6 ||
            fabs(new_boundary[i].y - last_map_boundary_[i].y) > 1e-6 ||
            fabs(new_boundary[i].z - last_map_boundary_[i].z) > 1e-6)
        {
            return true;
        }
    }

    return false;
}

// 添加新的回调函数来处理边界多边形
void AStarPlanner::boundaryCallback(const geometry_msgs::Polygon::ConstPtr &msg)
{
    if (msg->points.empty()) 
    {
        return;
    }
    if (msg->points.size() < 3)
    {
        ROS_WARN("Received boundary polygon with too few points (%zu), ignoring it to avoid invalid geometry.", msg->points.size());
        return;
    }
    ROS_INFO("Received boundary polygon with %zu points", msg->points.size());

    std::vector<geometry_msgs::Point> cur_boundary;

    // 转换点类型并保存
    for (const auto &point32 : msg->points)
    {
        geometry_msgs::Point point;
        point.x = point32.x;
        point.y = point32.y;
        point.z = point32.z; // 保留z坐标用于特殊标记
        // std::cout << "Point: " << point.x << ", " << point.y << ", " << point.z << std::endl;

        // 更新地图边界
        map_min_x_ = std::min(map_min_x_, point.x);
        map_min_y_ = std::min(map_min_y_, point.y);
        map_max_x_ = std::max(map_max_x_, point.x);
        map_max_y_ = std::max(map_max_y_, point.y);
        cur_boundary.push_back(point);
    }

    // 确保形成闭环 - 将第一个点添加到最后
    if (!cur_boundary.empty())
    {
        cur_boundary.push_back(cur_boundary.front());
        ROS_INFO("Added first point to the end to form a closed polygon");
    }
    map_boundarys_.push_back(cur_boundary);

    if (isBoundaryChanged(cur_boundary))
    {
        calculateMapDimensions(); // 计算地图尺寸
        generateGridMap();  // 生成栅格地图
        last_map_boundary_ = cur_boundary;
    }

    
    ROS_INFO("Map boundary processed: x=[%.2f, %.2f], y=[%.2f, %.2f]",
                map_min_x_, map_max_x_, map_min_y_, map_max_y_);
}

void AStarPlanner::calculateMapDimensions()
{
    // 根据边界点和分辨率计算地图尺寸
    std::cout << "map_min_x_:" << map_min_x_ << std::endl;
    std::cout << "map_max_x_:" << map_max_x_ << std::endl;
    std::cout << "map_min_y_:" << map_min_y_ << std::endl;
    std::cout << "map_max_y_:" << map_max_y_ << std::endl;

    // 计算地图宽度和高度（像素）
    map_width_ = static_cast<int>(ceil((map_max_x_ - map_min_x_) / resolution_));
    map_height_ = static_cast<int>(ceil((map_max_y_ - map_min_y_) / resolution_));

    // ROS_INFO("Map dimensions: x=[%.2f, %.2f], y=[%.2f, %.2f]",
    //          map_min_x_, map_max_x_, map_min_y_, map_max_y_);
    ROS_INFO("Map size: %d x %d cells (resolution: %.2f)",
                map_width_, map_height_, resolution_);
}

void AStarPlanner::generateGridMap()
{
    if (map_width_ <= 0 || map_height_ <= 0 || resolution_ <= 0.0)
    {
        ROS_WARN("Grid map dimensions are invalid (width=%d, height=%d, resolution=%.4f), skipping map generation.",
                 map_width_, map_height_, resolution_);
        return;
    }

    // 创建一个空的栅格地图
    grid_map_ = std::vector<std::vector<bool>>(map_height_, std::vector<bool>(map_width_, true));
    std::cout << "地图数量：" << map_boundarys_.size() << std::endl;
    
    for (const auto &map_boundary : map_boundarys_)
    {
        // 将边界点转换为OpenCV点
        cv::Mat map_img;
        cv::Mat dist;
        // 将OpenCV图像转换回栅格地图,对边界/障碍进行膨胀，暂时不膨胀
        int inflation_cells = 0;
        if (map_boundary.front().z == 999.0) // 如果z为999，表示障碍物
        {
            boundaryOpenCVImg(map_img, dist, map_boundary, true);
            for (int y = 0; y < map_height_; y++)
            {
                for (int x = 0; x < map_width_; x++)
                {
                    if (map_img.at<uchar>(y, x) == 0)
                    {
                        grid_map_[y][x] = true; // 障碍
                    }
                }
            }
            continue;
        }
        else
        {
            boundaryOpenCVImg(map_img, dist, map_boundary, false);
        }
        for (int y = 0; y < map_height_; y++)
        {
            for (int x = 0; x < map_width_; x++)
            {
                if (grid_map_[y][x] == false)
                    continue;
                bool is_obstacle = false;
                // 原始障碍（多边形外部）
                if (map_img.at<uchar>(y, x) == 0)
                {
                    is_obstacle = true;
                }
                else
                {
                    // 如果距离最近障碍的像素距离小于等于膨胀像素数，则视为障碍
                    float d = dist.at<float>(y, x);
                    if (d <= static_cast<float>(inflation_cells))
                        is_obstacle = true;
                }
                grid_map_[y][x] = is_obstacle;
            }
        }
    }

#ifdef DEBUG_LOGGING
    // 发布占用栅格地图用于调试（使用成员发布器，已在构造函数中 advertise）
    nav_msgs::OccupancyGrid occupancy_grid;
    occupancy_grid.header.stamp = ros::Time::now();
    occupancy_grid.header.frame_id = "map";
    occupancy_grid.info.resolution = resolution_;
    occupancy_grid.info.width = map_width_;
    occupancy_grid.info.height = map_height_;
    occupancy_grid.info.origin.position.x = map_min_x_;
    occupancy_grid.info.origin.position.y = map_min_y_;
    occupancy_grid.info.origin.position.z = 0.0;
    occupancy_grid.info.origin.orientation.w = 1.0;
    occupancy_grid.data.resize(map_width_ * map_height_);
    for (int y = 0; y < map_height_; y++)
    {
        for (int x = 0; x < map_width_; x++)
        {
            occupancy_grid.data[y * map_width_ + x] = grid_map_[y][x] ? 100 : 0; // true->100(障碍), false->0(空闲)
        }
    }
    grid_map_pub_.publish(occupancy_grid);
#endif
}

// 创建边界opencv图像
void AStarPlanner::boundaryOpenCVImg(cv::Mat &map_img, cv::Mat &dist, const std::vector<geometry_msgs::Point> &map_boundary, const bool &is_obstacle)
{
    // 将边界点转换为OpenCV点
    std::vector<cv::Point> boundary_points;
    for (const auto &point : map_boundary)
    {
        int grid_x = std::floor((point.x - map_min_x_) / resolution_);
        int grid_y = std::floor((point.y - map_min_y_) / resolution_);
        boundary_points.emplace_back(grid_x, grid_y);
    }
    
    // 绘制边界多边形，填充内部
    if (boundary_points.size() > 2)
    {
        std::vector<std::vector<cv::Point>> contours = {boundary_points};
        // 先填充多边形内部（边界也会一起变成255）
        if (!is_obstacle)
        {
            // 创建OpenCV图像
            map_img = cv::Mat::zeros(map_height_, map_width_, CV_8UC1);
            cv::fillPoly(map_img, contours, cv::Scalar(255));
            // 再把边界重新画成0
            cv::polylines(map_img, contours,
                true,                // 闭合
                cv::Scalar(0),       // 障碍
                2                    // 边界线宽
            );
            // 使用距离变换：对每个像素计算到最近障碍(像素值==0)的像素距离
            // distanceTransform 要求非零像素为前景（这里前景=自由空间=255），零像素为障碍
            cv::distanceTransform(map_img, dist, cv::DIST_L2, 3);
        }
        else
        {
            map_img = cv::Mat::ones(map_height_, map_width_, CV_8UC1);
            cv::fillPoly(map_img, contours, cv::Scalar(0)); // 障碍物区域填充为0
        }
    }
}

void AStarPlanner::pathPointsCallback(const util::LocalPath::ConstPtr &msg)
{
    path_points_ = *msg;
    path_ = convertPathPointsToPath(path_points_);
}

nav_msgs::Path AStarPlanner::convertPathPointsToPath(const util::LocalPath &path_points)
{
    std_msgs::UInt8 is_avoiding_copy;
    {
        std::lock_guard<std::mutex> lock(avoiding_mutex_);
        is_avoiding_copy = is_avoiding_;
    }
    nav_msgs::Path path;
    path.header.stamp = ros::Time::now();
    path.header.frame_id = "map";
    if(is_avoiding_copy.data == 1 && path_points.x.size()==1 )
    {
        std::cout << getLogTime() << "避障路线已经走完" << std::endl;
        std::lock_guard<std::mutex> lock(avoidfinish_mutex_);
        avoidfinish_flag_ =true;
    }

    size_t num_points = path_points.x.size();
    if (num_points == 0)
    {
        return path;
    }
    size_t min_length = num_points;
    min_length = std::min(min_length, path_points.y.size());

    for (size_t i = 0; i < min_length; i++)
    {
        geometry_msgs::PoseStamped pose;
        pose.header = path.header;
        pose.pose.position.x = path_points.x[i];
        pose.pose.position.y = path_points.y[i];
        pose.pose.position.z = 0.0;

        // 如果有航向信息，可以设置方向
        if (i < path_points.heading.size())
        {
            double yaw = path_points.heading[i];
            pose.pose.orientation.x = 0.0;
            pose.pose.orientation.y = 0.0;
            pose.pose.orientation.z = sin(yaw / 2.0);
            pose.pose.orientation.w = cos(yaw / 2.0);
        }
        else
        {
            pose.pose.orientation.w = 1.0;
        }

        path.poses.push_back(pose);
    }

    return path;
}

util::Obstacle AStarPlanner::inflateObstacle(const util::Obstacle &obstacle, const double &inflation_distance)
{
    // 创建膨胀后的障碍物对象
    util::Obstacle inflated_obstacle = obstacle;
    // 按逆时针顺序定义原始顶点（确保顺序与原始obstacle结构一致）
    double corners_x[4] = {obstacle.x1, obstacle.x2, obstacle.x3, obstacle.x4};
    double corners_y[4] = {obstacle.y1, obstacle.y2, obstacle.y3, obstacle.y4};
    
    // 检查顶点顺序是否为逆时针，如果不是则重新排序
    double total = 0;
    for (int i = 0; i < 4; i++) {
        int j = (i + 1) % 4;
        total += (corners_x[j] - corners_x[i]) * (corners_y[j] + corners_y[i]);
    }
    if (total >= 0) {
        // 如果不是逆时针，则反转顺序
        std::reverse(corners_x, corners_x + 4);
        std::reverse(corners_y, corners_y + 4);
        ROS_WARN("Obstacle vertex order is not counterclockwise, auto-adjusted");
    }
    // 计算中心点
    double center_x = obstacle.global_x;
    double center_y = obstacle.global_y;

    // 膨胀后的顶点坐标
    double inflated_x[4];
    double inflated_y[4];

    // 边膨胀算法：每条边沿其法向量方向向外平移
    // 计算每条边的法向量
    double edge_normals[4][2]; // 存储每条边的法向量
    for (int i = 0; i < 4; i++) {
        int next_i = (i + 1) % 4;
        // 计算边的向量
        double edge_dx = corners_x[next_i] - corners_x[i];
        double edge_dy = corners_y[next_i] - corners_y[i];
        
        // 计算边的法向量（右转90度）
        double normal_dx = -edge_dy;
        double normal_dy = edge_dx;
        // 归一化
        double length = std::sqrt(normal_dx * normal_dx + normal_dy * normal_dy);
        if (length > 0) {
            normal_dx /= length;
            normal_dy /= length;
        }
        
        // 确保法向量指向外部
        double to_center_dx = center_x - corners_x[i];
        double to_center_dy = center_y - corners_y[i];
        
        if (normal_dx * to_center_dx + normal_dy * to_center_dy > 0) {
            normal_dx = -normal_dx;
            normal_dy = -normal_dy;
        }
        
        edge_normals[i][0] = normal_dx;
        edge_normals[i][1] = normal_dy;
    }
    
    // 计算膨胀后的顶点（两条膨胀边的交点）
    for (int i = 0; i < 4; i++) {
        int prev_i = (i + 3) % 4;
        int next_i = (i + 1) % 4;
        
        // 获取两条相邻边的法向量
        double normal1_x = edge_normals[prev_i][0]; // 前一条边的法向量
        double normal1_y = edge_normals[prev_i][1];
        double normal2_x = edge_normals[i][0];       // 当前边的法向量
        double normal2_y = edge_normals[i][1];
        double p1_x = corners_x[prev_i];
        double p1_y = corners_y[prev_i];
        double p2_x = corners_x[i];
        double p2_y = corners_y[i];
        
        // 当前边的下一个端点
        double p3_x = corners_x[next_i];
        double p3_y = corners_y[next_i];      
        double a11 = p2_x - p1_x;
        double a12 = -(p3_x - p2_x);
        double a21 = p2_y - p1_y;
        double a22 = -(p3_y - p2_y);
        
        double b1 = (p2_x - p1_x) + inflation_distance * (normal2_x - normal1_x);
        double b2 = (p2_y - p1_y) + inflation_distance * (normal2_y - normal1_y);
        
        double det = a11 * a22 - a12 * a21;
        
        if (std::abs(det) > 1e-6) {
            // 求解线性方程组
            double t = (b1 * a22 - b2 * a12) / det;
            
            // 计算交点
            inflated_x[i] = p1_x + t * (p2_x - p1_x) + inflation_distance * normal1_x;
            inflated_y[i] = p1_y + t * (p2_y - p1_y) + inflation_distance * normal1_y;
        } else {
            // 如果两条边平行，使用简单的法向量膨胀
            double avg_normal_dx = (normal1_x + normal2_x) / 2.0;
            double avg_normal_dy = (normal1_y + normal2_y) / 2.0;
            double avg_length = std::sqrt(avg_normal_dx * avg_normal_dx + avg_normal_dy * avg_normal_dy);
            if (avg_length > 0) {
            avg_normal_dx /= avg_length;
            avg_normal_dy /= avg_length;
            }
            inflated_x[i] = corners_x[i] + inflation_distance * avg_normal_dx;
            inflated_y[i] = corners_y[i] + inflation_distance * avg_normal_dy;
        }
    }

    // 将膨胀后的顶点赋值给新的障碍物对象
    inflated_obstacle.x1 = inflated_x[0];
    inflated_obstacle.y1 = inflated_y[0];
    inflated_obstacle.x2 = inflated_x[1];
    inflated_obstacle.y2 = inflated_y[1];
    inflated_obstacle.x3 = inflated_x[2];
    inflated_obstacle.y3 = inflated_y[2];
    inflated_obstacle.x4 = inflated_x[3];
    inflated_obstacle.y4 = inflated_y[3];

    // 保持中心点不变
    inflated_obstacle.global_x = obstacle.global_x;
    inflated_obstacle.global_y = obstacle.global_y;
    return inflated_obstacle;
}

bool AStarPlanner::isObstacleInView(const util::Obstacle &obstacle, const std::vector<geometry_msgs::Point> &costmap_corners)
{
    // 检查障碍物的四个顶点是否在视野区域内
    std::vector<std::pair<double, double>> obstacle_corners = {
        {obstacle.x1, obstacle.y1},
        {obstacle.x2, obstacle.y2},
        {obstacle.x3, obstacle.y3},
        {obstacle.x4, obstacle.y4}
    };

    // 如果顶点都在视野区域内，则认为障碍物在视野内
    for (const auto &corner : obstacle_corners)
    {
        if (!isPositionInBoundary(corner.first, corner.second, costmap_corners))
        {
            return false; // 只要有一个顶点不在视野区域内，就认为障碍物不在视野内
        }
    }
    return true;
}

/* 
1.订阅障碍物四边形，并对障碍物进行膨胀处理，生成新的障碍物边界点。
2.检查当前路径是否与膨胀后的障碍物边界相交，如果相交则认为存在碰撞风险。
3.根据车辆当前位置与碰撞点的距离进行判断：
    3.1 如果距离较近（小于6个路径点），则认为是紧急情况，进入状态3，立即触发避障规划
    3.2 如果距离较远（小于13个路径点），则认为是潜在风险，进入状态2，但不立即触发避障规划
4.如果连续3帧都认为是紧急情况，则确认情况真的紧急，触发避障路径规划
5.规划避障路径时，如果发现处于避障状态，则不进行二次避障
6.如果未能规划出避障路径，则进入倒车状态，如果能规划出避障路径，则进入避障执行状态
*/
void AStarPlanner::obstaclesCallback(const util::ObstacleList::ConstPtr &msg) {
    std::lock_guard<std::mutex> lock(obstacles_mutex_);

    costmap_corners_.clear();
    for (const auto &costmap_corner : msg->costmap_corners)
        costmap_corners_.emplace_back(costmap_corner);
    if (costmap_corners_.size() != 4)
    {
        ROS_WARN("视野区域异常，视野区域点数：%ld", costmap_corners_.size());
        return;
    }

    // 判断之前的障碍物列表是否在当前视野区域内，如果在则删除该障碍物
    obstacles_.erase(std::remove_if(obstacles_.begin(), obstacles_.end(),
        [this](const util::Obstacle& obs) {
            return isObstacleInView(obs, costmap_corners_);
        }), obstacles_.end());

    for (const auto &obstacle : msg->obstacles) {
        if (obstacle.is_dynamic)
        {
            if (obstacle.local_x < planning_distance_threshold_+1.2)
            {
                std_msgs::Bool stop_msg;
                stop_msg.data = true;
                pub_stopflag2.publish(stop_msg);
                std::cout << getLogTime() << "检测到紧急动态障碍物，触发停车信号" << std::endl;
                return;
            }
            continue;
        }
        else
        {
            bool obstacle_in_boundary = false;
            // 判断障碍物是否在割草区域内
            for (const auto &boundary : map_boundarys_)
            {
                if (isPositionInBoundary(obstacle.global_x, obstacle.global_y, boundary))
                {
                    if (boundary[0].z == 999.0) // 表示障碍物区域
                    {
                        obstacle_in_boundary = false;
                        break;
                    }
                    else
                        obstacle_in_boundary = true;
                }
            }
            if (!obstacle_in_boundary)
                continue;
        }
        // 创建膨胀后的障碍物对象
        util::Obstacle inflated_obstacle =  inflateObstacle(obstacle, inflation_distance_);

        // 将膨胀后的障碍物添加到列表中
        obstacles_.push_back(inflated_obstacle);    
    }

    // 遍历障碍物列表，清理一部分重叠的障碍物，防止内存占用过大
    for (size_t i = 0; i < obstacles_.size(); ++i)
    {
        for (size_t j = i + 1; j < obstacles_.size(); ++j)
        {
            double dx = obstacles_[i].global_x - obstacles_[j].global_x;
            double dy = obstacles_[i].global_y - obstacles_[j].global_y;
            double distance = std::sqrt(dx * dx + dy * dy);
            if (distance < 0.2) // 如果两个障碍物中心点距离小于0.2米，则认为是重叠的障碍物
            {
                // 删除后面的障碍物
                obstacles_.erase(obstacles_.begin() + j);
                --j; // 调整索引，继续检查下一个障碍物
            }
        }
    }
    
#ifdef DEBUG_LOGGING
    // 发布视野区域
    visualization_msgs::Marker view_marker;
    view_marker.header.frame_id = "map";
    view_marker.header.stamp = ros::Time::now();
    view_marker.ns = "costmap_corners";
    view_marker.id = 0;
    view_marker.type = visualization_msgs::Marker::LINE_STRIP;
    view_marker.action = visualization_msgs::Marker::ADD;
    view_marker.scale.x = 0.05; // 线宽
    view_marker.color.r = 0.0;
    view_marker.color.g = 1.0;
    view_marker.color.b = 0.0;
    view_marker.color.a = 1.0;
    view_marker.points.clear();
    for (const auto &corner : costmap_corners_)
    {
        geometry_msgs::Point point;
        point.x = corner.x;
        point.y = corner.y;
        point.z = corner.z;
        view_marker.points.push_back(point);
    }
    // 闭合多边形
    if (!view_marker.points.empty())
    {
        view_marker.points.push_back(view_marker.points.front());
    }
    view_marker_pub_.publish(view_marker);

    visualization_msgs::MarkerArray markers;
    int marker_id = 0;
    for (const auto &obstacle : obstacles_) {
        visualization_msgs::Marker marker;
        marker.header.frame_id = "map";
        marker.header.stamp = ros::Time::now();
        marker.ns = "obstacles";
        marker.id = marker_id++;
        marker.type = visualization_msgs::Marker::LINE_STRIP;
        marker.action = visualization_msgs::Marker::ADD;
        marker.scale.x = 0.05; // 线宽
        marker.color.r = 1.0;
        marker.color.g = 0.0;
        marker.color.b = 0.0;
        marker.color.a = 1.0;

        geometry_msgs::Point p1, p2, p3, p4;
        p1.x = obstacle.x1; p1.y = obstacle.y1; p1.z = 0.0;
        p2.x = obstacle.x2; p2.y = obstacle.y2; p2.z = 0.0;
        p3.x = obstacle.x3; p3.y = obstacle.y3; p3.z = 0.0;
        p4.x = obstacle.x4; p4.y = obstacle.y4; p4.z = 0.0;

        // 添加四个顶点，形成闭环
        marker.points.push_back(p1);
        marker.points.push_back(p2);
        marker.points.push_back(p3);
        marker.points.push_back(p4);
        marker.points.push_back(p1); // 闭合

        markers.markers.push_back(marker);
    }
    visualization_msgs::MarkerArray delete_array;
    visualization_msgs::Marker del;
    del.ns = "obstacles";
    del.action = visualization_msgs::Marker::DELETEALL;
    del.header.frame_id = "map";
    delete_array.markers.push_back(del);
    obstacle_markers_pub_.publish(delete_array);

    // ROS_INFO("障碍物数量: %d", markers.markers.size());
    obstacle_markers_pub_.publish(markers);
#endif
  
    if (!path_.poses.empty() && !obstacles_.empty()) // 移除backup_finished检查，始终进行碰撞检测
    {
        if (backup_success_.load())
        {
            avoid_counter = 3; // 轨迹类型，没什么用
            planAndPublishPath();
            backup_success_.store(false);
            emergency_confirmed_.store(false);
            return;
        }
        if (checkPathCollision()) {
            // 只有在确认紧急状态后才进行避障规划
            if (emergency_confirmed_.load()) {
                avoid_counter = 3; // 轨迹类型，没什么用
                planAndPublishPath();
                // 重置确认状态，避免重复触发
                emergency_confirmed_.store(false);
            }
        }
    }
    if (obstacles_.empty())
    {
        std_msgs::Bool stop_msg;
        stop_msg.data = false;
        pub_stopflag2.publish(stop_msg);
        emergency_detection_frames_ = 0;
        if (is_avoiding_.data == 2 || is_avoiding_.data == 3 || is_avoiding_.data == 4) // 避障过程中不切换
        {
            ROS_INFO("状态切换: %d -> 0(正常状态) - 障碍物消失，切换为正常状态", is_avoiding_.data);
            is_avoiding_.data = 0; // 切换到正常状态
            avoidstate_pub.publish(is_avoiding_);
        }
    }
}
bool AStarPlanner::isPositionStable(double threshold, double threshold_yaw) // TODO
{
    if (last_positions_.size() < 5)
        return false; // 数据不足，无法判断稳定

    for (size_t i = 1; i < last_positions_.size(); ++i)
    {
        double dx = std::abs(last_positions_[i].position_x - last_positions_[i - 1].position_x);
        double dy = std::abs(last_positions_[i].position_y - last_positions_[i - 1].position_y);
        double dyaw = std::abs(last_positions_[i].yaw - last_positions_[i - 1].yaw);

        // 判断是否超过阈值
        if (dx > threshold || dy > threshold || dyaw > threshold_yaw)
            return false;
    }
    return true;
}
bool AStarPlanner::isPositionInBoundary(double x, double y, const std::vector<geometry_msgs::Point> &boundary)
{
    bool result = false;
    if (boundary.empty())
    {
        result = true; // 如果没有边界信息，认为在边界内
    }
    else
    {
        // 首先使用射线法判断点是否在原始边界内
        bool inside_original = false;
        bool on_boundary = false;

        int j = boundary.size() - 1;
        for (int i = 0; i < boundary.size(); i++)
        {
            // 处理水平线段的情况
            if (std::abs(boundary[j].y - boundary[i].y) <= 1e-9)
            {
                // 线段是水平的，检查点是否在线段上
                if (std::abs(boundary[i].y - y) <= 1e-9)
                {
                    double x_min = std::min(boundary[i].x, boundary[j].x);
                    double x_max = std::max(boundary[i].x, boundary[j].x);
                    if (x >= x_min && x <= x_max)
                    {
                        on_boundary = true;
                        break;
                    }
                }
            }
            else
            {
                // 线段不是水平的，使用射线法
                if (((boundary[i].y > y) != (boundary[j].y > y)) &&
                    (x < (boundary[j].x - boundary[i].x) * (y - boundary[i].y) / 
                            (boundary[j].y - boundary[i].y) + boundary[i].x))
                {
                    inside_original = !inside_original;
                }
            }
            j = i;
        }

        if (on_boundary)
        {
            result = true;
        }
        else if (!inside_original)  // 如果不在原始边界内，直接返回false
        {
            result = false;
        }
        else if (boundary_shrink_distance_ <= 0.0)  // 如果内缩距离为0，直接返回原始边界检测结果
        {
            result = inside_original;
        }
        else
        {
            // 使用内缩后的边界进行检测
            // 计算点到边界的最短距离
            double min_distance = std::numeric_limits<double>::max();

            for (size_t i = 0; i < boundary.size() - 1; i++)
            {
                const geometry_msgs::Point& p1 = boundary[i];
                const geometry_msgs::Point& p2 = boundary[i + 1];

                // 计算点到线段的距离
                double distance = pointToLineSegmentDistance(x, y, p1.x, p1.y, p2.x, p2.y);
                min_distance = std::min(min_distance, distance);
            }

            // 如果点在原始边界内且距离边界大于等于内缩距离，认为在有效边界内
            result = (min_distance >= boundary_shrink_distance_);
        }
    }
    return result;
}

// TODO: 2025/01/28 - 计算点到线段距离的辅助函数
double AStarPlanner::pointToLineSegmentDistance(double px, double py, double x1, double y1, double x2, double y2)
{
    double A = px - x1;
    double B = py - y1;
    double C = x2 - x1;
    double D = y2 - y1;
    
    double dot = A * C + B * D;
    double len_sq = C * C + D * D;
    
    if (len_sq == 0) {
        // 线段退化为点
        return std::sqrt(A * A + B * B);
    }
    
    double param = dot / len_sq;
    
    double xx, yy;
    if (param < 0) {
        xx = x1;
        yy = y1;
    } else if (param > 1) {
        xx = x2;
        yy = y2;
    } else {
        xx = x1 + param * C;
        yy = y1 + param * D;
    }
    
    double dx = px - xx;
    double dy = py - yy;
    return std::sqrt(dx * dx + dy * dy);
}

// 订阅小车位置，并在避障过程中进行状态监测和边界检测
void AStarPlanner::poseCallback(const util::Position::ConstPtr &msg)
{
    std::lock_guard<std::mutex> lock(avoiding_mutex_);
    util::Position current_pose = *msg;
    {
        std::lock_guard<std::mutex> lock(current_pose_mutex_);
        current_pose_ = current_pose;
    }
    checkAvoidanceCompletion(current_pose); 
    //cnk0904
    if (is_avoiding_.data == 4) {
        double elapsed_time = (ros::Time::now() - state4_start_time_).toSec();
        if (elapsed_time > STATE4_TIMEOUT_SECONDS) // 倒车状态超时保护，防止长时间停留在倒车状态**************************
        {
            ROS_WARN("State 4 (backup) timeout (%.1fs), forcing reset to normal state", elapsed_time);
            ROS_INFO("状态切换: 4(倒车状态) -> 0(正常状态) - 倒车超时，强制恢复正常");
            is_avoiding_.data = 0;
            avoidstate_pub.publish(is_avoiding_);
            is_fisrt_avoid_ = true;
        }
        else
        {
            double backup_distance = std::sqrt(
            std::pow(current_pose.position_x - backup_start_position.position_x, 2) + 
            std::pow(current_pose.position_y - backup_start_position.position_y, 2));
        
            if (backup_distance >= 1) {
                ROS_INFO("cnk0822: Backup distance reached: %.2f m, resetting avoidance state", backup_distance);
                ROS_INFO("状态切换: 4(倒车状态) -> 3(紧急停车) - 倒车完成，重新避障");
                is_avoiding_.data = 3;
                avoidstate_pub.publish(is_avoiding_);  
                is_fisrt_avoid_ = true;
                backup_success_.store(true);
            }
        }
    }
    
    // 状态3超时保护机制
    if (is_avoiding_.data == 3) {
        double elapsed_time = (ros::Time::now() - state3_start_time_).toSec();
        if (elapsed_time > STATE3_TIMEOUT_SECONDS)  // 紧急预警状态超时保护，防止长时间停留在紧急预警状态**************************
        {
            ROS_WARN("State 3 timeout (%.1fs), forcing reset to normal state", elapsed_time);
            ROS_INFO("状态切换: 3(紧急预警) -> 0(正常状态) - 紧急预警超时，强制恢复正常");
            is_avoiding_.data = 0;
            avoidstate_pub.publish(is_avoiding_);
            is_fisrt_avoid_ = true;
            emergency_detection_frames_ = 0;
            emergency_confirmed_.store(false);
        }
    }
    
    // 只有在非运动时才进行稳定性检测
    /* if (is_avoiding_.data != 0 && is_avoiding_.data != 1) 
    {
        last_positions_.push_back(current_pose);
        if (last_positions_.size() > 10)
            last_positions_.pop_front();
        is_stable_ = isPositionStable(0.08, 0.1);
        ROS_INFO("the car is_stable_   %d", is_stable_);
    }
    else
    {
        last_positions_.clear(); // 非状态3时清空数据
    } */
    
    // TODO: 2025/01/28 - 边界检测逻辑
    if (boundary_check_enabled_) {
        static int boundary_false_frame_count = 0; // 连续边界内帧计数
        static bool last_boundary_stop_state = false; // 上一帧的边界停车状态
        // ROS_WARN("Enter boundary_check_enabled_ model.................. ");
        
        bool current_frame_in_boundary = false;
        bool currrnt_frame_in_connect_path = false;
        for (const auto &boundary : map_boundarys_)
        {
            if (boundary[0].z == 999.0 && isPositionInBoundary(current_pose.position_x, current_pose.position_y, boundary))
            {
                current_frame_in_boundary = false;
                break;
            }
            if (boundary[0].z == 666.0 && isPositionInBoundary(current_pose.position_x, current_pose.position_y, boundary))
            {
                current_frame_in_boundary = true;
                currrnt_frame_in_connect_path = true;
                break;
            }
            if (boundary[0].z == 0 && isPositionInBoundary(current_pose.position_x, current_pose.position_y, boundary))
            {
                current_frame_in_boundary = true;
            }
        }
        if (currrnt_frame_in_connect_path && cutter_runing_)
        {
            cutter_runing_ = false;
            ROS_INFO("发送停止刀盘信号 - 当前帧在连接路径上，优先保证连接路径的安全");
        }
        else if (!currrnt_frame_in_connect_path && !cutter_runing_)
        {
            cutter_runing_ = true;
            ROS_INFO("发送启动刀盘信号 - 当前帧不在连接路径上，允许刀盘运行");
        }
        
        if (!current_frame_in_boundary) {
            // 当前帧在边界外，立即发送停车信号并重置计数器
            std_msgs::Bool slow_down;
            slow_down.data = true;
            out_of_bound_pub_.publish(slow_down);
            boundary_false_frame_count = 0; // 重置false帧计数
            last_boundary_stop_state = true;
            ROS_WARN("------------------------------Position out of boundary (%.2f, %.2f) - sending stop signal", 
                    current_pose.position_x, current_pose.position_y);
        } else {
            // 当前帧在边界内
            if (last_boundary_stop_state) {
                // 如果上一帧还在边界停车状态，增加false帧计数
                boundary_false_frame_count++;
                ROS_INFO("Boundary false frame count: %d/5", boundary_false_frame_count);
                
                if (boundary_false_frame_count >= 5) {
                    // 连续5帧都在边界内，发送停止停车信号
                    std_msgs::Bool slow_down;
                    slow_down.data = false;
                    out_of_bound_pub_.publish(slow_down);
                    last_boundary_stop_state = false;
                    boundary_false_frame_count = 0; // 重置计数器
                    ROS_INFO("Sending resume signal - position in boundary for 5 consecutive frames");
                }
            }
        }
    }
}

// 检测路径是否与障碍物碰撞，并根据碰撞点位置设置不同的避障状态
bool AStarPlanner::checkPathCollision()
{
    std::lock_guard<std::mutex> lock(avoiding_mutex_);
    if (path_.poses.empty() || obstacles_.empty()|| is_avoiding_.data == 4)
    {
        return false;
    }

    // 紧急障碍物检测阈值、减速阈值
    int emergency_threshold = ceil(planning_distance_threshold_ / step_size_);
    int slowdown_threshold = ceil((slowdown_threshold_) / step_size_);

    // 记录最近的碰撞点索引
    int earliest_collision_idx = -1;

    // 检查路径点是否与障碍物碰撞
    for (size_t i = 0; i < path_.poses.size(); i++)
    {
        geometry_msgs::Pose path_pose = path_.poses[i].pose;
        double current_yaw = tf2::getYaw(path_pose.orientation);
        double next_yaw = current_yaw + M_PI_2; // 检测碰撞，默认后一个点的航向为当前点的航向角加90度
        if (i + 1 < path_.poses.size() - 1)
        {
            geometry_msgs::Pose next_path_pose = path_.poses[i + 1].pose;
            next_yaw = tf2::getYaw(next_path_pose.orientation);
        }
        for (const auto &obstacle : obstacles_)
        {
            if (intersectsWithObstacle(path_pose.position.x, path_pose.position.y, current_yaw, next_yaw, obstacle))
            {
                // 记录第一个碰撞点的索引
                earliest_collision_idx = i;
                break; // 已找到该点的碰撞，无需检查其他障碍物
            }
        }
        if (earliest_collision_idx != -1)
        {
            // std::cout << getLogTime() << "检测到碰撞点索引: " << earliest_collision_idx << std::endl;
            // 往前找旋转360度都安全的点
            int safe_idx = 0;
            for (int j = earliest_collision_idx - emergency_threshold; j >= 0; --j)
            {
                bool is_safe = true;
                geometry_msgs::Pose temp_pose = path_.poses[j].pose;
                double temp_pose_yaw = tf2::getYaw(path_.poses[j].pose.orientation);
                for (const auto &obstacle : obstacles_)
                {
                    if (intersectsWithObstacle(temp_pose.position.x, temp_pose.position.y, temp_pose_yaw, temp_pose_yaw + M_PI, obstacle) ||
                        intersectsWithObstacle(temp_pose.position.x, temp_pose.position.y, temp_pose_yaw, temp_pose_yaw - M_PI, obstacle))
                    {
                        is_safe = false;
                        break;
                    }
                }
                if (is_safe)
                {
                    safe_idx = j;
                    break;
                }
            }
            emergency_threshold = earliest_collision_idx - safe_idx;
            // std::cout << getLogTime() << "安全点索引：" << safe_idx << " 距离碰撞点的紧急预警点数：" << emergency_threshold << std::endl;
            break; // 已找到碰撞点，退出循环
        }
    }

    if (earliest_collision_idx == -1)
    {
        std_msgs::Bool stop_msg;
        stop_msg.data = false;
        pub_stopflag2.publish(stop_msg);
    }

    if (is_avoiding_.data != 0 && earliest_collision_idx == -1) // 没有检测到碰撞
    {
        if (is_avoiding_.data == 1) // 如果当前在避障执行中（状态1），不切换状态，继续执行避障
        {
            return false;
        }
        // 重置为正常状态
        ROS_INFO("状态切换: %d -> 0(正常状态) - 路径无碰撞，恢复正常行驶", is_avoiding_.data);
        is_avoiding_.data = 0;
        avoidstate_pub.publish(is_avoiding_);
        return false;
    }

    // 根据最近碰撞点的位置设置不同状态
    if (earliest_collision_idx != -1) // TODO
    {
        if (earliest_collision_idx <= emergency_threshold)
        {
            // 如果当前在避障执行中（状态1），检测到非常紧急的障碍物，进行二次避障
            if (is_avoiding_.data == 1)
            {
                if (earliest_collision_idx <= emergency_threshold) // 防止第一次避障和第二次避障同时触发，第二次避障条件紧一些
                {
                    emergency_detection_frames_++;
                    ROS_WARN("Emergency obstacle detected, frame count: %d/%d", emergency_detection_frames_, required_memrgency_frames_ * 2);
                    if (emergency_detection_frames_ >= required_memrgency_frames_ * 2)
                    {
                        // 连续20帧都检测到紧急障碍物，确认紧急状态
                        emergency_confirmed_.store(true);
                        emergency_detection_frames_ = 0;
                        ROS_INFO("状态切换: %d -> 3(紧急预警) - 避障过程中检测到紧急障碍物，准备避障规划", is_avoiding_.data);
                        is_avoiding_.data = 3; // 3表示紧急预警状态
                        avoidstate_pub.publish(is_avoiding_);
                        // 关闭停车信号，控制层会通过紧急状态进行停车
                        std_msgs::Bool stop_msg;
                        stop_msg.data = false;
                        pub_stopflag2.publish(stop_msg);
                        return true; // 触发避障规划
                    }
                    else
                    {
                        std_msgs::Bool stop_msg;
                        stop_msg.data = true;
                        pub_stopflag2.publish(stop_msg);
                        std::cout << getLogTime() << "在避障执行中检测到紧急障碍物，触发停车信号" << std::endl;
                        return false;
                    }
                }
                else
                {
                    std_msgs::Bool stop_msg;
                    stop_msg.data = false;
                    pub_stopflag2.publish(stop_msg);
                    std::cout << getLogTime() << "在避障执行中检测到非紧急障碍物，不触发停车信号" << std::endl;
                    return false; 
                }
            }
            else
            {
                // 进行多帧检测确认
                emergency_detection_frames_++;
                ROS_WARN("Emergency obstacle detected, frame count: %d/%d", 
                            emergency_detection_frames_, required_memrgency_frames_);
                
                if (emergency_detection_frames_ >= required_memrgency_frames_)
                {
                    // 连续10帧都检测到紧急障碍物，确认紧急状态
                    emergency_confirmed_.store(true);
                    emergency_detection_frames_ = 0;
                    if (is_avoiding_.data != 3) {
                        state3_start_time_ = ros::Time::now(); // 记录状态3开始时间
                    }
                    ROS_INFO("状态切换: %d -> 3(紧急预警) - 检测到紧急障碍物，准备避障规划", is_avoiding_.data);
                    is_avoiding_.data = 3; // 3表示紧急预警状态
                    avoidstate_pub.publish(is_avoiding_);
                    // 关闭停车信号，控制层会通过紧急状态进行停车
                    std_msgs::Bool stop_msg;
                    stop_msg.data = false;
                    pub_stopflag2.publish(stop_msg);
                    ROS_WARN("Emergency state confirmed after %d frames - triggering avoidance", 
                                required_memrgency_frames_);
                    return true; // 触发避障规划
                }
                else
                {
                    // 还未达到确认帧数，保持当前状态但增加计数
                    if (is_avoiding_.data != 3) {
                        state3_start_time_ = ros::Time::now(); // 记录状态3开始时间
                        ROS_INFO("状态切换: %d -> 3(紧急预警) - 检测到紧急障碍物第%d帧，等待确认", is_avoiding_.data, emergency_detection_frames_);
                    }
                    ROS_INFO("状态切换: %d -> 3(紧急预警) - 检测到紧急障碍物第%d帧，等待确认", is_avoiding_.data, emergency_detection_frames_);
                    is_avoiding_.data = 3; // 3表示紧急预警状态
                    avoidstate_pub.publish(is_avoiding_);
                    // 关闭停车信号，控制层会通过紧急状态进行停车
                    std_msgs::Bool stop_msg;
                    stop_msg.data = false;
                    pub_stopflag2.publish(stop_msg);
                    return false; // 暂不触发避障规划
                }
            }
        }
        else if (earliest_collision_idx <= slowdown_threshold)
        {
            std_msgs::Bool stop_msg;
            stop_msg.data = false;
            pub_stopflag2.publish(stop_msg);
            // 如果当前在避障执行中（状态1），
            if (is_avoiding_.data == 1)
            {
                std::cout << getLogTime() << "在避障执行中检测到较远障碍物，不会切换避障状态" << std::endl;
                return false;
            }
            // 如果碰撞在11-14个点之间，发出普通预警
            ROS_WARN("Slow down, obstacle ahead");
            ROS_INFO("状态切换: %d -> 2(普通预警) - 检测到较远障碍物，需要减速", is_avoiding_.data);
            is_avoiding_.data = 2; // 2表示普通预警状态
            avoidstate_pub.publish(is_avoiding_);
            // 重置紧急检测计数器
            emergency_detection_frames_ = 0;
            emergency_confirmed_.store(false);
            return false; // 不立即触发避障规划
        }
        else
        {
            std_msgs::Bool stop_msg;
            stop_msg.data = false;
            pub_stopflag2.publish(stop_msg);
            // 如果当前在避障执行中（状态1），
            if (is_avoiding_.data == 1)
            {
                std::cout << getLogTime() << "在避障执行中检测到很远障碍物，不会切换避障状态" << std::endl;
                return false;
            }
            // 碰撞点距离较远，重置紧急检测计数器
            emergency_detection_frames_ = 0;
            emergency_confirmed_.store(false);
            
            // 如果当前是状态2或3但碰撞点很远，切换为正常状态
            if (is_avoiding_.data == 2 || is_avoiding_.data == 3)
            {
                ROS_INFO("状态切换: %d -> 0(正常状态) - 障碍物距离变远，切换为正常状态", is_avoiding_.data);
                is_avoiding_.data = 0; // 切换到正常状态
                avoidstate_pub.publish(is_avoiding_);
            }
            return false;
        }
    }
    else
    {
        // 没有检测到碰撞，重置紧急检测计数器
        emergency_detection_frames_ = 0;
        emergency_confirmed_.store(false);
    }
    return false;
}

double AStarPlanner::cross(const double& ax, const double& ay, const double& bx, const double& by, const double& px, const double& py)
{
    return (bx - ax) * (py - ay)
        - (by - ay) * (px - ax);
}

bool AStarPlanner::isPointInQuadrilateral(const double& px, const double& py, const ObstacleMsg &obstacle)
{
    const double eps = 1e-9;

    double c1 = cross(obstacle.x1, obstacle.y1, obstacle.x2, obstacle.y2, px, py);
    double c2 = cross(obstacle.x2, obstacle.y2, obstacle.x3, obstacle.y3, px, py);
    double c3 = cross(obstacle.x3, obstacle.y3, obstacle.x4, obstacle.y4, px, py);
    double c4 = cross(obstacle.x4, obstacle.y4, obstacle.x1, obstacle.y1, px, py);

    // 四条边的叉积符号全部相同
    bool all_positive =
        (c1 >= -eps &&
         c2 >= -eps &&
         c3 >= -eps &&
         c4 >= -eps);

    bool all_negative =
        (c1 <= eps &&
         c2 <= eps &&
         c3 <= eps &&
         c4 <= eps);

    return all_positive || all_negative;
}
bool AStarPlanner::intersectsWithObstacle(double px, double py, double yaw, double next_yaw, const ObstacleMsg &obstacle)
{
    const double vehicle_length = 1.0;  // 车长，px/py 是车尾中心，不考虑车宽
    
    if (isPointInQuadrilateral(px, py, obstacle)) {
        // std::cout << "车尾中心点在障碍物包围框内"<< std::endl;
        return true; // 车尾中心点在障碍物包围框内
    }

    // 计算车头位置
    double head_x = px + vehicle_length * std::cos(yaw);
    double head_y = py + vehicle_length * std::sin(yaw);
    if (isPointInQuadrilateral(head_x, head_y, obstacle)) {
        // std::cout << "车尾点： (" << px << ", " << py << ")" << std::endl;
        // std::cout << "车头点在障碍物包围框内" << std::endl;
        return true; // 车头点在障碍物包围框内
    }

    // 计算车中心位置
    double center_x = px + (vehicle_length / 2.0) * std::cos(yaw);
    double center_y = py + (vehicle_length / 2.0) * std::sin(yaw);
    if (isPointInQuadrilateral(center_x, center_y, obstacle)) {
        // std::cout << "车尾点： (" << px << ", " << py << ")" << std::endl;
        // std::cout << "车中心点在障碍物包围框内" << std::endl;
        return true; // 车中心点在障碍物包围框内
    }

    // 计算车头扫过的区域
    double delta_yaw = next_yaw - yaw;
    // 归一化
    while (delta_yaw > M_PI) delta_yaw -= 2 * M_PI;
    while (delta_yaw < -M_PI) delta_yaw += 2 * M_PI;
    if (std::fabs(delta_yaw) < 0.1) {
        return false; // 如果角度变化非常小，认为没有扫过区域
    }
    for (double t = 0; t < 1.0; t += 0.1) {
        double interp_yaw = yaw + t * delta_yaw;
        double interp_x = px + vehicle_length * std::cos(interp_yaw);
        double interp_y = py + vehicle_length * std::sin(interp_yaw);
        if (isPointInQuadrilateral(interp_x, interp_y, obstacle)) {
            // std::cout << "车尾点： (" << px << ", " << py << ")" << std::endl;
            // std::cout << "yaw变化: " << yaw << " -> " << next_yaw << ", delta_yaw: " << delta_yaw << std::endl;
            // std::cout << "扫过的点在障碍物包围框内" << std::endl;
            return true; // 扫过的点在障碍物包围框内
        }
    }

    // 计算车中心扫过的区域
    for (double t = 0; t < 1.0; t += 0.1) {
        double interp_yaw = yaw + t * delta_yaw;
        double interp_x = px + (vehicle_length / 2.0) * std::cos(interp_yaw);
        double interp_y = py + (vehicle_length / 2.0) * std::sin(interp_yaw);
        if (isPointInQuadrilateral(interp_x, interp_y, obstacle)) {
            // std::cout << "车尾点： (" << px << ", " << py << ")" << std::endl;
            // std::cout << "yaw变化: " << yaw << " -> " << next_yaw << ", delta_yaw: " << delta_yaw << std::endl;
            // std::cout << "扫过的中心点在障碍物包围框内" << std::endl;
            return true; // 扫过的中心点在障碍物包围框内
        }
    }
    return false;
}

// 从最后一个碰撞点开始向后寻找安全的避障目标点，返回的是栅格坐标
std::pair<int, int> AStarPlanner::findSafeGoalPoint(int collision_idx)
{
    if (path_.poses.empty())
    {
        ROS_WARN("findSafeGoalPoint: current path is empty, cannot select safe goal");
        return {-1, -1};
    }

    // 从第一个碰撞点的下4个点开始寻找安全点
    int goal_idx = collision_idx + 4;

    // 确保终点索引在有效范围内
    if (goal_idx >= static_cast<int>(path_.poses.size()))
    {
        goal_idx = static_cast<int>(path_.poses.size()) - 1;
    }

    // 找到安全的非碰撞的点作为终点
    while (goal_idx < static_cast<int>(path_.poses.size()))
    {
        bool is_safe = true;
        geometry_msgs::Pose path_pose = path_.poses[goal_idx].pose;
        double current_yaw = tf2::getYaw(path_pose.orientation);
        // double next_yaw = current_yaw + 0.75 * M_PI; // 寻找终点，默认后一个点和终点差135度
        for (const auto &obstacle : obstacles_)
        {
            // 对障碍物再进行一次膨胀
            double inflation_distance = inflation_distance_ + 0.1;
            util::Obstacle inflated_obstacle = inflateObstacle(obstacle, inflation_distance);
            if (intersectsWithObstacle(path_pose.position.x, path_pose.position.y, current_yaw, current_yaw + M_PI, inflated_obstacle) || 
                intersectsWithObstacle(path_pose.position.x, path_pose.position.y, current_yaw, current_yaw - M_PI, inflated_obstacle))
            {
                is_safe = false;
                goal_idx += 1;
                break;
            }
        }

        if (is_safe)
        {
            break;
        }
    }

    // 如果找不到安全的终点，使用路径的最后一个点
    if (goal_idx >= static_cast<int>(path_.poses.size()))
    {
        goal_idx = path_.poses.size() - 1;
        ROS_WARN("Could not find safe goal point, using the last path point");
    }

    no_first_avoid_relay_path_.poses.assign(path_.poses.begin() + goal_idx, path_.poses.end());

    const geometry_msgs::Point &goal_point = path_.poses[goal_idx].pose.position;
    goal_point_pub_.publish(goal_point);//TODO
    ROS_INFO("Selected path point %d as goal: (%.2f, %.2f)", goal_idx, goal_point.x, goal_point.y);
    auto [goal_x, goal_y] = worldToGrid(goal_point.x, goal_point.y);
    if (goal_x < 0 || goal_y < 0)
    {
        ROS_WARN("findSafeGoalPoint: selected goal point is outside map bounds, aborting planning");
        return {-1, -1};
    }

    ROS_INFO("Selected path point %d as goal: (%.2f, %.2f) -> (%d, %d)",
                goal_idx, goal_point.x, goal_point.y, goal_x, goal_y);

    return {goal_x, goal_y};
}

std::pair<int, int> AStarPlanner::findSafeGoalPoint2(const nav_msgs::Path &path)
{
    if (path.poses.empty())
    {
        ROS_WARN("findSafeGoalPoint2: dependency path is empty, cannot select safe goal");
        return {-1, -1};
    }

    std::cout << "依赖路径长度：" << static_cast<int>(path.poses.size()) << std::endl;
    // 从第一次避障的终点开始找
    int goal_idx = 0;

    // 找到第一个非碰撞的点作为终点
    while (goal_idx < static_cast<int>(path.poses.size()))
    {
        bool is_safe = true;
        geometry_msgs::Pose path_pose = path.poses[goal_idx].pose;
        double current_yaw = tf2::getYaw(path_pose.orientation);
        // double next_yaw = current_yaw + 0.75 * M_PI; // 寻找终点默认后一个点和终点差135度
        for (const auto &obstacle : obstacles_)
        {
            // 对障碍物再进行一次膨胀
            double inflation_distance = inflation_distance_ + 0.1;
            util::Obstacle inflated_obstacle = inflateObstacle(obstacle, inflation_distance);
            if (intersectsWithObstacle(path_pose.position.x, path_pose.position.y, current_yaw, current_yaw + M_PI, inflated_obstacle) ||
                intersectsWithObstacle(path_pose.position.x, path_pose.position.y, current_yaw, current_yaw - M_PI, inflated_obstacle))
            {
                is_safe = false;
                goal_idx += 1;
                break;
            }
        }

        if (is_safe)
        {
            break;
        }
    }

    // 如果找不到安全的终点，使用路径的最后一个点
    if (goal_idx >= static_cast<int>(path.poses.size()))
    {
        goal_idx = static_cast<int>(path.poses.size()) - 1;
        ROS_WARN("Could not find safe goal point, using the last path point");
    }

    // 更新连续避障依赖的路径
    if (goal_idx + 1 <= static_cast<int>(no_first_avoid_relay_path_.poses.size()))
    {
        no_first_avoid_relay_path_.poses.assign(no_first_avoid_relay_path_.poses.begin() + goal_idx + 1, no_first_avoid_relay_path_.poses.end());
    }

    const geometry_msgs::Point &goal_point = path.poses[goal_idx].pose.position;
    goal_point_pub_.publish(goal_point);
    ROS_INFO("Selected path point %d as goal: (%.2f, %.2f)", goal_idx, goal_point.x, goal_point.y);
    auto [goal_x, goal_y] = worldToGrid(goal_point.x, goal_point.y);
    if (goal_x < 0 || goal_y < 0)
    {
        ROS_WARN("findSafeGoalPoint2: selected goal point is outside map bounds, aborting planning");
        return {-1, -1};
    }

    ROS_INFO("Selected path point %d as goal: (%.2f, %.2f) -> (%d, %d)",
                goal_idx, goal_point.x, goal_point.y, goal_x, goal_y);

    return {goal_x, goal_y};
}

void AStarPlanner::planAndPublishPath()
{
    if (reset_requested_.load(std::memory_order_acquire))
    {
        ROS_WARN("A* planning canceled by reset request before start");
        return;
    }

    // 使用 unique_lock 以便在需要时释放锁，让回调能够运行并更新 current_pose_
    std::unique_lock<std::mutex> lock(avoiding_mutex_);
    if (path_.poses.empty() || obstacles_.empty())   //0926
    {
        ROS_WARN("Path or obstacle data is empty, cannot plan");
        return;
    }
    if (reset_requested_.exchange(false, std::memory_order_acq_rel))
    {
        ROS_WARN("A* planning canceled by reset request after lock acquisition");
        return;
    }
    std::vector<int> collision_indices;
    for (size_t i = 0; i < path_.poses.size(); i++)
    {
        geometry_msgs::Pose path_pose = path_.poses[i].pose;
        double current_yaw = tf2::getYaw(path_pose.orientation);
        double next_yaw = current_yaw + M_PI_2; // 检测碰撞，默认后一个点的航向为当前点的航向加90度
        if (i < path_.poses.size() - 1)
        {
            geometry_msgs::Pose next_path_pose = path_.poses[i + 1].pose;
            next_yaw = tf2::getYaw(next_path_pose.orientation);
        }
        for (const auto &obstacle : obstacles_)
        {
            if (intersectsWithObstacle(path_pose.position.x, path_pose.position.y, current_yaw, next_yaw, obstacle))
            {
                collision_indices.push_back(i);
                break;
            }
        }
    }

    int nearest_collision_idx = -1; // 最近的碰撞点索引
    if (!collision_indices.empty())
    {
        nearest_collision_idx = *std::min_element(collision_indices.begin(), collision_indices.end());
    }

    if (nearest_collision_idx < 0) // 执行到这里来了，应该不可能会出现这种情况******************************
    {
        ROS_WARN("意料之外的情况发生了");
        ROS_INFO("状态切换: %d -> 0(正常状态) - 未找到碰撞点，重置为正常状态", is_avoiding_.data);
        is_avoiding_.data = 0; // 重置为正常状态
        avoidstate_pub.publish(is_avoiding_);
        is_fisrt_avoid_ = true; // 重置第一次避障标志
        return;
    }

    // 在读取当前位置前释放 avoiding_mutex_，允许 poseCallback 运行并更新 current_pose_
    lock.unlock();
    ros::Duration(0.5).sleep();
    lock.lock();
    // 直接使用当前位置作为起点（读取 current_pose_ 时加锁并复制）
    util::Position pose_copy;
    {
        std::lock_guard<std::mutex> lock_pose(current_pose_mutex_);
        pose_copy = current_pose_;
    }
    auto [startx_num, starty_num] = worldToGrid(pose_copy.position_x, pose_copy.position_y);
    int start_x = startx_num;
    int start_y = starty_num;
    int goal_x, goal_y;

    if (start_x < 0 || start_y < 0 || map_width_ <= 0 || map_height_ <= 0)
    {
        ROS_WARN("Current pose is outside the valid map region; aborting A* planning. pose=(%.2f, %.2f), grid=(%d, %d), map=(%d, %d)",
                 pose_copy.position_x, pose_copy.position_y, start_x, start_y, map_width_, map_height_);
        return;
    }
    
    ROS_INFO("Using current position (%.2f, %.2f)-> (%d, %d) as start point",
                pose_copy.position_x, pose_copy.position_y, start_x, start_y);
                
    if (is_fisrt_avoid_) // TODO  第一次避障
    {
        ROS_INFO("First time avoidance - finding safe goal point");
        auto [x_num, y_num] = findSafeGoalPoint(nearest_collision_idx);
        goal_x = x_num;
        goal_y = y_num;
        if (goal_x < 0 || goal_y < 0)
        {
            ROS_WARN("Safe goal point invalid, aborting A* planning");
            return;
        }
        auto [world_goal_x, world_goal_y] = gridToWorld(goal_x, goal_y);
        avoidance_goal_.x = world_goal_x;
        avoidance_goal_.y = world_goal_y;
        avoidance_goal_.z = 0.0;
        is_fisrt_avoid_ = false;
        ROS_WARN("Entering obstacle avoidance mode, goal point: (%.2f, %.2f)-> (%d, %d)",
                    avoidance_goal_.x, avoidance_goal_.y, goal_x, goal_y);
    }
    else
    {
        std::cout << getLogTime() << "触发连续避障" << std::endl;
        auto [x_num, y_num] = findSafeGoalPoint2(no_first_avoid_relay_path_);
        goal_x = x_num;
        goal_y = y_num;
        auto [world_goal_x, world_goal_y] = gridToWorld(goal_x, goal_y);
        avoidance_goal_.x = world_goal_x;
        avoidance_goal_.y = world_goal_y;
        avoidance_goal_.z = 0.0;
    }
    
    // ROS_WARN("Goal point: (%.2f, %.2f) ", avoidance_goal_.x, avoidance_goal_.y);

    // // cnk0811: 添加详细的调试信息
    // ROS_WARN("cnk0811: Start point analysis:");
    ROS_WARN("cnk0811: - World coords: (%.2f, %.2f)", pose_copy.position_x, pose_copy.position_y);
    ROS_WARN("cnk0811: - Grid coords: (%d, %d)", start_x, start_y);
    ROS_WARN("cnk0811: - Is valid: %s", isValidPoint(start_x, start_y) ? "YES" : "NO");

    ROS_WARN("cnk0811: Goal point analysis:");
    ROS_WARN("cnk0811: - World coords: (%.2f, %.2f)", avoidance_goal_.x, avoidance_goal_.y);
    ROS_WARN("cnk0811: - Grid coords: (%d, %d)", goal_x, goal_y);
    ROS_WARN("cnk0811: - Is valid: %s", isValidPoint(goal_x, goal_y) ? "YES" : "NO");

    // 计算起点到终点的直线距离
    double straight_distance = std::sqrt((goal_x - start_x) * (goal_x - start_x) + 
                                        (goal_y - start_y) * (goal_y - start_y));
    ROS_WARN("cnk0811: Grid distance from start to goal: %.2f cells", straight_distance);

    ROS_WARN("cnk0811: Number of obstacles: %zu", obstacles_.size());
    if (!obstacles_.empty()) {
        ROS_WARN("cnk0811: First obstacle center: (%.2f, %.2f)", obstacles_[0].global_x, obstacles_[0].global_y);
    }

    if (!isValidPoint(start_x, start_y) || !isValidPoint(goal_x, goal_y))
    {
        ROS_WARN("Invalid start or goal point, cannot plan");
        return;
    }
    // std::cout<<"goal_x:  "<<goal_x<<" goal_y:   "<<goal_y<<std::endl;
    std::vector<Node> path_nodes = findPath(start_x, start_y, goal_x, goal_y);
    if (path_nodes.empty())
    {
        ROS_WARN("No valid path found");
        callback_counter++; // 第一次没找到路径会再次进入碰撞检测的循环，那时会清理历史障碍物用新的障碍物
        if (callback_counter > 2)
        {
            ROS_INFO("状态切换: %d -> 4(倒车状态) - 无法找到避障路径，开始倒车", is_avoiding_.data);
            is_avoiding_.data = 4;
            state4_start_time_ = ros::Time::now(); // 记录状态4开始时间
            avoidstate_pub.publish(is_avoiding_);
            callback_counter = 0;
            backup_start_position = pose_copy;
            // 不再需要保存避障目标点，倒车后重新寻找碰撞点
        }
        
        return;
    }
    else if(is_avoiding_.data!=1)
    {
        ROS_INFO("状态切换: %d -> 1(避障执行中) - 找到避障路径，开始执行避障", is_avoiding_.data);
        is_avoiding_.data = 1;
        avoidstate_pub.publish(is_avoiding_);
        publishPath(path_nodes);
        ROS_INFO("Path found, publishing");
        clearPath(path_nodes);
        return;
    }
    return;
}

std::vector<Node> AStarPlanner::findPath(int start_x, int start_y, int goal_x, int goal_y)
{
    if (reset_requested_.exchange(false, std::memory_order_acq_rel))
    {
        ROS_WARN("A* search canceled by reset request before starting");
        return {};
    }

    // 每次新的搜索前清理节点池，防止跨搜索残留
    a_star_nodes_.clear();

    // cnk0811: 添加A*搜索开始的调试信息
    ROS_WARN("cnk0811: Starting A* search from (%d, %d) to (%d, %d)", start_x, start_y, goal_x, goal_y);
    
    // 检查起点和终点的有效性
    if (!isValidPoint(start_x, start_y) || !isValidPoint(goal_x, goal_y))
    {
        ROS_ERROR("cnk0811: Invalid start or goal point!");
        return {};
    }

    // cnk0811: 添加地图信息调试
    ROS_WARN("cnk0811: Map bounds - width: %d, height: %d", map_width_, map_height_);
    ROS_WARN("cnk0811: Map world bounds - x: [%.2f, %.2f], y: [%.2f, %.2f]",
                map_min_x_, map_max_x_, map_min_y_, map_max_y_);

    // 创建起点和终点节点
    Node start_node(start_x, start_y);
    Node goal_node(goal_x, goal_y);
    std::priority_queue<std::size_t, std::vector<std::size_t>, NodeIndexCompare> open_set{NodeIndexCompare(&a_star_nodes_)};
    std::unordered_map<int, std::unordered_map<int, std::size_t>> closed_set;
    a_star_nodes_.emplace_back(start_x, start_y);
    std::size_t start_index = a_star_nodes_.size() - 1;
    a_star_nodes_[start_index].g_cost = 0;
    a_star_nodes_[start_index].h_cost = calculateHeuristic(a_star_nodes_[start_index], goal_node);
    a_star_nodes_[start_index].f_cost = a_star_nodes_[start_index].g_cost + a_star_nodes_[start_index].h_cost;

    open_set.push(start_index);
    const int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    const int dy[8] = {1, 1, 0, -1, -1, -1, 0, 1};

    // cnk0811: 添加A*循环调试信息
    ROS_WARN("cnk0811: Starting A* loop");
    int iterations = 0;
    const int max_iterations = 5000;

    // A*主循环
    while (!open_set.empty() && iterations < max_iterations)
    {
        if (reset_requested_.load(std::memory_order_acquire))
        {
            if (reset_requested_.exchange(false, std::memory_order_acq_rel))
            {
                ROS_WARN("A* search canceled due to reset request during expansion");
                return {};
            }
        }

        iterations++;
        std::size_t current_index = open_set.top();
        open_set.pop();

        // 关键修正：先拷贝当前节点，避免后续 emplace_back 导致 vector 重分配后引用失效
        const Node current_node = a_star_nodes_[current_index];
        const int current_x = current_node.x;
        const int current_y = current_node.y;

        // cnk0811: 每1000次迭代打印进度
        if (iterations % 1000 == 0) {
            ROS_WARN("cnk0811: A* iteration %d, current: (%d, %d), open_set size: %zu",
                        iterations, current_x, current_y, open_set.size());
        }
        
        if (current_x == goal_x && current_y == goal_y)
        {
            std::cout << "Arrived at goal: (" << current_x << ", " << current_y << ")" << std::endl; // TODO
            ROS_WARN("cnk0811: Path found after %d iterations!", iterations);
            return reconstructPath(current_index);
        }
        closed_set[current_y][current_x] = current_index;

        for (int i = 0; i < 8; i++)
        {
            int pre_x = current_x;
            int pre_y = current_y;
            if (current_node.parent_index >= 0 && static_cast<std::size_t>(current_node.parent_index) < a_star_nodes_.size())
            {
                pre_x = a_star_nodes_[current_node.parent_index].x;
                pre_y = a_star_nodes_[current_node.parent_index].y;
            }
            int nx = current_x + dx[i]; 
            int ny = current_y + dy[i];
            if (!isValidPoint(nx, ny) || isCollision(pre_x, pre_y, current_x, current_y, nx, ny) || (closed_set.count(ny) && closed_set[ny].count(nx)))
            {
                continue;
            }
            a_star_nodes_.emplace_back(nx, ny);
            std::size_t neighbor_index = a_star_nodes_.size() - 1;
            Node &neighbor = a_star_nodes_[neighbor_index];
            double step_cost = (i % 2 == 0) ? 1.0 : 1.4; // 直线代价 1.0，对角代价 1.4
            double tentative_g_cost = current_node.g_cost + step_cost;
            int new_direction = i;
            bool is_turn = (current_node.direction != -1 && current_node.direction != new_direction);
            if (is_turn)
            {
                tentative_g_cost += 2.0; // 转向惩罚
            }
            neighbor.g_cost = tentative_g_cost;
            neighbor.h_cost = calculateHeuristic(neighbor, goal_node);
            neighbor.f_cost = neighbor.g_cost + neighbor.h_cost;
            neighbor.parent_index = static_cast<int>(current_index);
            neighbor.direction = new_direction;
            neighbor.turn_count = current_node.turn_count + (is_turn ? 1 : 0);

            // 添加到open set
            open_set.push(neighbor_index);
        }
    }
    
    ROS_ERROR("cnk0811: A* failed after %d iterations! Open set empty: %s", 
                iterations, open_set.empty() ? "YES" : "NO");
    return std::vector<Node>();
}

double AStarPlanner::calculateHeuristic(const Node &current, const Node &goal)
{
    // 使用欧几里得距离作为启发式函数，减少沿边界长折线绕路
    const double dx = static_cast<double>(current.x - goal.x);
    const double dy = static_cast<double>(current.y - goal.y);
    return std::sqrt(dx * dx + dy * dy);
}

// x,y是被判断是否安全的点
bool AStarPlanner::isCollision(int pre_pre_x, int pre_pre_y, int pre_x, int pre_y, int x, int y)
{
    double vehicle_length = 0.9;
    auto [pre_pre_world_x, pre_pre_world_y] = gridToWorld(pre_pre_x, pre_pre_y);
    auto [pre_world_x, pre_world_y] = gridToWorld(pre_x, pre_y);
    auto [world_x, world_y] = gridToWorld(x, y);
    double pre_dx = pre_world_x - pre_pre_world_x;
    double pre_dy = pre_world_y - pre_pre_world_y;
    double pre_yaw = std::atan2(pre_dy, pre_dx);
    double dx = world_x - pre_world_x;
    double dy = world_y - pre_world_y;
    double yaw = std::atan2(dy, dx);
    while (pre_yaw > M_PI){pre_yaw -= 2 * M_PI;}
    while (pre_yaw < -M_PI){pre_yaw += 2 * M_PI;}
    while (yaw > M_PI){yaw -= 2 * M_PI;}
    while (yaw < -M_PI){yaw += 2 * M_PI;}
    // 计算车头位置
    double world_head_x = world_x + vehicle_length * std::cos(yaw);
    double world_head_y = world_y + vehicle_length * std::sin(yaw);
    auto [head_x, head_y] = worldToGrid(world_head_x, world_head_y);
    // 检查是否是有效点
    if (!isValidPoint(x, y) || !isValidPoint(head_x, head_y))
    {
        return true;
    }

    if (x >= 0 && x < map_width_ && y >= 0 && y < map_height_)
    {
        if (grid_map_[y][x])
        {
            return true;
        }
    }
    if (head_x >= 0 && head_x < map_width_ && head_y >= 0 && head_y < map_height_)
    {
        if (grid_map_[head_y][head_x])
        {
            return true;
        }
    }

    // 检查检索的避障路径点是否会与障碍物发生碰撞

    for (const auto &obstacle : obstacles_)
    {
        // 对障碍物再进行一次膨胀
        double inflation_distance = inflation_distance_ + 0.1;
        util::Obstacle inflated_obstacle = inflateObstacle(obstacle, inflation_distance);
        if (intersectsWithObstacle(world_x, world_y, pre_yaw, yaw, inflated_obstacle))
        {
            return true;
        }
    }

    return false;
}

bool AStarPlanner::isValidPoint(int x, int y)
{
    if (x < 0 || x >= map_width_ || y < 0 || y >= map_height_)
    {
        return false;
    }
    auto [world_x, world_y] = gridToWorld(x, y);

    // 不再检查格子坐标，而是检查世界坐标在合理范围
    return (world_x >= map_min_x_ - map_width_ * resolution_ &&
            world_x <= map_max_x_ + map_width_ * resolution_ &&
            world_y >= map_min_y_ - map_height_ * resolution_ &&
            world_y <= map_max_y_ + map_height_ * resolution_);
}

std::vector<Node> AStarPlanner::reconstructPath(std::size_t goal_index) const
{
    std::vector<Node> path;
    std::size_t current_index = goal_index;

    while (current_index < a_star_nodes_.size())
    {
        path.push_back(a_star_nodes_[current_index]);
        int parent_index = a_star_nodes_[current_index].parent_index;
        if (parent_index < 0)
        {
            break;
        }
        current_index = static_cast<std::size_t>(parent_index);
    }

    std::reverse(path.begin(), path.end());
    if (path.size() > 1) // TODO 删除起点
    {
        path.erase(path.begin());
        path.erase(path.end() - 1);
    }
    return path;
}

void AStarPlanner::publishPath(const std::vector<obstacle_avoidance::Node> &path)
{
    if (path.empty())
    {
        ROS_INFO("[publishPath]: Path is empty!");
        return;
    }
    // TODO: Yeager.
    util::LocalPath global_path_temp;
    global_path_temp.pathtype = avoid_counter;
    global_path_temp.x.reserve(path.size()+1);
    global_path_temp.y.reserve(path.size()+1);
    global_path_temp.heading.reserve(path.size()+1);
    global_path_temp.gear.reserve(path.size()+1);
    global_path_temp.speed.reserve(path.size()+1);

    // 先保存x、y
    for (const auto &node : path)
    {
        std::pair<double, double> point = gridToWorld(node.x, node.y);
        global_path_temp.x.push_back(point.first);
        global_path_temp.y.push_back(point.second);
    }
    // 加上避障终点
    global_path_temp.x.push_back(avoidance_goal_.x);
    global_path_temp.y.push_back(avoidance_goal_.y);

    // 再计算heading
    for (size_t i = 0; i < global_path_temp.x.size() - 1; ++i)
    {
        double dx = global_path_temp.x[i + 1] - global_path_temp.x[i];
        double dy = global_path_temp.y[i + 1] - global_path_temp.y[i];
        if (std::fabs(dx) < 1e-3)
            dx = 0.0;
        if (std::fabs(dy) < 1e-3)
            dy = 0.0;
        float heading = std::atan2(dy, dx);

        // 确保航向在[-π, π]范围内
        if (heading > M_PI)
            heading -= 2 * M_PI;
        else if (heading < -M_PI)
            heading += 2 * M_PI;

        if (heading > M_PI)
            heading -= 2 * M_PI;
        else if (heading < -M_PI)
            heading += 2 * M_PI;

        global_path_temp.heading.push_back(heading);
    }

    // 最后一个点补一个heading
    if (!global_path_temp.heading.empty())
    {
        global_path_temp.heading.push_back(global_path_temp.heading.back());
    }
    else
    {
        global_path_temp.heading.push_back(0.0f);
    }

    for (int i = 0; i < global_path_temp.x.size(); ++i)
    {
        int gear = 1;
        float speed = 0.25;
        if (i == 0)
        {
            gear = 2; // 第一个点设为档位2
            speed = 0.0;
        }
        else
        {
            float heading = global_path_temp.heading[i];
            float ahead_heading = global_path_temp.heading[i - 1];

            float heading_diff = std::fabs(heading - ahead_heading);
            if (heading_diff > M_PI)
            {
                heading_diff = 2 * M_PI - heading_diff;
            }
            if (heading_diff > 0.2 && heading_diff < (M_PI - 0.6)) //11度到145度之间，设为转弯点，速度降低
            {
                gear = 2;
                speed = 0.0;
            }
        }
        global_path_temp.gear.push_back(gear);
        global_path_temp.speed.push_back(speed);
    }
    if(global_path_temp.x.size() > 3)     //cnk 0515
    {
        local_path_pub_.publish(global_path_temp);
        ROS_INFO("[publishPath]: Path published successfully.");
        end_point_.x=global_path_temp.x.back();
        end_point_.y=global_path_temp.y.back();
        end_point_.z=global_path_temp.heading.back(); 
    }
    else
    {
        ROS_INFO("状态切换: %d -> 0(正常状态) - 避障路径过短，重置为正常状态", is_avoiding_.data);
        is_avoiding_.data=0; // 重置避障状态
        avoidstate_pub.publish(is_avoiding_);
    }
}

void AStarPlanner::clearPath(std::vector<obstacle_avoidance::Node> &path) // TODO cnk
{
    // 路径结果为值对象，不需要手动 delete，直接清空即可。
    path.clear();
}

std::pair<int, int> AStarPlanner::worldToGrid(double x, double y)
{
    if (map_width_ <= 0 || map_height_ <= 0 || !std::isfinite(map_min_x_) || !std::isfinite(map_min_y_) ||
        !std::isfinite(map_max_x_) || !std::isfinite(map_max_y_) || resolution_ <= 0.0)
    {
        ROS_WARN("worldToGrid: map is not initialized or invalid; x=%.3f, y=%.3f, map=(%d,%d)",
                 x, y, map_width_, map_height_);
        return {-1, -1};
    }

    // 考虑地图原点的世界坐标到栅格坐标转换
    int grid_x = std::floor((x - map_min_x_) / resolution_);
    int grid_y = std::floor((y - map_min_y_) / resolution_);
    if (grid_x < 0 || grid_x >= map_width_ || grid_y < 0 || grid_y >= map_height_)
    {
        ROS_WARN("worldToGrid: (%f, %f) is outside map bounds -> grid=(%d, %d), map=(%d, %d)",
                 x, y, grid_x, grid_y, map_width_, map_height_);
        return {-1, -1}; // 返回无效坐标
    }
    return {grid_x, grid_y};
}

std::pair<double, double> AStarPlanner::gridToWorld(int x, int y)
{
    // 考虑地图原点的栅格坐标到世界坐标转换
    double world_x = map_min_x_ + (x + 0.5) * resolution_;
    double world_y = map_min_y_ + (y + 0.5) * resolution_;
    return {world_x, world_y};
}



bool AStarPlanner::checkAvoidanceCompletion(const util::Position& current_pose)
{
    std::lock_guard<std::mutex> lock2(avoidfinish_mutex_);
    if (is_avoiding_.data != 1)
    {
        return false; // 如果不在避障中，则直接返回
    }

    if(avoidfinish_flag_)
    {
        ROS_INFO("状态切换: 1(避障执行中) -> 0(正常状态) - 避障完成标志触发，恢复正常行驶");
        is_avoiding_.data = 0; // 重置避障状态
        avoidstate_pub.publish(is_avoiding_);
        is_fisrt_avoid_ = true; // 重置第一次避障标志
        avoidfinish_flag_ = false;
        return true;
    }

    double error_heading = current_pose.yaw - end_point_.z;
    if (error_heading > M_PI)
        error_heading -= 2 * M_PI;
    else if (error_heading < -M_PI)
        error_heading += 2 * M_PI;
    double distance = std::sqrt(std::pow(current_pose.position_x - end_point_.x, 2) +
                                std::pow(current_pose.position_y - end_point_.y, 2));

    // TODO: @Yeager.
    // bool is_stable_ = positionIsStable(distance);
    if ((distance <= avoidance_completion_dis_threshold_ && fabs(error_heading) < avoidance_completion_ang_threshold_) )    // 避障完成条件**********************
    {
        ROS_INFO("Obstacle avoidance completed. Distance to goal: %.2f m", distance);
        ROS_INFO("状态切换: 1(避障执行中) -> 0(正常状态) - 避障完成，到达目标点");
        is_avoiding_.data = 0; // 重置避障状态
        avoidstate_pub.publish(is_avoiding_);
        is_fisrt_avoid_ = true; // 重置第一次避障标志
        return true;
    }
    else if (distance <= 0.3)
    {
        // 更稳健的车辆静止判断：使用局部 pose、明确初始化静态变量、并在时间窗口到达时判断位移
        static ros::Time last_check_time = ros::Time(0);
        static double last_x = std::numeric_limits<double>::quiet_NaN();
        static double last_y = std::numeric_limits<double>::quiet_NaN();
        ros::Time now = ros::Time::now();

        // 首次进入时初始化参考位姿和时间
        if (std::isnan(last_x) || std::isnan(last_y) || last_check_time.isZero()) {
            last_x = current_pose.position_x;
            last_y = current_pose.position_y;
            last_check_time = now;
            return false;
        }

        double time_diff = (now - last_check_time).toSec();
        if (time_diff >= 5) {
            double pos_change = std::hypot(current_pose.position_x - last_x,
                                            current_pose.position_y - last_y);
            // 如果5秒内位置变化小于0.05米，认为车辆静止**********************
            if (pos_change < 0.05) {
                ROS_INFO("Obstacle avoidance completed. Vehicle stopped near goal.");
                ROS_INFO("状态切换: 1(避障执行中) -> 0(正常状态) - 车辆静止，避障完成");
                is_avoiding_.data = 0;
                avoidstate_pub.publish(is_avoiding_);
                is_fisrt_avoid_ = true;

                // 重置参考位置，用于下一次比较
                last_x = std::numeric_limits<double>::quiet_NaN();
                last_y = std::numeric_limits<double>::quiet_NaN();
                last_check_time = ros::Time(0);

                return true;
            }
        }
    }
    return false;
}


// cnk0811: 添加signal话题回调函数
void AStarPlanner::signalCallback(const std_msgs::String::ConstPtr& msg)
{
    const std::string& data = msg->data;
    
    if (data == "reset") 
    {
        reset();
        std::cout << getLogTime() << "避障节点重置" << std::endl;
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
    else if(data == "borderprotection_true")
    {
        boundary_check_enabled_=true;
        std::cout << getLogTime() << "边界检测已开启" << std::endl;
    }
    // else if(data == "borderprotection_false")
    // {
    //     boundary_check_enabled_=false;
    //     std::cout << getLogTime() << "边界检测已关闭" << std::endl;
    // }
}
void AStarPlanner::reset()
{
    reset_requested_.store(true, std::memory_order_release);

    // 不在这里等待避免锁，避免与正在执行的 A* 搜索发生互相阻塞导致卡死。
    std::unique_lock<std::mutex> avoid_lock(avoiding_mutex_, std::try_to_lock);
    if (avoid_lock.owns_lock())
    {
        ROS_INFO("Resetting A* planner to initial state...");
        ROS_INFO("状态切换: %d -> 0(正常状态) - 系统重置，恢复初始状态", is_avoiding_.data);
        is_avoiding_.data = 0;
        avoidstate_pub.publish(is_avoiding_);

        // 清空地图
        map_width_ = 0;
        map_height_ = 0;
        map_min_x_ = map_min_y_ = std::numeric_limits<double>::max();
        map_max_x_ = map_max_y_ = std::numeric_limits<double>::lowest();
        grid_map_.clear();
        last_map_boundary_.clear();

        // 重置标志位
        is_fisrt_avoid_ = true;
        is_stable_ = false;
        callback_counter = 0;
        avoid_counter = -1;

        // 重置多帧检测相关变量
        emergency_detection_frames_ = 0;
        emergency_confirmed_.store(false);

        // 清空路径和障碍物数据
        map_boundarys_.clear();
        path_.poses.clear();
        path_.header = std_msgs::Header();
        obstacles_.clear();

        // 重置路径点数据
        clearLocalPath(path_points_);
        clearLocalPath(avoidance_path_points_);

        // 清空位置历史数据
        last_positions_.clear();

        ROS_INFO("A* planner state reset to initial values completed");
        reset_requested_.store(false, std::memory_order_release);
    }
    else
    {
        ROS_WARN("A* planner is busy; reset request queued and will cancel the active search");
    }

    // 如果障碍物 mutex 可用，则尽快清空障碍物列表；如果正在被回调持有，就让当前回调自然退出即可。
    std::unique_lock<std::mutex> obstacle_lock(obstacles_mutex_, std::try_to_lock);
    if (obstacle_lock.owns_lock())
    {
        obstacles_.clear();
    }
}

// cnk0811: 添加清空局部路径的辅助函数
void AStarPlanner::clearLocalPath(util::LocalPath& path)
{
    path.x.clear();
    path.y.clear();
    path.heading.clear();
    path.gear.clear();
    path.speed.clear();
    path.pathtype = 0;
}
} // namespace obstacle_avoidance

std::string getVersion()
{
    std::stringstream strs;
    std::string ss;
    strs << VERSION << "_2025/05/06_1043";
    ss = strs.str();
    return ss;
}
