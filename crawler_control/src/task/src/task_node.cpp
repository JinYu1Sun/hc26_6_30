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
#include <cmath> // 添加数学函数库



using namespace std;
// pub android
ros::Publisher pub_direct_control;
ros::Publisher pub_vehicle_cmd;
ros::Publisher pub_stopflag;
ros::Publisher pub_init_request; // 发布初始化请求
// sub android
ros::Subscriber sub_manual_driving;
ros::Subscriber sub_singal;
ros::Subscriber sub_fusionmap;
ros::Subscriber sub_speedinfo;
ros::Subscriber sub_yolofront;
ros::Subscriber sub_imu;
ros::Subscriber sub_mower_dynamicflag_;
ros::Time init_start_time;    // 初始化开始时间

ros::Time figure8_phase_start; // 当前8字形阶段的开始时间
int figure8_last_count = -1;   // 上一次处理的阶段序号，用于检测阶段切换

bool manual_better = false;
std_msgs::Bool stop_car;

// 初始化程序相关变量
bool init_mode = false;       // 初始化模式标志
bool has_position = false;    // 是否有定位数据

// 初始化确认相关变量
bool init_requested = false;     // 是否请求初始化
bool init_confirmed = false;     // 是否确认初始化

// init check status
bool check_fusion = false;

// safe warning  *stop car
int rangewarn_flag = 0; // 范围停车预警
int figure8_count = 0; // 8字形初始化动作序号（原名 count，与 std::count 冲突故改名）
double figure8_period = 20.0; // "8"字形周期（秒）
pid_t pid_controller_pid = -1; // 全局变量，记录进程 PID
bool mower_dynamic_flag=false;
bool yolofront_flag = false;
bool rollover_flag = false; // 侧翻状态
bool lowpower_flag = false; // 低电量 现定义-30%
bool appsignal_flag = false; // 默认  /signal == pause | stop  1; continue  0;
bool init_finish = false;
void goStraight()
{
    mower_msgs::VehicleCmd cmd_msg;
    cmd_msg.turn_value = 0;
    cmd_msg.drive_value = 10000;
    cmd_msg.ad_control_enable = 1;
    cmd_msg.gear_model = 3;
    cmd_msg.mover_bool = 0;
    cmd_msg.mower_height = 11;
    cmd_msg.header.stamp = ros::Time::now(); // 设置时间戳
    pub_vehicle_cmd.publish(cmd_msg);
}
void controlFigure8_turnleft()
{
    mower_msgs::VehicleCmd cmd_msg;
    cmd_msg.drive_value = 10000;
    cmd_msg.turn_value = -12566; // 向左转，持续走圆
    cmd_msg.ad_control_enable = 1;           // 自动驾驶控制开启
    cmd_msg.gear_model = 3;                  // 前进档
    cmd_msg.mover_bool = 0;                  // 不割草
    cmd_msg.mower_height = 11;                // 割草高度（随便设一个）
    cmd_msg.header.stamp = ros::Time::now(); // 时间戳
    pub_vehicle_cmd.publish(cmd_msg);
    ROS_INFO("Driving in circle: drive_value=%d, turn_value=%d",
             cmd_msg.drive_value, cmd_msg.turn_value);
}

void controlFigure8_turnright()
{
    mower_msgs::VehicleCmd cmd_msg;
    cmd_msg.drive_value = 10000;
    cmd_msg.turn_value = 12566; // 向右转，持续走圆
    cmd_msg.ad_control_enable = 1;           // 自动驾驶控制开启
    cmd_msg.gear_model = 3;                  // 前进档
    cmd_msg.mover_bool = 0;                  // 不割草
    cmd_msg.mower_height = 11;                // 割草高度（随便设一个）
    cmd_msg.header.stamp = ros::Time::now(); // 时间戳
    pub_vehicle_cmd.publish(cmd_msg);
    ROS_INFO("Driving in circle: drive_value=%d, turn_value=%d",
             cmd_msg.drive_value, cmd_msg.turn_value);
}
void parking()
{
    cout << "~~~stop~~~ yolofront_flag " << yolofront_flag << " ,rollover_flag " << rollover_flag << " ,appsignal_flag " << appsignal_flag << " ,mower_dynamic_flag " << mower_dynamic_flag << endl;
    if (!rollover_flag && !yolofront_flag && !appsignal_flag && !mower_dynamic_flag)
    {
        stop_car.data = false; // all false
    }else
    {
        cout << "~~~stop~~~ yolofront_flag " << yolofront_flag << " ,rollover_flag " << rollover_flag << " ,appsignal_flag " << appsignal_flag << " ,mower_dynamic_flag " << mower_dynamic_flag << endl;
        stop_car.data = true;
    }
    pub_stopflag.publish(stop_car);
}
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

