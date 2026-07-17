#ifndef TASK_H
#define TASK_H

#include <ros/ros.h>
#include <vector>
#include <std_msgs/UInt8.h>
#include <std_msgs/Bool.h>
#include <std_msgs/String.h>
#include <string.h>

// android msg
#include "mower_msgs/TaskStatus.h"
#include "mower_msgs/CheckResult.h"
#include "mower_msgs/Manual_Set.h"
#include "mower_msgs/Manual_Driving_Cmd.h"
#include "mower_msgs/Fault_Code.h"
#include "mower_msgs/Direct_Control.h"
#include "mower_msgs/VehicleCmd.h"

// check msg
#include "mower_msgs/LidarSelfDtect.h"
#include "mower_msgs/PerceptionSelfDetect.h"
#include "mower_msgs/MultiMapSelfDetect.h"
#include "mower_msgs/PlaningOK.h"
#include "mower_msgs/PlanType.h"
#include "mower_msgs/CamerargbState.h"
#include "mower_msgs/SegState.h"
#include "mower_msgs/ControlOk.h"
#include "mower_msgs/ControlState.h"
#include "mower_msgs/GnssOK.h"
#include "mower_msgs/VslamState.h"
#include "mower_msgs/CameraState.h"

#include "mower_msgs/Position.h"
#include "mower_msgs/Monitor.h"
#include "util/LocalPose.h"
#include "mower_msgs/LocalPath.h"
#include "nav_msgs/Path.h"
#include <tf/transform_datatypes.h>

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <cmath>

using namespace std;

// 任务状态枚举
enum tasks
{
    Holding,
    Working,                 // 工作状态
    Local_path_error,        // 绕障路径异常
    Vehicle_rollover,        // 侧翻状态
    Returning,               // 返回
    Pausing,                 // 暂停工作
    Self_checking,           // 自检状态
    Self_check_fault,        // 自检失败
    Obstacle_parking,        // 遇障停车
    Passing_connecting_space // 通过连通区
};

// 任务状态单例类
// 所有原本的全局变量都封装在这里，通过 TaskState::instance() 访问
class TaskState
{
public:
    static TaskState &instance()
    {
        static TaskState s_instance;
        return s_instance;
    }

    // ROS 消息对象
    mower_msgs::TaskStatus status;
    mower_msgs::Fault_Code fault;
    mower_msgs::Monitor monitor;
    std_msgs::Bool stop_car;

    // check sub
    ros::Subscriber sub_camera;
    ros::Subscriber sub_vslam;
    ros::Subscriber sub_camerargb;
    ros::Subscriber sub_seg;
    ros::Subscriber sub_lidar;
    ros::Subscriber sub_perception;
    ros::Subscriber sub_gnss;
    ros::Subscriber sub_planche;
    ros::Subscriber sub_ctrlche;
    ros::Subscriber sub_multimap;
    ros::Subscriber sub_vehistatus;

    // sub android
    ros::Subscriber sub_manual;
    ros::Subscriber sub_manual_driving;
    ros::Subscriber sub_singal;
    ros::Subscriber sub_plantype;
    ros::Subscriber sub_controlstate;
    ros::Subscriber sub_warn;

    // pub android
    ros::Publisher pub_status;
    ros::Publisher pub_result;
    ros::Publisher pub_fault;
    ros::Publisher pub_monitor;
    ros::Publisher pub_direct_control;
    ros::Publisher pub_vehicle_cmd;
    ros::Publisher pub_stopflag;
    ros::Publisher pub_init_request;

    // 自检标志
    bool check_flag = false;

    // 手动/任务控制标志
    bool manual_better = false;
    bool warn_2 = false;

    // 初始化程序相关变量
    bool init_mode = false;          // 初始化模式标志
    bool has_position = false;       // 是否有定位数据
    ros::Time init_start_time;       // 初始化开始时间
    double figure8_radius = 1.5;     // "8"字形半径（米）
    double figure8_period = 20.0;    // "8"字形周期（秒）

    // 初始化确认相关变量
    bool init_requested = false;     // 是否请求初始化
    bool init_confirmed = false;     // 是否确认初始化

    // 各算法/模块自检状态
    bool check_gnss = false;
    bool check_fusion = false;
    bool check_cam = false;
    bool check_vslam = false;
    bool check_lidar = false;
    bool check_perception = false;
    bool check_multimap = false;
    bool check_battery = false;
    bool check_warn = false;
    bool check_camerargb = true;
    bool check_planok = false;
    bool check_controlok = false;
    bool check_seg = false;

    // 安全/停障相关标志
    int rangewarn_flag = 0;          // 范围停车预警
    bool front_flag = false;
    bool rear_flag = false;
    bool yolofront_flag = false;
    bool rollover_flag = false;      // 侧翻状态
    bool outboundary_flag = false;
    bool lowpower_flag = false;      // 低电量 现定义-30%

    bool appsignal_flag = false;     // 默认 /signal == pause | stop 为 1; continue 为 0
    bool init_finish = false;
    bool current_has_position = false; // 当前是否有定位数据

    bool straight_done = false;
    bool figure8_done = false;
    bool straight_active = false;
    bool second_straight_active = false;
    
    enum tasks taskstatus = Holding;

    // pure_pursuit 子进程 PID
    pid_t pid_controller_pid = -1;

    // 节点名称表（下标对应 fault_code / monitor.node_normal）
    static const std::string node_names[13];

private:
    TaskState() = default;
    ~TaskState() = default;
    TaskState(const TaskState &) = delete;
    TaskState &operator=(const TaskState &) = delete;
};

// 控制函数
void goStraight();
void controlFigure8();
void parking();
void task_run();
bool checkAndResetNodes(const mower_msgs::Monitor &msgs);

// 回调函数声明
void ManualDriveCallBack(const mower_msgs::Manual_Driving_Cmd &manual_drive_msgs);
void SingalCallBack(const std_msgs::String &singal_msgs);
void FusionMapCallBack(const mower_msgs::Position &msgs);
void ControlStateCallBack(const mower_msgs::ControlState &controlsate_msgs);
void PlanTypeCallBack(const mower_msgs::PlanType &plantype_msgs);
void VslamCallBack(const mower_msgs::VslamState &vslam_msgs);
void CameraRGBCallBack(const mower_msgs::CamerargbState &camerargb_msgs);
void SegCallBack(const mower_msgs::SegState &seglok_msgs);
void LidarCallBack(const mower_msgs::LidarSelfDtect &lidar_msgs);
void PerceptionCallBack(const mower_msgs::PerceptionSelfDetect &perception_msgs);
void GnssCallBack(const mower_msgs::GnssOK &gnssok_msgs);
void PlanCheckCallBack(const mower_msgs::PlaningOK &planok_msgs);
void CtrlCheckCallBack(const mower_msgs::ControlOk &controlok_msgs);
void MultiMapCallBack(const mower_msgs::MultiMapSelfDetect &multimap_msgs);
void CamCallBack(const mower_msgs::CameraState &cam_msgs);
void ImuCallBack(const std_msgs::Bool &imu_msgs);
void OutBoundaryCallBack(const std_msgs::Bool &msgs);
void SpeedInfoCallBack(const util::LocalPose &v_pose);
void YoloflagfrontCallBack(const std_msgs::Bool &range_msgs);

#endif // TASK_H
