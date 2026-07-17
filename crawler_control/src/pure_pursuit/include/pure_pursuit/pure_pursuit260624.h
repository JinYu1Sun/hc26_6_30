
#ifndef PURE_PURSUIT_PURE_PURSUIT_H
#define PURE_PURSUIT_PURE_PURSUIT_H
#include <cmath>
#include <vector>
#include <memory>
// ROS includes
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TwistStamped.h>
#include <ros/ros.h>
#include <std_msgs/Float32.h>
#include <std_msgs/String.h>
#include <visualization_msgs/Marker.h>
#include <geometry_msgs/PointStamped.h>
#include <iostream>
#include <fstream>
// User defined includes
//#include <util/PlanningResult.h>
#include <util/LocalPath.h>
#include <util/LocalPose.h>
#include <util/Position.h>
#include <mower_msgs/VehicleCmd.h>
#include <mower_msgs/ControlOk.h>
#include <mower_msgs/Direct_Control.h>
#include <std_msgs/UInt8.h>
#include <std_msgs/Bool.h>
#include <std_msgs/String.h>
#include <mower_msgs/ControlError.h>  //add

// time
#include <sstream>
#include <iomanip>
#include <ctime>
#include <geometry_msgs/Point.h>  //@cnk


#define PROJECT_VERSION 0
#define MAJOR_VERSION 0
#define MINOR_VERSION 0
#define PATCH_VERSION 0

// 车辆状态
enum class CarState : int{
    Idle = 0,                // 空状态
    GetLocalPath = 1,        // 获取局部路径
    GetLocalPathOver = 2,    // 得到局部路径
    LocalPathError   = 3,    // 局部路径有误
    Driving = 4,             // 单行驶状态
    Working = 5,             // 工作状态
    Arrive  = 7,             // 到达
    Pause   = 8,             // 暂停
    Loss    = 9,             // 丢失，特指无定位
    ChassisError     = 10    // 底盘不通   
};

enum PathState
{
    PATH_INIT = 0,
    PATH_OK = 1,
    PATH_TOO_FAR = 2,
    PATH_FINISHED = 3,
    PATH_INVALID = 4
};
enum ControlState  
{
    CONTROL_INIT = 0,
    CONTROL_OK = 1,
    CONTROL_FINISHED = 3,
    CONTROL_OUT_OF_BOUND = 4
};

typedef struct
{
    float x;
    float y;
} waypoint;

typedef struct
{
    waypoint point;
    float heading; //点位航向角
} waypose;

struct ControlCommand
{
    
    //float left_wheel_speed;   //左轮速度
    //float right_wheel_speed;  //右轮速度
    int   turn_value;         //通道1 值  (700~1500~2300)
    int   drive_value;        //通道2 值 （ >=1500 ）
};
// @xhj  pub_CtlError  当前偏差 - 期望值
struct ControlError {
    float e_lat;     // 横向误差 （lateral error） 相对于目标点的横向距离
    float e_pos;     // 位置误差 （position error）车辆于路径的最短距离,侧向误差
    //float e_pos2;    // 累计位置标准差 （calculated based on standard deviation）
    float e_theta;   // 航向误差 （heading error between goal and current heading）
};

// PID控制器  @xhj add:debug
class PIDController {
public:
    PIDController(float kp, float ki, float kd) : kp(kp), ki(ki), kd(kd), prev_error(0.00f), integral(0.00f) {}

    float compute(float setpoint, float actual, float dt) {
        float error = setpoint - actual;
        integral += error * dt;
        float derivative = (error - prev_error) / dt;
        prev_error = error;

        return kp * error + ki * integral + kd * derivative;
    }

private:
    float kp, ki, kd;
    float prev_error;
    float integral;
};

std::string timestampToDate(const ros::Time &time);
std::string getVersion();
// 工具函数
//int quadraticFunction(double a, double b, double c, double xq);
int calculateDriveValue(float current_speed,float desire_speed,bool flag,uint8_t avoid_state_data);
int calculateTurnValue(bool latest_gear_flag, waypose local_waypose, waypose *c_goal, 
                       unsigned char cgoal_id, int last_tv,float desire_speed);

double getDelay(const ros::Time &now, const ros::Time &last);

class PurePursuit
{
public:
    PurePursuit();
    ~PurePursuit();

    void run();
    
private:
    // handle
    ros::NodeHandle nh_;
    ros::NodeHandle private_nh_;

