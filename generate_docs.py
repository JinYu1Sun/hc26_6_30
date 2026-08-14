# -*- coding: utf-8 -*-
"""生成 task_node 与 pure_pursuit 的 Word 逻辑说明文档与 Excel 详情表，每秒显示完成度"""
import time, datetime, sys
from docx import Document
from docx.shared import Pt, RGBColor, Inches
from docx.enum.text import WD_ALIGN_PARAGRAPH
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter

TOTAL_STEPS = 10
_step = [0]

def progress(msg):
    _step[0] += 1
    pct = min(100, int(_step[0] * 100 / TOTAL_STEPS))
    ts = datetime.datetime.now().strftime("%H:%M:%S")
    print(f"[{ts}] 完成度 {pct:3d}% | {msg}", flush=True)
    time.sleep(1.0)

# ---------------- 公共样式 ----------------
HEADER_FILL = PatternFill("solid", fgColor="4472C4")
HEADER_FONT = Font(name="微软雅黑", size=11, bold=True, color="FFFFFF")
CELL_FONT = Font(name="微软雅黑", size=10)
THIN = Border(*[Side(style="thin", color="B0B0B0")] * 4)
WRAP = Alignment(vertical="top", wrap_text=True)

def style_sheet(ws, widths):
    for i, w in enumerate(widths, 1):
        ws.column_dimensions[get_column_letter(i)].width = w
    for cell in ws[1]:
        cell.fill = HEADER_FILL
        cell.font = HEADER_FONT
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.border = THIN
    for row in ws.iter_rows(min_row=2):
        for cell in row:
            cell.font = CELL_FONT
            cell.alignment = WRAP
            cell.border = THIN
    ws.freeze_panes = "A2"

def fill_sheet(ws, headers, rows, widths):
    ws.append(headers)
    for r in rows:
        ws.append(r)
    style_sheet(ws, widths)

def doc_title(doc, text):
    p = doc.add_heading(text, level=0)
    p.alignment = WD_ALIGN_PARAGRAPH.CENTER

def doc_h(doc, text, lv=1):
    doc.add_heading(text, level=lv)

def doc_p(doc, text, bold=False):
    p = doc.add_paragraph()
    run = p.add_run(text)
    run.font.size = Pt(11)
    run.bold = bold
    return p

def doc_bullets(doc, items):
    for it in items:
        p = doc.add_paragraph(style="List Bullet")
        run = p.add_run(it)
        run.font.size = Pt(11)

def doc_table(doc, headers, rows, widths=None):
    t = doc.add_table(rows=1, cols=len(headers))
    t.style = "Light Grid Accent 1"
    for i, h in enumerate(headers):
        t.rows[0].cells[i].text = h
    for r in rows:
        cells = t.add_row().cells
        for i, v in enumerate(r):
            cells[i].text = str(v)
    if widths:
        for i, w in enumerate(widths):
            for row in t.rows:
                row.cells[i].width = Inches(w)

# ============================================================
# 1. task_node Word 文档
# ============================================================
progress("开始生成 task_node 代码逻辑说明 Word 文档")

doc = Document()
doc_title(doc, "task_node.cpp 代码逻辑详细说明")
doc_p(doc, "源文件：crawler_control/src/task/src/task_node.cpp（734 行）")
doc_p(doc, "节点名：task_node    主循环频率：30Hz")

doc_h(doc, "一、节点功能概述")
doc_p(doc, "task_node 是割草机系统的任务调度与整机状态管理节点，承担四大核心职责：")
doc_bullets(doc, [
    "整机自检：监听 13 个子系统（电池、底盘、融合定位、VSLAM、RGB相机、分割、激光雷达、感知、GNSS、规划、控制、多地图、相机）的健康状态，开机后 5 秒内完成自检并向安卓端上报结果。",
    "任务状态管理：维护任务状态机（Holding/Working/Pausing/Returning/Self_checking 等），通过 /mower/task_status 话题上报给安卓 APP。",
    "安全停车监控：综合 YOLO 动态障碍物、IMU 侧翻、APP 暂停/停止信号，通过 /mower/stop_car 话题向控制层下发停车标志。",
    "初始化定位（8字形）：在无定位时按“直行-右转-直行-左转”的 8 字轨迹激励 RTK/融合定位收敛，并在获得定位后自动退出。",
    "pure_pursuit 进程管理：响应 start_execution/stop_execution 信号，通过 fork+execl 启动 roslaunch pure_pursuit，并通过 rosnode kill + 进程组 SIGKILL 停止。",
])

doc_h(doc, "二、全局变量与标志位")
doc_p(doc, "节点大量使用全局变量保存状态，主要分为五组：")
doc_table(doc,
    ["分组", "代表变量", "说明"],
    [
        ["自检状态组", "check_gnss / check_fusion / check_cam / check_vslam / check_lidar / check_perception / check_multimap / check_battery / check_camerargb / check_planok / check_controlok / check_seg", "各子系统自检通过标志，由各传感器回调函数刷新，并写入 fault.fault_code[] 与 monitor.node_normal[]"],
        ["初始化组", "init_mode / has_position / init_requested / init_confirmed / init_finish / init_start_time", "控制 8 字形初始化流程的状态标志"],
        ["8字形参数", "figure8_radius=1.5, figure8_period=20.0", "8 字形半径（米）与周期（秒）；实际直行用时 period/2=10s，转弯用时 period/4=5s"],
        ["安全预警组", "rangewarn_flag / front_flag / rear_flag / yolofront_flag / rollover_flag / lowpower_flag / appsignal_flag", "各类停车触发源；parking() 据此计算 stop_car"],
        ["进程管理", "pid_controller_pid = -1", "记录 fork 出的 pure_pursuit 进程组 PID，-1 表示未运行"],
    ],
    widths=[1.2, 2.8, 3.2])

