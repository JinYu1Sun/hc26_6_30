#include "task.h"

using namespace std;


void goStraight()
{
    mower_msgs::VehicleCmd cmd_msg;
    cmd_msg.turn_value = 0;
    cmd_msg.drive_value = 100;
    cmd_msg.ad_control_enable = 1;
    cmd_msg.gear_model = 3;
    cmd_msg.mover_bool = 0;
    cmd_msg.mower_height = 1;
    cmd_msg.header.stamp = ros::Time::now(); // 设置时间戳
    TaskState::instance().pub_vehicle_cmd.publish(cmd_msg);
}
// "8"字形控制函数
void controlFigure8()
{
    if (!TaskState::instance().init_mode || !TaskState::instance().init_confirmed)
        return;
    mower_msgs::VehicleCmd cmd_msg;
    cmd_msg.drive_value = 100;
    cmd_msg.turn_value = -5; // 向左转，持续走圆

    // 其他参数
    cmd_msg.ad_control_enable = 1;           // 自动驾驶控制开启
    cmd_msg.gear_model = 3;                  // 前进档
    cmd_msg.mover_bool = 0;                  // 不割草
    cmd_msg.mower_height = 1;                // 割草高度（随便设一个）
    cmd_msg.header.stamp = ros::Time::now(); // 时间戳

    // 发布命令
    TaskState::instance().pub_vehicle_cmd.publish(cmd_msg);

    ROS_INFO("Driving in circle: drive_value=%d, turn_value=%d",
             cmd_msg.drive_value, cmd_msg.turn_value);
}

//** @xhj: add total control on parking **//
void parking()
{
    cout << "~~~stop~~~ flag front " << TaskState::instance().yolofront_flag << " ,rear " << TaskState::instance().rear_flag << " ,appsignal_flag " << TaskState::instance().appsignal_flag << " ,outboundary_flag " << TaskState::instance().outboundary_flag << endl;
    if (!TaskState::instance().rollover_flag && !TaskState::instance().yolofront_flag && !TaskState::instance().appsignal_flag && !TaskState::instance().outboundary_flag)
    {
        TaskState::instance().stop_car.data = false; // all false
    }
    else
    {
        
        cout << "~~~stop~~~ flag front " << TaskState::instance().yolofront_flag << " ,rear " << TaskState::instance().rear_flag << " ,appsignal_flag " << TaskState::instance().appsignal_flag << " ,outboundary_flag " << TaskState::instance().outboundary_flag<< endl;
        TaskState::instance().stop_car.data = true;
    }
    TaskState::instance().pub_stopflag.publish(TaskState::instance().stop_car);
}

void task_run()
{
    switch (TaskState::instance().taskstatus)
    {
    case Holding:
        TaskState::instance().status.task_status = "Holding";
        break;

    case Working:
        TaskState::instance().status.task_status = "Working";
        break;
    case Local_path_error:

        break;
    case Vehicle_rollover:

        break;
    case Returning:
        TaskState::instance().status.task_status = "Returning";
        break;
    case Pausing:
        TaskState::instance().status.task_status = "Pausing";
        break;
    case Self_checking:
        TaskState::instance().status.task_status = "Self_checking";
        break;
    case Self_check_fault:
        TaskState::instance().status.task_status = "Self_check_fault";
        break;
    case Obstacle_parking:
        TaskState::instance().status.task_status = "Obstacle_parking";
        break;

    case Passing_connecting_space:
        TaskState::instance().status.task_status = "Passing_connecting_space";
        break;

    default:
        break;
    }

    TaskState::instance().pub_status.publish(TaskState::instance().status);
}

// pid_controller_pid 已作为 TaskState 成员初始化，无需再次定义

