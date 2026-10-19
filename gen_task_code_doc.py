# -*- coding: utf-8 -*-
"""生成《task代码详解》Word 文档
用法: python gen_task_code_doc.py
产物: 代码详解/task代码详解.docx
"""
import os
from docx import Document
from docx.shared import Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.oxml.ns import qn

CN_FONT = "宋体"
EN_FONT = "Times New Roman"
SIZE = Pt(12)  # 小四


def set_run_font(run, bold=False, size=SIZE):
    run.font.name = EN_FONT
    run.font.size = size
    run.font.bold = bold
    run._element.rPr.rFonts.set(qn('w:eastAsia'), CN_FONT)


def add_para(doc, text, bold=False, size=SIZE):
    p = doc.add_paragraph()
    r = p.add_run(text)
    set_run_font(r, bold=bold, size=size)
    return p


def add_heading(doc, text, level):
    h = doc.add_heading("", level=level)
    r = h.add_run(text)
    set_run_font(r, bold=True, size=Pt(16 - level))
    r.font.color.rgb = RGBColor(0, 0, 0)
    return h


def add_code(doc, text):
    """把一段代码按行逐段写入，Consolas 10pt。"""
    for line in text.split("\n"):
        p = doc.add_paragraph()
        p.paragraph_format.space_before = Pt(0)
        p.paragraph_format.space_after = Pt(0)
        r = p.add_run(line if line else " ")
        r.font.name = "Consolas"
        r.font.size = Pt(10)
        r._element.rPr.rFonts.set(qn('w:eastAsia'), CN_FONT)


def add_table(doc, headers, rows):
    """生成一个带表头的 Word 表格，表头加粗。"""
    table = doc.add_table(rows=1 + len(rows), cols=len(headers))
    table.style = "Table Grid"
    for j, h in enumerate(headers):
        cell = table.rows[0].cells[j]
        cell.text = ""
        r = cell.paragraphs[0].add_run(h)
        set_run_font(r, bold=True, size=Pt(10.5))
    for i, row in enumerate(rows):
        for j, val in enumerate(row):
            cell = table.rows[i + 1].cells[j]
            cell.text = ""
            r = cell.paragraphs[0].add_run(val)
            set_run_font(r, size=Pt(10.5))
    return table


def add_section(doc, title, level, blocks):
    """blocks: [(intro, code, explain), ...]，code/explain 可为 None。"""
    if title:
        add_heading(doc, title, level)
    for intro, code, explain in blocks:
        if intro:
            add_para(doc, intro)
        if code:
            add_code(doc, code)
        if explain:
            add_para(doc, explain)


# ==================== 第1章 包总体介绍 ====================

def chapter_overview(doc):
    add_heading(doc, "1. 包总体介绍", 1)
    add_para(doc, "task 是履带割草机车端 crawler_control 工作空间中的一个 ROS1 功能包，"
                  "在整车系统里扮演“任务调度与安全监督”的角色。它本身不做路径跟踪控制"
                  "（那是 pure_pursuit 包的工作），而是处在 APP/云端指令与底层控制之间，"
                  "承担四类职责：")
    add_para(doc, "（1）手动驾驶指令透传：把 APP 下发的 mower_msgs/Manual_Driving_Cmd 原样转成 "
                  "mower_msgs/VehicleCmd 发到 /vehicle/cmd，交给 agv_can_bridge 上 CAN 总线；")
    add_para(doc, "（2）定位初始化（“8”字标定）：响应 /signal 话题的 init_location/true 指令，"
                  "按“停车 2 秒 → 直行 3 秒 → 周期性 8 字绕圈”的固定动作序列激励 LIO/RTK 组合定位，"
                  "直到定位状态有效后自动退出；")
    add_para(doc, "（3）自动作业进程管理：收到 start_execution 时 fork 子进程 roslaunch 启动 "
                  "pure_pursuit，收到 stop_execution 时 rosnode kill 加 SIGKILL 杀整个进程组；")
    add_para(doc, "（4）安全停车聚合：以 30Hz 把侧翻、YOLO 前向动态障碍物、割盘附近人员、"
                  "APP 暂停/停止四类危险源聚合成一个 std_msgs/Bool 发到 /mower/stop_car，"
                  "供 pure_pursuit 等下游节点执行紧急停车。")
    add_para(doc, "整个包只编译出一个可执行节点 task（节点名 task_node）。目录结构如下：", bold=True)
    add_code(doc, """crawler_control/src/task/
├── msg/
│   └── stamped_bool.msg        自定义消息（未加入构建，当前版本未使用）
├── src/
│   ├── task_node.cpp           实际编译的节点源码（517 行，本文档详解对象）
│   ├── task_node_old.cpp       历史版本（736 行，含开机自检）
│   ├── task_node260623.cpp     历史版本（913 行）
│   └── task_node260714.cpp     历史版本（931 行，自检+任务状态上报最全）
├── include/                    空目录
├── CMakeLists.txt              构建配置（只编译 src/task_node.cpp）
└── package.xml                 包清单与依赖""")
    add_para(doc, "src 目录下有三个带日期或 _old 后缀的历史版本 cpp。CMakeLists.txt 中实际编译的是 "
                  "src/task_node.cpp（add_executable(task src/task_node.cpp)），因此本文档以 task_node.cpp "
                  "为唯一详解对象，三个历史版本只在第 10 章简要对比说明，不逐行展开。")

    add_heading(doc, "1.1 整体数据流", 2)
    add_para(doc, "节点的数据流可以概括为“八入四出、一个 30Hz 主循环”：")
    add_code(doc, """  APP/手动  --/mower/manual_driving_cmd (Manual_Driving_Cmd)-+
  APP/云端  --/signal (std_msgs/String)---------------------+
  定位端    --/Mower/position (mower_msgs/Position)---------+
  定位端    --/nanobot/localpose (util/LocalPose)-----------+   task_node
  避障端    --/YoloSeg/yolocontrol_publisher (std_msgs/Bool)-+   30Hz
  车身状态  --/Mower/car_state (std_msgs/Bool)---------------+  主循环
  感知端    --/mower/stop_car2 (std_msgs/Bool)---------------+
  LIO      --/Mower/lio_static_initialized (std_msgs/Bool)--+
                                                                |
        +--> /vehicle/cmd (mower_msgs/VehicleCmd) --------------+--> agv_can_bridge -> CAN -> 底盘
        +--> /mower/stop_car (std_msgs/Bool) ------------------+--> pure_pursuit 紧急停车
        +--> /init_request (std_msgs/Bool) --------------------+--> APP 初始化请求/完成提示
        +--> /mower/direct_control (Direct_Control，仅注册) ---+""")
    add_para(doc, "此外，收到 /signal=start_execution 时节点会 fork 一个子进程执行 "
                  "“roslaunch pure_pursuit pure_pursuit.launch”，把路径跟踪节点拉起来；"
                  "收到 stop_execution 时将其整体杀掉。详见第 6 章。")


