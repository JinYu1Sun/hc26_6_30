# -*- coding: utf-8 -*-
"""生成《云平台远程连接接口文档》Word + Excel
用法: python gen_cloud_interface_doc.py
产物: 云平台远程连接接口文档.docx / 云平台远程连接接口文档.xlsx（当前目录）
"""
from docx import Document
from docx.shared import Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml.ns import qn
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter

CN_FONT = "宋体"
EN_FONT = "Times New Roman"
SIZE = Pt(12)  # 小四
RED = RGBColor(0xC0, 0x00, 0x00)
BLUE = RGBColor(0x00, 0x70, 0xC0)
GREEN = RGBColor(0x00, 0xB0, 0x50)

HEADER_FILL = PatternFill("solid", fgColor="4472C4")
HEADER_FONT = Font(name="微软雅黑", size=11, bold=True, color="FFFFFF")
CELL_FONT = Font(name="微软雅黑", size=10)
THIN = Border(*[Side(style="thin", color="B0B0B0")] * 4)
WRAP = Alignment(vertical="top", wrap_text=True)

DEVICE_ID = "mower_001"
PREFIX = "mower/{" + "设备ID" + "}"


def set_run_font(run, bold=False, size=SIZE, color=None):
    run.font.name = EN_FONT
    run.font.size = size
    run.font.bold = bold
    if color is not None:
        run.font.color.rgb = color
    run._element.rPr.rFonts.set(qn('w:eastAsia'), CN_FONT)


def add_para(doc, text, bold=False, size=SIZE, color=None):
    p = doc.add_paragraph()
    r = p.add_run(text)
    set_run_font(r, bold=bold, size=size, color=color)
    return p


def add_heading(doc, text, level):
    h = doc.add_heading("", level=level)
    r = h.add_run(text)
    set_run_font(r, bold=True, size=Pt(16 - level))
    r.font.color.rgb = RGBColor(0, 0, 0)
    return h


def add_code(doc, text, color=None):
    p = doc.add_paragraph()
    r = p.add_run(text)
    r.font.name = "Consolas"
    r.font.size = Pt(10)
    if color is not None:
        r.font.color.rgb = color
    r._element.rPr.rFonts.set(qn('w:eastAsia'), CN_FONT)
    return p


def add_table(doc, header, rows, color=None):
    t = doc.add_table(rows=1, cols=len(header))
    t.style = "Table Grid"
    t.alignment = WD_TABLE_ALIGNMENT.CENTER
    for i, h in enumerate(header):
        cell = t.rows[0].cells[i]
        r = cell.paragraphs[0].add_run(h)
        set_run_font(r, bold=True, size=Pt(10), color=color)
    for row in rows:
        cells = t.add_row().cells
        for i, v in enumerate(row):
            r = cells[i].paragraphs[0].add_run(str(v))
            set_run_font(r, size=Pt(10), color=color)
    return t


def style_sheet(ws, widths):
    for i, w in enumerate(widths, 1):
        ws.column_dimensions[get_column_letter(i)].width = w
    for cell in ws[1]:
        cell.fill = HEADER_FILL
        cell.font = HEADER_FONT
        cell.alignment = Alignment(vertical="center", horizontal="center", wrap_text=True)
        cell.border = THIN
    for row in ws.iter_rows(min_row=2):
        for cell in row:
            cell.font = CELL_FONT
            cell.alignment = WRAP
            cell.border = THIN


# ==================== 接口数据（Word 与 Excel 共用） ====================

OVERVIEW = [
    ["指令/状态通道", "MQTT over TCP", "1883", "双向", "遥控/刀盘/任务下发；定位/状态上报",
     "明文，无TLS；broker如需用户名密码在车端launch配置"],
]