void ManualDriveCallBack(const mower_msgs::Manual_Driving_Cmd &manual_drive_msgs)
{
    mower_msgs::VehicleCmd vehicle_cmd;
    vehicle_cmd.ad_control_enable = manual_drive_msgs.ad_control_enable;
    vehicle_cmd.drive_value = manual_drive_msgs.drive_value;
    vehicle_cmd.turn_value = manual_drive_msgs.turn_value;
    vehicle_cmd.mover_bool = manual_drive_msgs.mover_bool;
    vehicle_cmd.gear_model = manual_drive_msgs.gear_model;
    // vehicle_cmd.speed_cmd = manual_drive_msgs.speed_cmd;

    vehicle_cmd.mower_height = manual_drive_msgs.mow_height;
    TaskState::instance().pub_vehicle_cmd.publish(vehicle_cmd);
}

void SingalCallBack(const std_msgs::String &singal_msgs)
{
    std::string signal = singal_msgs.data;

    // 处理初始化模式信号
    if (signal == "init_location")
    {
        if (1)
        {
            TaskState::instance().init_requested = true;
            TaskState::instance().init_confirmed = false;
            TaskState::instance().init_mode = false; // 先不进入初始化模式，等待确认
            ROS_INFO("Initialization requested - waiting for confirmation");

            // 发布初始化请求
            std_msgs::Bool init_request_msg;
            init_request_msg.data = true;
            TaskState::instance().pub_init_request.publish(init_request_msg);
        }
        else
        {
            ROS_WARN("Position already available, ignoring location signal");
        }
        return;
    }

    // 处理初始化确认信号
    if (signal == "true")
    {
        TaskState::instance().straight_done=false;
        TaskState::instance().figure8_done = false;
        if (TaskState::instance().init_requested)
        {   
            TaskState::instance().init_confirmed = true;
            TaskState::instance().init_mode = true;
            TaskState::instance().init_start_time = ros::Time::now();
            TaskState::instance().init_finish = false;
            ROS_INFO("Initialization confirmed - starting Figure8 pattern");
        }
        else
        {
            ROS_WARN("No initialization request or position already available");
        }
        return;
    }

    if (signal == "reset")
    {
        TaskState::instance().straight_done=false;
        TaskState::instance().figure8_done = false;
        TaskState::instance().init_finish = false;
        return;
    }

    // 处理初始化取消信号
    if (signal == "false")
    {
        if (TaskState::instance().init_mode)
        {
            // 停止车辆
            mower_msgs::VehicleCmd cmd_msg;
            cmd_msg.turn_value = 0;
            cmd_msg.drive_value = 0;
            cmd_msg.ad_control_enable = 1;
            cmd_msg.gear_model = 0;
            cmd_msg.mover_bool = 0;
            cmd_msg.mower_height = 0;
            cmd_msg.header.stamp = ros::Time::now(); // 设置时间戳
            TaskState::instance().pub_vehicle_cmd.publish(cmd_msg);

            ROS_INFO("Initialization cancelled - stopping vehicle");
        }
        TaskState::instance().straight_done=false;
        TaskState::instance().figure8_done = false;
        

        TaskState::instance().init_requested = false;
        TaskState::instance().init_confirmed = false;
        TaskState::instance().init_mode = false;
        return;
    }

    if (signal == "start_execution")
    {
        if (TaskState::instance().pid_controller_pid == -1)
        {
            TaskState::instance().pid_controller_pid = fork();
            if (TaskState::instance().pid_controller_pid == 0)
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
            }
             else if (TaskState::instance().pid_controller_pid > 0)
             {
                 ROS_INFO("Started pid_controller.launch with PID: %d", TaskState::instance().pid_controller_pid);
             }
            else
            {
                ROS_ERROR("Failed to fork process for roslaunch");
                TaskState::instance().pid_controller_pid = -1;
            }
        }
        else
        {
            ROS_WARN("pp_controller already running with PID: %d", TaskState::instance().pid_controller_pid);
        }
    }

    else if (signal == "stop_execution")
    {
        if (TaskState::instance().pid_controller_pid > 0)
        {
            int ret = system("rosnode kill /pure_pursuit");
            if (ret == 0)
            {
                ROS_INFO("Sent rosnode kill /pure_pursuit");
            }
            // ROS_INFO("Stopping pp_controller with PGID: %d", TaskState::instance().pid_controller_pid);

            // // 杀掉整个进程组（负号表示 PGID）
            // if (kill(-TaskState::instance().pid_controller_pid, SIGKILL) == 0)
            // {
            //     ROS_INFO("SIGTERM sent to process group %d", TaskState::instance().pid_controller_pid);
            // }
            // else
            // {
            //     ROS_ERROR("Failed to send SIGTERM to process group %d", TaskState::instance().pid_controller_pid);
            // }

            // // 等待子进程退出（清理僵尸）
            // waitpid(TaskState::instance().pid_controller_pid, nullptr, WNOHANG);
            // TaskState::instance().pid_controller_pid = -1;
            // 先停车，避免 pure_pursuit 被杀后车辆乱跑
        

        // 杀整个进程组
        kill(-TaskState::instance().pid_controller_pid, SIGKILL);

        // 循环等待确认退出
        int child_status;
        for (int i = 0; i < 50; ++i)
        {
            if (waitpid(TaskState::instance().pid_controller_pid, &child_status, WNOHANG) == TaskState::instance().pid_controller_pid)
                break;
            ros::Duration(0.1).sleep();
        }

        TaskState::instance().pid_controller_pid = -1;
        }
        else
        {
            ROS_WARN("No running pid_controller to stop");
        }
    }

    // 原有的逻辑（保留）
    if (signal == "start_work")
    {
        TaskState::instance().taskstatus = Working;
        TaskState::instance().manual_better = false;

        TaskState::instance().appsignal_flag = false;
    }
    if (singal_msgs.data == "pause")
    {
        TaskState::instance().taskstatus = Pausing;
        TaskState::instance().manual_better = true;

        TaskState::instance().appsignal_flag = true; //@xhj：add in 250317
    }
    if (singal_msgs.data == "continue")
    {
        TaskState::instance().taskstatus = Working;
        TaskState::instance().manual_better = false;

        TaskState::instance().appsignal_flag = false;
    }
    if (singal_msgs.data == "return")
    {
        TaskState::instance().taskstatus = Returning;
        TaskState::instance().manual_better = false;
    }
    //@xhj：add in 250317
    if (singal_msgs.data == "stop")
    {
        TaskState::instance().appsignal_flag = true;
    }
    cout << "APP(appsignal_flag) @ " << (int)TaskState::instance().appsignal_flag << endl; // app
}