doc_h(doc, "三、枚举定义")
doc_table(doc,
    ["枚举", "取值", "含义"],
    [
        ["tasks（任务状态机）", "Holding / Working / Local_path_error / Vehicle_rollover / Returning / Pausing / Self_checking / Self_check_fault / Obstacle_parking / Passing_connecting_space",
         "整机任务状态；task_run() 将其映射为字符串通过 /mower/task_status 发布"],
        ["eight_figure_", "gostraight / turnright / turnleft", "8 字形初始化子状态（实际代码中 main 内使用 int 数组 eight_figure[]={0,1,0,2} 表示动作序列：直行→右转→直行→左转）"],
    ],
    widths=[1.5, 3.4, 2.3])

doc_h(doc, "四、回调函数逻辑")
doc_h(doc, "4.1 SingalCallBack（/signal 信号处理，核心调度入口）", 2)
doc_p(doc, "解析安卓端下发的字符串指令，行为如下表：")
doc_table(doc,
    ["指令", "行为"],
    [
        ["init_location", "置 init_requested=true，发布 /init_request=true，等待用户确认"],
        ["true", "若已请求，则 init_confirmed=true、init_mode=true，记录开始时间，开始 8 字形"],
        ["false", "取消初始化请求，清空 init_requested/init_confirmed/init_mode"],
        ["reset", "全部初始化标志复位（注意：此分支引用的 count 为 main 内局部变量，存在作用域问题）"],
        ["start_execution", "若 pid_controller_pid==-1，fork 子进程，setsid 独立进程组后 execl 执行 roslaunch pure_pursuit pure_pursuit.launch（日志重定向到 logs/pure_pursuit.logs）"],
        ["stop_execution", "先 system(\"rosnode kill /pure_pursuit\")，再 kill(-pid, SIGKILL) 杀整个进程组，waitpid 最多等 5 秒确认退出，PID 复位为 -1"],
        ["start_work / continue", "taskstatus=Working，manual_better=false，appsignal_flag=false"],
        ["pause", "taskstatus=Pausing，manual_better=true，appsignal_flag=true（触发停车）"],
        ["stop", "appsignal_flag=true（触发停车）"],
        ["return", "taskstatus=Returning"],
    ],
    widths=[1.6, 5.6])

doc_h(doc, "4.2 传感器自检回调（11 个）", 2)
doc_p(doc, "FusionMapCallBack、VslamCallBack、CameraRGBCallBack、SegCallBack、LidarCallBack、PerceptionCallBack、GnssCallBack、PlanCheckCallBack、CtrlCheckCallBack、MultiMapCallBack、CamCallBack 结构一致：从消息中取健康位写入对应 check_xxx 标志，并把 fault.fault_code[索引] 置 \"0\"/\"1\"。其中：")
doc_bullets(doc, [
    "FusionMapCallBack（/Mower/position）：position_state 为 1/2/5 视为定位正常；同时维护 has_position——若初始化过程中获得定位，则自动退出初始化模式并置 init_finish=true。",
    "fault_code 索引与子系统对应：2=融合定位，3=VSLAM，4=RGB相机，5=分割，6=激光雷达，7=感知，8=GNSS，9=规划，10=控制，11=多地图，12=相机（与 node_names[13] 表对应）。",
])

doc_h(doc, "4.3 安全相关回调", 2)
doc_bullets(doc, [
    "ImuCallBack（/Mower/car_state）：data 为 false 时判定侧翻 rollover_flag=true。",
    "SpeedInfoCallBack（/nanobot/localpose）：根据车速符号设置 rangewarn_flag：>0 为 3（前进），<0 为 1（后退），=0 为 0。",
    "YoloflagfrontCallBack（/YoloSeg/yolocontrol_publisher）：YOLO 检测到动态障碍物且非后退状态时置 yolofront_flag=true 并告警。",
    "ManualDriveCallBack（/mower/manual_driving_cmd）：手动遥控指令直接透传转发到 /vehicle/cmd。",
])

doc_h(doc, "五、关键函数逻辑")
doc_h(doc, "5.1 parking() 停车决策", 2)
doc_p(doc, "每循环周期执行：仅当 rollover_flag、yolofront_flag、appsignal_flag 三者全为 false 时 stop_car.data=false（放行），否则置 true（停车），结果通过 /mower/stop_car 发布给 pure_pursuit 等控制节点。")
doc_h(doc, "5.2 checkAndResetNodes() 节点监控汇总", 2)
doc_p(doc, "将入参 Monitor 消息转发到 /mower/monitor，检查 node_normal 数组是否全为 true，随后把全部标志清零（等待下一轮各回调重新置位）。注意：当前代码 if(1) 恒为真，始终返回 true，all_true 判定实际未生效。")
doc_h(doc, "5.3 task_run() 任务状态发布", 2)
doc_p(doc, "把 taskstatus 枚举映射为字符串写入 status.task_status 并通过 /mower/task_status 发布（Local_path_error 与 Vehicle_rollover 两个分支为空，不更新字符串）。")
doc_h(doc, "5.4 8字形控制函数组", 2)
doc_bullets(doc, [
    "goStraight()：drive_value=100，turn_value=0，D档直行。",
    "controlFigure8_turnright()：drive_value=10000（即 1m/s），turn_value=12566（约 1.2566 rad/s，右转画圆）。",
    "controlFigure8_turnleft()：同上，turn_value=-12566 左转。",
])

doc_h(doc, "六、主循环（main，30Hz）执行流程")
doc_p(doc, "1) 初始化阶段：注册全部订阅/发布器，monitor.node_normal 全部置 false，进入 30Hz 循环。每轮执行：")
doc_bullets(doc, [
    "ros::spinOnce() 处理回调；打印 init_mode/has_position/init_finish。",
    "若 init_finish：每 5 秒发布一次 /init_request=false（通知 APP 初始化结束）。",
    "若 init_mode 且尚无定位：按 eight_figure[]={0,1,0,2} 序列执行 8 字轨迹——直行 10s → 右转 5s → 直行 10s → 左转 5s（内部 while 循环阻塞式发指令，期间不 sleep 也不 spinOnce），count 递增；非初始化模式时 count 清零。",
    "开机 5 秒自检窗口：check_time<5s 期间 taskstatus=Self_checking；超过 5 秒且仅执行一次——check_flag 为假则 Self_check_fault 并上报 is_checkresult_ok=false，否则进入 Holding 并上报自检通过。",
    "每轮末尾依次调用 task_run()（发布任务状态）、parking()（停车决策）、check_flag=checkAndResetNodes(monitor)（汇总并清零节点状态），最后 loop_rate.sleep()。",
])

