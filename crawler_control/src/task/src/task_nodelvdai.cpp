#include <ros/ros.h>
#include <vector>
#include <std_msgs/UInt8.h>
#include <std_msgs/Bool.h>
#include <std_msgs/String.h>
#include <string.h>
#include <algorithm>
// android msg
#include "mower_msgs/Manual_Driving_Cmd.h"
#include "mower_msgs/Direct_Control.h"
#include "mower_msgs/VehicleCmd.h"
#include "mower_msgs/Fault_Code.h"
// check msg

//@ xhj： add
#include "mower_msgs/Position.h"
#include "util/LocalPose.h"

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <cmath>

using namespace std;

// ================== ROS 发布器/订阅器 ==================
ros::Publisher pub_direct_control;
ros::Publisher pub_vehicle_cmd;
ros::Publisher pub_stopflag;
ros::Publisher pub_init_request;

ros::Subscriber sub_manual_driving;
ros::Subscriber sub_singal;
ros::Subscriber sub_fusionmap;
ros::Subscriber sub_speedinfo;
ros::Subscriber sub_yolofront;
ros::Subscriber sub_imu;
ros::Subscriber sub_mower_dynamicflag_;
ros::Subscriber sub_lio_static_initialized_;

// ================== 车辆安全/业务状态 ==================
std_msgs::Bool stop_car;
bool manual_better = false;

bool init_mode = false;
bool has_position = false;
bool init_requested = false;
bool init_confirmed = false;
bool init_finish = false;
bool lio_static_initialized_flag = false;

bool check_fusion = false;
int rangewarn_flag = 0;
bool mower_dynamic_flag = false;
bool yolofront_flag = false;
bool rollover_flag = false;
bool lowpower_flag = false;
bool appsignal_flag = false;
bool init_request_cleared = false; 

pid_t pid_controller_pid = -1;
int has_position_count = 0;
ros::Time init_end_time;

// ================== 重复直行-自转初始化参数 ==================
const double SQUARE_INIT_STOP_DURATION = 2.0;      // 初始停车时间（秒）
const double SQUARE_INIT_STRAIGHT_DURATION = 3.0;  // 初始直行时间（秒）
const double SQUARE_STRAIGHT_DURATION = 2.0;       // 直行段时间（秒）
const double SQUARE_TURN_DURATION = 4.0;          // 自转段时间（秒），需根据实际转向速率/目标角度标定
const double PHASE_SWITCH_STOP_DURATION = 0.1;    // 直行/自转切换时停车时间（秒）

// 运动相位：0 直行，1 自转；两个相位循环“直行-自转-直行-自转...”
const int SQUARE_PATTERN[] = {0, 1};
const int SQUARE_PATTERN_LEN = sizeof(SQUARE_PATTERN) / sizeof(SQUARE_PATTERN[0]);

// ================== 初始化状态机 ==================
enum class InitState
{
    IDLE,       // 未进入初始化
    STOP,       // 初始停车
    STRAIGHT,   // 初始直行
    SQUARE      // 重复直行-自转
};

InitState init_state = InitState::IDLE;
ros::Time init_phase_start;
ros::Time phase_switch_start;   // 相位切换停车开始时间
bool phase_switching = false;   // 是否处于相位切换停车阶段
int square_phase_idx = 0;       // 当前运动相位索引
int square_last_phase_idx = -1; // 上一次相位索引，用于切换检测
int square_lap_count = 0;       // 已完成直行-自转周期数

// ================== 车辆底层控制函数 ==================
void stopCar()
{
    mower_msgs::VehicleCmd cmd_msg;
    cmd_msg.turn_value = 0;
    cmd_msg.drive_value = 0;
    cmd_msg.ad_control_enable = 1;
    cmd_msg.gear_model = 0; // P档停车
    cmd_msg.mover_bool = 0;
    cmd_msg.mower_height = 11;
    cmd_msg.header.stamp = ros::Time::now();
    pub_vehicle_cmd.publish(cmd_msg);
}