# ==================== 第2章 构建配置与包清单 ====================

def chapter_build(doc):
    add_heading(doc, "2. 构建配置与包清单", 1)

    add_heading(doc, "2.1 CMakeLists.txt", 2)
    add_para(doc, "构建文件非常精简，核心内容如下：")
    add_code(doc, """cmake_minimum_required(VERSION 3.0.2)
project(task)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
  util
  mower_msgs
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME} src/task_node.cpp)

target_link_libraries(${PROJECT_NAME}
   ${catkin_LIBRARIES}
)""")
    add_para(doc, "要点有三：")
    add_para(doc, "（1）catkin 组件依赖为 roscpp、rospy、std_msgs、util、mower_msgs。其中 util 就是工作空间里的 "
                  "minibus_msg_util 包（其 CMakeLists 中 project(util)），task 用它提供的 util/LocalPose 消息；"
                  "mower_msgs 提供 Manual_Driving_Cmd、VehicleCmd、Position 等消息。")
    add_para(doc, "（2）add_executable 只编译 src/task_node.cpp 一个文件，生成名为 task 的可执行文件；"
                  "src 目录下其余三个历史版本 cpp 不参与编译，改文件名里的日期只是人工备份。")
    add_para(doc, "（3）没有 add_message_files / generate_messages，因此 msg/stamped_bool.msg 并未真正参与消息生成，"
                  "它只是遗留在仓库里的一个消息草稿。")

    add_heading(doc, "2.2 package.xml", 2)
    add_para(doc, "package.xml 声明包名 task、维护者 hl，构建依赖 roscpp、rospy、std_msgs、util、message_generation，"
                  "运行依赖 roscpp、rospy、std_msgs。注意 package.xml 里没有声明 mower_msgs 依赖"
                  "（CMakeLists 里有），属于清单与构建脚本不同步的小瑕疵，"
                  "在工作空间整体 catkin_make 时不影响编译，但单独用 rosdep 解析依赖时会漏掉 mower_msgs。")

    add_heading(doc, "2.3 msg/stamped_bool.msg", 2)
    add_para(doc, "包内唯一的自定义消息定义如下：")
    add_code(doc, """uint32 frame_seq
time image_stamp
time process_stamp
bool data""")
    add_para(doc, "语义是“带帧序号、图像时间戳和处理时间戳的布尔量”，从字段命名看原本是为视觉检测结果"
                  "（带图像时间戳的 bool 标志）准备的。但如 2.1 所述，该消息没有加入 CMakeLists 的 "
                  "add_message_files，且实际编译的 task_node.cpp 也没有引用它，当前属于未启用的遗留文件。")


# ==================== 第3章 task_node.cpp 总体结构 ====================

def chapter_structure(doc):
    add_heading(doc, "3. task_node.cpp 总体结构", 1)
    add_para(doc, "task_node.cpp 共 517 行，采用典型的“全局发布器/订阅器 + 回调 + 主循环”单片式 ROS 节点写法，"
                  "没有类封装。整体分四段：")
    add_para(doc, "（1）头文件与全局对象（第 1~77 行）：ROS 头文件、mower_msgs 与 util 消息头文件、"
                  "进程管理头文件（unistd.h/signal.h/sys/wait.h），以及全部全局变量；")
    add_para(doc, "（2）运动指令函数（第 78~130 行）：stopCar、goStraight、controlFigure8_turnleft、"
                  "controlFigure8_turnright 四个直接发布 VehicleCmd 的函数；")
    add_para(doc, "（3）安全聚合与八个话题回调（第 131~377 行）；")
    add_para(doc, "（4）main 函数与 30Hz 主循环状态机（第 378~517 行）。")

    add_heading(doc, "3.1 全局变量一览", 2)
    add_para(doc, "节点状态全部保存在全局变量中，理解这些变量就理解了整个节点。按用途分组列表如下：")
    add_table(doc, ["变量", "类型", "用途"], [
        ["pub_direct_control", "ros::Publisher", "发布 /mower/direct_control（仅注册，当前代码未实际发布）"],
        ["pub_vehicle_cmd", "ros::Publisher", "发布 /vehicle/cmd，整车运动指令出口"],
        ["pub_stopflag", "ros::Publisher", "发布 /mower/stop_car 安全停车标志"],
        ["pub_init_request", "ros::Publisher", "发布 /init_request 初始化请求/完成提示"],
        ["sub_manual_driving 等 8 个", "ros::Subscriber", "8 路话题订阅器，详见第 7 章话题表"],
        ["init_mode", "bool", "是否处于“8 字”定位初始化模式"],
        ["init_requested / init_confirmed", "bool", "初始化请求、用户确认两段握手标志"],
        ["init_finish", "bool", "初始化全流程结束标志（退出后定期清 /init_request）"],
        ["has_position / check_fusion", "bool", "融合定位是否有效（position_state 为 1/2/5）"],
        ["lio_static_initialized_flag", "bool", "LIO 静态初始化完成标志，8 字动作的前提条件"],
        ["init_stop_phase / init_straight_phase", "bool", "初始化前置动作阶段：先停车、再直行"],
        ["init_start_time / init_end_time", "ros::Time", "初始化开始、（定位恢复）结束时刻"],
        ["init_stop_start / init_straight_start", "ros::Time", "停车/直行阶段的起始时刻（isZero 表示未启动）"],
        ["figure8_count / figure8_last_count", "int", "8 字动作阶段序号与上一次处理的序号（检测阶段切换）"],
        ["figure8_phase_start", "ros::Time", "当前 8 字阶段的起始时刻"],
        ["figure8_period", "double = 20.0", "8 字周期（秒），直行段取 period/10、转弯段取 period/6"],
        ["eight_figure[4]", "int 数组（main 内）", "动作序列 {1,0,2,0}：1 右转、0 直行、2 左转、0 直行"],
        ["stop_car", "std_msgs::Bool", "待发布的安全停车标志消息"],
        ["rollover_flag", "bool", "侧翻标志（来自 /Mower/car_state，取反）"],
        ["yolofront_flag", "bool", "YOLO 前向动态障碍物标志"],
        ["mower_dynamic_flag", "bool", "割盘附近动态人员标志（/mower/stop_car2）"],
        ["appsignal_flag", "bool", "APP 暂停/停止标志（pause/stop 置 1，start_work/continue 清 0）"],
        ["manual_better", "bool", "手动接管优先标志（pause 置 true），当前版本未参与控制输出"],
        ["lowpower_flag", "bool", "低电量标志（预留，当前代码未赋值也未使用）"],
        ["rangewarn_flag", "int", "车速方向标志：前进 3、倒车 1、静止 0；用于 YOLO 报警过滤"],
        ["pid_controller_pid", "pid_t = -1", "start_execution fork 出的子进程 PID，-1 表示未运行"],
    ])
    add_para(doc, "可以看到有两类“声明了但没真正用起来”的变量（lowpower_flag、manual_better、"
                  "pub_direct_control），它们是历史版本功能的残留，当前版本不再消费，详见第 10 章。")