doc_h(doc, "七、已知问题与注意事项")
doc_bullets(doc, [
    "SingalCallBack 的 \"reset\" 分支使用 count 变量，但 count 定义在 main 内部（局部变量），该分支实际无法编译/行为未定义。",
    "main 中引用了 turn_left_done / straight_done / turn_right_done / turn_left_active / straight_active / turn_right_active 等变量，但文件中未声明，属于遗留代码问题。",
    "checkAndResetNodes 中 if(1) 恒真，自检失败判定逻辑被绕过；sub_camera 等部分订阅器变量声明后未充分使用（局部与全局混用）。",
    "int eight_figure[] 数组与枚举 eight_figure_ 同名（作用域不同），可读性差。",
    "8 字形 while 循环内未调用 ros::spinOnce()，初始化期间回调暂停处理，定位获得后要等当前段结束才能退出。",
])

doc.save("task_node代码逻辑说明.docx")
progress("task_node Word 文档已保存")

# ============================================================
# 2. task_node Excel
# ============================================================
progress("开始生成 task_node 变量/话题/函数 Excel")

wb = Workbook()

ws = wb.active
ws.title = "全局变量"
fill_sheet(ws,
    ["变量名", "类型", "初始值", "用途说明", "定义行"],
    [
        ["status", "mower_msgs::TaskStatus", "-", "任务状态消息，发布到 /mower/task_status", 44],
        ["fault", "mower_msgs::Fault_Code", "-", "故障码消息，fault_code[2..12] 由各自检回调填写", 45],
        ["monitor", "mower_msgs::Monitor", "-", "节点监控消息，node_normal[13] 表示各节点是否正常", 46],
        ["check_flag", "bool", "false", "整机自检通过标志（由 checkAndResetNodes 返回）", 78],
        ["manual_better", "bool", "false", "手动优先/暂停辅助标志，pause 时置 true", 79],
        ["stop_car", "std_msgs::Bool", "false", "停车标志消息，由 parking() 发布到 /mower/stop_car", 80],
        ["init_mode", "bool", "false", "初始化（8字形）模式标志", 83],
        ["has_position", "bool", "false", "是否已有融合定位", 84],
        ["init_start_time", "ros::Time", "-", "初始化开始时间", 85],
        ["figure8_radius", "double", "1.5", "8 字形半径（米）（仅声明，实际控制用固定速度）", 86],
        ["figure8_period", "double", "20.0", "8 字形周期（秒）；直行=period/2，转弯=period/4", 87],
        ["init_requested", "bool", "false", "是否收到 init_location 请求", 90],
        ["init_confirmed", "bool", "false", "用户是否确认初始化（收到 \"true\"）", 91],
        ["check_gnss / check_fusion / check_cam / check_vslam / check_lidar / check_perception / check_multimap / check_battery / check_warn / check_camerargb / check_planok / check_controlok / check_seg", "bool", "false（check_camerargb 为 true）", "13 路子系统自检状态", 95-107],
        ["rangewarn_flag", "int", "0", "车速方向标志：3=前进，1=后退，0=静止", 110],
        ["front_flag / rear_flag", "bool", "false", "前/后方预警标志（预留）", 111-112],
        ["yolofront_flag", "bool", "false", "YOLO 前方动态障碍物标志，触发停车", 113],
        ["rollover_flag", "bool", "false", "IMU 侧翻标志，触发停车", 114],
        ["lowpower_flag", "bool", "false", "低电量标志（<30%，预留）", 116],
        ["appsignal_flag", "bool", "false", "APP 暂停/停止信号标志，触发停车", 117],
        ["init_finish", "bool", "false", "初始化完成（已获得定位）", 118],
        ["straight_start_time / turn_left_time / turn_right_time", "ros::Time", "-", "8 字形各段计时起点", 121-123],
        ["node_names[13]", "const std::string", "见代码", "13 个子系统名称表，与 fault_code/node_normal 索引对应", 129],
        ["pid_controller_pid", "pid_t", "-1", "pure_pursuit 进程组 PID，-1=未运行", 286],
        ["count", "int（main 内局部）", "0", "8 字形动作序列索引（0..3 循环递增）", 621],
        ["eight_figure[4]", "int（main 内局部）", "{0,1,0,2}", "8 字形动作序列：直行→右转→直行→左转", 620],
    ],
    [34, 16, 18, 52, 10])

ws = wb.create_sheet("枚举")
fill_sheet(ws,
    ["枚举名", "取值", "数值", "含义"],
    [
        ["tasks", "Holding", 0, "待机/保持"],
        ["tasks", "Working", 1, "工作状态"],
        ["tasks", "Local_path_error", 2, "绕障路径异常（task_run 中未处理）"],
        ["tasks", "Vehicle_rollover", 3, "侧翻状态（task_run 中未处理）"],
        ["tasks", "Returning", 4, "返回"],
        ["tasks", "Pausing", 5, "暂停工作"],
        ["tasks", "Self_checking", 6, "自检状态"],
        ["tasks", "Self_check_fault", 7, "自检失败"],
        ["tasks", "Obstacle_parking", 8, "遇障停车"],
        ["tasks", "Passing_connecting_space", 9, "通过连通区"],
        ["eight_figure_", "gostraight", 0, "8字形：直行段"],
        ["eight_figure_", "turnright", 1, "8字形：右转段"],
        ["eight_figure_", "turnleft", 2, "8字形：左转段"],
    ],
    [16, 26, 8, 40])