//@xhj add FusionMAP state to fault_code[2]
void FusionMapCallBack(const mower_msgs::Position &msgs)
{
    TaskState::instance().check_fusion = (msgs.position_state == 1 || msgs.position_state == 2 || msgs.position_state == 5) ? 1 : 0;
    TaskState::instance().fault.fault_code[2] = TaskState::instance().check_fusion ? "0" : "1";
    TaskState::instance().monitor.node_normal[2] = TaskState::instance().check_fusion ? 1 : 0;

    // 检测定位状态
    TaskState::instance().current_has_position = TaskState::instance().check_fusion; //(msgs.position_state == -1);//??????

    // 如果从无定位变为有定位，且正在初始化模式，则退出初始化
    if (TaskState::instance().has_position && TaskState::instance().current_has_position && TaskState::instance().init_mode)
    {
        TaskState::instance().init_mode = false;
        TaskState::instance().init_requested = false;
        TaskState::instance().init_confirmed = false;
        ROS_INFO("Position acquired! Exiting initialization mode.");

        // // 发送停车命令
        // mower_msgs::VehicleCmd cmd_msg;
        // cmd_msg.turn_value = 1500;
        // cmd_msg.drive_value = 1500;
        // cmd_msg.ad_control_enable = 1;
        // cmd_msg.gear_model = 0;
        // cmd_msg.mover_bool = 0;
        // cmd_msg.mower_height = 1;
        // cmd_msg.header.stamp = ros::Time::now(); // 设置时间戳
        // TaskState::instance().pub_vehicle_cmd.publish(cmd_msg);
        ROS_INFO("Stopping vehicle after acquiring position");
    }

    // TaskState::instance().has_position = TaskState::instance().current_has_position;
    TaskState::instance().has_position = TaskState::instance().check_fusion;
    if (TaskState::instance().has_position && !TaskState::instance().init_finish)
    {
        TaskState::instance().init_finish = true;
        // std_msgs::Bool init_request_msg;
        // init_request_msg.data = false;
        // TaskState::instance().pub_init_request.publish(init_request_msg);
    }
}