# ==================== 第4章 运动指令函数 ====================

def chapter_motion(doc):
    add_heading(doc, "4. 运动指令函数", 1)
    add_para(doc, "四个运动指令函数都只干一件事：填好一条 mower_msgs/VehicleCmd 消息并发布到 /vehicle/cmd。"
                  "VehicleCmd 的关键字段（见 mower_msgs/msg/VehicleCmd.msg）：gear_model 档位"
                  "（0=P 档、3=D 前进档）、ad_control_enable 自动驾驶使能、turn_value/drive_value "
                  "转向/驱动量（int16）、mover_bool 割草开关、mower_height 割草高度。")

    add_section(doc, "4.1 stopCar() —— P 档停车", 2, [(
        "发送 P 档、零速度指令。初始化阶段的停车段和安全停车都靠它：",
        """void stopCar()
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
}""",
        "drive_value/turn_value 全 0、gear_model=0（P 档），mover_bool=0 不割草。"
        "注意 ad_control_enable 仍置 1，表示这条指令走的是自动驾驶通道，"
        "底盘单片机按 AD 指令执行停车。")])

    add_section(doc, "4.2 goStraight() —— 直行", 2, [(
        None,
        """cmd_msg.turn_value = 0;
cmd_msg.drive_value = 10000;
cmd_msg.ad_control_enable = 1;
cmd_msg.gear_model = 3;   // D 前进档
cmd_msg.mover_bool = 0;
cmd_msg.mower_height = 11;""",
        "drive_value=10000 是初始化动作采用的固定驱动量（int16 满量程附近的一个经验值），"
        "转向为 0、D 档前进。除初始化流程外，定位刚恢复后的 5 秒过渡段也持续调用它（见 6.3）。")])

    add_section(doc, "4.3 controlFigure8_turnleft() / controlFigure8_turnright() —— 定圆转弯", 2, [(
        "两个函数结构完全对称，只是 turn_value 符号相反：",
        """// 左转
cmd_msg.drive_value = 10000;
cmd_msg.turn_value = -12566; // 向左转，持续走圆
// 右转
cmd_msg.turn_value = 12566;  // 向右转，持续走圆""",
        "turn_value=±12566（约等于 4000π，是一个带比例的角速度/转角经验指令值），配合 "
        "drive_value=10000 使车辆走出一个近似固定半径的圆。8 字初始化就是“右转圆→直行→左转圆→直行”"
        "循环拼接出来的（见 6.2）。两个函数每次发布还打印一条 ROS_INFO 记录 drive/turn 值，"
        "方便在日志里核对 8 字动作是否按序执行。")])


# ==================== 第5章 话题回调函数详解 ====================