ws = wb.create_sheet("订阅话题")
fill_sheet(ws,
    ["话题名", "消息类型", "回调函数", "用途说明"],
    [
        ["/mower/manual_driving_cmd", "mower_msgs::Manual_Driving_Cmd", "ManualDriveCallBack", "手动遥控指令，透传转发到 /vehicle/cmd"],
        ["/signal", "std_msgs::String", "SingalCallBack", "APP 指令总入口（初始化/启停/任务切换）"],
        ["/Mower/position", "mower_msgs::Position", "FusionMapCallBack", "融合定位状态自检 + has_position 维护"],
        ["/mower/camera/state", "mower_msgs::CameraState", "CamCallBack", "相机自检"],
        ["/mower/vslam_state", "mower_msgs::VslamState", "VslamCallBack", "VSLAM 自检"],
        ["/mower/lidar_ok", "mower_msgs::LidarSelfDtect", "LidarCallBack", "激光雷达自检"],
        ["/mower/perception_ok", "mower_msgs::PerceptionSelfDetect", "PerceptionCallBack", "感知自检"],
        ["/mower/multimap_ok", "mower_msgs::MultiMapSelfDetect", "MultiMapCallBack", "多地图自检"],
        ["/mower/camerargb_ok", "mower_msgs::CamerargbState", "CameraRGBCallBack", "RGB 相机自检"],
        ["/mower/seg_ok", "mower_msgs::SegState", "SegCallBack", "分割模块自检"],
        ["/mower/gnss_ok", "mower_msgs::GnssOK", "GnssCallBack", "GNSS 自检"],
        ["/mower/planing_ok", "mower_msgs::PlaningOK", "PlanCheckCallBack", "规划模块自检"],
        ["/mower/control_ok", "mower_msgs::ControlOk", "CtrlCheckCallBack", "控制模块自检"],
        ["/nanobot/localpose", "util::LocalPose", "SpeedInfoCallBack", "车速反馈，设置 rangewarn_flag"],
        ["/YoloSeg/yolocontrol_publisher", "std_msgs::Bool", "YoloflagfrontCallBack", "YOLO 动态障碍物预警"],
        ["/Mower/car_state", "std_msgs::Bool", "ImuCallBack", "IMU 侧翻检测"],
    ],
    [30, 30, 24, 46])

ws = wb.create_sheet("发布话题")
fill_sheet(ws,
    ["话题名", "消息类型", "发布器变量", "发布时机/用途"],
    [
        ["/mower/task_status", "mower_msgs::TaskStatus", "pub_status", "task_run() 每循环发布任务状态字符串"],
        ["/mower/check_result", "mower_msgs::CheckResult", "pub_result", "开机 5 秒自检结束后发布一次自检结果"],
        ["/mower/monitor", "mower_msgs::Monitor", "pub_monitor", "checkAndResetNodes() 转发节点健康数组"],
        ["/mower/stop_car", "std_msgs::Bool", "pub_stopflag", "parking() 每循环发布停车标志"],
        ["/mower/direct_control", "mower_msgs::Direct_Control", "pub_direct_control", "预留（声明后未实际发布）"],
        ["/vehicle/cmd", "mower_msgs::VehicleCmd", "pub_vehicle_cmd", "8字形控制指令与手动遥控透传"],
        ["/init_request", "std_msgs::Bool", "pub_init_request", "初始化请求/结束通知 APP"],
    ],
    [26, 28, 20, 52])

ws = wb.create_sheet("函数")
fill_sheet(ws,
    ["函数名", "返回类型", "参数", "功能说明", "代码行"],
    [
        ["goStraight", "void", "无", "8字形直行：drive=100, turn=0, D档", "165-176"],
        ["controlFigure8_turnleft", "void", "无", "8字形左转：drive=10000, turn=-12566", "177-192"],
        ["controlFigure8_turnright", "void", "无", "8字形右转：drive=10000, turn=12566", "194-209"],
        ["parking", "void", "无", "综合侧翻/YOLO/APP信号判定 stop_car 并发布", "210-223"],
        ["checkAndResetNodes", "bool", "const mower_msgs::Monitor&", "转发 monitor、判定全 true 后清零标志（注意 if(1) 恒真）", "227-246"],
        ["task_run", "void", "无", "任务状态枚举→字符串，发布 /mower/task_status", "248-284"],
        ["ManualDriveCallBack", "void", "Manual_Driving_Cmd", "手动遥控透传到 /vehicle/cmd", "288-298"],
        ["SingalCallBack", "void", "std_msgs::String", "APP 信号调度：初始化/进程启停/任务切换", "300-447"],
        ["FusionMapCallBack", "void", "mower_msgs::Position", "融合定位自检 + has_position 维护 + 退出初始化", "449-472"],
        ["VslamCallBack", "void", "mower_msgs::VslamState", "VSLAM 自检，fault_code[3]", "473-477"],
        ["CameraRGBCallBack", "void", "mower_msgs::CamerargbState", "RGB 相机自检，fault_code[4]", "479-483"],
        ["SegCallBack", "void", "mower_msgs::SegState", "分割自检，fault_code[5]", "485-489"],
        ["LidarCallBack", "void", "mower_msgs::LidarSelfDtect", "激光雷达自检，fault_code[6]", "491-495"],
        ["PerceptionCallBack", "void", "mower_msgs::PerceptionSelfDetect", "感知自检，fault_code[7]", "497-501"],
        ["GnssCallBack", "void", "mower_msgs::GnssOK", "GNSS 自检，fault_code[8]", "503-507"],
        ["PlanCheckCallBack", "void", "mower_msgs::PlaningOK", "规划自检，fault_code[9]", "509-513"],
        ["CtrlCheckCallBack", "void", "mower_msgs::ControlOk", "控制自检，fault_code[10] + node_normal[10]", "515-520"],
        ["MultiMapCallBack", "void", "mower_msgs::MultiMapSelfDetect", "多地图自检，fault_code[11]", "522-526"],
        ["CamCallBack", "void", "mower_msgs::CameraState", "相机自检，fault_code[12]", "528-532"],
        ["ImuCallBack", "void", "std_msgs::Bool", "侧翻检测，维护 rollover_flag", "534-543"],
        ["SpeedInfoCallBack", "void", "util::LocalPose", "按车速方向设置 rangewarn_flag", "557-569"],
        ["YoloflagfrontCallBack", "void", "std_msgs::Bool", "YOLO 动态障碍物预警，维护 yolofront_flag", "571-581"],
        ["main", "int", "argc/argv", "节点初始化 + 30Hz 主循环（8字形/自检/状态发布/停车决策）", "583-734"],
    ],
    [26, 10, 26, 56, 12])