void goStraight()
{
    mower_msgs::VehicleCmd cmd_msg;
    cmd_msg.turn_value = 0;
    cmd_msg.drive_value = 6000;
    cmd_msg.ad_control_enable = 1;
    cmd_msg.gear_model = 3;
    cmd_msg.mover_bool = 0;
    cmd_msg.mower_height = 11;
    cmd_msg.header.stamp = ros::Time::now();
    pub_vehicle_cmd.publish(cmd_msg);
}

// 原地自转（配合 SQUARE_TURN_DURATION 使用）
void squareTurnLeftInPlace()
{
    mower_msgs::VehicleCmd cmd_msg;
    cmd_msg.drive_value = 0;
    cmd_msg.turn_value = 3927;
    cmd_msg.ad_control_enable = 1;
    cmd_msg.gear_model = 3;
    cmd_msg.mover_bool = 0;
    cmd_msg.mower_height = 11;
    cmd_msg.header.stamp = ros::Time::now();
    pub_vehicle_cmd.publish(cmd_msg);
    ROS_INFO("Turn in place: drive_value=%d, turn_value=%d",
             cmd_msg.drive_value, cmd_msg.turn_value);
}

// 发布安全停车标志
void parking()
{
    cout << "~~~stop~~~ yolofront_flag " << yolofront_flag
         << " ,rollover_flag " << rollover_flag
         << " ,appsignal_flag " << appsignal_flag
         << " ,mower_dynamic_flag " << mower_dynamic_flag << endl;
    if (!rollover_flag && !yolofront_flag && !appsignal_flag && !mower_dynamic_flag)
    {
        stop_car.data = false;
    }
    else
    {
        cout << "~~~stop~~~ yolofront_flag " << yolofront_flag
             << " ,rollover_flag " << rollover_flag
             << " ,appsignal_flag " << appsignal_flag
             << " ,mower_dynamic_flag " << mower_dynamic_flag << endl;
        stop_car.data = true;
    }
    pub_stopflag.publish(stop_car);
}

// ================== 回调函数 ==================
void ManualDriveCallBack(const mower_msgs::Manual_Driving_Cmd &manual_drive_msgs)
{
    mower_msgs::VehicleCmd vehicle_cmd;
    vehicle_cmd.ad_control_enable = manual_drive_msgs.ad_control_enable;
    vehicle_cmd.drive_value = manual_drive_msgs.drive_value;
    vehicle_cmd.turn_value = manual_drive_msgs.turn_value;
    vehicle_cmd.mover_bool = manual_drive_msgs.mover_bool;
    vehicle_cmd.gear_model = manual_drive_msgs.gear_model;
    vehicle_cmd.mower_height = manual_drive_msgs.mow_height;
    pub_vehicle_cmd.publish(vehicle_cmd);
}

void resetInitState()
{
    init_state = InitState::IDLE;
    square_phase_idx = 0;
    square_last_phase_idx = -1;
    square_lap_count = 0;
    phase_switching = false;
}