    // publisher
    ros::Publisher pub_Command, pub_Controlok , pub_CtlError, path_too_far_pub_; //TODO


    // subscriber
    std::string sub_planning_result_topic, sub_speed_info_topic, sub_gpsposition_topic, sub_pos_topic, sub_direct_control_topic,sub_stop_car_topic,sub_avoidstate_topic;
    ros::Subscriber sub1_, sub2_, sub3_, sub4_, sub5_,substop,sub_avoid,sub_signal_;

    // constant
    const int LOOP_RATE_; // processing frequency

    // variables
    waypose local_waypoints[50];
    waypose *goal;
    unsigned char goal_id; // 0～255 [50]
    ControlCommand command;

    //@xhj ：limiting 
    //std::vector<double> prev_path_x; //CNK 0511
    //std::vector<double> prev_path_y;
    //size_t prev_path_size = 0;
   // bool first_path = true;
    bool need_turn_for_new_path = false; //cnk 0511
    //@xhj : control_status
    bool c_start = false;
    bool c_finish = false;

    bool stop_tem_flag = false; //正式转弯前临时停车
    bool stop_car_flag =false;
    bool stop_path_flag = false; //对路径状态判断
    uint8_t avoid_state_data = 0; // 0 :无障碍物；1：避障模式（10个点以内）；2：给车降速（20个点）；3：紧急停车模式（3个点以内）
 
    int path_state = PATH_INIT;
    int lat_track_state = CONTROL_INIT;
    int look_ahead_index = 0;
    int nearest_waypoint_id = -1;

    const float kMaxPathDistance = 20000;    // cm
    const float kMaxLatError = 2000;        // cm
    const float kStoppingLatError = 0.5;   // meter
    //const float L = 1.26;                  // wheel base

    float nearest_dis = 99.9;
    int   chassis_type;
    float wheel_track ;
    float wheel_base ; 
    // basic config
    int mover_bool_cfg = 0;
    int mower_height_cfg = 0;

    float vir_wheel_track; //d_lr 履带虚拟轮间距
    float kb; //1±kb，作为speed_com 的系数
    float outdir_speed; //外径速度 更大
    float indir_speed;  //内径速度
    int   drive_value_tmp = 1500;
    int   last_turn_value = 1501;

    float pl0_heading = 0; //未使用
    uint8_t last_pathtype = 0;

    float lat_error = 0;
    float desire_ICR; //瞬时转向半径
    int error_status = 0;
    float max_goal_lat_error = 0.0; // meter
    unsigned char stopping_req_by_lat_control = 0;

    util::LocalPath planning_result;
    util::LocalPose vehicle_info;
    util::Position car_position;
    mower_msgs::Direct_Control direct_control;

    bool is_planning_result_set_;
    bool is_velocity_set_;
    bool is_carposition_set_;
    ros::Time last_pos_time;   // 新增：记录上次收到位置的时间
    float desire_speed_com;
   

    // callbacks
    void callbackFromPlanningResult(const util::LocalPathConstPtr &msg_planning_result);
    void callbackFromSpeedInfo(const util::LocalPoseConstPtr &msg_speed_info);
    void callbackFromPos(const util::PositionConstPtr &msg_carposition);
    void callbackFromDirectControl(const mower_msgs::Direct_ControlConstPtr &msg_direct_control);
    void callbackFromTaskstop(const std_msgs::Bool &task_msg);	
    void callbackFromAvoidstate(const std_msgs::UInt8 &avoid_msg);
    void callbackFromSignal(const std_msgs::StringConstPtr &string);
    //bool isSamePath(const util::LocalPath& new_path); 	  //cnk 0511
    //void savePreviousPath(const util::LocalPath& path);   //cnk 0511
    

    // initializer
    void initForROS();
    
    // functions
    void setPath_main(void);

    void updatestatus(void);  
    int updatePathState(void);
    int updateLatTrackState(void);

    int getNearestIndex(int *index);  
    int getLatDistance(float *distance);  //计算当前侧向偏差
    float getDistanceToLine(waypose *p1, waypose *p2);
  
    void setControl_main(void);

    int findGoalPoint(void);
    int isGoalValid(void);
    
    void act(void);
    void latcontrol_algorithm(void);
    void command_publish();
    void controlError_publish(); //add ctlE pub
};

#endif // PURE_PURSUIT_PURE_PURSUIT_H