def chapter_callbacks(doc):
    add_heading(doc, "5. 话题回调函数详解", 1)

    add_section(doc, "5.1 ManualDriveCallBack —— 手动驾驶透传", 2, [(
        "订阅 /mower/manual_driving_cmd（mower_msgs/Manual_Driving_Cmd），把 APP 摇杆指令逐字段"
        "拷进 VehicleCmd 直接下发：",
        """void ManualDriveCallBack(const mower_msgs::Manual_Driving_Cmd &manual_drive_msgs)
{
    mower_msgs::VehicleCmd vehicle_cmd;
    vehicle_cmd.ad_control_enable = manual_drive_msgs.ad_control_enable;
    vehicle_cmd.drive_value = manual_drive_msgs.drive_value;
    vehicle_cmd.turn_value = manual_drive_msgs.turn_value;
    vehicle_cmd.mover_bool = manual_drive_msgs.mover_bool;
    vehicle_cmd.gear_model = manual_drive_msgs.gear_model;
    vehicle_cmd.mower_height = manual_drive_msgs.mow_height;
    pub_vehicle_cmd.publish(vehicle_cmd);
}""",
        "Manual_Driving_Cmd 的字段（uint8 ad_control_enable、uint16 turn_value、uint16 drive_value、"
        "uint8 gear_model、uint8 mover_bool、uint8 mow_height）与 VehicleCmd 一一对应，仅 mow_height "
        "改名为 mower_height。两个细节：一是透传完全无条件，不回查 appsignal_flag 或初始化状态，"
        "也就是说手动模式下该节点是纯转发器；二是转发的消息没有设置 header.stamp，时间戳为 0，"
        "下游若依赖时间戳需注意。")])

    add_section(doc, "5.2 SingalCallBack —— APP 指令协议解析（本节点最核心的回调）", 2, [(
        "订阅 /signal（std_msgs/String），这是 APP/云端对车端任务层的唯一命令通道。"
        "回调按字符串分派，共识别 11 个命令字。整体结构：",
        """void SingalCallBack(const std_msgs::String &singal_msgs)
{
    std::string signal = singal_msgs.data;
    if (signal == "init_location") { ... return; }
    if (signal == "true")          { ... return; }
    if (signal == "reset")         { ... return; }
    if (signal == "false")         { ... return; }
    if (signal == "start_execution") { ... fork/roslaunch ... }
    else if (signal == "stop_execution") { ... rosnode kill + SIGKILL ... }
    if (signal == "start_work") { manual_better = false; appsignal_flag = false; }
    if (singal_msgs.data == "pause")    { manual_better = true;  appsignal_flag = true; }
    if (singal_msgs.data == "continue") { manual_better = false; appsignal_flag = false; }
    if (singal_msgs.data == "return")   { manual_better = false; }
    if (singal_msgs.data == "stop")     { appsignal_flag = true; }
}""",
        "命令字语义见下表：")])
    add_table(doc, ["命令字", "作用", "关键状态变化"], [
        ["init_location", "请求进入定位初始化", "init_requested=true，向 /init_request 发 true 请用户确认"],
        ["true", "用户确认初始化", "init_confirmed/init_mode=true，进入“停车→直行→8 字”流程"],
        ["false", "取消初始化", "init_requested/init_confirmed/init_mode 全清 false"],
        ["reset", "复位初始化状态机", "figure8_count=0，四个初始化标志全清"],
        ["start_execution", "启动自动作业", "fork 子进程 roslaunch pure_pursuit pure_pursuit.launch"],
        ["stop_execution", "停止自动作业", "rosnode kill /pure_pursuit + kill(-pid, SIGKILL)"],
        ["start_work", "开始/恢复作业", "appsignal_flag=false（解除 APP 侧停车）"],
        ["pause", "暂停作业", "appsignal_flag=true，manual_better=true"],
        ["continue", "继续作业", "appsignal_flag=false"],
        ["return", "返航", "manual_better=false（当前版本仅此一处使用 manual_better）"],
        ["stop", "停止", "appsignal_flag=true"],
    ])
    add_para(doc, "其中 appsignal_flag 是安全聚合的一路输入（见 5.8）：pause/stop 会立刻让 "
                  "/mower/stop_car 输出 true，车辆紧急停车；start_work/continue 解除。")

    add_heading(doc, "5.2.1 初始化两段握手（init_location + true）", 3)
    add_para(doc, "收到 init_location 后的处理：")
    add_code(doc, """if (signal == "init_location")
{
    if (1)
    {
        init_requested = true;
        init_confirmed = false;
        init_mode = false; // 先不进入初始化模式，等待确认
        ROS_INFO("Initialization requested - waiting for confirmation");
        std_msgs::Bool init_request_msg;
        init_request_msg.data = true;
        pub_init_request.publish(init_request_msg);
    }else
    {
        ROS_WARN("Position already available, ignoring location signal");
    }
    return;
}""")
    add_para(doc, "这是一个“请求—确认”两段握手：第一步只置 init_requested 并通过 /init_request=true "
                  "通知 APP 弹确认框，并不真正开始动作。注意 if(1) 是硬编码的——else 分支里"
                  "“已有定位则忽略请求”的保护逻辑被永久旁路了，从注释看原本条件应是 !has_position，"
                  "属于一处被临时改成恒真的代码。")
    add_para(doc, "用户在 APP 上确认后，APP 再发 \"true\"，回调进入第二段：")
    add_code(doc, """if (signal == "true")
{
    if (init_requested)
    {
        init_confirmed = true;
        init_mode = true;
        init_start_time = ros::Time::now();
        init_finish = false;
        init_stop_phase = true;          // 初始化开始先停车5秒
        init_stop_start = ros::Time(0);
        init_straight_phase = true;      // 停车结束后直行5秒
        init_straight_start = ros::Time(0);
        lio_static_initialized_flag = false; // 进入初始化模式时，假设 LIO 已初始化
        ROS_INFO("Initialization confirmed - starting Figure8 pattern");
    }
    ...
}""")
    add_para(doc, "这里把初始化状态机复位到起点：先进入停车段、再直行段，两个阶段的计时起点都置 "
                  "ros::Time(0)（主循环里用 isZero() 判断“尚未开始计时”）。有两处注释与代码不一致"
                  "需要留意：注释写“停车 5 秒/直行 5 秒”，但主循环里实际是停车 2 秒、直行 3 秒（见 6.2）；"
                  "最后一行注释写“假设 LIO 已初始化”，代码却把 lio_static_initialized_flag 置 false——"
                  "实际效果是强制等待 LIO 重新上报静态初始化完成后才允许做 8 字动作。")

    add_heading(doc, "5.2.2 自动作业进程管理（start_execution / stop_execution）", 3)
    add_para(doc, "start_execution 用 fork+setsid+execl 三板斧把 pure_pursuit 拉成独立进程组：")
    add_code(doc, """if (pid_controller_pid == -1)
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
}""")
    add_para(doc, "设计要点：")
    add_para(doc, "（1）pid_controller_pid==-1 才允许重复启动，防止 APP 连点拉起多个 pure_pursuit；")
    add_para(doc, "（2）子进程先 setsid() 脱离本节点的会话和进程组，这样后面 stop_execution 可以按"
                  "“进程组”整体击杀，roslaunch 及其拉起的所有子节点一个不漏；")
    add_para(doc, "（3）execl 启动 bash -c，先 source devel/setup.bash 再 roslaunch，"
                  "日志重定向到 /home/nvidia/crawler_control/logs/pure_pursuit.logs（工作目录下的 "
                  "pure_pursuit.logs 即由此而来）。路径 /home/nvidia 是硬编码的，换部署机要改代码；")
    add_para(doc, "（4）execl 成功不会返回，返回即失败，perror 后 exit(1)。")
    add_para(doc, "stop_execution 则是双保险击杀：")
    add_code(doc, """if (pid_controller_pid > 0)
{
    int ret = system("rosnode kill /pure_pursuit");   // 先礼：按节点名 kill
    ...
    kill(-pid_controller_pid, SIGKILL);               // 后兵：SIGKILL 整个进程组

    int status;
    for (int i = 0; i < 50; ++i)                      // 最多等 5 秒确认退出
    {
        if (waitpid(pid_controller_pid, &status, WNOHANG) == pid_controller_pid)
            break;
        ros::Duration(0.1).sleep();
    }
    pid_controller_pid = -1;
}""")
    add_para(doc, "先 system(\"rosnode kill /pure_pursuit\") 走 ROS 的优雅退出通道，"
                  "再 kill(-pid, SIGKILL) 对负 PID（即整个进程组）发强制杀死信号——这正是启动时 "
                  "setsid() 的原因。随后用 waitpid(WNOHANG) 轮询最多 50×0.1=5 秒回收僵尸进程，"
                  "最后把 pid 复位为 -1，允许下次启动。")

    add_section(doc, "5.3 FusionMapCallBack —— 融合定位状态与初始化退出条件", 2, [(
        "订阅 /Mower/position（mower_msgs/Position），核心字段 position_state：",
        """void FusionMapCallBack(const mower_msgs::Position &msgs)
{
    check_fusion = (msgs.position_state == 1 || msgs.position_state == 2
                    || msgs.position_state == 5) ? 1 : 0;
    has_position = check_fusion;
    if (has_position && init_mode && lio_static_initialized_flag)
    {
        init_mode = false;
        init_requested = false;
        init_confirmed = false;
        ROS_INFO("Position acquired! Exiting initialization mode.");
        init_end_time = ros::Time::now();
    }
}""",
        "position_state 为 1、2、5 视为定位有效（具体枚举含义由定位端定义，通常对应 RTK 固定解/"
        "组合导航等可用状态），其余一律视为无定位。回调同时承担“8 字初始化”的自动退出判定："
        "定位恢复 且 仍在初始化模式 且 LIO 静态初始化完成，三者同时满足即清 init_mode 等标志，"
        "并记下 init_end_time——主循环随后进入 5 秒过渡段（见 6.3）。")])

    add_section(doc, "5.4 ImuCallBack —— 侧翻检测", 2, [(
        "订阅 /Mower/car_state（std_msgs/Bool）：",
        """if (!imu_msgs.data)
    rollover_flag = true;
else
    rollover_flag = false;""",
        "语义是“data 为 false 表示车身状态异常（侧翻/倾角超限）”。注意该话题名叫 car_state，"
        "变量名叫 rollover_flag，函数名叫 ImuCallBack，三者命名口径不统一，读代码时按"
        "“车身姿态安全标志，false=危险”理解即可。rollover_flag 是安全聚合的输入之一。")])

    add_section(doc, "5.5 SpeedInfoCallBack —— 车速方向判定", 2, [(
        "订阅 /nanobot/localpose（util/LocalPose，由 minibus_msg_util 包提供，包名 util），"
        "使用其中的 vehicle_speed 字段（单位 cm/s，负值表示倒车）：",
        """if (v_pose.vehicle_speed > 0)
    rangewarn_flag = 3;
else if (v_pose.vehicle_speed < 0)
    rangewarn_flag = 1;
else
    rangewarn_flag = 0;""",
        "rangewarn_flag 取值：3=前进、1=倒车、0=静止。它在当前版本唯一的消费点是 "
        "YoloflagfrontCallBack（倒车时忽略前向 YOLO 报警，见 5.6）。变量名带 warn 是历史遗留——"
        "在早期版本里它同时参与前后向测距预警，那部分逻辑已删。")])

    add_section(doc, "5.6 YoloflagfrontCallBack —— 前向动态障碍物", 2, [(
        "订阅 /YoloSeg/yolocontrol_publisher（std_msgs/Bool），YOLO 分割节点的前向动态障碍物检测结果：",
        """if (range_msgs.data && rangewarn_flag != 1)
{
    yolofront_flag = true;
    ROS_WARN("!!! YOLO found dynamic obstalces !!!");
}else
{
    yolofront_flag = false;
}""",
        "只有“检测到障碍物 且 车辆不在倒车（rangewarn_flag!=1）”才置报警。倒车时忽略前向检测是合理的："
        "此时前向是车尾运动的反方向，不构成碰撞风险，且能避免倒车过程中前摄像头看到静止背景误判。"
        "yolofront_flag 是安全聚合的输入之一。")])

    add_section(doc, "5.7 MowerDynamicFlagCallBack —— 割盘附近人员检测", 2, [(
        "订阅 /mower/stop_car2（std_msgs/Bool），感知端对割草刀盘附近动态目标（人员）的检测结果：",
        """if (mower_dynamicflag_msgs.data)
{
    mower_dynamic_flag = true;
    ROS_WARN("!!! i see you people !!!");
}else
{
    mower_dynamic_flag = false;
}""",
        "data 为 true 即置 mower_dynamic_flag，安全聚合会立即请求停车——割盘转动的场景下人员靠近"
        "是最高优先级的危险源之一。")])

    add_section(doc, "5.8 liostaticinitializedCallBack —— LIO 静态初始化标志", 2, [(
        "订阅 /Mower/lio_static_initialized（std_msgs/Bool），仅做透传：",
        """lio_static_initialized_flag = lio_static_initialized_msgs.data;""",
        "该标志是 8 字动作的两个前提之一（另一个是 init_mode）：LIO 必须先完成静态初始化，"
        "主循环才允许发动作指令，否则 8 字激励建立在未收敛的航向上没有意义。"
        "回调里被注释掉的一段（lio 初始化完成时强制 has_position=false）说明曾经用 LIO 完成事件"
        "反向重置定位状态，现已停用。")])

    add_section(doc, "5.9 parking() —— 安全停车聚合（每周期调用）", 2, [(
        "parking() 不是回调，而是主循环每周期（30Hz）必调的安全聚合函数：",
        """void parking()
{
    if (!rollover_flag && !yolofront_flag && !appsignal_flag && !mower_dynamic_flag)
    {
        stop_car.data = false; // all false
    }else
    {
        stop_car.data = true;
    }
    pub_stopflag.publish(stop_car);
}""",
        "逻辑是一目了然的四路或运算：侧翻、YOLO 前向障碍、APP 暂停/停止、割盘附近人员，"
        "任何一路为真，/mower/stop_car 就持续输出 true；全部安全才输出 false。"
        "30Hz 周期发布保证下游（pure_pursuit 的紧急停车分支）拿到的是“活的”安全状态——"
        "即使所有标志不变，消息也持续刷新，避免下游把超时当成通信中断误判。"
        "函数开头的 cout 调试打印把四路标志原样输出，是现场排查“车为什么不动”的第一手线索。")])