void ControlStateCallBack(const mower_msgs::ControlState &controlsate_msgs)
{
    if (controlsate_msgs.control_state)
    {
        printf("finished mowing\n");
    }
    else
    {
        printf("Unfinished mowing\n");
    }
}

void PlanTypeCallBack(const mower_msgs::PlanType &plantype_msgs)
{
    if (!TaskState::instance().manual_better) // 停车置true, 其它情况进循环
    {
        if (!TaskState::instance().warn_2)
        {
            if (plantype_msgs.plan_type_state == 0)
            {
                TaskState::instance().taskstatus = Working;
            }
            if (plantype_msgs.plan_type_state == 1)
            {
                TaskState::instance().taskstatus = Passing_connecting_space;
            }
        }
    }
}

void VslamCallBack(const mower_msgs::VslamState &vslam_msgs)
{
    TaskState::instance().check_vslam = vslam_msgs.is_vslam_ok;
    TaskState::instance().fault.fault_code[3] = TaskState::instance().check_vslam ? "0" : "1";
}

void CameraRGBCallBack(const mower_msgs::CamerargbState &camerargb_msgs)
{
    TaskState::instance().check_camerargb = camerargb_msgs.is_camerargb_ok;
    TaskState::instance().fault.fault_code[4] = TaskState::instance().check_camerargb ? "0" : "1";
}

void SegCallBack(const mower_msgs::SegState &seglok_msgs)
{
    TaskState::instance().check_seg = seglok_msgs.is_seg_ok;
    TaskState::instance().fault.fault_code[5] = TaskState::instance().check_seg ? "0" : "1";
}

void LidarCallBack(const mower_msgs::LidarSelfDtect &lidar_msgs)
{
    TaskState::instance().check_lidar = lidar_msgs.is_lidar_ok;
    TaskState::instance().fault.fault_code[6] = TaskState::instance().check_lidar ? "0" : "1";
}

void PerceptionCallBack(const mower_msgs::PerceptionSelfDetect &perception_msgs)
{
    TaskState::instance().check_perception = perception_msgs.is_perception_ok;
    TaskState::instance().fault.fault_code[7] = TaskState::instance().check_perception ? "0" : "1";
}

void GnssCallBack(const mower_msgs::GnssOK &gnssok_msgs)
{
    TaskState::instance().check_gnss = gnssok_msgs.is_gnss_ok;
    TaskState::instance().fault.fault_code[8] = TaskState::instance().check_gnss ? "0" : "1";
}

void PlanCheckCallBack(const mower_msgs::PlaningOK &planok_msgs)
{
    TaskState::instance().check_planok = planok_msgs.is_planing_ok;
    TaskState::instance().fault.fault_code[9] = TaskState::instance().check_planok ? "0" : "1";
}

void CtrlCheckCallBack(const mower_msgs::ControlOk &controlok_msgs)
{
    TaskState::instance().check_controlok = controlok_msgs.is_control_ok;
    TaskState::instance().fault.fault_code[10] = TaskState::instance().check_controlok ? "0" : "1";
    TaskState::instance().monitor.node_normal[10] = TaskState::instance().check_controlok ? 1 : 0;
}

void MultiMapCallBack(const mower_msgs::MultiMapSelfDetect &multimap_msgs)
{
    TaskState::instance().check_multimap = multimap_msgs.is_multi_map_ok;
    TaskState::instance().fault.fault_code[11] = TaskState::instance().check_multimap ? "0" : "1";
}

void CamCallBack(const mower_msgs::CameraState &cam_msgs)
{
    TaskState::instance().check_cam = cam_msgs.is_camera_ok;
    TaskState::instance().fault.fault_code[12] = TaskState::instance().check_cam ? "0" : "1";
}

