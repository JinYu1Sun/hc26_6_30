
#ifndef PURE_PURSUIT_PURE_PURSUIT_H
#define PURE_PURSUIT_PURE_PURSUIT_H
#include <algorithm>
#include <cmath>
#include <memory>
#include <mutex>
#include <vector>
// ROS includes
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TwistStamped.h>
#include <ros/ros.h>
#include <std_msgs/Float32.h>
#include <std_msgs/String.h>
#include <iostream>
#include <fstream>
// User defined includes
#include <util/LocalPath.h>
#include <util/LocalPose.h>
#include <util/Position.h>
#include <mower_msgs/VehicleCmd.h>
#include <mower_msgs/ControlOk.h>
#include <mower_msgs/Direct_Control.h>
#include <std_msgs/UInt8.h>
#include <std_msgs/Bool.h>
#include <std_msgs/String.h>

// time
#include <sstream>
#include <iomanip>
#include <ctime>
#include <geometry_msgs/Point.h>  //@cnk 历史遗留 include，当前 /turn_completed 已改用 std_msgs/Bool，保留以防其它模块依赖

#include "pure_pursuit/TurnCompleted.h"


enum PathState
{
    PATH_INIT = 0,
    PATH_OK = 1,
    PATH_TOO_FAR = 2,
    PATH_FINISHED = 3,
    PATH_INVALID = 4
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
    int   turn_value;         //通道1 值  (700~1500~2300)
    int   drive_value;        //通道2 值 （ >=1500 ）
};
int calculateDriveValue(float current_speed,float desire_speed,bool flag,uint8_t avoid_state_data);
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
    ros::Publisher pub_Command, pub_Controlok;
    ros::ServiceServer turn_completed_srv_;

    std::mutex turn_completed_mutex_;
    pure_pursuit::TurnCompleted turn_completed_msg_;

    // subscriber
    ros::Subscriber sub1_, sub2_, sub3_, sub4_, sub5_, substop, sub_avoid, sub_signal_;

    // constant
    const int LOOP_RATE_; // processing frequency

    // variables
    waypose local_waypoints[50];
    ControlCommand command;

    // 转弯状态机：用于处理 gear=2 的转弯点
    // TRACKING：    正常跟踪，未遇到转弯点
    // STOP：停止 gear=2 转弯点，已减速
    // EXECUTING：到达转弯点，执行原地旋转对准下一段路径
    // REVERSE：     开始反向行驶
    enum class VehicleState { TRACKING,EXECUTING,STOP,REVERSE };
    VehicleState vehicle_state_ = VehicleState::TRACKING;
    float nearest_speed_m_s = 0.0f;
    // 转弯完成保护：避免 planner 还没删掉旧 gear=2 点时再次进入同一转弯点
    bool has_last_completed_turn_ = false;
    float last_completed_turn_x_ = 0.0f;
    float last_completed_turn_y_ = 0.0f;

    bool c_finish = false;
    bool Task_stop =false;
    bool stop_car_flag =false;
    bool stop_path_flag = false; //对路径状态判断
    uint8_t avoid_state_data = 0; // 0 :无障碍物；1：避障模式（10个点以内）；2：给车降速（20个点）；3：紧急停车模式（3个点以内）
 
    int path_state = PATH_INIT;
    int look_ahead_index = 0;
    int nearest_waypoint_id = -1;

    ros::Time last_pos_time;  // 上次收到定位数据的时间戳，用于定位超时检测
    ros::Time last_planning_result_time; // 上次收到路径数据的时间戳，用于路径超时检测
    const float kMaxPathDistance = 20000;    // cm
    const float kMaxLatError = 2000;        // cm
    const float kStoppingLatError = 0.5;   // meter
    //const float L = 1.26;                  // wheel base

    // basic config
    int mover_bool_cfg = 0;
    int mower_height_cfg = 0;

    util::LocalPath planning_result;
    util::LocalPose vehicle_info;
    util::Position car_position;

    bool is_planning_result_set_;
    bool is_velocity_set_;
    bool is_carposition_set_;
   

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

    bool callbackTurnCompleted(pure_pursuit::TurnCompletedRequest &req, pure_pursuit::TurnCompletedResponse &res);
    

    // initializer
    void initForROS();
    
    // functions
    void setPath_main(void);

    void updatestatus(void);  
    int updatePathState(void);

    int getNearestIndex(int *index);  
    int getLatDistance(float *distance);  //计算当前侧向偏差
    float getDistanceToLine(waypose *p1, waypose *p2);
    int calculateLookAheadIndex(int base_idx, float speed_m_s);
    int findNextTurnPoint(int base_idx, int search_end_idx);
    void updateStateMachine();

    void setControl_main(void);

    // void act(void);
    void latcontrol_algorithm(void);
    void command_publish();
};

#endif // PURE_PURSUIT_PURE_PURSUIT_H