void SingalCallBack(const std_msgs::String &singal_msgs)
{
    std::string signal = singal_msgs.data;

    if (signal == "init_location")
    {
        init_requested = true;
        init_confirmed = false;
        init_mode = false;
        ROS_INFO("Initialization requested - waiting for confirmation");

        std_msgs::Bool init_request_msg;
        init_request_msg.data = true;
        pub_init_request.publish(init_request_msg);
        return;
    }

    if (signal == "true")
    {
        if (init_requested)
        {
            init_confirmed = true;
            init_mode = true;
            init_finish = false;
            lio_static_initialized_flag = false;
            resetInitState();
            init_state = InitState::STOP;
            init_phase_start = ros::Time(0);
            ROS_INFO("Initialization confirmed - starting repeated straight-turn pattern");
        }
        else
        {
            ROS_WARN("No initialization request or position already available");
        }
        return;
    }

    if (signal == "reset")
    {
        init_requested = false;
        init_confirmed = false;
        init_mode = false;
        init_finish = false;
        resetInitState();
        return;
    }

    if (signal == "false")
    {
        init_requested = false;
        init_confirmed = false;
        init_mode = false;
        resetInitState();
        return;
    }

    if (signal == "start_execution")
    {
        if (pid_controller_pid == -1)
        {
            pid_controller_pid = fork();
            if (pid_controller_pid == 0)
            {
                setsid();
                execl("/bin/bash", "bash", "-c",
                      "source /home/nvidia/crawler_control/devel/setup.bash && "
                      "roslaunch pure_pursuit pure_pursuit.launch > /home/nvidia/crawler_control/logs/pure_pursuit.logs 2>&1 &",
                      (char *)0);
                perror("execl failed");
                exit(1);
            }
            else if (pid_controller_pid > 0)
            {
                ROS_INFO("Started pid_controller.launch with PID: %d", pid_controller_pid);
            }
            else
            {
                ROS_ERROR("Failed to fork process for roslaunch");
                pid_controller_pid = -1;
            }
        }
        else
        {
            ROS_WARN("pp_controller already running with PID: %d", pid_controller_pid);
        }
    }
    else if (signal == "stop_execution")
    {
        if (pid_controller_pid > 0)
        {
            int ret = system("rosnode kill /pure_pursuit");
            if (ret == 0)
            {
                ROS_INFO("Sent rosnode kill /pure_pursuit");
            }
            kill(-pid_controller_pid, SIGKILL);

            int status;
            for (int i = 0; i < 50; ++i)
            {
                if (waitpid(pid_controller_pid, &status, WNOHANG) == pid_controller_pid)
                    break;
                ros::Duration(0.1).sleep();
            }

            pid_controller_pid = -1;
        }
        else
        {
            ROS_WARN("No running pid_controller to stop");
        }
    }

    if (signal == "start_work")
    {
        manual_better = false;
        appsignal_flag = false;
    }
    if (singal_msgs.data == "pause")
    {
        manual_better = true;
        appsignal_flag = true;
    }
    if (singal_msgs.data == "continue")
    {
        manual_better = false;
        appsignal_flag = false;
    }
    if (singal_msgs.data == "return")
    {
        manual_better = false;
    }
    if (singal_msgs.data == "stop")
    {
        appsignal_flag = true;
    }
}

void FusionMapCallBack(const mower_msgs::Position &msgs)
{
    check_fusion = (msgs.position_state == 1 || msgs.position_state == 2 || msgs.position_state == 5) ? 1 : 0;
    has_position = check_fusion;

    if (has_position && init_mode && lio_static_initialized_flag)
    {
        init_mode = false;
        init_requested = false;
        init_confirmed = false;
        resetInitState();
        ROS_INFO("Position acquired! Exiting initialization mode.");
        ROS_INFO("Stopping vehicle after acquiring position");
        init_end_time = ros::Time::now();
    }else if(has_position&&!init_mode&&init_request_cleared)
    {
        has_position_count++;
    }
}

void ImuCallBack(const std_msgs::Bool &imu_msgs)
{
    rollover_flag = !imu_msgs.data;
}

void SpeedInfoCallBack(const util::LocalPose &v_pose)
{
    if (v_pose.vehicle_speed > 0)
    {
        rangewarn_flag = 3;
    }
    else if (v_pose.vehicle_speed < 0)
    {
        rangewarn_flag = 1;
    }
    else
    {
        rangewarn_flag = 0;
    }
}

void YoloflagfrontCallBack(const std_msgs::Bool &range_msgs)
{
    if (range_msgs.data && rangewarn_flag != 1)
    {
        yolofront_flag = true;
        ROS_WARN("!!! YOLO found dynamic obstacles !!!");
    }
    else
    {
        yolofront_flag = false;
    }
}

void MowerDynamicFlagCallBack(const std_msgs::Bool &mower_dynamicflag_msgs)
{
    if (mower_dynamicflag_msgs.data)
    {
        mower_dynamic_flag = true;
        ROS_WARN("!!! i see you people !!!");
    }
    else
    {
        mower_dynamic_flag = false;
    }
}