void ImuCallBack(const std_msgs::Bool &imu_msgs)
{
    if (!imu_msgs.data)
    {
        TaskState::instance().rollover_flag = true;
        cout << "rollover  stop / / rollover stop / /rollover stop//" << endl;
    }
    else
    {
        TaskState::instance().rollover_flag = false;
    }
}

void OutBoundaryCallBack(const std_msgs::Bool &msgs)
{
    if (msgs.data)
    {
        TaskState::instance().outboundary_flag = true;
    }
    else
    {
        TaskState::instance().outboundary_flag = false;
    }
}

//** @xhj: add range warning **//
void SpeedInfoCallBack(const util::LocalPose &v_pose)
{
    if (v_pose.vehicle_speed > 0)
    {
        TaskState::instance().rangewarn_flag = 3;
    }
    else if (v_pose.vehicle_speed < 0)
    {
        TaskState::instance().rangewarn_flag = 1;
    }
    else
    {
        TaskState::instance().rangewarn_flag = 0;
    }
}
//@xhj 2503  yolov8
void YoloflagfrontCallBack(const std_msgs::Bool &range_msgs)
{
    // cout<<"front(Yolo_flag)&(V_dir) @ "<<(int)range_msgs.data<<" &&  "<<TaskState::instance().rangewarn_flag<<endl; //履带车停障判断
    if (range_msgs.data && TaskState::instance().rangewarn_flag != 1)
    {
        TaskState::instance().yolofront_flag = true;
        ROS_WARN("!!! YOLO found dynamic obstalces !!!");
    }
    else
    {
        TaskState::instance().yolofront_flag = false;
    }
}

bool checkAndResetNodes(const mower_msgs::Monitor &msgs)
{
    bool all_true = true;
    TaskState::instance().monitor = msgs;
    TaskState::instance().monitor.node_normal[2] = 1; // 测试
    TaskState::instance().pub_monitor.publish(TaskState::instance().monitor);
    bool test = TaskState::instance().monitor.node_normal[2];
    // 将所有标志位 置为false
    for (int i = 0; i < 13; ++i)
    {
        TaskState::instance().monitor.node_normal[i] = false;
    }
    // 如果所有节点都为true，执行相应操作
    if (1)
    { //  all_true
        return true;
    }
    else
    {
        return false;
    }
}
// @xhj check
// 定义标志位对应的字符串表
const std::string TaskState::node_names[13] = {
    "Battery_soc",     // 无，去除
    "Chassis_warning", // 无
    "FusionMap",
    "Vslam",
    "Camerargb",
    "Seg",
    "Lidar",
    "Perception",
    "Gnss",
    "Planning",
    "Control",
    "Multimap",
    "Camera"};