DOWN_CMDS = [
    {
        "name": "遥控移动", "topic": PREFIX + "/cmd/move",
        "example": '{"linear": 0.5, "angular": -0.3}',
        "fields": [
            ["linear", "number", "-1.0 ~ 1.0", "线速度比例：正=前进，负=后退，0=停"],
            ["angular", "number", "-1.0 ~ 1.0", "角速度比例：正=左转，负=右转，0=直行"],
        ],
        "notes": "需持续发送(建议≥5Hz)，车端看门狗0.5s超时自动停车；"
                 "自动任务运行中收到本指令会先自动pause任务再接管底盘；"
                 "车端映射 drive_value=linear×10000, turn_value=-angular×12566",
    },
    {
        "name": "刀盘控制", "topic": PREFIX + "/cmd/blade",
        "example": '{"state": 1, "height": 6}',
        "fields": [
            ["state", "int", "0 / 1", "1=开启刀盘，0=关闭刀盘"],
            ["height", "int(可选)", "2 ~ 11", "割草高度档位，不携带则保持当前档位"],
        ],
        "notes": "手动/自动模式下均可使用，车端自动选择控制通道",
    },
    {
        "name": "自动割草任务", "topic": PREFIX + "/cmd/task",
        "example": '{"action": "start", "map_name": "map_0630", "map_mode": "single_map"}',
        "fields": [
            ["action", "string", "start/stop/pause/continue", "任务操作"],
            ["map_name", "string(start时)", "-", "要使用的地图名；空=沿用当前已加载地图"],
            ["map_mode", "string(可选)", "single_map/multi_map", "单地图/多地图模式，默认single_map"],
        ],
        "notes": "start：车端依次执行 启动路径跟踪→复位→加载地图→选模式→开工(约2~3秒)；"
                 "stop：停止任务并复位，随后关闭路径跟踪节点",
    },
    {
        "name": "定位初始化", "topic": PREFIX + "/cmd/init_location", "new": True,
        "example": '{"action": "request"}',
        "fields": [
            ["action", "string", "request/confirm/cancel",
             "request=请求定位初始化；confirm=确认（车辆开始走8字形动作）；cancel=取消"],
        ],
        "notes": "云平台下发request并确认场地安全后再下发confirm，车辆才开始走8字形；"
                 "初始化过程中可随时cancel",
    },
    {
        "name": "建图控制", "topic": PREFIX + "/cmd/mapping", "new": True,
        "example": '{"action": "enter"}',
        "fields": [
            ["action", "string",
             "enter/start_boundary/stop_boundary/start_obstacle/stop_obstacle/"
             "start_parking/stop_parking/start_path/stop_path/save/delete/list/reset",
             "建图操作，详见下方说明"],
            ["map_name", "string(save/delete时)", "不能含 / 和 ..", "要保存或删除的地图名"],
        ],
        "notes": "建图过程中车辆完全由云平台遥控（cmd/move），建图页面需集成遥控功能。"
                 "建图流程：① 下发 enter 进入建图模式（车端发 m_mode 通知雷达/RTK准备，"
                 "随后定位坐标会被外部系统归零为(0,0,0)）；"
                 "② 观察 state/mapping，等定位状态有效(state为1/2/5)且 x、y 归零到(0,0)附近后，"
                 "下发 start_boundary；③ 遥控车沿边界行驶，云平台用 state/mapping 轨迹流实时描边；"
                 "④ 到终点下发 stop_boundary，如需障碍物/停车位/连接路径按需重复 start/stop；"
                 "⑤ 下发 save 保存地图，保存后本次建图会话结束。"
                 "注意：录制过程中请勿下发 blade/task 等其他指令，否则车端会中断当前录制",
    },
]