ws = wb.create_sheet("信号指令表")
fill_sheet(ws,
    ["/signal 指令", "触发行为", "影响的标志位"],
    [
        ["init_location", "请求初始化定位，发布 /init_request=true", "init_requested=true"],
        ["true", "确认初始化，开始 8 字形", "init_confirmed=true, init_mode=true"],
        ["false", "取消初始化", "init_requested/init_confirmed/init_mode=false"],
        ["reset", "复位初始化状态（含 count，存在作用域问题）", "全部 init 标志=false"],
        ["start_execution", "fork+execl 启动 roslaunch pure_pursuit", "pid_controller_pid=子进程PID"],
        ["stop_execution", "rosnode kill + 进程组 SIGKILL 停止 pure_pursuit", "pid_controller_pid=-1"],
        ["start_work", "进入工作状态", "taskstatus=Working, manual_better=false, appsignal_flag=false"],
        ["pause", "暂停，触发停车", "taskstatus=Pausing, manual_better=true, appsignal_flag=true"],
        ["continue", "继续工作", "taskstatus=Working, appsignal_flag=false"],
        ["return", "返回", "taskstatus=Returning"],
        ["stop", "停车", "appsignal_flag=true"],
    ],
    [18, 52, 40])

wb.save("task_node变量话题函数详情.xlsx")
progress("task_node Excel 已保存")

# ============================================================
# 3. pure_pursuit Word 文档
# ============================================================
progress("开始生成 pure_pursuit 代码逻辑说明 Word 文档")

doc = Document()
doc_title(doc, "pure_pursuit.cpp 代码逻辑详细说明")
doc_p(doc, "源文件：crawler_control/src/pure_pursuit/src/pure_pursuit.cpp（553 行）")
doc_p(doc, "头文件：crawler_control/src/pure_pursuit/include/pure_pursuit/pure_pursuit.h")
doc_p(doc, "注意：本文档对应 pure_pursuit 包，而非 pure_pursuit_syq 包。主循环频率：20Hz")

doc_h(doc, "一、节点功能概述")
doc_p(doc, "pure_pursuit 是割草机的路径跟踪控制节点，实现经典 Pure Pursuit（纯跟踪）算法：接收规划层发布的五维局部路径（x, y, heading, gear, speed），结合车辆实时位姿，计算线速度/角速度指令下发底盘。节点以“状态机 + 预瞄跟踪”的方式工作，支持：")
doc_bullets(doc, [
    "路径点坐标系转换：把全局路径点实时转换到车辆坐标系。",
    "动态预瞄距离：预瞄距离 = min_lookahead_distance + v_expect × 0.7，随期望速度自适应。",
    "掉头/转弯点（gear==2）特殊处理：到达转弯点后停车原地旋转，对准方向后继续跟线。",
    "多级安全降级：位置丢失、路径丢失、紧急避障（倒车/停车）、普通预警低速、出边界低速。",
    "卡死脱困：跟线卡住（100 周期无进展）时强制前进 20 周期；转弯卡住（200 周期）时后退 20 周期。",
    "转弯完成记录与查询服务：/turn_completed 服务供其他节点查询某转弯点是否已完成。",
    "割草机构控制：透传 /signal 的 open/close/高度指令到底盘。",
])

doc_h(doc, "二、ROS 参数")
doc_table(doc,
    ["参数名", "默认值", "含义"],
    [
        ["pure_pursuit/v_min", "0.2 m/s", "最小线速度（跟踪速度下限）"],
        ["pure_pursuit/w_max", "0.15 rad/s", "跟线时最大角速度"],
        ["pure_pursuit/min_lookahead_distance", "1.5 m", "最小预瞄距离"],
        ["pure_pursuit/regulated_min_radius", "2.0 m", "最小转弯半径：半径小于此值时按比例降低线速度，防止角速度过大"],
        ["pure_pursuit/low_speed", "0.3 m/s", "低速模式（预警/出边界）下的期望速度"],
    ],
    widths=[2.6, 1.2, 3.4])

doc_h(doc, "三、数据结构（头文件）")
doc_bullets(doc, [
    "struct waypoint：路径点，含全局坐标 global_x/global_y/global_yaw、车体系坐标 local_x/local_y/local_yaw、gear（档位/点类型，2=转弯点）、speed（期望速度），重载了 operator==。",
    "struct Twist：linear / angular 双成员速度指令。",
    "enum class RunStateValue : uint8_t { Stop, Follow, Turn, Reverse }：运行状态机四态——停车、跟线、原地转向、倒车。",
])

doc_h(doc, "四、回调函数逻辑")
doc_table(doc,
    ["回调", "订阅话题", "逻辑"],
    [
        ["positionCallback", "/Mower/position", "更新车辆位姿与时间戳；遍历最新五维路径，把每个点经旋转平移变换到车辆坐标系（local_x = dx·cosθ + dy·sinθ，local_y = -dx·sinθ + dy·cosθ，local_yaw 归一化），存入 local_waypoints_（互斥锁保护）"],
        ["localPathCallback", "/lawn_mower/global_path", "更新路径时间戳；路径内容变化时更新 last_local_path_ 并打印“五维路径更新”"],
        ["stopSignalCallback", "/mower/stop_car", "收到 true 置 stop_car_（原子量），publishCommand 会强制输出零速+P档"],
        ["avoidstateCallback", "/lawn_mower/avoid_state", "更新避障状态 avoid_state_（2=普通预警低速，3=紧急停车，4=紧急倒车）"],
        ["OutBoundaryCallBack", "/lawn_mower/out_of_bounds", "更新出边界标志 outboundary_flag_（1 时低速）"],
        ["signalCallback", "/signal", "解析字符串：数字 3~11 → 割草高度；open/close → 割盘开关；stop/mowing_finished/reset → 全量复位（状态、路径、计数器、转弯记录，五把锁全上）"],
        ["turnCompletedCallback", "服务 /turn_completed", "查询 (x,y) 是否在已完成转弯点队列中（容差 1e-4），返回 res.turn_finish"],
    ],
    widths=[1.9, 2.0, 3.3])