void SingalCallBack(const std_msgs::String &singal_msgs)
{
    std::string signal = singal_msgs.data;

    // 处理初始化模式信号
    if (signal == "init_location")
    {
        if (1)
        {
            init_requested = true;
            init_confirmed = false;
            init_mode = false; // 先不进入初始化模式，等待确认
            ROS_INFO("Initialization requested - waiting for confirmation");
           
            // 发布初始化请求
            std_msgs::Bool init_request_msg;
            init_request_msg.data = true;
            pub_init_request.publish(init_request_msg);
        }else
        {
            ROS_WARN("Position already available, ignoring location signal");
        }
        return;
    }

    // 处理初始化确认信号
    if (signal == "true")
    {
        if (init_requested)
        {   
            init_confirmed = true;
            init_mode = true;
            init_start_time = ros::Time::now();
            init_finish = false;
            ROS_INFO("Initialization confirmed - starting Figure8 pattern");
        }else
        {
            ROS_WARN("No initialization request or position already available");
        }
        return;
    }

    if (signal == "reset")
    {
        figure8_count = 0;
        init_requested = false;
        init_confirmed = false;
        init_mode = false;
        init_finish = false;
        return;
    }

    // 处理初始化取消信号
    if (signal == "false")
    {
        init_requested = false;
        init_confirmed = false;
        init_mode = false;
        return;
    }

    if (signal == "start_execution")
    {
        if (pid_controller_pid == -1)
        {
            pid_controller_pid = fork();
            if (pid_controller_pid == 0)
            {
                // 子进程：创建新会话，独立进程组，执行 roslaunch 命令
                setsid(); // 非常关键，保证新进程组
                execl("/bin/bash", "bash", "-c",
                  "source /home/nvidia/crawler_control/devel/setup.bash && "
                  "roslaunch pure_pursuit pure_pursuit.launch > /home/nvidia/crawler_control/logs/pure_pursuit.logs 2>&1 &",
                  (char *)0);
                // 如果 execl 失败
                perror("execl failed");
                exit(1);
            }else if (pid_controller_pid > 0)
            {
                ROS_INFO("Started pid_controller.launch with PID: %d", pid_controller_pid);
            }else
            {
                ROS_ERROR("Failed to fork process for roslaunch");
                pid_controller_pid = -1;
            }
        }else{
            ROS_WARN("pp_controller already running with PID: %d", pid_controller_pid);
        }
    }else if (signal == "stop_execution")
    {
        if (pid_controller_pid > 0)
        {
            int ret = system("rosnode kill /pure_pursuit");
            if (ret == 0)
            {
                ROS_INFO("Sent rosnode kill /pure_pursuit");
            }
        // 杀整个进程组
        kill(-pid_controller_pid, SIGKILL);

        // 循环等待确认退出
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

    // 原有的逻辑（保留）
    if (signal == "start_work")
    {
        manual_better = false;
        appsignal_flag = false;
    }
    if (singal_msgs.data == "pause")
    {
        manual_better = true;
        appsignal_flag = true; //@xhj：add in 250317
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
    //@xhj：add in 250317
    if (singal_msgs.data == "stop")
    {
        appsignal_flag = true;
    }
}
void FusionMapCallBack(const mower_msgs::Position &msgs)
{
    check_fusion = (msgs.position_state == 1 || msgs.position_state == 2 || msgs.position_state == 5) ? 1 : 0;

    // 检测定位状态
    has_position = check_fusion; //(msgs.position_state == -1);//??????

    // 如果从无定位变为有定位，且正在初始化模式，则退出初始化
    if (has_position && init_mode)
    {
        init_mode = false;
        init_requested = false;
        init_confirmed = false;
        ROS_INFO("Position acquired! Exiting initialization mode.");
        ROS_INFO("Stopping vehicle after acquiring position");
    }

    if (has_position && !init_finish)
    {
        init_finish = true;
    }
}
void ImuCallBack(const std_msgs::Bool &imu_msgs)
{
    if (!imu_msgs.data)
    {
        rollover_flag = true;
    }else
    {
        rollover_flag = false;
    }
}
void SpeedInfoCallBack(const util::LocalPose &v_pose)
{
    if (v_pose.vehicle_speed > 0)
    {
        rangewarn_flag = 3;
    }
    else if (v_pose.vehicle_speed < 0){
        rangewarn_flag = 1;
    }else
    {
        rangewarn_flag = 0;
    }
}
void YoloflagfrontCallBack(const std_msgs::Bool &range_msgs)
{
    if (range_msgs.data && rangewarn_flag != 1)
    {
        yolofront_flag = true;
        ROS_WARN("!!! YOLO found dynamic obstalces !!!");
    }else
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
    }else
    {
        mower_dynamic_flag = false;
    }
}
int main(int argc, char **argv)
{
    ros::init(argc, argv, "task_node");
    ros::NodeHandle nh;

    sub_manual_driving = nh.subscribe("/mower/manual_driving_cmd", 1, ManualDriveCallBack);
    sub_singal = nh.subscribe("/signal", 1, SingalCallBack); //@app 信号 单次下发
    sub_fusionmap = nh.subscribe("/Mower/position", 1, FusionMapCallBack);
    sub_speedinfo = nh.subscribe("/nanobot/localpose", 1, SpeedInfoCallBack);                 //@ 履带车接 定位速度反馈
    sub_yolofront = nh.subscribe("/YoloSeg/yolocontrol_publisher", 1, YoloflagfrontCallBack); //@xhj 2503
    sub_imu = nh.subscribe("/Mower/car_state", 1, ImuCallBack);
    sub_mower_dynamicflag_ = nh.subscribe("/mower/stop_car2", 1, MowerDynamicFlagCallBack);

    pub_direct_control = nh.advertise<mower_msgs::Direct_Control>("/mower/direct_control", 1);
    pub_vehicle_cmd = nh.advertise<mower_msgs::VehicleCmd>("/vehicle/cmd", 1); //@xhj 2406
    pub_init_request = nh.advertise<std_msgs::Bool>("/init_request", 1); // 添加初始化请求发布器
    pub_stopflag = nh.advertise<std_msgs::Bool>("/mower/stop_car", 1);
    
    int eight_figure[] = {0, 1, 0, 2};

    stop_car.data = false;
    double time = ros::Time::now().toSec();
    ros::Rate loop_rate(30);

    while (ros::ok())
    {
        ros::spinOnce();
        
        ROS_INFO("init_mode=%d, has_position=%d, init_finish=%d", init_mode, has_position, init_finish);

        if (init_finish)
        {
            if (ros::Time::now().toSec() - time > 5.0)
            {
                std_msgs::Bool init_request_msg;
                init_request_msg.data = false;
                pub_init_request.publish(init_request_msg);
                time = ros::Time::now().toSec();
            }
        }
        
        // 如果在初始化模式，执行"8"字形控制
        if (init_mode&&!has_position)
        {
            // 进入新阶段时记录阶段起始时间（只记录一次，不能每次循环都重置）
            if (figure8_count != figure8_last_count)
            {
                figure8_last_count = figure8_count;
                if (figure8_count == 0)
                {
                    ROS_INFO("Waiting 3 seconds before starting straight driving...");
                    ros::Duration(3.0).sleep(); // 等待RTK重启
                }
                figure8_phase_start = ros::Time::now();
                int phase = eight_figure[figure8_count % 4];
                if (phase == 0)      ROS_INFO("Starting straight driving...");
                else if (phase == 1) ROS_INFO("Starting right turn...");
                else                 ROS_INFO("Starting left turn...");
            }

            int phase = eight_figure[figure8_count % 4];
            double phase_duration = (phase == 0) ? figure8_period / 10.0 : figure8_period / 4.0;

            if (ros::ok() && (ros::Time::now() - figure8_phase_start).toSec() < phase_duration)
            {
                if (phase == 0)      goStraight();
                else if (phase == 1) controlFigure8_turnright();
                else                 controlFigure8_turnleft();
            }
            else
            {
                figure8_count++;
                ROS_INFO("Finished phase, figure8_count=%d", figure8_count);
            }
            
        }else
        {
            figure8_count = 0;
            figure8_last_count = -1;
        }
        parking(); //@xhj：2406
        loop_rate.sleep();
    }

    return 0;
}
