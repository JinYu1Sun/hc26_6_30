#include "location_map.h"
#include <iostream>
#include <cfloat>
#include <cmath>
#include <std_msgs/Float64MultiArray.h>
#include <vector>
#include <algorithm>
#include <ros/package.h>
#include <filesystem>
#include <unistd.h>    // for fork(), execl(), setsid(), sleep()
#include <sys/types.h> // for pid_t
#include <signal.h>    // for SIGTERM
#include <sys/wait.h>  // for waitpid()

#define EPSILON 0.000001
#define SAME_POINT 1
#define DEG2RAD M_PI / 180

// 信号位定义：0-开始录制边界，1-开始录制洞，2-开始录制路径，3-开始录制停车位，4-开始工作，5-删除地图，6-保存地图
std::bitset<7> LocationMap::signal;
namespace fs = std::filesystem;

// 添加多地图处理相关的静态成员变量
// TODO: 多地图路径拼接功能 - 简化的多地图处理变量定义 cnk 0805
int LocationMap::current_map_index_ = -1;   // 多地图处理过程中，地图的索引
bool LocationMap::is_multi_map_processing_ = false; // 多地图处理进行中标志
geometry_msgs::Point32 LocationMap::current_start_point_;   // 多地图处理过程中，当前地图用于规划全覆盖路径的起始点
geometry_msgs::Point32 LocationMap::current_end_point_;     // 多地图处理过程中，当前地图用于规划全覆盖路径的终点