doc_h(doc, "五、核心算法函数")
doc_h(doc, "5.1 findNearestPoint —— 最近点搜索", 2)
doc_p(doc, "线性扫描车体系路径点，返回距车辆欧氏距离最近的点索引；空路径返回 -1（主循环据此进入 Stop）。")
doc_h(doc, "5.2 calculateLookaheadPoint —— 预瞄点计算", 2)
doc_bullets(doc, [
    "预瞄距离 lookahead_distance = min_lookahead_distance_ + v_expect × 0.7（速度越快预瞄越远）。",
    "按 0.25m 点距换算预瞄点数 point_num = ceil(lookahead_distance / 0.25)，预瞄点 = 最近点索引 + point_num，越界时取末点。",
    "从最近点到预瞄点之间若存在 gear==2 的转弯点，则优先把该转弯点作为预瞄点（保证先到转弯点掉头）。",
])
doc_h(doc, "5.3 calculate_PurePursuit —— 纯跟踪速度解算", 2)
doc_bullets(doc, [
    "曲率公式：curvature = 2·local_y / (local_x² + local_y²)，即纯跟踪几何关系 κ = 2y/Ld²。",
    "Ld² < 1e-4 防除零返回零速；Ld < 0.25m 时线速度压到 ≤0.1m/s（接近预瞄点减速）。",
    "转弯半径 radius = |1/κ| 小于 regulated_min_radius_（2m）时，线速度按 radius/2.0 比例缩减，间接限制角速度。",
    "线速度下限 v_min_；角速度 ω = v·κ，按符号限幅在 ±w_max_。",
    "草地补偿：|ω| 在 0.04~0.09 之间时附加 0.05 同号偏置，避免小角速度在草地上转不动。",
])
doc_h(doc, "5.4 calculate_rotate_vel —— 原地转向角速度分级", 2)
doc_bullets(doc, [
    "角度差在 180°±5° 附近时强制按 π 处理，防止左右符号横跳。",
    "|Δyaw| > 45°：±0.5 rad/s；15°~45°：±0.4 rad/s；≤15°：±0.3 rad/s（角度越小转得越慢，平稳对准）。",
])
doc_h(doc, "5.5 publishCommand —— 指令发布", 2)
doc_bullets(doc, [
    "线速度、角速度 ×10000 转整型（cm/s、μrad/s 量级）填入 drive_value/turn_value；底盘为左手系，turn_value 取反。",
    "速度与角速度均接近 0 时挂 P 档（p_Gear），否则 D 档（D_Gear）；ad_control_enable=1。",
    "携带割盘开关 mover_bool_cfg_ 与割草高度 mower_height_cfg_（互斥锁读取）。",
    "stop_car_ 为真时强制 drive=0、turn=0、P 档。",
    "发布到 /vehicle/cmd 并打印当前线/角速度。",
])

doc_h(doc, "六、状态机 state_machine_run（20Hz 主循环）")
doc_p(doc, "每周期先 spinOnce 处理回调，然后按以下顺序执行：")
doc_h(doc, "6.1 前置安全检查（goto STATEMACHINE 跳转）", 2)
doc_bullets(doc, [
    "位置信息超过 1 秒未更新 → Stop；五维路径超过 3 秒未更新 → Stop。",
    "avoid_state_==4 → Reverse（紧急倒车）；avoid_state_==3 → Stop（紧急停车）。",
    "avoid_state_==2 或 outboundary_flag_==1 → low_speed_flag_=true（本周期低速 0.3m/s）。",
    "拷贝 local_waypoints_ 快照（互斥锁）。",
])
doc_h(doc, "6.2 Turn 状态前置处理", 2)
doc_p(doc, "若当前处于 Turn：在新路径快照中找回原预瞄点（按全局坐标+gear==2 匹配），刷新其 local_yaw 用于对向判断，然后直接进状态机（不再重新选预瞄点，防止转弯过程中预瞄点漂移）。")
doc_h(doc, "6.3 Follow 前置处理：最近点与期望速度", 2)
doc_bullets(doc, [
    "findNearestPoint 找最近点；若最近点是已完成的转弯点（查 turn_completed_points_）：是末点则到终点停车，否则索引+1 跳过。",
    "期望速度：低速标志优先（0.3）；最近点为转弯点时 0.25；否则取该点的 speed 字段。",
    "calculateLookaheadPoint 计算预瞄点并存入 lookahead_waypoint_。",
])
doc_h(doc, "6.4 四态状态机", 2)
doc_table(doc,
    ["状态", "行为"],
    [
        ["Stop", "发布零速指令后立刻切回 Follow（即“停车一拍”语义，阻塞条件解除后自动恢复跟踪）"],
        ["Follow", "预瞄距离 < 0.05m 或预瞄点落到车身后（local_x<0.03）且为转弯点 → 停车切 Turn；否则 calculate_PurePursuit 解算速度。卡死检测：同一预瞄点且距离缩减 < 0.05m 时 follow_count_ 累加，超过 100（约5秒）强制以 1.0m/s 前进 20 个周期脱困"],
        ["Turn", "对向完成判定：|local_yaw| < 5°（M_PI_4/9）→ 若为转弯点则记录进 turn_completed_points_（只保留最近3个）→ 停车回 Follow。turn_count_ > 200（约10秒）未完成 → 后退 20 周期脱困。正常时 linear=0、angular=calculate_rotate_vel(local_yaw) 原地旋转"],
        ["Reverse", "以 -0.15m/s 匀速倒车（紧急避障状态，状态解除后下周期由 Stop 自动回 Follow）"],
    ],
    widths=[1.0, 6.2])