# ==================== 第6章 main 与 30Hz 主循环状态机 ====================

def chapter_mainloop(doc):
    add_heading(doc, "6. main() 与 30Hz 主循环状态机", 1)

    add_heading(doc, "6.1 main() 初始化", 2)
    add_para(doc, "main 的前半段是常规节点搭建：ros::init 节点名 task_node，注册 8 路订阅、4 路发布，"
                  "然后定义 8 字动作序列数组，进入 30Hz 循环：")
    add_code(doc, """ros::init(argc, argv, "task_node");
ros::NodeHandle nh;

sub_manual_driving = nh.subscribe("/mower/manual_driving_cmd", 1, ManualDriveCallBack);
sub_singal = nh.subscribe("/signal", 1, SingalCallBack); //@app 信号 单次下发
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

int eight_figure[] = {1, 0, 2, 0};
ros::Rate loop_rate(30);""")
    add_para(doc, "所有话题队列深度均为 1（只保留最新消息，控制类话题的常见做法）。"
                  "eight_figure={1,0,2,0} 是 8 字动作的一个周期模板：1=右转圆、0=直行、2=左转圆、0=直行，"
                  "主循环用 figure8_count%4 索引它循环执行。")

    add_heading(doc, "6.2 初始化状态机：停车 → 直行 → 8 字绕圈", 2)
    add_para(doc, "主循环里 if(init_mode && lio_static_initialized_flag) 分支就是定位初始化状态机，"
                  "分三个顺序阶段：")
    add_para(doc, "阶段一，停车段（init_stop_phase）：首次进入用 isZero() 记录起点，2 秒内持续 stopCar()，"
                  "到时后清标志进入直行段。代码里的 ROS_INFO 打印仍写“stopping for 2 seconds”，"
                  "而 SingalCallBack 里的注释写“停车 5 秒”——以实际代码 2.0 秒为准。", bold=False)
    add_code(doc, """if (init_stop_phase)
{
    if (init_stop_start.isZero())
        init_stop_start = ros::Time::now();
    if ((ros::Time::now() - init_stop_start).toSec() < 2.0)
        stopCar();
    else
        init_stop_phase = false;   // 进入直行段
}""")
    add_para(doc, "阶段二，直行段（init_straight_phase）：同样的计时模式，3 秒内持续 goStraight()，"
                  "结束后把 figure8_count 清 0、figure8_last_count 置 -1，进入 8 字循环。"
                  "先停车再低速直行的目的，是让 LIO 在静止中完成收尾收敛、再给一个已知的直线激励"
                  "帮助航向对准。")
    add_code(doc, """else if (init_straight_phase)
{
    if (init_straight_start.isZero())
        init_straight_start = ros::Time::now();
    if ((ros::Time::now() - init_straight_start).toSec() < 3.0)
        goStraight();
    else
    {
        init_straight_phase = false;
        figure8_count = 0;
        figure8_last_count = -1;
    }
}""")
    add_para(doc, "阶段三，8 字循环段：这是状态机的主体，代码如下：")
    add_code(doc, """// 进入新阶段时记录阶段起始时间（只记录一次，不能每次循环都重置）
if (figure8_count != figure8_last_count)
{
    figure8_last_count = figure8_count;
    if (figure8_count == 0)
    {
        ROS_INFO("Waiting 3 seconds before starting straight driving...");
        ros::Duration(1.0).sleep(); // 等待RTK重启
    }
    figure8_phase_start = ros::Time::now();
    int phase = eight_figure[figure8_count % 4];
    ...
}

int phase = eight_figure[figure8_count % 4];
double phase_duration = (phase == 0) ? figure8_period / 10.0 : figure8_period / 6.0;

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
}""")
    add_para(doc, "设计要点：")
    add_para(doc, "（1）阶段切换检测用 figure8_count != figure8_last_count，只在进入新阶段的第一个周期"
                  "记录 figure8_phase_start，避免每个循环都把计时起点刷新成“现在”导致阶段永不结束——"
                  "这是本文件最容易写错的一处，代码注释也专门强调了它；")
    add_para(doc, "（2）首次进入（count==0）先阻塞睡眠 1 秒“等待 RTK 重启”（日志文字写 3 秒，实际 1 秒），"
                  "注意这个 sleep 会阻塞整个 30Hz 循环，期间 parking() 也停一拍，但仅一次、可接受；")
    add_para(doc, "（3）阶段时长：直行段取 figure8_period/10=2.0 秒，转弯段取 figure8_period/6≈3.33 秒。"
                  "按模板 {1,0,2,0} 一个完整 8 字周期为 右转3.33s→直行2s→左转3.33s→直行2s≈10.7 秒，"
                  "不到 figure8_period=20 秒——period 只是比例基准而非真实周期时长；")
    add_para(doc, "（4）每个周期都重新发布一次 VehicleCmd（30Hz 持续指令流），满足底盘单片机对指令"
                  "超时刷新的要求。")

    add_heading(doc, "6.3 定位恢复后的 5 秒过渡段", 2)
    add_code(doc, """else if(has_position && !init_finish && !init_mode)
{
    if(ros::Time::now().toSec() - init_end_time.toSec() < 5.0)
    {
        ROS_INFO("Position acquired! Exiting initialization mode.");
        goStraight();
    }else{
        ROS_INFO("Timeout reached. Exiting initialization mode.");
        init_finish = true;
    }
}""")
    add_para(doc, "FusionMapCallBack 检测到定位恢复后只是退出 init_mode，主循环随即进入这个过渡分支："
                  "自 init_end_time 起 5 秒内继续 goStraight()（低速直行把车辆带离绕圈姿态），"
                  "5 秒后置 init_finish=true，整个初始化流程宣告结束。此后主循环每 5 秒向 "
                  "/init_request 发一次 false，通知 APP 收起初始化界面：")
    add_code(doc, """if (init_finish)
{
    if (ros::Time::now().toSec() - time > 5.0)
    {
        std_msgs::Bool init_request_msg;
        init_request_msg.data = false;
        pub_init_request.publish(init_request_msg);
        time = ros::Time::now().toSec();
    }
}""")

    add_heading(doc, "6.4 状态机的兜底分支与每周期安全聚合", 2)
    add_code(doc, """else
{
    figure8_count = 0;
    figure8_last_count = -1;
}
parking(); //@xhj：2406
loop_rate.sleep();""")
    add_para(doc, "既不在初始化、也不在过渡段时（例如初始化被 cancel、或车辆正常作业中），"
                  "每周期把 8 字计数器清零，保证下次进入初始化一定从阶段 0 重新开始。"
                  "循环末尾无条件调用 parking() 发布安全聚合标志——无论车辆处于哪个模式，"
                  "安全监督通道都不中断，这是整个节点“安全兜底”的设计核心。")

    add_heading(doc, "6.5 主循环状态图", 2)
    add_code(doc, """                    /signal: init_location
   空闲 ────────────────────────► 已请求(init_requested)
    ▲                                 │ /signal: true（确认）
    │ /signal: false/reset            ▼
    │                     ┌── 初始化模式(init_mode) ──┐
    │                     │  停车2s → 直行3s → 8字循环 │（前提：lio_static_initialized）
    │                     └────────────┬─────────────┘
    │                                  │ position_state∈{1,2,5}（FusionMapCallBack）
    │                                  ▼
    │                        过渡段：继续直行 5s
    │                                  │ 到时 init_finish=true
    └──────────────────────────────────┘
        （任意状态）parking() 每周期聚合 4 路危险源 → /mower/stop_car""")