UP_DATA = [
    {
        "name": "定位上报", "topic": PREFIX + "/state/location", "freq": "2Hz(可调)，开机即上报",
        "example": '{"x":12.34,"y":56.78,"z":0.0,"roll":0.01,"pitch":-0.02,"yaw":1.57,'
                   '"state":4,"stamp":1751356800.123}',
        "fields": [
            ["x / y / z", "number", "局部平面坐标(米)，来自车端融合定位"],
            ["roll / pitch / yaw", "number", "姿态(弧度)"],
            ["state", "int", "定位状态字：0=无定位；1/2/5=有效融合定位"],
            ["stamp", "number", "Unix秒(double，车端ROS时间)"],
        ],
        "notes": "当前为局部坐标系；如需经纬度需约定地图原点换算，或后续增加GNSS转发",
    },
    {
        "name": "车辆状态", "topic": PREFIX + "/state/vehicle", "freq": "1Hz(可调)",
        "example": '{"battery_soc":85,"warning_state_one":0,"warning_state_two":0,'
                   '"mower_height":6,"stamp":1751356800.5}',
        "fields": [
            ["battery_soc", "int", "电量百分比"],
            ["warning_state_one / two", "int", "下位机报警状态字"],
            ["mower_height", "int", "刀盘高度反馈档位"],
            ["stamp", "number", "Unix秒(double)"],
        ],
        "notes": "",
    },
    {
        "name": "建图轨迹", "topic": PREFIX + "/state/mapping", "new": True,
        "freq": "与 state/location 相同(默认10Hz)，仅建图会话期间上报",
        "example": '{"x":0.0,"y":0.0,"z":0.0,"roll":0.01,"pitch":-0.02,"yaw":1.57,'
                   '"state":4,"stamp":1751356800.6}',
        "fields": [
            ["x / y / z", "number", "局部平面坐标(米)，即建图期间车辆走过的轨迹"],
            ["roll / pitch / yaw", "number", "姿态(弧度)"],
            ["state", "int", "定位状态字：0=无定位；1/2/5=有效融合定位"],
            ["stamp", "number", "Unix秒(double)"],
        ],
        "notes": "从收到 cmd/mapping enter 起，到 save 或 reset 止；字段含义与 state/location "
                 "完全一致。云平台建图页面用它实时描出行驶轨迹（即正在录制的边界走向），"
                 "并据此观察 enter 之后坐标是否已归零",
    },
    {
        "name": "地图列表", "topic": PREFIX + "/state/map_list", "new": True,
        "freq": "收到 cmd/mapping list 后回传一次",
        "example": '{"maps":["map_0630","map_0701"],"stamp":1751356800.8}',
        "fields": [
            ["maps", "array<string>", "车端已保存的地图名列表"],
            ["stamp", "number", "Unix秒(double)"],
        ],
        "notes": "",
    },
    {
        "name": "地图原点", "topic": PREFIX + "/state/map_origin", "color": GREEN,
        "freq": "收到 /signal 选图信号 use_map/编号 时回传一次",
        "example": '{"map_name":"map_0630","found":true,"map_index":0,'
                   '"latitude":28.236557,"longitude":112.876617,"height":86.60,'
                   '"gauss_yaw":73.89,"stamp":1751356800.9}',
        "fields": [
            ["map_name", "string", "请求的地图编号"],
            ["found", "bool", "是否找到并解析成功；文件缺失时为false（此时无以下字段）"],
            ["map_index", "int", "地图内部索引"],
            ["latitude / longitude / height", "number", "地图原点 WGS84 纬度/经度/椭球高(米)"],
            ["gauss_yaw", "number", "建图时的高斯坐标系偏航角(度)"],
            ["stamp", "number", "Unix秒(double)"],
        ],
        "notes": "车端订阅 /signal，匹配 use_map/编号 时从 map_dir(launch可配) 读取 "
                 "<编号>.mp 原点文件并上发；cmd/task start 带 map_name、安卓端选图均会触发。"
                 "云平台可据此把 state/location 的局部坐标换算成经纬度",
    },
]

ROS_MAP = [
    ["cmd/move", "发布 /mower/manual_driving_cmd (mower_msgs/Manual_Driving_Cmd)",
     "task_node 透传 /vehicle/cmd → UDP下发单片机"],
    ["cmd/blade 手动模式", "随 /mower/manual_driving_cmd 的 mover_bool/mow_height 下发", "同上"],
    ["cmd/blade 自动模式", "发布 /signal = open/close 及高度档位 \"2\"~\"11\"", "pure_pursuit 接管"],
    ["cmd/task start", "依次发布 /signal = start_execution→reset→use_map/地图名→single_map→start_work",
     "task_node 拉起 pure_pursuit；location_map 触发任务"],
    ["cmd/task stop", "/signal = stop，延时2s后 stop_execution", "各规划节点复位，pure_pursuit 退出"],
    ["cmd/task pause/continue", "/signal = pause / continue", "pure_pursuit 暂停/恢复跟踪"],
    ["定位上报", "订阅 /Mower/position (mower_msgs/Position)，2Hz节流后上报", "外部定位模块发布"],
    ["车辆状态", "订阅 /vehicle/status、/vehicle/mower_height_to_app",
     "udp_com_main 节点发布"],
    ["cmd/init_location", "/signal = init_location / true / false",
     "task_node 定位初始化流程（8字动作），外部定位系统重启定位"],
    ["cmd/mapping enter", "/signal = m_mode", "雷达/RTK进入建图准备，定位坐标由外部系统归零"],
    ["cmd/mapping start/stop_boundary等", "/signal = start_brd/cease_brd/start_obs/cease_obs 等",
     "location_map 录制边界/障碍物/停车位/连接路径"],
    ["cmd/mapping save/delete", "/signal = save_map|delete_name/地图名", "location_map 保存/删除地图文件"],
    ["cmd/mapping list", "/signal = p_mode", "location_map 发布 /map_name"],
    ["建图轨迹上报", "建图会话期间（enter→save/reset）转发 /Mower/position 到 state/mapping",
     "外部定位模块发布"],
    ["地图列表", "订阅 /map_name，解析后转发 state/map_list", "location_map 发布"],
    ["地图原点", "订阅 /signal，匹配 use_map/编号 时读取 map_dir/<编号>.mp，回传 state/map_origin",
     "cloud_bridge 直接读 fusion 的 .mp 原点文件"],
]