namespace
{
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
    bool isPointOnLine(double px0, double py0, double px1, double py1, double px2, double py2)
    {
        bool flag = false;
        double d1 = (px1 - px0) * (py2 - py0) - (px2 - px0) * (py1 - py0);
        if ((abs(d1) < EPSILON) && ((px0 - px1) * (px0 - px2) <= 0) && ((py0 - py1) * (py0 - py2) <= 0))
        {
            flag = true;
        }
        return flag;
    }
    bool isIntersect(double px1, double py1, double px2, double py2, double px3, double py3, double px4, double py4)
    {
        bool flag = false;
        double d = (px2 - px1) * (py4 - py3) - (py2 - py1) * (px4 - px3);
        if (d != 0)
        {
            double r = ((py1 - py3) * (px4 - px3) - (px1 - px3) * (py4 - py3)) / d;
            double s = ((py1 - py3) * (px2 - px1) - (px1 - px3) * (py2 - py1)) / d;
            if ((r >= 0) && (r <= 1) && (s >= 0) && (s <= 1))
            {
                flag = true;
            }
        }
        return flag;
    }
    /*判断是否在包围框中*/
    bool inBoudingBox(const geometry_msgs::Point32 &point, LocationMap::map_hull &polygon)
    {
        if (point.x < polygon.xmin || point.x > polygon.xmax || point.y < polygon.ymin || point.y > polygon.ymax)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

} // namespace
LocationMap::LocationMap()
{
    nh = new ros::NodeHandle("~");
    nh->param("hull_name", hull_name_topic, std::string("/hull_name"));
    nh->param("filename", filename, std::string("./"));
    nh->param("android_signal_topic", android_signal_topic, std::string("/signal"));
    nh->param("localization_topic", localization_topic, std::string("/Mower/position"));
    nh->param("map_hull_topic", map_hull_topic, std::string("/send_hull_info"));
    nh->param("map_hole_topic", map_hole_topic, std::string("/send_hole_info"));
    nh->param("map_path_topic", map_path_topic, std::string("/send_path_info"));
    nh->param("map_point_topic", map_point_topic, std::string("/send_point_info"));
    nh->param("planning_hull_topic", planning_hull_topic, std::string("/planning_hull_info"));
    nh->param("planning_path_topic", planning_path_topic, std::string("/planning_path_info"));
    nh->param("planning_point_topic", planning_point_topic, std::string("/clicked_point"));
    nh->param("planning_status_topic", planning_status_topic, std::string("/planning_status"));
    nh->param("vehicle_info_topic", vehicle_info_topic, std::string("/vehicle/info_test"));
    nh->param("map_name_topic", map_name_topic, std::string("/map_name"));
    nh->param("planning_ready_topic", planning_ready_topic, std::string("/planning_ready")); // 新增：planning准备就绪话题
    nh->param<double>("polygon_thr", polygon_thr, 6);
    nh->param<double>("path_thr", path_thr, 0.5);

    get_signal_sub = nh->subscribe<std_msgs::String>(android_signal_topic, 1, &LocationMap::GetSignalCallback, this);
    localization_sub = nh->subscribe<util::Position>(localization_topic, 1, &LocationMap::StatusCallback, this);
    planning_ready_sub = nh->subscribe<std_msgs::String>(planning_ready_topic, 1, &LocationMap::PlanningReadyCallback, this); // 新增：订阅planning准备就绪信号

    map_hull_pub = nh->advertise<geometry_msgs::Polygon>(map_hull_topic, 10, true);  // 发布边界多边形，由避障节点订阅来设置避障规划用的栅格地图，UI节点订阅显示
    map_hole_pub = nh->advertise<geometry_msgs::Polygon>(map_hole_topic, 1, true);  // 发布录制的洞多边形，是Android端需要用到
    map_path_pub = nh->advertise<util::MapPath>(map_path_topic, 1, true);           // 发布录制的路线，是Android端需要用到
    map_point_pub = nh->advertise<geometry_msgs::Point32>(map_point_topic, 1, true);// 发布录制的停车点数据，是Android端需要用到
    planning_hull_pub = nh->advertise<util::PolygonWithHolesStamped>(planning_hull_topic, 10);  // 发布边界多边形和洞，由全覆盖节点订阅来规划全覆盖路径
    planning_path_pub = nh->advertise<util::MapPath>(planning_path_topic, 1, true); // 准备用来发布录制的轨迹，由global_planning节点订阅，暂时无用********************
    planning_point_pub = nh->advertise<geometry_msgs::PointStamped>(planning_point_topic, 10);
    vehicle_info_pub = nh->advertise<util::VehicleInfoTest>(vehicle_info_topic, 10);    // 暂时无用********************
    name_pub = nh->advertise<std_msgs::Float64MultiArray>(hull_name_topic, 1, true);    // 发布地图中包含的边界名称列表，时无用********************
    file_pub = nh->advertise<std_msgs::String>(map_name_topic, 1, true);    // 发布本地保存的地图文件名列表
    hull_tag = 0;
    tmp_path.name = 0;
    // target_name = 0;
    hulls.clear(); // 清空之前的内容
    single_map = false;
    multi_map = false;
    // use_map = 0;
    delete_name = 0;
    std::string base_file;
    // first_recv_gps = true;
    // 延迟初始化，让ROS节点有时间完全启动
    ros::Duration(1.0).sleep();
    // 初始化后立即处理p_mode，加载所有地图名称
    publishMapList();

    ROS_INFO("LocationMap initialized with interactive multi-map support");
    ROS_INFO("Listening for planning ready signals on topic: %s", planning_ready_topic.c_str());
}
void LocationMap::reset()
{
    geometry_msgs::Polygon empty_hull;
    map_hull_pub.publish(empty_hull);
    map_hole_pub.publish(empty_hull);
    planning_status = false;
    single_map = false;
    multi_map = false;
    // use_map = 0;
    delete_name = 0;
    hull_tag = 0;
    point_stop = geometry_msgs::Point32();
    lastPoint = geometry_msgs::Point32();
    hulls.clear();
    paths.clear();
    yaml_hulls.clear();
    // yaml_paths.clear();
    tmp_hull = LocationMap::map_hull();
    last_hull = LocationMap::map_hull();
    tmp_path = LocationMap::map_path();
    signal.reset();
}

// 发布所有本地的可用地图名
void LocationMap::publishMapList()
{
    std::vector<std::string> file_names;
    try
    {
        for (const auto &entry : fs::directory_iterator(filename))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".yaml")
            {
                file_names.push_back(entry.path().filename().string());
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
        ROS_ERROR("Error occurred while traversing the directory: %s", e.what());
        return;
    }

    std_msgs::String file_msg;
    std::string file_list_str;
    for (const auto &file_name : file_names)
    {
        std::string file_pre = file_name.substr(0, file_name.find('.'));
        file_list_str += file_pre + "/";
    }
    file_msg.data = file_list_str;
    ros::Duration(0.1).sleep();
    file_pub.publish(file_msg);
    ROS_INFO("The saved map list: %s", file_list_str.c_str());
}
bool LocationMap::loadMap(const std::string &map_file)
{
    try
    {
        yaml_hulls.clear();
        // yaml_paths.clear();
        std::set<int> hull_names;

        // 检查文件是否存在
        std::string full_path = filename + map_file;
        std::ifstream file_check(full_path);
        if (!file_check.good())
        {
            ROS_ERROR("Map file does not exist.: %s", full_path.c_str());
            return false;
        }
        file_check.close();

        // 加载YAML文件
        YAML::Node polygon_with_path;
        try
        {
            polygon_with_path = YAML::LoadFile(full_path);
        }
        catch (const YAML::Exception &e)
        {
            ROS_ERROR("YAML format error.: %s", e.what());
            return false;
        }

        // 检查必要的节点是否存在
        if (!polygon_with_path["Polygons"] || !polygon_with_path["Polygons"]["polygon"])
        {
            ROS_ERROR("Map file format error: missing Polygons.polygon node.");
            return false;
        }

        // 解析多边形数据
        for (const auto &m_hul : polygon_with_path["Polygons"]["polygon"])
        {
            try
            {
                map_hull single_hull;
                single_hull.iscomplete = false;

                if (!m_hul["name"])
                {
                    ROS_WARN("Polygon is missing the 'name' field, skipping.");
                    continue;
                }
                single_hull.name = m_hul["name"].as<int>();
                hull_names.insert(single_hull.name);
                
                if (!m_hul["brd"])
                {
                    ROS_WARN("Polygon %d missing boundary points.", single_hull.name);
                    continue;
                }
                // 设置边界信息
                if (m_hul["x_min"] && m_hul["x_max"] && m_hul["y_min"] && m_hul["y_max"])
                {
                    single_hull.xmin = m_hul["x_min"].as<double>();
                    single_hull.xmax = m_hul["x_max"].as<double>();
                    single_hull.ymin = m_hul["y_min"].as<double>();
                    single_hull.ymax = m_hul["y_max"].as<double>();
                }
                else
                {
                    ROS_WARN("Polygon %d missing boundary", single_hull.name);
                    // 设置默认值
                    single_hull.xmin = single_hull.ymin = std::numeric_limits<double>::max();
                    single_hull.xmax = single_hull.ymax = std::numeric_limits<double>::lowest();
                }

                // 解析边界点
                if (m_hul["brd"])
                {
                    for (const auto &brd : m_hul["brd"])
                    {
                        geometry_msgs::Point32 point;
                        point.x = brd["x"].as<double>();
                        point.y = brd["y"].as<double>();
                        point.z = brd["z"].as<double>();
                        single_hull.polygon_with_holes.hull.points.emplace_back(point);
                    }
                }

                // 解析洞
                if (m_hul["holes"])
                {
                    for (const auto &hole : m_hul["holes"])
                    {
                        geometry_msgs::Polygon hole_;
                        for (const auto &hp : hole)
                        {
                            geometry_msgs::Point32 hole_point_;
                            hole_point_.x = hp["x"].as<double>();
                            hole_point_.y = hp["y"].as<double>();
                            hole_point_.z = hp["z"].as<double>();
                            hole_.points.emplace_back(hole_point_);
                        }
                        single_hull.polygon_with_holes.holes.emplace_back(hole_);
                    }
                }

                // TODO: 多地图路径拼接功能 - 直接在hull中存储连接路径信息 cnk 0805
                // 解析连接路径点，直接存储在hull结构中
                single_hull.connection_path.clear(); // 清空连接路径
                if (m_hul["path"])
                {
                    ROS_INFO("Found path data for map %d", single_hull.name);
                    for (const auto &hull_path_point : m_hul["path"])
                    {
                        geometry_msgs::Point32 point;
                        point.x = hull_path_point["x"].as<double>();
                        point.y = hull_path_point["y"].as<double>();
                        point.z = hull_path_point["z"].as<double>();
                        single_hull.connection_path.emplace_back(point);
                        ROS_DEBUG("  Path point: (%.3f, %.3f, %.3f)", point.x, point.y, point.z);
                    }
                    ROS_INFO("Loaded connection path for map %d with %zu points",
                             single_hull.name, single_hull.connection_path.size());
                }
                else
                {
                    ROS_WARN("No path data found for map %d", single_hull.name);
                }

                yaml_hulls.emplace_back(single_hull);
            }
            catch (const std::exception &e)
            {
                ROS_ERROR("Error occurred while parsing polygon data.: %s", e.what());
                // 继续处理下一个多边形
                continue;
            }
        }

        // 发布多边形名称
        std_msgs::Float64MultiArray names_msg;
        for (const auto &name : hull_names)
        {
            names_msg.data.push_back(name);
            ROS_INFO("The map contains polygons: %d", name);
        }
        name_pub.publish(names_msg);

        return true;
    }
    catch (const std::exception &e)
    {
        ROS_ERROR("An unhandled exception occurred while loading the map.: %s", e.what());
        return false;
    }
}

LocationMap::~LocationMap()
{
    delete nh;
}
std::vector<LocationMap::Point2d> LocationMap::douglasPeucker(const std::vector<LocationMap::Point2d> &points, double epsilon)
{
    if (points.size() < 2)
    {
        throw std::invalid_argument("A curve must contain at least two points.");
    }

    // Find the point with the maximum distance
    double dmax = 0.0;
    size_t index = 0;
    for (size_t i = 1; i < points.size() - 1; ++i)
    {
        double d = perpendicularDistance(points[i], points[0], points.back());
        if (d > dmax)
        {
            index = i;
            dmax = d;
        }
    }
    if (dmax > epsilon)
    {
        std::vector<LocationMap::Point2d> recResults1 = douglasPeucker(std::vector<LocationMap::Point2d>(points.begin(), points.begin() + index + 1), epsilon);
        std::vector<LocationMap::Point2d> recResults2 = douglasPeucker(std::vector<LocationMap::Point2d>(points.begin() + index, points.end()), epsilon);

        // Build the result list
        recResults1.pop_back(); // Remove the last point of the first list as it is duplicated in the second list
        recResults1.insert(recResults1.end(), recResults2.begin(), recResults2.end());
        return recResults1;
    }
    else
    {
        // If max distance is less than epsilon, return the original curve with its endpoints
        return {points[0], points.back()};
    }
}

// Helper function to calculate the perpendicular distance from a point to a line segment
double LocationMap::perpendicularDistance(const LocationMap::Point2d &point, const LocationMap::Point2d &lineStart, const LocationMap::Point2d &lineEnd)
{
    double numerator = std::abs((lineEnd.y - lineStart.y) * point.x - (lineEnd.x - lineStart.x) * point.y +
                                lineEnd.x * lineStart.y - lineEnd.y * lineStart.x);
    double denominator = std::sqrt((lineEnd.y - lineStart.y) * (lineEnd.y - lineStart.y) +
                                   (lineEnd.x - lineStart.x) * (lineEnd.x - lineStart.x));
    return numerator / denominator;
}

bool LocationMap::ComputeDistance(const geometry_msgs::Point32 &point, const double &thresh_distance)
{
    double res = std::sqrt(pow(point.x - lastPoint.x, 2) + pow(point.y - lastPoint.y, 2));

    if (res >= thresh_distance)
    {
        lastPoint.x = point.x;
        lastPoint.y = point.y;
        return true;
    }
    return false;
}

// 根据边界点数据设定边界的包围框信息
void LocationMap::getBoundingBox(LocationMap::map_hull &polygon_hull)
{
    polygon_hull.xmin = std::numeric_limits<double>::max();
    polygon_hull.xmax = std::numeric_limits<double>::min();
    polygon_hull.ymin = std::numeric_limits<double>::max();
    polygon_hull.ymax = std::numeric_limits<double>::min();
    for (auto &point : polygon_hull.polygon_with_holes.hull.points)
    {
        if (point.x < polygon_hull.xmin)
        {
            polygon_hull.xmin = point.x;
        }
        else if (point.x > polygon_hull.xmax)
        {
            polygon_hull.xmax = point.x;
        }
        if (point.y < polygon_hull.ymin)
        {
            polygon_hull.ymin = point.y;
        }
        else if (point.y > polygon_hull.ymax)
        {
            polygon_hull.ymax = point.y;
        }
    }
}

bool LocationMap::isPointInPolygon(const geometry_msgs::Point32 &point, LocationMap::map_hull &polygon_hull)
{
    bool isInside = false;
    int count = 0;
    // 首先做一个快速判断
    if (!inBoudingBox(point, polygon_hull))
    {
        return false;
    }
    // 求出多边形的最小X
    double minX = DBL_MAX;
    const auto &points = polygon_hull.polygon_with_holes.hull.points;
    for (size_t i = 0; i < points.size(); i++)
    {
        minX = std::min(minX, double(points[i].x));
    }
    double px = point.x;
    double py = point.y;
    // 负X方向的水平射线,(x,y)做起点,(minX, y)做终点
    double linePoint1x = px;
    double linePoint1y = py;
    double linePoint2x = minX - 10;
    double linePoint2y = py;
    // 遍历每一条边

    double cx1, cy1, cx2, cy2;
    for (size_t i = 0; i < points.size(); i++)
    {
        if (i + 1 == points.size())
        {
            cx1 = points[points.size() - 1].x;
            cy1 = points[points.size() - 1].y;
            cx2 = points[0].x;
            cy2 = points[0].y;
        }
        else
        {
            cx1 = points[i].x; // 多边形的第i个点
            cy1 = points[i].y;
            cx2 = points[i + 1].x; // 多边形的第i+1个点
            cy2 = points[i + 1].y;
        }
        // 点在多边形上,算是在内部
        if (isPointOnLine(px, py, cx1, cy1, cx2, cy2))
        {
            return true;
        }
        // X方向水平的边,不用计算,肯定不会和射线相交
        if (fabs(cy2 - cy1) < EPSILON)
        {
            continue;
        }
        // 多边形的一个顶点在射线上,且该顶点是上顶点(y值较高),算一个交点
        if (isPointOnLine(cx1, cy1, linePoint1x, linePoint1y, linePoint2x, linePoint2y))
        {
            if (cy1 > cy2)
            {
                count++;
            }
        }
        // 多边形的一个顶点在射线上,且该顶点是上顶点(y值较高),算一个交点
        else if (isPointOnLine(cx2, cy2, linePoint1x, linePoint1y, linePoint2x, linePoint2y))
        {
            if (cy2 > cy1)
            {
                count++;
            }
        }
        // 已经排除平行的情况,其他相交的都算一个交点
        else if (isIntersect(cx1, cy1, cx2, cy2, linePoint1x, linePoint1y, linePoint2x, linePoint2y))
        {
            count++;
        }
    }
    // 交点数为奇数,则在多边形内,反之在多边形外
    if (count % 2 == 1)
    {
        isInside = true;
    }
    return isInside;
}

// pid_t global_path_pid = -1;
// pid_t read_waypoints_pid = -1;//TODO 0811
void LocationMap::GetSignalCallback(const std_msgs::StringConstPtr &string)
{

    signal.reset();
    std::cout << getLogTime() << "string->data:" << string->data << std::endl;
    if (string->data == "p_mode")   // 发布所有本地的可用地图名，可能是供UI选择
    {
        ROS_INFO("p_mode!");
        publishMapList();
    }
    if ((string->data).substr(0, (string->data).find('/')) == "use_map") // 加载指定的地图文件数据到yaml_hulls变量中
    {
        ROS_INFO("use_map!");
        // use_map = 1;
        base_file = (string->data).substr((string->data).find('/') + 1) + std::string(".yaml");
        if (loadMap(base_file))
        {
            ROS_INFO("load Map success: %s", base_file.c_str());
        }
        else
        {
            ROS_ERROR("loaded Map error: %s", base_file.c_str());
        }
    }
    if (string->data == "test_map")
    {
        ROS_INFO("test_map!");
        signal.set(signal_work, 1);
    }
    if ((string->data).substr(0, (string->data).find('/')) == "delete_name")
    {
        ROS_INFO("delete_name!");
        delete_name = 1;
        base_file = (string->data).substr((string->data).find('/') + 1) + std::string(".yaml");

        try
        {
            std::string full_path = filename + base_file;
            if (fs::exists(full_path))
            {
                fs::remove(full_path);
                ROS_INFO("Successfully deleted the map: %s", full_path.c_str());
                hulls.clear();
                paths.clear();
                hull_tag = 0;
                publishMapList();
            }
            else
            {
                ROS_WARN("The map file does not exist.: %s", full_path.c_str());
            }
            delete_name = 0;
        }
        catch (const std::exception &e)
        {
            ROS_ERROR("Error occurred while deleting the map file.: %s", e.what());
            delete_name = 0;
        }
    }
    if (string->data == "single_map")
    {

        ROS_INFO("single_map!");
        single_map = true;
        multi_map = false;
    }
    if (string->data == "multi_map")
    {
        ROS_INFO("multi_map!");
        multi_map = true;
        single_map = false;
    }
    else if (string->data == "start_work")
    {
        ROS_INFO("Start work!");
        signal.set(signal_work, 1);
    }
    else if ((string->data).substr(0, (string->data).find('/')) == "save_map")
    {
        ROS_INFO("Start_save!");
        signal.set(signal_save, 1);
        base_file = (string->data).substr((string->data).find('/') + 1) + std::string(".yaml");
    }
    // 开始录制地图
    else if (string->data == "start_brd")
    {
        ROS_INFO("%s开始录制割草区域!", getLogTime().c_str());
        signal.set(signal_brd, 1);
        lastPoint.x = 0, lastPoint.y = 0;
        hull_tag++;
    }
    // 开始录制障碍物
    else if (string->data == "start_obs")
    {
        ROS_INFO("%s开始录制障碍物区域!", getLogTime().c_str());
        signal.set(signal_hole, 1);
        lastPoint.x = 0, lastPoint.y = 0;
    }
    // 开始录制停车位
    else if (string->data == "start_point")
    {
        ROS_INFO("Start transcribe stop point!");
        signal.set(signal_point, 1);
    }
    else if (string->data == "start_path")
    {
        ROS_INFO("%s开始录制割草区域之间的路线!", getLogTime().c_str());
        signal.set(signal_path, 1);
        lastPoint.x = 0, lastPoint.y = 0;
    }

    // else if (string->data == "delete_map")
    // {
    //     ROS_INFO("Start delete!");
    //     signal.set(5, 1);
    //     publishMapList();
    // }

    else if (string->data == "cease_brd")
    {
        std::cout << getLogTime() << "停止录制割草区域, 并发布已录制的地图给UI!" << std::endl;
        getBoundingBox(tmp_hull);
        hulls.push_back(tmp_hull);
        ROS_INFO("%s当前地图编号: %d", getLogTime().c_str(), tmp_hull.name);
        map_hull_pub.publish(tmp_hull.polygon_with_holes.hull); // 发布割草区域给UI
        last_hull = tmp_hull;
        tmp_hull.polygon_with_holes.hull.points.clear();
        ROS_INFO("%s最新地图编号: %d", getLogTime().c_str(), hulls[(hulls.size() - 1)].name);
    }
    else if (string->data == "cease_obs")
    {
        std::cout << getLogTime() << "停止录制障碍物区域, 并发布已录制的障碍物给UI" << std::endl;
        for (auto &map_hull : hulls)
        {
            if (map_hull.name == tmp_hull.name)
            {
                map_hull.polygon_with_holes.holes.emplace_back(tmp_hull.polygon_hole);
            }
        }
        map_hole_pub.publish(tmp_hull.polygon_hole);
        last_hull.polygon_hole.points = tmp_hull.polygon_hole.points;
        last_hull.polygon_with_holes.holes.clear();
        last_hull.polygon_with_holes.holes.emplace_back(last_hull.polygon_hole);
        tmp_hull.polygon_hole.points.clear();
        ROS_INFO("Success save and pub hole!");
    }

    else if (string->data == "cease_point")
    {
        map_point_pub.publish(point_stop);
        ROS_INFO("Start save and pub stop point!");
    }
    else if (string->data == "cease_path")
    {
        paths.push_back(tmp_path);
        map_path_pub.publish(tmp_path.link_path);
        tmp_path.link_path.path_points.clear();
        tmp_path.name = 0;
        ROS_INFO("Success save and pub path point!");
    }
    else if (string->data == "reset")
    {
        ROS_INFO("Reset all data!");
        reset();
    }
    else if (string->data == "stop")
    {
        reset();
        std::cout << getLogTime() << "割草任务停止" << std::endl;
    }
    else if (string->data == "mowing_finished")
    {
        reset();
        std::cout << getLogTime() << "割草任务完成" << std::endl;
    }
}

// 添加割草区域边界点
void LocationMap::handleHullPoint(const geometry_msgs::Point32 &point)
{
    tmp_hull.name = hull_tag;
    if (ComputeDistance(point, polygon_thr))    // 每1米记录一个点
        tmp_hull.polygon_with_holes.hull.points.emplace_back(point);
}

// 添加割草区域洞洞点(障碍物)
void LocationMap::handleHolePoint(const geometry_msgs::Point32 &point)
{
    if (ComputeDistance(point, polygon_thr * 0.1))  // 每0.1米记录一个点
        tmp_hull.polygon_hole.points.emplace_back(point);
}

void LocationMap::handleStopPoint(const geometry_msgs::Point32 &point)
{
    std::cout << "save stop point!" << std::endl;
    std::cout << "point.x:" << point.x << std::endl;
    point_stop = point;
    std::cout << "point_stop.x:" << point_stop.x << std::endl;
}

// 添加割草区域之间的路径点
void LocationMap::handlePathPoint(const geometry_msgs::Point32 &point)
{
    for (auto &map_hull : hulls)
    {
        if (isPointInPolygon(point, map_hull))
        {
            std::cout << "hull name is :" << map_hull.name << std::endl;
            if (tmp_path.name == 0)
            {
                tmp_path.name = map_hull.name;
                std::cout << "path name is :" << tmp_path.name << std::endl;
                break;
            }
        }
    }
    if (ComputeDistance(point, path_thr))
        tmp_path.link_path.path_points.push_back(point);
}

void LocationMap::handleSaveSignal()
{
    std::cout << getLogTime() << "开始保存地图!" << std::endl;
    try
    {
        int idx = 0;
        for (auto map : hulls)
        {
            std::vector<LocationMap::Point2d> initPoints; // 原始边界点
            std::vector<LocationMap::Point2d> finalPoints;  // 抽稀后的边界点
            LocationMap::Point2d initPoint;
            if (map.polygon_with_holes.hull.points.size() <= 2)
            {
                ROS_INFO("%s割草区域至少需要两个边界点!", getLogTime().c_str());
                continue;
            }
            for (auto brd_point : map.polygon_with_holes.hull.points)
            {
                initPoint.x = brd_point.x;
                initPoint.y = brd_point.y;

                initPoints.push_back(initPoint);
            }
            finalPoints = douglasPeucker(initPoints, 1.0); // 1.0为阈值,参数
            map.polygon_with_holes.hull.points.clear();
            for (size_t i = 0; i < finalPoints.size(); i++)
            {
                geometry_msgs::Point32 temPoint;
                temPoint.x = finalPoints[i].x;
                temPoint.y = finalPoints[i].y;
                temPoint.z = 0;

                map.polygon_with_holes.hull.points.push_back(temPoint);
            }
            std::vector<geometry_msgs::Polygon> new_holes;
            for (const auto &brd_hole : map.polygon_with_holes.holes)
            {
                if (brd_hole.points.size() <= 2)
                {
                    ROS_INFO("%s障碍物区域至少需要两个边界点!", getLogTime().c_str());
                    continue;
                }

                std::vector<LocationMap::Point2d> initHolePoints;
                for (const auto &p : brd_hole.points)
                    initHolePoints.push_back({p.x, p.y});

                auto finalHolePoints = douglasPeucker(initHolePoints, 0.3);

                geometry_msgs::Polygon simplified_hole;
                for (const auto &pt : finalHolePoints)
                {
                    geometry_msgs::Point32 temp;
                    temp.x = pt.x;
                    temp.y = pt.y;
                    temp.z = 0;
                    simplified_hole.points.push_back(temp);
                }

                new_holes.push_back(simplified_hole);
            }

            map.polygon_with_holes.holes = new_holes;
            hulls[idx] = map;
            idx++;
        }
        std::string full_path = filename + base_file;
        std::ofstream fout_brd(full_path);
        if (!fout_brd.is_open())
        {
            ROS_ERROR("%s无法打开文件: %s, 退出保存!", getLogTime().c_str(), full_path.c_str());
            return;
        }

        fout_brd << "Polygons:" << std::endl;
        fout_brd << "  polygon:" << std::endl;
        for (auto map : hulls)
        {
            fout_brd << "    - hull:" << std::endl;
            fout_brd << "      name: " << map.name << std::endl;
            fout_brd << "      x_min: " << std::fixed << map.xmin << std::endl;
            fout_brd << "      x_max: " << std::fixed << map.xmax << std::endl;
            fout_brd << "      y_min: " << std::fixed << map.ymin << std::endl;
            fout_brd << "      y_max: " << std::fixed << map.ymax << std::endl;
            fout_brd << "      brd:" << std::endl;
            for (auto brd_point : map.polygon_with_holes.hull.points)
            {
                fout_brd << "        - {"
                         << "x: " << brd_point.x << ", "
                         << "y: " << brd_point.y << ", "
                         << "z: " << brd_point.z << "}" << std::endl;
            }
            fout_brd << "      holes:" << std::endl;
            for (auto brd_hole : map.polygon_with_holes.holes)
            {
                fout_brd << "        -" << std::endl;
                ROS_INFO("hole point.size: %zu", brd_hole.points.size());
                for (auto hole_point : brd_hole.points)
                {
                    fout_brd << "          - {"
                             << "x: " << hole_point.x << ", "
                             << "y: " << hole_point.y << ", "
                             << "z: " << hole_point.z << "}" << std::endl;
                }
            }
            for (auto path_ : paths)
            {
                if (path_.name == map.name)
                {
                    fout_brd << "      path:" << std::endl;
                    for (auto path_point : path_.link_path.path_points)
                    {
                        fout_brd << "        - {"
                                 << "x: " << path_point.x << ", "
                                 << "y: " << path_point.y << ", "
                                 << "z: " << path_point.z << "} " << std::endl;
                    }
                }
            }
        }
        fout_brd.close();
        ROS_INFO("%s地图保存成功: %s", getLogTime().c_str(), full_path.c_str());
        publishMapList();
    }
    catch (const std::exception &e)
    {
        ROS_ERROR("%sERROR WHEN SAVE MAP: %s", getLogTime().c_str(), e.what());
    }
}

/* 
订阅到小车的有效位置并处在工作状态会执行该函数；
该函数会发布地图边界、洞、起点终点供全覆盖路径规划节点使用；
还会发布边界供避障节点使用；
正是这个函数来触发小车工作
*/
void LocationMap::handleWorkSignal(const geometry_msgs::Point32 &point)
{
    geometry_msgs::PointStamped plan_point;
    util::PolygonWithHolesStamped poylgon;
    plan_point.header.frame_id = "world";
    poylgon.header.frame_id = "world";
    if (single_map || multi_map)
    {
        try
        {
            hulls.clear();
            paths.clear();
            // int map_id;
            if (single_map)
            {
                ROS_INFO("single_map selected");
                for (auto &map_hull : yaml_hulls)
                {
                    if (isPointInPolygon(point, map_hull))
                    {
                        ROS_WARN("Single_map mode confirm, now in map_hull: %d", map_hull.name);
                        poylgon.polygon = map_hull.polygon_with_holes;
                        planning_hull_pub.publish(poylgon);
                        // map_id = (int)map_hull.name;
                        // std::cout<<"The BRD of the map is  "<<map_hull.polygon_with_holes.hull<<std::endl;
                        map_hull.iscomplete = true;
                        map_hull_pub.publish(poylgon.polygon.hull);
                        for (auto &hole : poylgon.polygon.holes)
                        {
                            map_hole_pub.publish(hole);
                            hole.points[0].z = 999.0; // 设置z为999，表示障碍物
                            map_hull_pub.publish(hole);
                        }
                        std::cout << getLogTime() << "发布割草区域给UI显示, 地图编号: " << map_hull.name << std::endl;
                        hulls.push_back(map_hull);
                        break;
                    }
                    else
                    {
                        std::cout << getLogTime() << "起始点不在地图" << map_hull.name << "的区域内"<< std::endl;
                        return;
                    }
                }
                single_map = false;
            }
            if (multi_map)
            {
                // TODO: 多地图路径拼接功能 - 简化的多地图序列推送逻辑 cnk 0805
                // yaml_hulls和yaml_paths已经通过use_map信号加载，无需重复加载
                if (yaml_hulls.empty())
                {
                    ROS_ERROR("No map data loaded! Please use 'use_map/filename' first.");
                    return;
                }

                std::cout << getLogTime() << yaml_hulls.size() << "幅地图已完成加载" << std::endl;
                std::cout << getLogTime() << "当前坐标: (" << point.x << ", " <<  point.y << ")" << std::endl;

                // 1. 确定起始地图
                int start_map_id = -1;
                for (size_t i = 0; i < yaml_hulls.size(); i++)
                {
                    // 添加调试信息：打印每个地图的边界信息
                    /* ROS_WARN("Map %zu (name: %d) - Boundary: x[%.6f, %.6f], y[%.6f, %.6f]",
                             i, yaml_hulls[i].name,
                             yaml_hulls[i].xmin, yaml_hulls[i].xmax,
                             yaml_hulls[i].ymin, yaml_hulls[i].ymax);
                    ROS_WARN("Map %zu has %zu hull points", i, yaml_hulls[i].polygon_with_holes.hull.points.size()); */

                    // 打印多边形的所有顶点坐标
                    /* ROS_WARN("Map %zu polygon vertices:", i);
                    for (size_t j = 0; j < yaml_hulls[i].polygon_with_holes.hull.points.size(); j++)
                    {
                        const auto &pt = yaml_hulls[i].polygon_with_holes.hull.points[j];
                        ROS_WARN("  Vertex %zu: (%.6f, %.6f, %.6f)", j, pt.x, pt.y, pt.z);
                    } */

                    if (isPointInPolygon(point, yaml_hulls[i]))
                    {
                        start_map_id = static_cast<int>(i);
                        std::cout << getLogTime() << "起始点在地图" << yaml_hulls[i].name << "的区域内，将从该地图开始作业" << std::endl;
                        break;
                    }
                    else
                    {
                        std::cout << getLogTime() << "起始点不在地图" << yaml_hulls[i].name << "的区域内"<< std::endl;
                    }
                }

                if (start_map_id == -1)
                {
                    std::cout << getLogTime() << "起始点不在任何地图中"<< std::endl;
                    return;
                }

                // 2. 初始化多地图处理状态
                current_map_index_ = start_map_id;
                is_multi_map_processing_ = true;

                // 3. 开始处理第一个地图
                current_start_point_ = point; // 起点是当前位置

                // 找到第一个地图对应的path路径作为终点
                if (!yaml_hulls[start_map_id].connection_path.empty())
                {
                    current_end_point_ = yaml_hulls[start_map_id].connection_path[0]; // path的第一个点作为终点
                    ROS_INFO("First map end point set to: (%.3f, %.3f, %.3f)",
                             current_end_point_.x, current_end_point_.y, current_end_point_.z);
                }
                else
                {
                    // 如果第一个地图就没有path，说明只有一个地图，终点就是起点
                    current_end_point_ = current_start_point_;
                    std::cout << getLogTime() << "没有找到连接路径，终点设置为起点" << std::endl;
                }

                // 4. 推送第一个地图
                processNextMapInSequence();
                ROS_INFO("First map sent, waiting for global_planning feedback to continue...");
                multi_map = false;
            }
            if (!poylgon.polygon.hull.points.empty())
            {
                plan_point.point.x = point.x;
                plan_point.point.y = point.y;
                plan_point.point.z = point.z;
                planning_point_pub.publish(plan_point); // 当前位置作为规划起点
                planning_point_pub.publish(plan_point); // 当前位置作为规划终点
            }
            signal.set(signal_work, 0);
        }
        catch (const std::exception &e)
        {
            ROS_ERROR("ERROR when process map: %s", e.what());
        }
    }
    else if (!last_hull.polygon_with_holes.hull.points.empty()) // 为了可以刚录制完地图直接规划
    {
        poylgon.polygon = last_hull.polygon_with_holes;
        planning_hull_pub.publish(poylgon);
        plan_point.point.x = point.x;
        plan_point.point.y = point.y;
        plan_point.point.z = point.z;
        planning_point_pub.publish(plan_point);
        planning_point_pub.publish(plan_point);
        last_hull.polygon_with_holes.hull.points.clear();
    }
}

void LocationMap::StatusCallback(const util::PositionConstPtr &position_msg)
{
    if (position_msg->position_state == 0)
    {
        ROS_WARN("Received zero position, ignoring.");
        return;
    }

    geometry_msgs::Point32 point;
    point.x = position_msg->position_x;
    point.y = position_msg->position_y;
    point.z = 0;

    if (signal[signal_brd])
        handleHullPoint(point);

    if (signal[signal_hole])
        handleHolePoint(point);

    if (signal[signal_point])
        handleStopPoint(point);

    if (signal[signal_path])
        handlePathPoint(point);

    // if (signal[signal_delete])
    //     handleDeleteSignal();

    if (signal[signal_save])
    {
        handleSaveSignal();
        signal.set(signal_save, 0);
    }

    if (signal[signal_work])
    {
        handleWorkSignal(point);
    }
}

// 添加处理下一个地图的方法
// TODO: 多地图路径拼接功能 - 序列处理每个地图的方法 cnk 0805
void LocationMap::processNextMapInSequence()
{
    if (current_map_index_ >= yaml_hulls.size())
    {
        // 所有地图处理完成
        ROS_INFO("All maps have been processed");
        is_multi_map_processing_ = false;
        return;
    }

    ROS_INFO("当前处理地图索引: %d, 地图名: %d", current_map_index_, yaml_hulls[current_map_index_].name);
    ROS_INFO("Start point: (%.2f, %.2f), End point: (%.2f, %.2f)",
             current_start_point_.x, current_start_point_.y,
             current_end_point_.x, current_end_point_.y);

    // 发布当前地图给规划器
    util::PolygonWithHolesStamped polygon_msg;
    polygon_msg.header.frame_id = "world";
    polygon_msg.polygon = yaml_hulls[current_map_index_].polygon_with_holes;
    planning_hull_pub.publish(polygon_msg);

    // 发布地图边界（与single_map模式保持一致）给UI显示正在工作的地图
    map_hull_pub.publish(yaml_hulls[current_map_index_].polygon_with_holes.hull);
    for (auto &hole : yaml_hulls[current_map_index_].polygon_with_holes.holes)
    {
        map_hole_pub.publish(hole);
        hole.points[0].z = 999.0; // 设置z为999，表示障碍物
        map_hull_pub.publish(hole);
    }
    std::cout << getLogTime() << "发布地图" << yaml_hulls[current_map_index_].name << "给UI显示"<< std::endl;

    // 发布规划起点
    geometry_msgs::PointStamped start_point;
    start_point.header.frame_id = "world";
    start_point.point.x = current_start_point_.x;
    start_point.point.y = current_start_point_.y;
    start_point.point.z = 0;
    planning_point_pub.publish(start_point);

    // 发布规划终点
    geometry_msgs::PointStamped end_point;
    end_point.header.frame_id = "world";
    end_point.point.x = current_end_point_.x;
    end_point.point.y = current_end_point_.y;
    end_point.point.z = 0;
    planning_point_pub.publish(end_point);

    std::cout << getLogTime() << "发布规划的起点和终点" << std::endl;

    // 准备下一个地图（如果有的话）
    current_map_index_++;
    if (current_map_index_ < yaml_hulls.size())
    {
        // 设置下一个地图的起点和终点
        // 获取前一个地图的连接路径作为下一个地图的起点
        if (!yaml_hulls[current_map_index_ - 1].connection_path.empty())
        {
            current_start_point_ = yaml_hulls[current_map_index_ - 1].connection_path.back(); // path的最后一个点作为起点
            ROS_INFO("Next map start point from previous path: (%.3f, %.3f, %.3f)",
                     current_start_point_.x, current_start_point_.y, current_start_point_.z);
        }
        else
        {
            std::cout << getLogTime() << "上一个地图没有连接路径，不设置起点，上一个地图编号为：" << yaml_hulls[current_map_index_ - 1].name << std::endl;
            return;
        }

        if (current_map_index_ < yaml_hulls.size() - 1)
        {
            // 不是最后一个地图，设置终点为当前地图的path第一个点
            if (!yaml_hulls[current_map_index_].connection_path.empty())
            {
                current_end_point_ = yaml_hulls[current_map_index_].connection_path[0];
                ROS_INFO("Next map end point set to: (%.3f, %.3f, %.3f)",
                         current_end_point_.x, current_end_point_.y, current_end_point_.z);
            }
            else
            {
                ROS_ERROR("Map %d has no connection path! Cannot set end point.", yaml_hulls[current_map_index_].name);
                return;
            }
        }
        else
        {
            // 最后一个地图，没有path参数，终点与起点一致（闭环）
            current_end_point_ = current_start_point_;
            ROS_INFO("Last map: end point same as start point (closed loop)");
        }

        // 等待global_planning的反馈信号，不再自动递归
        ROS_INFO("Waiting for global_planning feedback before sending next map...");
    }
    else
    {
        // 所有地图处理完成
        is_multi_map_processing_ = false;
        ROS_INFO("All maps have been sent to planning");
    }
}

// 多地图模式下处理每一幅地图全覆盖路径准备就绪，和所有地图路径都处理完毕的信号
void LocationMap::PlanningReadyCallback(const std_msgs::StringConstPtr &ready_msg)
{
    ROS_INFO("Received planning ready signal: %s", ready_msg->data.c_str());

    // 检查是否正在进行多地图处理
    if (!is_multi_map_processing_)
    {
        ROS_WARN("Not in multi-map processing mode, ignoring ready signal");
        return;
    }

    // 检查信号内容，确保是waypoints处理完成的信号
    if (ready_msg->data == "waypoints_received")
    {
        ROS_INFO("Global planning has processed the waypoints, sending next map...");

        // 延时一下确保planning完全处理完成
        ros::Duration(0.5).sleep();

        // 处理下一个地图
        processNextMapInSequence();
    }
    else if (ready_msg->data == "all_maps_processed")
    {
        ROS_INFO("All maps have been processed by global planning");
        is_multi_map_processing_ = false;
        current_map_index_ = -1;
        ROS_INFO("Multi-map processing completed successfully");
    }
    else
    {
        ROS_WARN("Unknown ready signal: %s", ready_msg->data.c_str());
    }
}