// 检查所有标志位是否为true，并处理不符合的标志位
int main(int argc, char **argv)
{
    ros::init(argc, argv, "task_node");
    ros::NodeHandle nh;
    ROS_INFO_STREAM("task start");

    // TaskState::instance().sub_manual = nh.subscribe("/mower/manual_set", 1, ManualCallBack);
    TaskState::instance().sub_manual_driving = nh.subscribe("/mower/manual_driving_cmd", 1, ManualDriveCallBack);
    TaskState::instance().sub_singal = nh.subscribe("/signal", 1, SingalCallBack); //@app 信号 单次下发

    // check sub
    ros::Subscriber sub_fusionmap = nh.subscribe("/Mower/position", 1, FusionMapCallBack);
    TaskState::instance().sub_camera = nh.subscribe("/mower/camera/state", 1, CamCallBack);
    TaskState::instance().sub_vslam = nh.subscribe("/mower/vslam_state", 1, VslamCallBack);
    TaskState::instance().sub_lidar = nh.subscribe("/mower/lidar_ok", 1, LidarCallBack);
    TaskState::instance().sub_perception = nh.subscribe("/mower/perception_ok", 1, PerceptionCallBack);
    TaskState::instance().sub_multimap = nh.subscribe("/mower/multimap_ok", 1, MultiMapCallBack);

    TaskState::instance().sub_camerargb = nh.subscribe("/mower/camerargb_ok", 1, CameraRGBCallBack);
    TaskState::instance().sub_seg = nh.subscribe("/mower/seg_ok", 1, SegCallBack);
    TaskState::instance().sub_gnss = nh.subscribe("/mower/gnss_ok", 1, GnssCallBack);
    TaskState::instance().sub_planche = nh.subscribe("/mower/planing_ok", 1, PlanCheckCallBack);
    TaskState::instance().sub_ctrlche = nh.subscribe("/mower/control_ok", 1, CtrlCheckCallBack);
    // sub
    TaskState::instance().sub_plantype = nh.subscribe("/mower/plan_type_state", 1, PlanTypeCallBack);
    TaskState::instance().sub_controlstate = nh.subscribe("/mower/control_state", 1, ControlStateCallBack);

    ros::Subscriber sub_speedinfo = nh.subscribe("/nanobot/localpose", 1, SpeedInfoCallBack);                 //@ 履带车接 定位速度反馈
    ros::Subscriber sub_yolofront = nh.subscribe("/YoloSeg/yolocontrol_publisher", 1, YoloflagfrontCallBack); //@xhj 2503
    ros::Subscriber sub_imu = nh.subscribe("/Mower/car_state", 1, ImuCallBack);
    ros::Subscriber sub_outboundary = nh.subscribe("/mower/stop_car1", 1, OutBoundaryCallBack);
    // android->control
    TaskState::instance().pub_direct_control = nh.advertise<mower_msgs::Direct_Control>("/mower/direct_control", 1);
    TaskState::instance().pub_vehicle_cmd = nh.advertise<mower_msgs::VehicleCmd>("/vehicle/cmd", 1); //@xhj 2406
    // pub_vehicle_control = nh.advertise<util::VehicleControl>("/vehicle/cmd", 1);  // 修改为正确的話題
    TaskState::instance().pub_init_request = nh.advertise<std_msgs::Bool>("/init_request", 1); // 添加初始化请求发布器
    // task <-> Android
    TaskState::instance().pub_status = nh.advertise<mower_msgs::TaskStatus>("/mower/task_status", 1);
    TaskState::instance().pub_result = nh.advertise<mower_msgs::CheckResult>("/mower/check_result", 1);
    // TaskState::instance().pub_fault = nh.advertise<mower_msgs::Fault_Code>("/mower/fault_code", 1);
    TaskState::instance().pub_monitor = nh.advertise<mower_msgs::Monitor>("/mower/monitor", 1); //@xhj

    TaskState::instance().pub_stopflag = nh.advertise<std_msgs::Bool>("/mower/stop_car", 1);

    // 初始化所有监控节点为false(异常状态)，节点正常为true
    for (int i = 0; i < 13; ++i)
    {
        TaskState::instance().monitor.node_normal[i] = false;
    }

    TaskState::instance().stop_car.data = false;
    

    ros::Time straight_start_time;
    ros::Time second_straight_start_time;

    

    double time = ros::Time::now().toSec();
    ros::Rate loop_rate(30);
    while (ros::ok())
    {
        ros::spinOnce();

        if (TaskState::instance().init_finish)
        {
            if (ros::Time::now().toSec() - time > 5.0)
            {
                std_msgs::Bool init_request_msg;
                init_request_msg.data = false;
                TaskState::instance().pub_init_request.publish(init_request_msg);
                time = ros::Time::now().toSec();
            }
        }

        // 如果在初始化模式，执行"8"字形控制
        if (TaskState::instance().init_mode)
        {
            // 第一阶段：初始直行 3 秒
            if (!TaskState::instance().straight_done)
            {
                if (!straight_active)
                {
                    straight_active = true;
                    // 先sleep 3秒
                    ROS_INFO("Waiting 3 seconds before starting straight driving...");
                    ros::Duration(3.0).sleep(); // 等待RTK重启
                    straight_start_time = ros::Time::now();
                }

                double elapsed = (ros::Time::now() - straight_start_time).toSec();
                if (elapsed < 3.0) // 直行3s
                {
                    goStraight();
                    ROS_INFO("Straight driving for %.2f seconds", elapsed);
                }
                else
                {
                    // 停车后，结束直行阶段
                    mower_msgs::VehicleCmd cmd_msg;
                    cmd_msg.turn_value = 0;
                    cmd_msg.drive_value = 0;
                    cmd_msg.ad_control_enable = 1;
                    cmd_msg.gear_model = 0;
                    cmd_msg.mover_bool = 0;
                    cmd_msg.mower_height = 1;
                    cmd_msg.header.stamp = ros::Time::now(); // 设置时间戳
                    TaskState::instance().pub_vehicle_cmd.publish(cmd_msg);

                    TaskState::instance().straight_done = true;
                    straight_active = false;
                }

                loop_rate.sleep();
                continue;
            }

            // 第二阶段："8"字形控制直到定位完成
            if (!TaskState::instance().figure8_done)
            {
                if (!TaskState::instance().current_has_position)
                {
                     controlFigure8();
                    loop_rate.sleep();
                    continue;
                }
                else
                {
                    TaskState::instance().figure8_done = true;
                }
            }

            // 第三阶段：如果 TaskState::instance().check_fusion 为 false，再直行 3 秒（只执行一次）
            if (!TaskState::instance().check_fusion)
            {
                if (!second_straight_active)
                {
                    second_straight_start_time = ros::Time::now();
                    second_straight_active = true;
                }

                double elapsed2 = (ros::Time::now() - second_straight_start_time).toSec();
                if (elapsed2 < 4.0)
                {
                     goStraight();
                }
                else
                {
                    mower_msgs::VehicleCmd cmd_msg;
                    cmd_msg.turn_value = 0;
                    cmd_msg.drive_value = 0;
                    cmd_msg.ad_control_enable = 1;
                    cmd_msg.gear_model = 0;
                    cmd_msg.mover_bool = 0;
                    cmd_msg.mower_height = 1;
                    cmd_msg.header.stamp = ros::Time::now(); // 设置时间戳
                    TaskState::instance().pub_vehicle_cmd.publish(cmd_msg);

                    // 可选：不再执行这一段
                    second_straight_active = false;
                    // TaskState::instance().check_fusion = true; // 强行跳过后续逻辑（也可以用 done 标志位）
                }

                loop_rate.sleep();
                continue;
            }
        }

        mower_msgs::CheckResult result;
        static auto t0 = ros::Time::now();
        ros::Time t1 = ros::Time::now();
        double check_time = (t1 - t0).toSec();
        static bool check_once = 0;
        if (check_time < 5.0)
        { // 20.0
            // if(TaskState::instance().check_flag){
            //   result.is_checkresult_ok = true;
            //   TaskState::instance().pub_result.publish(result);
            // }
            if (std::fmod(check_time, 2.0) < 0.1)
            {
                ROS_INFO("~check~ time : %.2f ===> %d ", check_time, TaskState::instance().check_flag);
            }
            TaskState::instance().taskstatus = Self_checking;
        }
        else if (!check_once)
        {
            if (!TaskState::instance().check_flag)
            {
                ROS_INFO("~mointor~ check ==>Fail");
                TaskState::instance().taskstatus = Self_check_fault;
                result.is_checkresult_ok = false;
            }
            else
            {
                ROS_INFO("~mointor~ check ok");
                TaskState::instance().taskstatus = Holding;
                result.is_checkresult_ok = true;
            }
            check_once = 1;
            task_run();
            TaskState::instance().pub_status.publish(TaskState::instance().status);
            TaskState::instance().pub_result.publish(result);
        }

        task_run();
        parking(); //@xhj：2406
        TaskState::instance().check_flag = checkAndResetNodes(TaskState::instance().monitor);
        loop_rate.sleep();
    }

    return 0;
}