SAFETY = [
    "车端0.5s收不到move指令自动停车，云端断连不会导致车辆持续行驶",
    "车端原有避障急停/侧翻保护/出边界保护与云端指令并行生效，云端无需处理",
    "MQTT断线后车端自动重连(指数退避，最长30s)，重连后自动恢复订阅",
]

MOSQ_EXAMPLES = [
    ("订阅全部MQTT上行", "mosquitto_sub -h <broker> -t 'mower/mower_001/#' -v", False),
    ("前进半速", "mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/move' -m '{\"linear\":0.5,\"angular\":0}'", False),
    ("停车", "mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/move' -m '{\"linear\":0,\"angular\":0}'", False),
    ("开刀盘", "mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/blade' -m '{\"state\":1,\"height\":6}'", False),
    ("启动自动任务", "mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/task' -m '{\"action\":\"start\",\"map_name\":\"map_0630\",\"map_mode\":\"single_map\"}'", False),
    ("停止任务", "mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/task' -m '{\"action\":\"stop\"}'", False),
    ("进入建图模式", "mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/mapping' -m '{\"action\":\"enter\"}'", True),
    ("开始录制边界", "mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/mapping' -m '{\"action\":\"start_boundary\"}'", True),
    ("停止录制边界", "mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/mapping' -m '{\"action\":\"stop_boundary\"}'", True),
    ("保存地图", "mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/mapping' -m '{\"action\":\"save\",\"map_name\":\"map_0701\"}'", True),
    ("请求地图列表", "mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/mapping' -m '{\"action\":\"list\"}'", True),
]

# 2026-09-11 起删除的接口（不再支持，仅存档备查）
DELETED_NOTE = "以下接口已于 2026-09-11 从车端 cloud_bridge 中删除，云平台请勿再使用："

DELETED_VIDEO = {
    "name": "视频控制（已删除）", "topic": PREFIX + "/cmd/video",
    "example": '{"enable": true, "fps": 5, "width": 640, "height": 480, "bitrate": 800}',
    "fields": [
        ["enable", "bool", "true/false", "开启/关闭 SRT 推流"],
        ["fps", "number", "0.1 ~ 30", "推流帧率"],
        ["width / height", "int", "16~1920 / 16~1080", "输出分辨率"],
        ["bitrate", "int", "100 ~ 8000", "H.264 码率 kbps"],
    ],
    "notes": "整条 SRT 视频通道（H.264 + MPEG-TS，车端 caller 推流）已随本接口一并移除，"
             "车端不再订阅 /camera/image_rect，launch 中 srt_target/video_* 参数均已删除",
}

DELETED_VEHICLE_FIELDS = [
    ["left_wheel_speed", "int", "左轮速度（原来自 /vehicle/left_wheel_speed）"],
    ["right_wheel_speed", "int", "右轮速度（原来自 /vehicle/right_wheel_speed）"],
]


def entry_color(entry):
    # "color" 优先（用于区分不同批次的更新），否则 "new" 为蓝色
    color = entry.get("color")
    if color is None and entry.get("new"):
        color = BLUE
    return color


# ==================== Word ====================