void liostaticinitializedCallBack(const std_msgs::Bool &lio_static_initialized_msgs)
{
    lio_static_initialized_flag = lio_static_initialized_msgs.data;
}

// ================== 初始化模式控制 ==================
// 执行重复直行-自转：STOP -> STRAIGHT -> SQUARE (循环)
void runInitMode()
{
    switch (init_state)
    {
    case InitState::STOP:
    {
        if (init_phase_start.isZero())
        {
            init_phase_start = ros::Time::now();
            ROS_INFO("[Init] Stopping for %.1f seconds...", SQUARE_INIT_STOP_DURATION);
        }
        if ((ros::Time::now() - init_phase_start).toSec() < SQUARE_INIT_STOP_DURATION)
        {
            ROS_INFO("[Init] Stopping");
            stopCar();
        }
        else
        {
            init_state = InitState::STRAIGHT;
            init_phase_start = ros::Time(0);
            ROS_INFO("[Init] Stop done, starting initial straight driving");
        }
        break;
    }
    case InitState::STRAIGHT:
    {
        if (init_phase_start.isZero())
        {
            init_phase_start = ros::Time::now();
            ROS_INFO("[Init] Driving straight for %.1f seconds...", SQUARE_INIT_STRAIGHT_DURATION);
        }
        if ((ros::Time::now() - init_phase_start).toSec() < SQUARE_INIT_STRAIGHT_DURATION)
        {
            goStraight();
        }
        else
        {
            init_state = InitState::SQUARE;
            init_phase_start = ros::Time(0);
            square_phase_idx = 0;
            square_last_phase_idx = -1;
            square_lap_count = 0;
            ROS_INFO("[Init] Straight done, starting repeated straight-turn pattern");
        }
        break;
    }
    case InitState::SQUARE:
    {
        // 检测相位切换
        if (square_phase_idx != square_last_phase_idx)
        {
            square_last_phase_idx = square_phase_idx;
            int phase = SQUARE_PATTERN[square_phase_idx % SQUARE_PATTERN_LEN];

            // 第一次进入时等待 1 秒（兼容原 RTK 重启等待逻辑）
            if (square_phase_idx == 0 && square_lap_count == 0)
            {
                phase_switching = true;
                phase_switch_start = ros::Time::now();
                ROS_INFO("[StraightTurn] Waiting 1 second before first straight...");
            }
            // 只有“直行 -> 自转”切换时停车 0.1 秒
            else if (phase == 1)
            {
                phase_switching = true;
                phase_switch_start = ros::Time::now();
                ROS_INFO("[StraightTurn] Straight -> turn: stopping for %.1f seconds...",
                         PHASE_SWITCH_STOP_DURATION);
            }
            // “自转 -> 直行”切换，不停，直接开始直行
            else
            {
                init_phase_start = ros::Time::now();
                ROS_INFO("[StraightTurn] Step %d: starting straight driving",
                         square_phase_idx / 2 + 1);
            }
        }

        // 相位切换停车阶段
        if (phase_switching)
        {
            double switch_duration = (square_phase_idx == 0 && square_lap_count == 0)
                                         ? 1.0
                                         : PHASE_SWITCH_STOP_DURATION;
            if ((ros::Time::now() - phase_switch_start).toSec() < switch_duration)
            {
                stopCar();
            }
            else
            {
                phase_switching = false;
                init_phase_start = ros::Time::now();
                int phase = SQUARE_PATTERN[square_phase_idx % SQUARE_PATTERN_LEN];
                if (phase == 0)
                {
                    ROS_INFO("[StraightTurn] Step %d: starting straight driving",
                             square_phase_idx / 2 + 1);
                }
                else
                {
                    ROS_INFO("[StraightTurn] Step %d: starting turn in place",
                             square_phase_idx / 2 + 1);
                }
            }
            break;
        }

        int phase = SQUARE_PATTERN[square_phase_idx % SQUARE_PATTERN_LEN];
        double phase_duration = (phase == 0) ? SQUARE_STRAIGHT_DURATION : SQUARE_TURN_DURATION;

        if (ros::ok() && (ros::Time::now() - init_phase_start).toSec() < phase_duration)
        {
            if (phase == 0)
            {
                goStraight();
            }
            else
            {
                squareTurnLeftInPlace();
            }
        }
        else
        {
            int finished_phase = square_phase_idx;
            square_phase_idx++;
            // 完成一个“直行-自转”周期后从头循环
            if (square_phase_idx >= SQUARE_PATTERN_LEN)
            {
                square_phase_idx = 0;
                square_lap_count++;
                ROS_INFO("[StraightTurn] Completed cycle %d (finished phase %d), repeating straight-turn...",
                         square_lap_count, finished_phase);
            }
            else
            {
                ROS_INFO("[StraightTurn] Finished phase %d", finished_phase);
            }
        }
        break;
    }
    default:
        break;
    }
}