# ==================== 第7章 话题/服务/参数/launch 汇总 ====================

def chapter_interfaces(doc):
    add_heading(doc, "7. 话题、服务、参数与 launch 汇总", 1)

    add_heading(doc, "7.1 订阅话题（8 个）", 2)
    add_table(doc, ["话题", "消息类型", "回调", "说明"], [
        ["/mower/manual_driving_cmd", "mower_msgs/Manual_Driving_Cmd", "ManualDriveCallBack", "APP 手动摇杆指令，无条件透传到 /vehicle/cmd"],
        ["/signal", "std_msgs/String", "SingalCallBack", "APP/云端命令通道，11 个命令字（见 5.2 表）"],
        ["/Mower/position", "mower_msgs/Position", "FusionMapCallBack", "融合定位，position_state∈{1,2,5} 视为有效"],
        ["/nanobot/localpose", "util/LocalPose", "SpeedInfoCallBack", "定位速度反馈，vehicle_speed>0 前进 / <0 倒车"],
        ["/YoloSeg/yolocontrol_publisher", "std_msgs/Bool", "YoloflagfrontCallBack", "YOLO 前向动态障碍物（倒车时忽略）"],
        ["/Mower/car_state", "std_msgs/Bool", "ImuCallBack", "车身姿态，false=侧翻/异常"],
        ["/mower/stop_car2", "std_msgs/Bool", "MowerDynamicFlagCallBack", "割盘附近动态人员检测"],
        ["/Mower/lio_static_initialized", "std_msgs/Bool", "liostaticinitializedCallBack", "LIO 静态初始化完成标志"],
    ])

    add_heading(doc, "7.2 发布话题（4 个）", 2)
    add_table(doc, ["话题", "消息类型", "发布位置", "说明"], [
        ["/vehicle/cmd", "mower_msgs/VehicleCmd", "stopCar/goStraight/turnleft/turnright/ManualDriveCallBack", "整车运动指令，经 agv_can_bridge 上 CAN"],
        ["/mower/stop_car", "std_msgs/Bool", "parking()（30Hz）", "四路危险源聚合的安全停车标志"],
        ["/init_request", "std_msgs/Bool", "SingalCallBack / 主循环", "true=请用户确认初始化；init_finish 后每 5s 发 false"],
        ["/mower/direct_control", "mower_msgs/Direct_Control", "（仅注册，未发布）", "历史功能残留，当前版本无任何 publish 调用"],
    ])

    add_heading(doc, "7.3 服务", 2)
    add_para(doc, "无。当前编译版本既不提供 service server，也不调用任何 service client；"
                  "与 pure_pursuit 的交互完全通过 fork/roslaunch 进程管理和 /signal、/mower/stop_car 话题完成。")

    add_heading(doc, "7.4 参数", 2)
    add_para(doc, "无。代码中没有 ros::param / nh.param 调用，所有常量（drive_value=10000、"
                  "turn_value=±12566、停车 2s、直行 3s、figure8_period=20.0、过渡段 5s、"
                  "roslaunch 路径 /home/nvidia/...）全部硬编码在源码里，调整需要改代码重新编译。"
                  "这也是本包与 pure_pursuit（launch 加载 5 个控制参数）在工程化程度上的一个差距。")

    add_heading(doc, "7.5 launch 说明", 2)
    add_para(doc, "task 包自身没有 launch 文件，节点一般由车端启动脚本直接 rosrun task task 拉起。"
                  "唯一与 launch 相关的逻辑在 SingalCallBack 里：收到 /signal=start_execution 时，"
                  "以子进程方式执行内嵌命令行——")
    add_code(doc, """source /home/nvidia/crawler_control/devel/setup.bash && \
roslaunch pure_pursuit pure_pursuit.launch > /home/nvidia/crawler_control/logs/pure_pursuit.logs 2>&1 &""")
    add_para(doc, "即按需启动 pure_pursuit 包的 pure_pursuit.launch（该 launch 加载 pure_pursuit 节点"
                  "及其控制参数，详见《pure_pursuit代码详解》），并把输出重定向到 pure_pursuit.logs。"
                  "stop_execution 时先用 rosnode kill /pure_pursuit 优雅退出，再 SIGKILL 整个进程组兜底"
                  "（机制见 5.2.2）。")