def build_docx(path):
    doc = Document()
    title = doc.add_heading("", level=0)
    r = title.add_run("割草机云平台远程连接接口文档")
    set_run_font(r, bold=True, size=Pt(22))
    title.alignment = WD_ALIGN_PARAGRAPH.CENTER
    add_para(doc, "版本 V1.1    适用：车端 cloud_bridge 节点（ROS1）与云平台对接", size=Pt(10.5))
    add_para(doc, "2026-09-20 更新：新增建图相关接口（cmd/mapping、state/mapping、state/map_list）"
                  "及定位初始化接口（cmd/init_location），蓝色字体为新增协议，红色字体为已删除协议。",
             size=Pt(10.5), color=BLUE)
    add_para(doc, "2026-09-22 更新：新增地图原点上发接口（state/map_origin），绿色字体为本次新增协议。",
             size=Pt(10.5), color=GREEN)

    add_heading(doc, "1. 概述", 1)
    add_para(doc, "车端与云平台之间通过一条 MQTT 通道通信，明文传输、无加密无认证。设备ID默认为 "
                  + DEVICE_ID + "，MQTT主题前缀 mower/{设备ID}/，均在车端launch文件中配置。")
    add_table(doc, ["通道", "协议", "端口", "方向", "用途", "说明"], OVERVIEW)

    add_heading(doc, "2. 下行指令（云平台 → 割草机，MQTT）", 1)
    add_para(doc, "payload 均为 UTF-8 JSON，QoS 1。")
    for i, cmd in enumerate(DOWN_CMDS, 1):
        color = entry_color(cmd)
        add_heading(doc, f"2.{i} {cmd['name']}", 2)
        add_para(doc, "MQTT主题：" + cmd["topic"], bold=True, color=color)
        add_para(doc, "示例：", color=color)
        add_code(doc, cmd["example"], color=color)
        add_table(doc, ["字段", "类型", "取值范围", "说明"], cmd["fields"], color=color)
        if cmd["notes"]:
            add_para(doc, "说明：" + cmd["notes"], color=color)

    add_heading(doc, "3. 上行数据（割草机 → 云平台，MQTT）", 1)
    add_para(doc, "payload 均为 UTF-8 JSON，QoS 0。")
    for i, up in enumerate(UP_DATA, 1):
        color = entry_color(up)
        add_heading(doc, f"3.{i} {up['name']}", 2)
        add_para(doc, "MQTT主题：" + up["topic"] + "    频率：" + up["freq"], bold=True, color=color)
        add_para(doc, "示例：", color=color)
        add_code(doc, up["example"], color=color)
        add_table(doc, ["字段", "类型", "说明"], up["fields"], color=color)
        if up["notes"]:
            add_para(doc, "说明：" + up["notes"], color=color)

    add_heading(doc, "4. 车端 ROS 接口映射（参考）", 1)
    add_para(doc, "云平台无需关心，仅供双方联调排障时对照：")
    add_table(doc, ["云端接口", "车端ROS动作", "下游环节"], ROS_MAP)

    add_heading(doc, "5. 安全与异常行为", 1)
    for s in SAFETY:
        add_para(doc, "• " + s)

    add_heading(doc, "6. 联调示例（mosquitto 客户端模拟云平台）", 1)
    for name, cmd, is_new in MOSQ_EXAMPLES:
        color = BLUE if is_new else None
        add_para(doc, name + "：", color=color)
        add_code(doc, cmd, color=color)

    add_heading(doc, "7. 已删除接口（2026-09-11 起不再支持）", 1)
    add_para(doc, DELETED_NOTE, bold=True, color=RED)
    add_heading(doc, "7.1 " + DELETED_VIDEO["name"], 2)
    add_para(doc, "MQTT主题：" + DELETED_VIDEO["topic"], bold=True, color=RED)
    add_para(doc, "示例：", color=RED)
    add_code(doc, DELETED_VIDEO["example"], color=RED)
    add_table(doc, ["字段", "类型", "取值范围", "说明"], DELETED_VIDEO["fields"], color=RED)
    add_para(doc, "说明：" + DELETED_VIDEO["notes"], color=RED)
    add_heading(doc, "7.2 车辆状态上报已删除字段（state/vehicle）", 2)
    add_para(doc, "以下字段已从 " + PREFIX + "/state/vehicle 的 payload 中移除：", color=RED)
    add_table(doc, ["字段", "类型", "说明"], DELETED_VEHICLE_FIELDS, color=RED)

    doc.save(path)
    print("Word 文档已生成:", path)


# ==================== Excel ====================