doc_h(doc, "七、并发与安全设计")
doc_bullets(doc, [
    "五把互斥锁分管：local_path_mutex_、local_waypoints_mutex_、turn_completed_mutex_、avoid_state_mutex_、outboundary_mutex_、mover_mutex_（复位时全部加锁，避免中途状态不一致）。",
    "running_state_ / low_speed_flag_ / stop_car_ 使用 std::atomic，读写免锁。",
    "turn_completed_points_ 为 deque，最多保留 3 个（代码注释写 4 个，实际 >3 即弹出队首）。",
])

doc_h(doc, "八、注意事项")
doc_bullets(doc, [
    "头文件注释中 last_lookahead_distance_ 标注为“期望速度”是笔误，实际为上一周期预瞄距离（卡死检测用）。",
    "Stop 状态停车后立即回 Follow，因此路径/位置丢失表现为“停一拍再尝试跟踪”，丢失期间 v_expect=0、无路径点时会被最近点检查再次拦回 Stop。",
    "goto STATEMACHINE 集中跳转，安全分支与状态机解耦，但阅读时需注意 multiple entry。",
])

doc.save("pure_pursuit代码逻辑说明.docx")
progress("pure_pursuit Word 文档已保存")

# ============================================================
# 4. pure_pursuit Excel
# ============================================================
progress("开始生成 pure_pursuit 变量/话题/函数 Excel")

wb = Workbook()

ws = wb.active
ws.title = "成员变量"
fill_sheet(ws,
    ["变量名", "类型", "初始值", "线程安全", "用途说明"],
    [
        ["nh_", "ros::NodeHandle", "-", "-", "节点句柄"],
        ["running_state_", "std::atomic<RunStateValue>", "Follow", "原子", "运行状态机：Stop/Follow/Turn/Reverse"],
        ["car_position_", "util::Position", "-", "positionCallback 单写", "小车实时位姿"],
        ["last_pos_time_", "ros::Time", "-", "同上", "位姿最后更新时间，>1s 判定丢失"],
        ["last_local_path_", "util::LocalPath", "point_num=0", "local_path_mutex_", "最新五维局部路径"],
        ["last_local_path_time_", "ros::Time", "-", "local_path_mutex_", "路径最后更新时间，>3s 判定丢失"],
        ["local_waypoints_", "std::vector<waypoint>", "空", "local_waypoints_mutex_", "转换到车体系的路径点"],
        ["turn_completed_points_", "std::deque<waypoint>", "空", "turn_completed_mutex_", "已完成转弯的点（最多保留3个）"],
        ["turn_count_", "uint", "0", "主循环独占", "Turn 状态周期计数，>200 触发后退脱困"],
        ["follow_count_", "uint", "0", "主循环独占", "跟线卡死计数，>100 触发前进脱困"],
        ["min_lookahead_distance_", "float", "1.5（参数）", "只读", "最小预瞄距离（m）"],
        ["lookahead_waypoint_", "waypoint", "-", "主循环独占", "当前预瞄点"],
        ["last_lookahead_waypoint_", "waypoint", "-", "主循环独占", "上周期预瞄点（卡死检测）"],
        ["lookahead_distance_", "float", "-", "主循环独占", "当前预瞄距离"],
        ["last_lookahead_distance_", "float", "-", "主循环独占", "上周期预瞄距离（头文件注释误写为“期望速度”）"],
        ["v_min_", "float", "0.2（参数）", "只读", "最小线速度 m/s"],
        ["w_max_", "float", "0.15（参数）", "只读", "跟线最大角速度 rad/s"],
        ["regulated_min_radius_", "float", "2.0（参数）", "只读", "限速最小转弯半径 m"],
        ["low_speed_flag_", "std::atomic<bool>", "false", "原子", "低速模式标志"],
        ["low_speed_", "float", "0.3（参数）", "只读", "低速模式期望速度 m/s"],
        ["avoid_state_", "uint8_t", "0", "avoid_state_mutex_", "避障状态：2=低速，3=停车，4=倒车"],
        ["stop_car_", "std::atomic<bool>", "false", "原子", "停车信号标志"],
        ["outboundary_flag_", "bool", "0", "outboundary_mutex_", "出边界标志"],
        ["mover_bool_cfg_", "int", "0", "mover_mutex_", "割盘开关（1=open，0=close）"],
        ["mower_height_cfg_", "int", "0", "mover_mutex_", "割草高度（3~11）"],
        ["sub_position_ 等 6 个", "ros::Subscriber", "-", "-", "见“订阅话题”表"],
        ["pub_command_", "ros::Publisher", "-", "-", "发布 /vehicle/cmd"],
        ["turn_completed_srv_", "ros::ServiceServer", "-", "-", "/turn_completed 服务"],
    ],
    [26, 26, 16, 22, 48])

ws = wb.create_sheet("结构体与枚举")
fill_sheet(ws,
    ["名称", "类型", "成员/取值", "说明"],
    [
        ["waypoint", "struct", "global_x/global_y/global_yaw; local_x/local_y/local_yaw; gear; speed", "路径点：全局+车体系双坐标，gear=2 表示转弯点；重载 operator=="],
        ["Twist", "struct", "double linear; double angular", "速度指令"],
        ["RunStateValue", "enum class : uint8_t", "Stop / Follow / Turn / Reverse", "运行状态机四态"],
    ],
    [18, 22, 52, 46])

ws = wb.create_sheet("订阅话题")
fill_sheet(ws,
    ["话题名", "消息类型", "回调函数", "用途说明"],
    [
        ["/Mower/position", "util::Position", "positionCallback", "车辆位姿；触发路径点车体系转换"],
        ["/lawn_mower/global_path", "util::LocalPath", "localPathCallback", "五维局部路径（x,y,heading,gear,speed）"],
        ["/mower/stop_car", "std_msgs::Bool", "stopSignalCallback", "停车/继续信号"],
        ["/lawn_mower/avoid_state", "std_msgs::UInt8", "avoidstateCallback", "避障状态 2/3/4"],
        ["/lawn_mower/out_of_bounds", "std_msgs::Bool", "OutBoundaryCallBack", "出边界标志"],
        ["/signal", "std_msgs::String", "signalCallback", "割盘控制（open/close/3~11）与节点复位（stop/mowing_finished/reset）"],
    ],
    [28, 20, 24, 56])