# ==================== 第8章 核心逻辑专题 ====================

def chapter_core(doc):
    add_heading(doc, "8. 核心逻辑专题", 1)

    add_heading(doc, "8.1 “8 字”定位初始化的设计动机", 2)
    add_para(doc, "履带割草机使用 LIO（激光惯性里程计）+ RTK 组合定位。系统上电或定位丢失后，"
                  "LIO 需要先静止收敛（lio_static_initialized），而组合导航的航向可观性需要"
                  "充分的转弯激励才能建立。本节点把这套流程固化成无人值守的固定动作序列："
                  "停车 2s（静止收敛）→ 直行 3s（线速度激励）→ “右转圆-直行-左转圆-直行”循环"
                  "（角速度激励，轨迹呈 8 字），同时每周期检查 position_state，一旦定位有效"
                  "（1/2/5）立即退出。整个过程由 APP 的 init_location/true 两条消息触发和确认，"
                  "无需人工上车操作。")

    add_heading(doc, "8.2 /signal 命令协议小结", 2)
    add_para(doc, "/signal 是 APP/云端到车端任务层的单字符串命令总线，协议极其简单："
                  "一条消息一个命令字，单次下发（代码注释“@app 信号 单次下发”）。"
                  "命令字分四组：初始化握手（init_location/true/false/reset）、"
                  "作业进程管理（start_execution/stop_execution）、"
                  "作业状态（start_work/pause/continue/return/stop）。"
                  "优点是实现简单、跨平台（手机端随便拼字符串）；缺点同样明显——没有应答、没有参数、"
                  "字符串拼错静默无效，且 pause/continue 这类状态命令与 start_execution 这类进程命令"
                  "混在一个通道里，全靠回调里的 if 链维持秩序。")

    add_heading(doc, "8.3 进程模型：单线程节点 + 受管子进程", 2)
    add_para(doc, "task_node 本体是单线程节点：ros::spinOnce() + 30Hz 循环，所有回调在 spinOnce 里"
                  "串行执行，随后跑状态机和 parking()。没有自定义线程、没有互斥锁——全局变量被回调和"
                  "主循环共享但不存在并发问题，这是单线程事件循环模型的红利。"
                  "唯一的并发来自 start_execution fork 出的子进程：它 setsid 后 exec bash 跑 roslaunch，"
                  "与父进程只通过 pid_controller_pid 这一个变量关联；停止时 rosnode kill + "
                  "kill(-pid,SIGKILL) + waitpid 回收。需要警惕的边界情况是：task_node 自身崩溃重启后，"
                  "pid_controller_pid 重新为 -1，旧的 pure_pursuit 进程组成为“孤儿”仍在运行，"
                  "此时 start_execution 会再拉一套——代码对此没有防护。")

    add_heading(doc, "8.4 安全聚合的优先级与盲区", 2)
    add_para(doc, "/mower/stop_car 的四路输入（侧翻、前向 YOLO、割盘人员、APP 停止）是“或”关系，"
                  "无优先级之分，任何一路触发即停车，恢复也必须四路全清——属于保守（fail-safe）设计。"
                  "盲区有三个：一是手动透传通道（ManualDriveCallBack）完全不看 stop_car，"
                  "手动模式下安全责任在驾驶员；二是低电量 lowpower_flag 只是预留变量，"
                  "电量保护尚未接入聚合；三是 rangewarn_flag 的“前进=3”分支在当前版本没有任何消费者，"
                  "前进方向的后向障碍物检测也未接入。")

    add_heading(doc, "8.5 异常分支与已知问题清单", 2)
    add_table(doc, ["位置", "现象", "影响/风险"], [
        ["SingalCallBack init_location 分支", "if(1) 硬编码，else（已有定位则忽略）永久失效", "已有定位时仍可再次请求初始化，依赖用户确认把关"],
        ["SingalCallBack \"true\" 分支", "注释“假设 LIO 已初始化”，代码实际置 lio_static_initialized_flag=false", "注释误导维护者；实际行为是强制等 LIO 重新上报"],
        ["SingalCallBack \"true\" 分支", "注释“停车5秒/直行5秒”，实际 2s/3s", "仅注释失实"],
        ["主循环 8 字首阶段", "日志写 Waiting 3 seconds，实际 sleep 1s", "仅日志失实；sleep 期间安全聚合停一拍"],
        ["ManualDriveCallBack", "转发消息未设 header.stamp", "下游若按时间戳判超时需注意"],
        ["全局变量", "lowpower_flag、manual_better、pub_direct_control 声明后基本未使用", "历史残留，无运行风险，增加阅读成本"],
        ["stop_execution", "task 重启后旧 pure_pursuit 进程组失联", "可能拉起两套 pure_pursuit，需人工 rosnode kill"],
        ["部署路径", "/home/nvidia/... 硬编码在 execl 命令里", "换用户/换机器部署必须改源码"],
    ])