def build_xlsx(path):
    wb = Workbook()
    BLUE_FONT = Font(name="微软雅黑", size=10, color="0070C0")
    GREEN_FONT = Font(name="微软雅黑", size=10, color="00B050")

    def font_for(entry):
        return GREEN_FONT if entry.get("color") is GREEN else BLUE_FONT

    ws = wb.active
    ws.title = "1-协议总览"
    ws.append(["通道", "协议", "端口", "方向", "用途", "说明"])
    for row in OVERVIEW:
        ws.append(row)
    ws.append([])
    ws.append(["约定", "", "", "", "", ""])
    for row in [
        ["设备ID", DEVICE_ID + "（车端launch配置）", "", "", "", ""],
        ["MQTT主题前缀", "mower/{设备ID}/", "", "", "", ""],
        ["payload格式", "UTF-8 JSON", "", "", "", ""],
        ["时间戳stamp", "Unix秒(double，车端ROS时间)", "", "", "", ""],
        ["QoS", "下行指令QoS 1；上行QoS 0", "", "", "", ""],
    ]:
        ws.append(row)
    style_sheet(ws, [16, 26, 18, 16, 40, 46])

    ws = wb.create_sheet("2-MQTT下行指令")
    ws.append(["接口", "MQTT主题", "方向", "payload示例", "字段", "类型", "取值范围", "说明", "备注"])
    new_rows = []
    for cmd in DOWN_CMDS:
        first = True
        for f in cmd["fields"]:
            ws.append([
                cmd["name"] if first else "",
                cmd["topic"] if first else "",
                "云平台→割草机" if first else "",
                cmd["example"] if first else "",
                f[0], f[1], f[2], f[3],
                cmd["notes"] if first else "",
            ])
            if cmd.get("new") or cmd.get("color") is not None:
                new_rows.append((ws.max_row, font_for(cmd)))
            first = False
    style_sheet(ws, [12, 30, 14, 52, 14, 14, 20, 40, 44])
    for r, font in new_rows:
        for cell in ws[r]:
            cell.font = font

    ws = wb.create_sheet("3-MQTT上行数据")
    ws.append(["接口", "MQTT主题", "方向", "频率", "payload示例", "字段", "类型", "说明", "备注"])
    new_rows = []
    for up in UP_DATA:
        first = True
        for f in up["fields"]:
            ws.append([
                up["name"] if first else "",
                up["topic"] if first else "",
                "割草机→云平台" if first else "",
                up["freq"] if first else "",
                up["example"] if first else "",
                f[0], f[1], f[2],
                up["notes"] if first else "",
            ])
            if up.get("new") or up.get("color") is not None:
                new_rows.append((ws.max_row, font_for(up)))
            first = False
    style_sheet(ws, [12, 30, 14, 20, 56, 32, 10, 44, 40])
    for r, font in new_rows:
        for cell in ws[r]:
            cell.font = font

    ws = wb.create_sheet("4-车端ROS映射")
    ws.append(["云端接口", "车端ROS动作", "下游环节"])
    for row in ROS_MAP:
        ws.append(row)
    style_sheet(ws, [24, 62, 44])

    ws = wb.create_sheet("5-安全与联调")
    ws.append(["类别", "内容"])
    for s in SAFETY:
        ws.append(["安全约定", s])
    for name, cmd, is_new in MOSQ_EXAMPLES:
        ws.append(["联调示例-" + name, cmd])
    style_sheet(ws, [20, 110])

    RED_FONT = Font(name="微软雅黑", size=10, color="C00000")
    ws = wb.create_sheet("6-已删除接口")
    ws.append(["接口", "MQTT主题", "payload示例", "字段", "类型", "取值范围/说明"])
    ws.append([DELETED_NOTE, "", "", "", "", ""])
    first = True
    for f in DELETED_VIDEO["fields"]:
        ws.append([
            DELETED_VIDEO["name"] if first else "",
            DELETED_VIDEO["topic"] if first else "",
            DELETED_VIDEO["example"] if first else "",
            f[0], f[1], f[2] + "；" + f[3],
        ])
        first = False
    ws.append([DELETED_VIDEO["notes"], "", "", "", "", ""])
    first = True
    for f in DELETED_VEHICLE_FIELDS:
        ws.append([
            "车辆状态已删字段" if first else "",
            PREFIX + "/state/vehicle" if first else "",
            "", f[0], f[1], f[2],
        ])
        first = False
    style_sheet(ws, [22, 30, 52, 16, 12, 44])
    for row in ws.iter_rows(min_row=2):
        for cell in row:
            cell.font = RED_FONT

    wb.save(path)
    print("Excel 文档已生成:", path)


if __name__ == "__main__":
    build_docx("云平台远程连接接口文档.docx")
    build_xlsx("云平台远程连接接口文档.xlsx")
