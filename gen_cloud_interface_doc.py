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

HEADER_FILL = PatternFill("solid", fgColor="4472C4")
HEADER_FONT = Font(name="微软雅黑", size=11, bold=True, color="FFFFFF")
CELL_FONT = Font(name="微软雅黑", size=10)
THIN = Border(*[Side(style="thin", color="B0B0B0")] * 4)
WRAP = Alignment(vertical="top", wrap_text=True)

DEVICE_ID = "mower_001"
PREFIX = "mower/{" + "设备ID" + "}"


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
    p = doc.add_paragraph()
    r = p.add_run(text)
    r.font.name = "Consolas"
    r.font.size = Pt(10)
    r._element.rPr.rFonts.set(qn('w:eastAsia'), CN_FONT)
    return p


def add_table(doc, header, rows):
    t = doc.add_table(rows=1, cols=len(header))
    t.style = "Table Grid"
    t.alignment = WD_TABLE_ALIGNMENT.CENTER
    for i, h in enumerate(header):
        cell = t.rows[0].cells[i]
        r = cell.paragraphs[0].add_run(h)
        set_run_font(r, bold=True, size=Pt(10))
    for row in rows:
        cells = t.add_row().cells
        for i, v in enumerate(row):
            r = cells[i].paragraphs[0].add_run(str(v))
            set_run_font(r, size=Pt(10))
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
                   '"left_wheel_speed":120,"right_wheel_speed":118,"mower_height":6,'
                   '"stamp":1751356800.5}',
        "fields": [
            ["battery_soc", "int", "电量百分比"],
            ["warning_state_one / two", "int", "下位机报警状态字"],
            ["left_wheel_speed / right_wheel_speed", "int", "左右轮转速"],
            ["mower_height", "int", "刀盘高度反馈档位"],
            ["stamp", "number", "Unix秒(double)"],
        ],
        "notes": "",
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
    ["定位上报", "订阅 /Mower/position (util/Position)，2Hz节流后上报", "外部定位模块发布"],
    ["车辆状态", "订阅 /vehicle/status、/vehicle/left|right_wheel_speed、/vehicle/mower_height_to_app",
     "udp_com_main 节点发布"],
]

SAFETY = [
    "车端0.5s收不到move指令自动停车，云端断连不会导致车辆持续行驶",
    "车端原有避障急停/侧翻保护/出边界保护与云端指令并行生效，云端无需处理",
    "MQTT断线后车端自动重连(指数退避，最长30s)，重连后自动恢复订阅",
]

MOSQ_EXAMPLES = [
    ("订阅全部MQTT上行", "mosquitto_sub -h <broker> -t 'mower/mower_001/#' -v"),
    ("前进半速", "mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/move' -m '{\"linear\":0.5,\"angular\":0}'"),
    ("停车", "mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/move' -m '{\"linear\":0,\"angular\":0}'"),
    ("开刀盘", "mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/blade' -m '{\"state\":1,\"height\":6}'"),
    ("启动自动任务", "mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/task' -m '{\"action\":\"start\",\"map_name\":\"map_0630\",\"map_mode\":\"single_map\"}'"),
    ("停止任务", "mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/task' -m '{\"action\":\"stop\"}'"),
]


# ==================== Word ====================

def build_docx(path):
    doc = Document()
    title = doc.add_heading("", level=0)
    r = title.add_run("割草机云平台远程连接接口文档")
    set_run_font(r, bold=True, size=Pt(22))
    title.alignment = WD_ALIGN_PARAGRAPH.CENTER
    add_para(doc, "版本 V1.0    适用：车端 cloud_bridge 节点（ROS1）与云平台对接", size=Pt(10.5))

    add_heading(doc, "1. 概述", 1)
    add_para(doc, "车端与云平台之间通过一条 MQTT 通道通信，明文传输、无加密无认证。设备ID默认为 "
                  + DEVICE_ID + "，MQTT主题前缀 mower/{设备ID}/，均在车端launch文件中配置。")
    add_table(doc, ["通道", "协议", "端口", "方向", "用途", "说明"], OVERVIEW)

    add_heading(doc, "2. 下行指令（云平台 → 割草机，MQTT）", 1)
    add_para(doc, "payload 均为 UTF-8 JSON，QoS 1。")
    for i, cmd in enumerate(DOWN_CMDS, 1):
        add_heading(doc, f"2.{i} {cmd['name']}", 2)
        add_para(doc, "MQTT主题：" + cmd["topic"], bold=True)
        add_para(doc, "示例：")
        add_code(doc, cmd["example"])
        add_table(doc, ["字段", "类型", "取值范围", "说明"], cmd["fields"])
        if cmd["notes"]:
            add_para(doc, "说明：" + cmd["notes"])

    add_heading(doc, "3. 上行数据（割草机 → 云平台，MQTT）", 1)
    add_para(doc, "payload 均为 UTF-8 JSON，QoS 0。")
    for i, up in enumerate(UP_DATA, 1):
        add_heading(doc, f"3.{i} {up['name']}", 2)
        add_para(doc, "MQTT主题：" + up["topic"] + "    频率：" + up["freq"], bold=True)
        add_para(doc, "示例：")
        add_code(doc, up["example"])
        add_table(doc, ["字段", "类型", "说明"], up["fields"])
        if up["notes"]:
            add_para(doc, "说明：" + up["notes"])

    add_heading(doc, "4. 车端 ROS 接口映射（参考）", 1)
    add_para(doc, "云平台无需关心，仅供双方联调排障时对照：")
    add_table(doc, ["云端接口", "车端ROS动作", "下游环节"], ROS_MAP)

    add_heading(doc, "5. 安全与异常行为", 1)
    for s in SAFETY:
        add_para(doc, "• " + s)

    add_heading(doc, "6. 联调示例（mosquitto 客户端模拟云平台）", 1)
    for name, cmd in MOSQ_EXAMPLES:
        add_para(doc, name + "：")
        add_code(doc, cmd)

    doc.save(path)
    print("Word 文档已生成:", path)


# ==================== Excel ====================

def build_xlsx(path):
    wb = Workbook()

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
            first = False
    style_sheet(ws, [12, 30, 14, 52, 14, 14, 20, 40, 44])

    ws = wb.create_sheet("3-MQTT上行数据")
    ws.append(["接口", "MQTT主题", "方向", "频率", "payload示例", "字段", "类型", "说明", "备注"])
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
            first = False
    style_sheet(ws, [12, 30, 14, 20, 56, 32, 10, 44, 40])

    ws = wb.create_sheet("4-车端ROS映射")
    ws.append(["云端接口", "车端ROS动作", "下游环节"])
    for row in ROS_MAP:
        ws.append(row)
    style_sheet(ws, [24, 62, 44])

    ws = wb.create_sheet("5-安全与联调")
    ws.append(["类别", "内容"])
    for s in SAFETY:
        ws.append(["安全约定", s])
    for name, cmd in MOSQ_EXAMPLES:
        ws.append(["联调示例-" + name, cmd])
    style_sheet(ws, [20, 110])

    wb.save(path)
    print("Excel 文档已生成:", path)


if __name__ == "__main__":
    build_docx("云平台远程连接接口文档.docx")
    build_xlsx("云平台远程连接接口文档.xlsx")