// ================== 主函数 ==================
int main(int argc, char **argv)
{
    ros::init(argc, argv, "task_node");
    ros::NodeHandle nh;

    sub_manual_driving = nh.subscribe("/mower/manual_driving_cmd", 1, ManualDriveCallBack);
    sub_singal = nh.subscribe("/signal", 1, SingalCallBack);
    sub_fusionmap = nh.subscribe("/Mower/position", 1, FusionMapCallBack);
    sub_speedinfo = nh.subscribe("/nanobot/localpose", 1, SpeedInfoCallBack);
    sub_yolofront = nh.subscribe("/YoloSeg/yolocontrol_publisher", 1, YoloflagfrontCallBack);
    sub_imu = nh.subscribe("/Mower/car_state", 1, ImuCallBack);
    sub_mower_dynamicflag_ = nh.subscribe("/mower/stop_car2", 1, MowerDynamicFlagCallBack);
    sub_lio_static_initialized_ = nh.subscribe("/Mower/lio_static_initialized", 1, liostaticinitializedCallBack);

    pub_direct_control = nh.advertise<mower_msgs::Direct_Control>("/mower/direct_control", 1);
    pub_vehicle_cmd = nh.advertise<mower_msgs::VehicleCmd>("/vehicle/cmd", 1);
    pub_init_request = nh.advertise<std_msgs::Bool>("/init_request", 1);
    pub_stopflag = nh.advertise<std_msgs::Bool>("/mower/stop_car", 1);

    stop_car.data = false;
    double time = ros::Time::now().toSec();
    ros::Rate loop_rate(30);

    while (ros::ok())
    {
        ros::spinOnce();

        ROS_INFO("init_mode=%d, has_position=%d, init_finish=%d, lio_static_initialized_flag=%d, init_state=%d, straight_turn_cycle=%d",
                 init_mode, has_position, init_finish, lio_static_initialized_flag,
                 static_cast<int>(init_state), square_lap_count);

        if (init_finish && has_position_count >=5)
        {
            has_position_count = 0;
            std_msgs::Bool init_request_msg;
            init_request_msg.data = false;
            init_request_cleared = false;
            pub_init_request.publish(init_request_msg);
             
        }

        if (init_mode && lio_static_initialized_flag)
        {
            has_position_count = 0;
            runInitMode();
        }
        else if (has_position && !init_finish && !init_mode)
        {
            if (ros::Time::now().toSec() - init_end_time.toSec() < 5.0)
            {
                ROS_INFO("Position acquired! Exiting initialization mode.");
                ROS_INFO("Stopping vehicle after acquiring position");
                goStraight();
            }
            else
            {
                ROS_INFO("Timeout reached. Exiting initialization mode.");
                init_finish = true;
                init_request_cleared = true;
                has_position_count = 0;
            }
        }
        parking();
        loop_rate.sleep();
    }

    return 0;
}