# ==================== 第9章 在整车系统中的角色与交互 ====================

def chapter_system(doc):
    add_heading(doc, "9. 在整车系统中的角色与交互", 1)
    add_para(doc, "task 节点位于履带割草机车端软件栈的“任务层”：上接 APP/云平台（经 cloud_bridge/"
                  "udp_com_main 转发来的 /signal 与手动指令），下接控制层（pure_pursuit）与执行层"
                  "（agv_can_bridge → CAN → 底盘单片机），横向上订阅定位、感知、车身状态等"
                  "各类安全相关话题。整车交互关系如下：")
    add_code(doc, """   APP/云平台
      │  /signal（命令字）、/mower/manual_driving_cmd（手动）
      ▼
   ┌────────────┐   fork/roslaunch   ┌───────────────┐
   │ task_node  │ ◄────────────────► │ pure_pursuit  │  路径跟踪（task 负责拉起/杀掉）
   │ （本包）    │   /mower/stop_car  │               │
   └─────┬──────┘ ─────────────────► └───────┬───────┘
         │ /vehicle/cmd（初始化/手动指令）      │ /vehicle/cmd（自动跟踪指令）
         ▼                                    ▼
              ┌────────────────────────────────┐
              │ agv_can_bridge → CAN → 底盘单片机 │
              └────────────────────────────────┘
   输入侧：定位(/Mower/position、/nanobot/localpose、/Mower/lio_static_initialized)
          感知(/YoloSeg/yolocontrol_publisher、/mower/stop_car2)
          车身(/Mower/car_state)""")
    add_para(doc, "与主要包的交互逐条说明：")
    add_para(doc, "（1）pure_pursuit：task 是它的“监护人”——通过 start_execution/stop_execution 管理其"
                  "进程生命周期，通过 /mower/stop_car 给它喂安全停车信号。注意 /vehicle/cmd 是两个节点"
                  "共同发布的：手动/初始化阶段由 task 发，自动作业阶段由 pure_pursuit 发，"
                  "靠作业状态（而不是互斥锁）保证同一时刻只有一个发布源。")
    add_para(doc, "（2）agv_can_bridge：task 的全部运动输出（VehicleCmd）都经它转 CAN 下发底盘，"
                  "task 本身不碰 CAN。")
    add_para(doc, "（3）定位栈（fast-lio / 融合定位节点）：task 消费其 /Mower/position（定位质量）、"
                  "/Mower/lio_static_initialized（静态初始化）、/nanobot/localpose（车速方向），"
                  "并以 8 字动作反向为定位收敛提供运动激励——这是双向配合关系。")
    add_para(doc, "（4）感知栈（YOLO 分割等）：/YoloSeg/yolocontrol_publisher 与 /mower/stop_car2 两路"
                  "安全输入在 task 里聚合成统一的停车标志。")
    add_para(doc, "（5）cloud_bridge/udp_com_main：APP 与云平台的消息经桥接包落到 /signal 等话题，"
                  "task 是这些命令在车端的最终执行者。")


# ==================== 第10章 历史版本对比（一笔带过） ====================

def chapter_history(doc):
    add_heading(doc, "10. 历史版本对比", 1)
    add_para(doc, "src 目录下除实际编译的 task_node.cpp（517 行）外，还有三个历史备份，"
                  "均不参与编译，仅作演进参考：")
    add_table(doc, ["文件", "行数", "相对当前版本的差异"], [
        ["task_node_old.cpp", "736", "比当前版本多一整套开机自检：订阅 /mower/camera/state、/mower/vslam_state、"
                                      "/mower/lidar_ok、/mower/perception_ok、/mower/multimap_ok、/mower/camerargb_ok、"
                                      "/mower/seg_ok、/mower/gnss_ok、/mower/planing_ok、/mower/control_ok 共 10 路自检话题"],
        ["task_node260623.cpp", "913", "在 old 基础上继续扩展，自检项与状态上报更全"],
        ["task_node260714.cpp", "931", "历史版本中最全：额外订阅 /mower/plan_type_state、/mower/control_state、"
                                        "/mower/stop_car1（出边界），并发布 /mower/task_status（mower_msgs/TaskStatus）、"
                                        "/mower/check_result（mower_msgs/CheckResult）给 APP 做整机状态展示"],
    ])
    add_para(doc, "演进脉络很清晰：早期版本（old→260623→260714）定位是“任务管理 + 整机自检 + 状态上报”"
                  "的大节点，后来自检与状态上报功能被裁掉（或迁移到其它节点），只保留手动透传、"
                  "8 字初始化、进程管理和安全聚合四件事，形成当前 517 行的精简版。"
                  "维护时若需要恢复自检功能，可从 task_node260714.cpp 摘取对应回调与发布器。")


# ==================== 第11章 总结 ====================

def chapter_summary(doc):
    add_heading(doc, "11. 总结", 1)
    add_para(doc, "task 包是履带割草机车端软件栈中代码量不大但位置关键的“任务调度与安全监督”节点。"
                  "回顾全文，要点如下：")
    add_para(doc, "（1）四大职责：手动指令透传（Manual_Driving_Cmd→VehicleCmd）、8 字定位初始化"
                  "（停车 2s→直行 3s→右/左转圆循环，position_state∈{1,2,5} 自动退出）、"
                  "pure_pursuit 进程管理（fork+setsid 启动，rosnode kill+进程组 SIGKILL 停止）、"
                  "四路危险源 30Hz 安全聚合成 /mower/stop_car；")
    add_para(doc, "（2）接口面：8 路订阅、4 路发布、无服务、无参数、无 launch，"
                  "全部常量硬编码，/signal 单字符串命令协议共 11 个命令字；")
    add_para(doc, "（3）实现风格：单线程 spinOnce+30Hz 主循环，全局变量保存全部状态，"
                  "无锁无线程，简单直接；代价是灵活性和工程化程度有限；")
    add_para(doc, "（4）已知改进点（详见 8.5）：if(1) 旁路的定位保护、注释与代码多处不一致、"
                  "低电量保护未接入、task 重启后 pure_pursuit 进程组失联、部署路径硬编码。"
                  "这些不影响当前功能正确性，但都是下一轮维护值得处理的项。")


def main():
    doc = Document()
    chapter_overview(doc)
    chapter_build(doc)
    chapter_structure(doc)
    chapter_motion(doc)
    chapter_callbacks(doc)
    chapter_mainloop(doc)
    chapter_interfaces(doc)
    chapter_core(doc)
    chapter_system(doc)
    chapter_history(doc)
    chapter_summary(doc)
    os.makedirs("代码详解", exist_ok=True)
    doc.save("代码详解/task代码详解.docx")
    print("已生成: 代码详解/task代码详解.docx")


if __name__ == "__main__":
    main()