ws = wb.create_sheet("发布话题与服务")
fill_sheet(ws,
    ["名称", "类型", "方向", "消息/服务类型", "说明"],
    [
        ["/vehicle/cmd", "话题", "发布", "mower_msgs::VehicleCmd", "底盘速度指令（drive_value/turn_value ×10000 整型化，turn 取反）+ 割盘控制"],
        ["/turn_completed", "服务", "服务端", "pure_pursuit::TurnCompleted", "查询 (x,y) 转弯点是否已完成，res.turn_finish 返回"],
    ],
    [22, 8, 10, 30, 60])

ws = wb.create_sheet("ROS参数")
fill_sheet(ws,
    ["参数名", "类型", "默认值", "说明"],
    [
        ["pure_pursuit/v_min", "float", "0.2", "最小线速度 m/s"],
        ["pure_pursuit/w_max", "float", "0.15", "跟线最大角速度 rad/s"],
        ["pure_pursuit/min_lookahead_distance", "float", "1.5", "最小预瞄距离 m"],
        ["pure_pursuit/regulated_min_radius", "float", "2.0", "限速最小转弯半径 m"],
        ["pure_pursuit/low_speed", "float", "0.3", "低速模式期望速度 m/s"],
    ],
    [36, 8, 10, 50])

ws = wb.create_sheet("函数")
fill_sheet(ws,
    ["函数名", "返回类型", "参数", "功能说明", "代码行"],
    [
        ["PurePursuit（构造函数）", "-", "const ros::NodeHandle&", "初始化状态/参数/订阅发布/服务，最后进入 state_machine_run", "4-38"],
        ["positionCallback", "void", "util::PositionConstPtr", "更新位姿；五维路径全局系→车体系转换", "40-75"],
        ["localPathCallback", "void", "util::LocalPathConstPtr", "更新路径缓存与时间戳", "77-90"],
        ["stopSignalCallback", "void", "std_msgs::BoolConstPtr", "停车信号置位/清除 stop_car_", "92-104"],
        ["avoidstateCallback", "void", "std_msgs::UInt8ConstPtr", "更新 avoid_state_", "105-109"],
        ["OutBoundaryCallBack", "void", "std_msgs::BoolConstPtr", "更新 outboundary_flag_", "111-115"],
        ["signalCallback", "void", "std_msgs::StringConstPtr", "割盘高度/开关设置；stop 类指令全量复位", "116-156"],
        ["turnCompletedCallback", "bool", "TurnCompleted::Request/Response", "转弯完成查询服务（容差1e-4）", "158-179"],
        ["findNearestPoint", "int", "const vector<waypoint>&", "找车体系下最近路径点索引，空返回-1", "181-197"],
        ["calculateLookaheadPoint", "waypoint", "waypoints + 最近点索引 + 期望速度", "动态预瞄距离选点；区间内转弯点优先", "199-220"],
        ["calculate_PurePursuit", "Twist", "期望速度 + 预瞄点", "κ=2y/Ld² 解算线/角速度，含限速、限幅、草地补偿", "222-266"],
        ["publishCommand", "void", "const Twist&", "单位换算×10000、左手系取反、档位选择、停车强制、割盘透传，发布 /vehicle/cmd", "268-297"],
        ["calculate_rotate_vel", "float", "角度差 yaw", "原地转向角速度分级 0.5/0.4/0.3，180°防跳变", "300-315"],
        ["state_machine_run", "void", "无", "20Hz 主循环：安全检查→最近点→预瞄点→Stop/Follow/Turn/Reverse 状态机", "317-553"],
    ],
    [26, 10, 30, 56, 10])

ws = wb.create_sheet("状态机转移")
fill_sheet(ws,
    ["当前状态", "转移条件", "下一状态", "动作"],
    [
        ["任意", "位置丢失>1s / 路径丢失>3s / 未找到最近点 / 到终点", "Stop", "告警并跳 STATEMACHINE"],
        ["任意", "avoid_state_==4", "Reverse", "紧急倒车"],
        ["任意", "avoid_state_==3", "Stop", "紧急停车"],
        ["Stop", "停车一拍后无条件", "Follow", "发零速后恢复跟踪"],
        ["Follow", "到达预瞄点且 gear==2", "Turn", "停车后原地转向"],
        ["Follow", "follow_count_>100（卡死约5s）", "Follow（脱困）", "1.0m/s 强制前进 20 周期"],
        ["Turn", "|local_yaw|<5°", "Follow", "记录转弯完成点，停车回跟线"],
        ["Turn", "turn_count_>200（约10s）", "Turn（脱困）", "-0.5m/s 后退 20 周期"],
        ["Reverse", "避障状态解除（下周期 avoid_state_≠4）", "经 Stop → Follow", "-0.15m/s 倒车"],
    ],
    [12, 44, 20, 40])

wb.save("pure_pursuit变量话题函数详情.xlsx")
progress("pure_pursuit Excel 已保存")

# ============================================================
# 5. 校验输出
# ============================================================
progress("开始校验生成的 4 个文件")
import os
from docx import Document as D2
from openpyxl import load_workbook
ok = True
for f, kind in [("task_node代码逻辑说明.docx", "docx"), ("task_node变量话题函数详情.xlsx", "xlsx"),
                ("pure_pursuit代码逻辑说明.docx", "docx"), ("pure_pursuit变量话题函数详情.xlsx", "xlsx")]:
    if not os.path.exists(f):
        print(f"  [失败] {f} 未生成"); ok = False; continue
    size = os.path.getsize(f)
    if kind == "docx":
        d = D2(f); n = len(d.paragraphs) + len(d.tables)
        print(f"  [OK] {f}: {size} 字节, 段落+表格数 {n}")
    else:
        w = load_workbook(f)
        print(f"  [OK] {f}: {size} 字节, 工作表 {w.sheetnames}")
progress("全部文档生成并校验完成" if ok else "存在失败文件，请检查")
sys.exit(0 if ok else 1)
