# -*- coding: utf-8 -*-
"""生成《cloud_bridge代码详解》Word 文档
用法: python gen_cloud_bridge_code_doc.py
产物: 代码详解/cloud_bridge代码详解.docx
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
    """生成带表头的表格，中文宋体、英文 Times New Roman 10.5pt。"""
    table = doc.add_table(rows=1, cols=len(headers))
    table.style = "Table Grid"
    for i, h in enumerate(headers):
        cell = table.rows[0].cells[i]
        cell.text = ""
        r = cell.paragraphs[0].add_run(h)
        set_run_font(r, bold=True, size=Pt(10.5))
    for row in rows:
        cells = table.add_row().cells
        for i, v in enumerate(row):
            cells[i].text = ""
            r = cells[i].paragraphs[0].add_run(str(v))
            set_run_font(r, size=Pt(10.5))
    return table


# ==================== 第1章 包总体介绍 ====================

def chapter_overview(doc):
    add_heading(doc, "1. 包总体介绍", 1)

    add_heading(doc, "1.1 包的职责", 2)
    add_para(doc, "cloud_bridge 是履带割草机车端的 ROS1（Noetic）功能包，定位是“车端 ↔ 云平台”的协议桥："
                  "它把云平台通过 MQTT 下发的 JSON 指令翻译成车端 ROS 话题消息，"
                  "又把车端 ROS 话题上的定位、车辆状态等数据定时打包成 JSON 通过 MQTT 上报给云平台。"
                  "通信采用 MQTT over 明文 TCP，无加密、无强制认证（broker 如需用户名密码可在 launch 中配置）。")
    add_para(doc, "具体承担四类业务：", bold=True)
    add_para(doc, "1) 远程遥控：接收云平台 cmd/move（前进/后退/转向）、cmd/blade（割草刀盘启停与高度档位）指令，"
                  "转发为 /mower/manual_driving_cmd 消息交给 task_node 执行；")
    add_para(doc, "2) 自动任务控制：接收 cmd/task（start/stop/pause/continue）、cmd/init_location（定位初始化）、"
                  "cmd/mapping（遥控建图全流程）指令，翻译成 /signal 字符串信号，"
                  "由 task_node / pure_pursuit 等原有节点组成的状态机消费；")
    add_para(doc, "3) 状态上报：开机后按固定频率持续上报定位（state/location）与车辆状态（state/vehicle），"
                  "建图会话期间额外上报轨迹（state/mapping），选图后上报地图原点（state/map_origin），"
                  "收到地图列表时转发（state/map_list）；")
    add_para(doc, "4) 安全兜底：遥控指令看门狗（0.5s 无新指令自动停车）、自动任务中收到遥控先自动暂停任务、"
                  "MQTT 断线指数退避自动重连并恢复订阅。")

    add_heading(doc, "1.2 节点与可执行文件清单", 2)
    add_table(doc, ["名称", "类型", "来源", "说明"], [
        ["cloud_bridge_node", "C++ 可执行节点",
         "src/cloud_bridge_node.cpp + src/mqtt_client.cpp",
         "本包唯一节点，launch 中以节点名 cloud_bridge 启动"],
        ["test_cloud_bridge.py", "Python 测试脚本", "scripts/",
         "模拟云平台侧，用 paho-mqtt 收发指令/检查上行，用于公网联调"],
        ["mock_robot_data.py", "Python 仿真脚本", "scripts/",
         "模拟车端上位机，发布 /Mower/position、/vehicle/status 等话题，用于无实车测试"],
    ])
    add_para(doc, "说明：scripts/__pycache__/ 下的 .pyc 是 Python 运行缓存，非源码。"
                  "本包没有历史版本备份文件，CMakeLists.txt 编译的就是上述两个 .cpp。")

    add_heading(doc, "1.3 目录结构", 2)
    add_code(doc, """crawler_control/src/cloud_bridge/
├── CMakeLists.txt                      构建脚本（找 catkin / paho / nlohmann_json）
├── package.xml                         包清单（依赖 roscpp / std_msgs / mower_msgs）
├── README.md                           编译、启动与验证清单
├── docs/
│   └── CLOUD_INTERFACE.md              车云接口协议文档（云平台开发依据）
├── include/cloud_bridge/
│   └── mqtt_client.h                   MqttClient 封装（声明）
├── src/
│   ├── mqtt_client.cpp                 MqttClient 封装（实现）
│   └── cloud_bridge_node.cpp           主节点：指令处理 + 状态上报（约 615 行）
├── launch/
│   └── cloud_bridge.launch             全部运行参数
└── scripts/
    ├── test_cloud_bridge.py            模拟云平台的联调测试脚本
    └── mock_robot_data.py              模拟车端数据的仿真脚本""")

    add_heading(doc, "1.4 整体数据流", 2)
    add_para(doc, "下行（云平台 → 车）：", bold=True)
    add_para(doc, "云平台把 UTF-8 JSON 指令发布到 MQTT broker 的 mower/{device_id}/cmd/xxx 主题；"
                  "MqttClient 收到后回调 CloudBridgeNode::onMqttMessage，按主题分发到 handleMoveCmd / "
                  "handleBladeCmd / handleTaskCmd / handleInitLocationCmd / handleMappingCmd 五个处理函数。"
                  "遥控类指令转成 mower_msgs/Manual_Driving_Cmd 发到 /mower/manual_driving_cmd；"
                  "任务、初始化、建图类指令转成字符串发到 /signal，由 task_node / pure_pursuit 消费。")
    add_para(doc, "上行（车 → 云平台）：", bold=True)
    add_para(doc, "节点订阅 /Mower/position（融合定位）、/vehicle/status（车辆状态）、"
                  "/vehicle/mower_height_to_app（刀盘高度反馈）、/map_name（地图文件列表）、/signal（选图信号）；"
                  "订阅回调只缓存最新值，由 location_timer_ / status_timer_ / map_origin_timer_ 三个定时器"
                  "按配置频率取出拼成 JSON，经 MqttClient 发布到 mower/{device_id}/state/xxx 主题。")
    add_code(doc, """云平台(APP/网页)
    │  MQTT: mower/{id}/cmd/move|blade|task|init_location|mapping   (QoS1, JSON)
    ▼
cloud_bridge_node ──► /mower/manual_driving_cmd ──► task_node ──► 底盘/刀盘
    │   └──► /signal (字符串信号) ──► task_node / pure_pursuit 状态机
    │
    │  MQTT: mower/{id}/state/location|vehicle|mapping|map_list|map_origin (QoS0, JSON)
    ▲
/Mower/position (融合定位)  /vehicle/status (udp_com_main)  /map_name (location_map)""")

    add_heading(doc, "1.5 线程模型（阅读本包的关键）", 2)
    add_para(doc, "本节点是典型的多线程程序，共享数据的保护方式是理解代码的前提：")
    add_table(doc, ["线程", "来源", "执行的代码"], [
        ["ROS spinner 线程 ×2", "ros::AsyncSpinner(2)",
         "五个订阅回调、四个定时器回调（location/status/watchdog/map_origin）"],
        ["paho MQTT 回调线程", "paho 库内部",
         "MqttClient::message_arrived → onMqttMessage → 各 handleXxxCmd"],
        ["MQTT 重连线程", "MqttClient 自建 std::thread", "reconnectLoop 指数退避重连"],
        ["任务序列临时线程", "handleTaskCmd / startTaskSequence 中 detach",
         "stop 延时补发 stop_execution；start 按节奏连发一串 /signal"],
    ])
    add_para(doc, "对应的同步策略：单个简单值用 std::atomic（task_running_、blade_on_、mow_height_、"
                  "mapping_active_、task_seq_running_ 等）；一组必须一致读写的值共用一把 std::mutex"
                  "（运动指令 motion_mutex_、定位 position_mutex_、车辆状态 status_mutex_、"
                  "当前地图名 map_origin_mutex_、订阅列表 sub_mutex_）。")


# ==================== 第2章 启动与构建 ====================

def chapter_launch_build(doc):
    add_heading(doc, "2. 启动与构建", 1)

    add_heading(doc, "2.1 launch 文件与参数说明", 2)
    add_para(doc, "启动命令：roslaunch cloud_bridge cloud_bridge.launch。"
                  "launch/cloud_bridge.launch 以 <node pkg=\"cloud_bridge\" type=\"cloud_bridge_node\" "
                  "name=\"cloud_bridge\" output=\"screen\"> 启动唯一节点，节点标签内的 <param> 全部是私有参数，"
                  "与 loadParams() 中 pnh_.param(...) 读取的名字一一对应。当前 launch 中有两套 broker 配置："
                  "注释掉的本机 127.0.0.1:1884（配 mosquitto 自测），生效的是公网测试 broker "
                  "broker.emqx.io:1883、设备 mower_test_huilian。")
    add_code(doc, """<launch>
  <node pkg="cloud_bridge" type="cloud_bridge_node" name="cloud_bridge" output="screen">

    <!-- ===== MQTT broker（明文 TCP，无加密无认证）===== -->
    <!-- <param name="broker_host" value="127.0.0.1"/>   云平台 MQTT broker 地址 -->
    <!-- <param name="broker_port" value="1884"/> -->
    <param name="broker_host" value="broker.emqx.io"/>
    <param name="broker_port" value="1883"/>
    <param name="client_id" value="cloud_bridge_mower_test"/>
    ...
  </node>
</launch>""")
    add_para(doc, "全部参数一览（默认值取自 loadParams 的缺省值，launch 值取自当前 launch 文件）：", bold=True)
    add_table(doc, ["参数名", "代码默认值", "launch 当前值", "用途"], [
        ["broker_host", "127.0.0.1", "broker.emqx.io", "MQTT broker 地址"],
        ["broker_port", "1883", "1883", "MQTT broker 端口（明文 TCP）"],
        ["client_id", "cloud_bridge", "cloud_bridge_mower_test", "MQTT 客户端 ID"],
        ["username / password", "空", "空", "留空 = 匿名连接，无用户名密码"],
        ["keep_alive_interval", "30", "30", "MQTT 保活间隔（秒）"],
        ["connect_timeout", "5", "5", "连接超时（秒）"],
        ["device_id", "mower_001", "mower_test_huilian", "设备 ID，拼进 MQTT 主题"],
        ["topic_prefix", "mower", "mower", "MQTT 主题前缀"],
        ["location_hz", "2.0", "10.0", "定位上报频率（开机即上报）"],
        ["status_hz", "1.0", "20.0", "车辆状态上报频率"],
        ["map_origin_hz", "1.0", "1.0", "选图后地图原点周期重发频率"],
        ["cmd_timeout", "0.5", "0.5", "移动指令看门狗超时（秒），超时自动停车"],
        ["drive_max / turn_max", "10000 / 12566", "15000 / 12566",
         "设计上对应 linear/angular=1.0 的下位机数值（注意：当前 publishManualCmd 实际固定用 10000，"
         "这两个参数已读取但未被使用，见 6.11 节）"],
        ["auto_pause_on_manual", "true", "true", "自动任务中收到遥控指令时先 pause 任务"],
        ["task_step_delay", "0.5", "0.5", "start 流程中各 /signal 的间隔（秒）"],
        ["task_stop_delay", "2.0", "2.0", "stop 后延时发 stop_execution（秒）"],
        ["map_dir", "/home/nvidia/libo/lidar-slam-v2/src/fast-lio/Map", "同左",
         ".mp 地图原点文件目录，cmd/task start 带 map_name 时读取并回传 state/map_origin"],
        ["mow_height", "6", "11", "默认割草高度档位（合法范围 2~11）"],
    ])

    add_heading(doc, "2.2 依赖说明", 2)
    add_table(doc, ["依赖", "类别", "用途"], [
        ["roscpp", "ROS 包", "ROS C++ 客户端库"],
        ["std_msgs", "ROS 包", "String（/signal）、UInt16（刀盘高度反馈）"],
        ["mower_msgs", "同工作空间自定义消息包",
         "Manual_Driving_Cmd / Position / VehicleStatus 三种消息"],
        ["Paho MQTT C++（paho-mqttpp3）", "系统库",
         "MQTT 异步客户端；apt 包 libpaho-mqttpp-dev 或源码装到 /usr/local"],
        ["Paho MQTT C（paho-mqtt3as/paho-mqtt3a）", "系统库",
         "C++ 库的底层依赖，必须显式链接"],
        ["nlohmann_json", "系统库（header-only）", "JSON 解析与构造；apt 包 nlohmann-json3-dev"],
        ["Threads", "系统库", "std::thread 需要 -lpthread"],
        ["paho-mqtt（Python）", "Python 包", "scripts/test_cloud_bridge.py 使用"],
    ])
    add_para(doc, "编译方式（注意工作空间里 pure_pursuit_syq 与 pure_pursuit 是重名包，"
                  "不能整空间裸跑 catkin_make，必须用白名单）：")
    add_code(doc, """cd ~/crawler_control
catkin_make -DCATKIN_WHITELIST_PACKAGES="mower_msgs;cloud_bridge"
source devel/setup.bash""")

    add_heading(doc, "2.3 CMakeLists.txt 详解", 2)
    add_code(doc, """cmake_minimum_required(VERSION 3.0.2)
project(cloud_bridge)

add_compile_options(-std=c++17)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  mower_msgs
)

find_package(Threads REQUIRED)""")
    add_para(doc, "标准 catkin 开头。-std=c++17 启用 C++17（代码用到类内初始化、lambda、std::atomic 等现代特性）；"
                  "find_package(catkin ... COMPONENTS) 声明依赖的 ROS 包，mower_msgs 是同空间的自定义消息包；"
                  "Threads 单独查找。")
    add_code(doc, """find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
  find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp)
  if(NOT NLOHMANN_JSON_INCLUDE_DIR)
    message(FATAL_ERROR "nlohmann/json.hpp not found. Install: sudo apt install nlohmann-json3-dev")
  endif()
endif()""")
    add_para(doc, "nlohmann_json 先按 CMake config 包找（QUIET 不报错），找不到退化为 find_path 只找头文件"
                  "（该库 header-only，仅需头文件路径）；两级都找不到就 FATAL_ERROR 并提示安装命令。"
                  "末尾的 if(nlohmann_json_FOUND) 分支相应地区分“链接 CMake target”与“只加 include 目录”。")
    add_code(doc, """find_library(PAHO_MQTT_CPP_LIB NAMES paho-mqttpp3
  PATHS /usr/local/lib /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu)
...
find_library(PAHO_MQTT_C_LIB NAMES paho-mqtt3as paho-mqtt3a
  PATHS /usr/local/lib /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu)""")
    add_para(doc, "Paho 没有可靠的 CMake config，只能 find_library 按库文件名搜：paho-mqttpp3 是 C++ 库，"
                  "paho-mqtt3as/paho-mqtt3a 是底层 C 库（s=静态、a=异步）；PATHS 同时覆盖源码安装（/usr/local）"
                  "与 apt 安装的 x86/ARM(aarch64) 两种目录。C++ 库内部依赖 C 库，两个都必须显式链接，"
                  "否则链接期报 undefined reference。找不到时同样 FATAL_ERROR 并给出安装提示。")
    add_code(doc, """catkin_package(
  INCLUDE_DIRS include
  CATKIN_DEPENDS roscpp std_msgs mower_msgs
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_executable(cloud_bridge_node
  src/cloud_bridge_node.cpp
  src/mqtt_client.cpp
)

add_dependencies(cloud_bridge_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

target_link_libraries(cloud_bridge_node
  ${catkin_LIBRARIES}
  ${PAHO_MQTT_CPP_LIB}
  ${PAHO_MQTT_C_LIB}
  Threads::Threads
)""")
    add_para(doc, "catkin_package 声明本包对外头文件目录 include 与消息依赖。add_executable 把两个 .cpp 编成 "
                  "cloud_bridge_node 一个节点。add_dependencies 保证先完成 mower_msgs 消息代码生成再编本节点"
                  "（使用自定义消息必须写的一行）。target_link_libraries 链接 catkin 库、两个 paho 库与线程库。"
                  "文件末尾 install 规则把可执行文件装进 CATKIN_PACKAGE_BIN_DESTINATION、launch 目录装进 "
                  "CATKIN_PACKAGE_SHARE_DESTINATION，供 catkin_make install 部署。")

    add_heading(doc, "2.4 package.xml", 2)
    add_code(doc, """<package format="2">
  <name>cloud_bridge</name>
  <version>1.0.0</version>
  <description>MQTT cloud bridge for the mower: remote driving, blade control,
    auto task start/stop and location reporting.</description>

  <maintainer email="dev@todo.todo">huilian</maintainer>
  <license>TODO</license>

  <buildtool_depend>catkin</buildtool_depend>

  <depend>roscpp</depend>
  <depend>std_msgs</depend>
  <depend>mower_msgs</depend>
</package>""")
    add_para(doc, "format=\"2\" 格式下一条 <depend> 同时声明编译与运行依赖。这里只列 ROS 包；"
                  "nlohmann_json、paho 属系统库，不走 package.xml，只在 CMakeLists.txt 里 find。"
                  "license 还是 TODO 占位，正式发布前需要补上。")


# ==================== 第3章 通信接口总表 ====================

def chapter_interfaces(doc):
    add_heading(doc, "3. 通信接口总表", 1)
    add_para(doc, "本章汇总 cloud_bridge_node 的全部对外接口。ROS 侧话题名取自 cloud_bridge_node.cpp 构造函数；"
                  "MQTT 主题按 {topic_prefix}/{device_id}/... 规则拼接（以默认 mower/mower_001 为例）；"
                  "协议细节与 JSON 字段定义以包内 docs/CLOUD_INTERFACE.md 为准。")

    add_heading(doc, "3.1 ROS 订阅话题", 2)
    add_table(doc, ["话题", "消息类型", "队列", "回调", "用途"], [
        ["/Mower/position", "mower_msgs/Position", "1", "positionCb",
         "融合定位（位置+姿态+定位状态字），缓存最新值供定时上报"],
        ["/vehicle/status", "mower_msgs/VehicleStatus", "1", "vehicleStatusCb",
         "电量与两级报警字（udp_com_main 从下位机转发）"],
        ["/vehicle/mower_height_to_app", "std_msgs/UInt16", "1", "mowerHeightCb",
         "刀盘实际高度档位反馈"],
        ["/map_name", "std_msgs/String", "1", "mapNameCb",
         "本地地图文件名列表（\"name1/name2/\" 格式），转发到 state/map_list"],
        ["/signal", "std_msgs/String", "10", "signalCb",
         "自订阅：监听选图信号 use_map/<编号>，触发地图原点上发"],
    ])

    add_heading(doc, "3.2 ROS 发布话题", 2)
    add_table(doc, ["话题", "消息类型", "队列", "用途"], [
        ["/mower/manual_driving_cmd", "mower_msgs/Manual_Driving_Cmd", "1",
         "遥控底盘+刀盘指令，task_node 消费后下发下位机"],
        ["/signal", "std_msgs/String", "10",
         "任务/初始化/建图控制字符串信号，task_node、pure_pursuit 等消费"],
    ])

    add_heading(doc, "3.3 MQTT 下行（云平台 → 车，订阅，QoS 1）", 2)
    add_table(doc, ["主题", "处理函数", "关键字段", "动作"], [
        ["mower/{id}/cmd/move", "handleMoveCmd", "linear, angular（-1~1）",
         "限幅后转发 /mower/manual_driving_cmd；自动任务中先 pause"],
        ["mower/{id}/cmd/blade", "handleBladeCmd", "state(0/1), height(2~11 可选)",
         "任务中走 /signal(open/close/档位)，手动模式搭在 Manual_Driving_Cmd 里"],
        ["mower/{id}/cmd/task", "handleTaskCmd",
         "action=start/stop/pause/continue，start 带 map_name/map_mode",
         "start 走 startTaskSequence 发信号序列；stop 延时补发 stop_execution"],
        ["mower/{id}/cmd/init_location", "handleInitLocationCmd",
         "action=request/confirm/cancel",
         "映射为 /signal：init_location / true / false"],
        ["mower/{id}/cmd/mapping", "handleMappingCmd",
         "action=enter/start_boundary/.../save/delete/list/reset 等 13 种",
         "映射为 /signal：m_mode/start_brd/.../save_map/<名> 等"],
    ])

    add_heading(doc, "3.4 MQTT 上行（车 → 云平台，发布，QoS 0）", 2)
    add_table(doc, ["主题", "触发方式", "内容"], [
        ["mower/{id}/state/location", "location_timer_ 按 location_hz（launch 10Hz）",
         "x/y/z、roll/pitch/yaw、定位状态 state、stamp"],
        ["mower/{id}/state/vehicle", "status_timer_ 按 status_hz（launch 20Hz）",
         "battery_soc、warning_state_one/two、mower_height、stamp"],
        ["mower/{id}/state/mapping", "建图会话期间（mapping_active_）随 location 同步转发",
         "与 state/location 字段完全一致，供云平台实时描边"],
        ["mower/{id}/state/map_list", "收到 /map_name 时",
         "maps 字符串数组、stamp"],
        ["mower/{id}/state/map_origin", "选图（/signal=use_map/<名>）时一次 + map_origin_timer_ 周期重发",
         "map_name、found、map_index、latitude/longitude/height、gauss_yaw、stamp"],
    ])

    add_heading(doc, "3.5 用到的消息定义（mower_msgs）", 2)
    add_para(doc, "以下定义摘自 crawler_control/src/mower_msgs/msg/，用于准确理解上表中的字段：")
    add_code(doc, """# Manual_Driving_Cmd.msg
uint8 ad_control_enable
uint16 turn_value
uint16 drive_value
uint8 gear_model
uint8 mover_bool
uint8 mow_height

# Position.msg
Header header
float64 position_x
float64 position_y
float64 position_z
float64 roll
float64 pitch
float64 yaw
uint32  position_state

# VehicleStatus.msg
Header header
uint8 battery_soc
uint8 warning_state_one
uint8 warning_state_two""")
    add_para(doc, "要点：Manual_Driving_Cmd 的 turn_value/drive_value 是 uint16（无符号），"
                  "负数靠补码回绕传递（详见 8.7 节）；mover_bool 是刀盘开关、mow_height 是高度档位；"
                  "Position 的 position_state 是定位状态字（0=无定位，1/2/5=有效融合定位），"
                  "云平台据此判断坐标是否可信。")

    add_heading(doc, "3.6 /signal 信号字典", 2)
    add_para(doc, "cloud_bridge 是云平台指令到 /signal 字符串的主要翻译者。下表汇总本节点发出的所有信号值"
                  "（消费方为 task_node、pure_pursuit 及建图/定位侧节点）：")
    add_table(doc, ["/signal 值", "来源指令", "含义"], [
        ["pause / continue / stop", "cmd/task 或任务中收到 cmd/move", "自动任务暂停 / 继续 / 停止"],
        ["stop_execution", "cmd/task stop 延时 task_stop_delay 后", "关闭路径跟踪节点 pure_pursuit"],
        ["start_execution", "cmd/task start 序列第 1 步", "拉起 pure_pursuit"],
        ["reset", "cmd/task start 序列 / cmd/mapping reset", "复位状态机"],
        ["use_map/<地图名>", "cmd/task start 带 map_name", "加载指定地图（本节点自订阅此信号上发地图原点）"],
        ["single_map / multi_map", "cmd/task start 带 map_mode", "选单图 / 多图模式"],
        ["start_work", "cmd/task start 序列最后一步", "开工"],
        ["open / close / <2~11 数字>", "cmd/blade（任务运行中）", "刀盘开 / 关 / 高度档位"],
        ["init_location / true / false", "cmd/init_location request/confirm/cancel", "定位初始化请求 / 确认 / 取消"],
        ["m_mode / p_mode / reset", "cmd/mapping enter / list / reset", "建图模式进入 / 请求地图列表 / 放弃录制"],
        ["start_brd / cease_brd", "cmd/mapping start_boundary / stop_boundary", "开始 / 停止录边界"],
        ["start_obs / cease_obs", "cmd/mapping start_obstacle / stop_obstacle", "开始 / 停止录障碍物"],
        ["start_point / cease_point", "cmd/mapping start_parking / stop_parking", "开始 / 停止录停车位"],
        ["start_path / cease_path", "cmd/mapping start_path / stop_path", "开始 / 停止录连接路径"],
        ["save_map/<名> / delete_name/<名>", "cmd/mapping save / delete", "保存 / 删除地图文件"],
    ])


# ==================== 第4章 mqtt_client.h ====================

def chapter_header(doc):
    add_heading(doc, "4. mqtt_client.h —— MQTT 客户端封装（声明）", 1)
    add_para(doc, "该头文件定义 MqttConfig 配置结构体与 MqttClient 类。MqttClient 是对 Eclipse Paho MQTT C++ 库 "
                  "mqtt::async_client 的一层薄封装，对外只暴露 start/stop/publish/addSubscription/"
                  "setMessageCallback/isConnected 几个接口，内部处理断线自动重连与重连后恢复全部订阅。")

    add_heading(doc, "4.1 MqttConfig 配置结构体", 2)
    add_code(doc, """namespace cloud_bridge
{

  struct MqttConfig
  {
    std::string host = "127.0.0.1";
    int port = 1883;
    std::string client_id = "cloud_bridge";
    std::string username;      // 留空表示不使用用户名密码
    std::string password;
    int keep_alive_interval = 30;
    int connect_timeout = 5;
  };""")
    add_para(doc, "纯数据聚合结构体，所有成员用类内初始值给出默认值，调用方只改需要的字段。"
                  "keep_alive_interval 是 MQTT 保活秒数（期间无报文时客户端自动发 PINGREQ）；"
                  "connect_timeout 是 TCP/MQTT 连接超时秒数。username 留空表示匿名连接，"
                  "构造函数据此决定是否设置用户名密码。")

    add_heading(doc, "4.2 MqttClient 类声明", 2)
    add_code(doc, """  // paho async_client 的简单封装：明文 TCP 连接，断线指数退避自动重连，
  // 重连成功后自动恢复全部订阅。
  class MqttClient : public virtual mqtt::callback
  {
    public:
      using MessageCallback =
          std::function<void(const std::string& topic, const std::string& payload)>;

      explicit MqttClient(const MqttConfig& config);
      ~MqttClient() override;

      bool start();   // 阻塞式首次连接，成功返回 true
      void stop();    // 停止重连线程并断开连接
      bool isConnected() const { return connected_.load(); }

      bool publish(const std::string& topic, const std::string& payload, int qos = 0);
      bool publish(const std::string& topic, const void* payload, size_t len, int qos = 0);

      void addSubscription(const std::string& topic, int qos = 1);
      void setMessageCallback(MessageCallback cb) { message_cb_ = std::move(cb); }""")
    add_para(doc, "公有虚继承 paho 的回调接口 mqtt::callback（声明 connection_lost / message_arrived / "
                  "delivery_complete 三个虚函数），用 virtual 继承是 paho 官方示例写法，"
                  "避免与其它监听接口多继承时出现菱形继承问题。explicit 构造函数禁止 MqttConfig 的隐式转换。"
                  "MessageCallback 是“收到 (topic, payload) 就调用”的 std::function 类型别名，"
                  "使用者（主节点）用 lambda 注入处理逻辑，实现 MQTT 层与业务层解耦。")
    add_para(doc, "publish 两个重载：string 版本是便利包装，内部转调 (void*, len) 二进制版本；"
                  "默认 qos=0（最多一次，适合高频状态上报），addSubscription 默认 qos=1（至少一次，适合下行指令）。"
                  "isConnected() 头文件内联，connected_ 是 std::atomic<bool>，.load() 原子读保证跨线程可见。")

    add_heading(doc, "4.3 私有回调与成员变量", 2)
    add_code(doc, """    private:
      void connection_lost(const std::string& cause) override;
      void message_arrived(mqtt::const_message_ptr msg) override;
      void delivery_complete(mqtt::delivery_token_ptr /*token*/) override {}

      void reconnectLoop();
      void resubscribeAll();

      MqttConfig config_;
      mqtt::async_client client_;
      mqtt::connect_options conn_opts_;
      MessageCallback message_cb_;

      std::atomic<bool> connected_{false};
      std::atomic<bool> stopping_{false};
      std::atomic<bool> reconnect_needed_{false};
      std::thread reconnect_thread_;

      std::mutex sub_mutex_;
      std::vector<std::pair<std::string, int>> subscriptions_;""")
    add_para(doc, "delivery_complete 用 /*token*/ 注释掉形参名避免“未使用参数”警告，空函数体表示不关心 "
                  "QoS1 投递回执。mqtt::const_message_ptr 是 paho 的共享指针类型"
                  "（std::shared_ptr<const message> 别名），消息生命周期由库管理。")
    add_para(doc, "三个 std::atomic<bool>：connected_（在线标志）、stopping_（用户要求停止）、"
                  "reconnect_needed_（需要重连）。它们被 paho 回调线程、重连线程、ROS 线程同时读写，"
                  "用原子类型免锁保证安全。subscriptions_ 是 (topic, qos) 列表，addSubscription 与 "
                  "resubscribeAll 都可能访问且可能跑在不同线程，所以配 sub_mutex_。"
                  "含 mutex 成员的类不可拷贝，本类也确实不需要拷贝语义。")


# ==================== 第5章 mqtt_client.cpp ====================

def chapter_client_cpp(doc):
    add_heading(doc, "5. mqtt_client.cpp —— MQTT 客户端实现", 1)

    add_heading(doc, "5.1 构造函数：连接选项配置", 2)
    add_code(doc, """MqttClient::MqttClient(const MqttConfig& config)
  : config_(config),
    client_("tcp://" + config.host + ":" + std::to_string(config.port),
            config.client_id)
{
  conn_opts_.set_clean_session(true);
  conn_opts_.set_keep_alive_interval(config.keep_alive_interval);
  conn_opts_.set_connect_timeout(std::chrono::seconds(config.connect_timeout));
  if (!config.username.empty())
  {
    conn_opts_.set_user_name(config.username);
    conn_opts_.set_password(config.password);
  }
  client_.set_callback(*this);
}""")
    add_para(doc, "成员初始化列表中构造 paho 异步客户端 client_（无默认构造，必须在初始化列表给出"
                  "“服务器 URI + 客户端 ID”），URI 形如 tcp://broker.emqx.io:1883（明文 TCP，无 TLS）。"
                  "函数体配置连接选项：clean_session=true 表示 broker 不保留离线消息和旧订阅——配合重连后 "
                  "resubscribeAll() 重新订阅即可；connect_timeout 用 std::chrono::seconds 显式包装成时间段类型。"
                  "用户名非空才设置用户名密码，否则匿名连接。client_.set_callback(*this) 把本对象注册为回调，"
                  "之后断线、消息到达等事件都回到本类重写的虚函数。")

    add_heading(doc, "5.2 start()：阻塞式首次连接", 2)
    add_code(doc, """bool MqttClient::start()
{
  reconnect_thread_ = std::thread(&MqttClient::reconnectLoop, this);
  try
  {
    client_.connect(conn_opts_)->wait();
    connected_ = true;
    resubscribeAll();
    return true;
  }
  catch (const mqtt::exception&)
  {
    connected_ = false;
    reconnect_needed_ = true;  // 交给重连线程继续尝试
    return false;
  }
}""")
    add_para(doc, "先启动重连线程（std::thread(成员函数指针, this) 是新线程跑成员函数的标准写法），"
                  "此时 reconnect_needed_ 为 false，重连线程空转等待。client_.connect() 返回异步 token，"
                  "->wait() 阻塞等结果，故 start() 是阻塞式首连。失败时 paho 抛 mqtt::exception，"
                  "catch 后不退出，而是置 reconnect_needed_=true 交给重连线程后台重试，并返回 false "
                  "告知调用方首发失败——broker 没起时节点照常运行，broker 上线后自动连上。"
                  "成功后 resubscribeAll() 把已登记的订阅真正下发给 broker。")

    add_heading(doc, "5.3 stop() 与析构", 2)
    add_code(doc, """MqttClient::~MqttClient()
{
  stop();
}

void MqttClient::stop()
{
  stopping_ = true;
  reconnect_needed_ = false;
  if (reconnect_thread_.joinable())
    reconnect_thread_.join();
  if (connected_.load())
  {
    try
    {
      client_.disconnect()->wait();
    }
    catch (const mqtt::exception&)
    {
    }
  }
  connected_ = false;
}""")
    add_para(doc, "析构只调 stop()，集中释放。stop 顺序很关键：先置 stopping_=true 让重连线程下一轮退出；"
                  "joinable() 判断线程对象是否还关联未回收线程（stop 可能被调多次——main 里手动 stop 一次、"
                  "析构再一次——不判断就 join 会抛 std::system_error）；join() 阻塞等重连线程结束，"
                  "保证之后没有线程再碰 client_。确认安全后才 disconnect()->wait() 优雅断开，"
                  "断开异常（如恰好断网）空 catch 吞掉——对象都要销毁了，断不开也无所谓。")

    add_heading(doc, "5.4 publish：两个重载", 2)
    add_code(doc, """bool MqttClient::publish(const std::string& topic, const std::string& payload, int qos)
{
  return publish(topic, payload.data(), payload.size(), qos);
}

bool MqttClient::publish(const std::string& topic, const void* payload, size_t len, int qos)
{
  if (!connected_.load())
    return false;
  try
  {
    client_.publish(topic, payload, len, qos, false);
    return true;
  }
  catch (const mqtt::exception&)
  {
    return false;
  }
}""")
    add_para(doc, "string 版本用 data()/size() 取底层缓冲转发给二进制版本，避免逻辑重复。"
                  "发布前原子检查 connected_，断线直接返回 false 丢弃——上行状态高频且有时效，"
                  "丢了等下一帧即可，不值得缓存。publish 第五个参数 false 是 retained 标志（不用保留消息）；"
                  "调用不 ->wait()，纯异步发出，不阻塞 ROS 定时器回调线程。")

    add_heading(doc, "5.5 addSubscription 与 resubscribeAll", 2)
    add_code(doc, """void MqttClient::addSubscription(const std::string& topic, int qos)
{
  {
    std::lock_guard<std::mutex> lock(sub_mutex_);
    subscriptions_.emplace_back(topic, qos);
  }
  if (connected_.load())
  {
    try
    {
      client_.subscribe(topic, qos)->wait();
    }
    catch (const mqtt::exception&)
    {
    }
  }
}""")
    add_para(doc, "内层花括号人为制造作用域，std::lock_guard 析构即解锁（RAII），锁只覆盖 vector 修改，"
                  "不含后面的阻塞网络操作，临界区最小。emplace_back 原地构造 pair 比 push_back 少一次移动。"
                  "设计思路：订阅信息无论在线与否先落进列表；在线就立即 ->wait() 阻塞等 broker 确认，"
                  "断线时等重连成功后由 resubscribeAll 统一补订。")
    add_code(doc, """void MqttClient::resubscribeAll()
{
  std::lock_guard<std::mutex> lock(sub_mutex_);
  for (const auto& sub : subscriptions_)
  {
    try
    {
      client_.subscribe(sub.first, sub.second)->wait();
    }
    catch (const mqtt::exception&)
    {
    }
  }
}""")
    add_para(doc, "范围 for 遍历 (topic, qos) 列表逐个补订阅，单个失败只跳过不中断其它订阅。"
                  "持锁期间做阻塞网络操作临界区偏大，但只在连接建立时执行一次，可接受。")

    add_heading(doc, "5.6 paho 回调：connection_lost 与 message_arrived", 2)
    add_code(doc, """void MqttClient::connection_lost(const std::string& /*cause*/)
{
  connected_ = false;
  if (!stopping_.load())
    reconnect_needed_ = true;
}

void MqttClient::message_arrived(mqtt::const_message_ptr msg)
{
  if (message_cb_)
    message_cb_(msg->get_topic(), msg->get_payload_str());
}""")
    add_para(doc, "connection_lost 由 paho 线程在检测到断线时调用：标记离线并置 reconnect_needed_ 唤醒重连；"
                  "检查 stopping_ 是为主动 stop 过程中不再触发重连。message_arrived 把 paho 消息对象转成 "
                  "(topic, payload) 字符串转发给上层注入的回调，先 if (message_cb_) 判空"
                  "（未设置时调用空的 std::function 会抛 std::bad_function_call）。"
                  "注意这两个函数都跑在 paho 线程里，所以上层回调访问共享数据必须加锁或用原子变量。")

    add_heading(doc, "5.7 reconnectLoop：指数退避重连", 2)
    add_code(doc, """void MqttClient::reconnectLoop()
{
  int delay = 1;
  while (!stopping_.load())
  {
    if (!reconnect_needed_.load())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      continue;
    }
    try
    {
      client_.connect(conn_opts_)->wait();
      connected_ = true;
      reconnect_needed_ = false;
      delay = 1;
      resubscribeAll();
    }
    catch (const mqtt::exception&)
    {
      std::this_thread::sleep_for(std::chrono::seconds(delay));
      delay = std::min(delay * 2, 30);
    }
  }
}""")
    add_para(doc, "重连线程主循环：无需重连时每 200ms 醒一次查标志（轮询而非条件变量，简单够用）。"
                  "需要重连时 connect 并阻塞等结果；成功后清标志、退避 delay 重置 1 秒、补订阅。"
                  "失败后睡 delay 秒再翻倍并用 std::min 封顶 30 秒——重试间隔 1s、2s、4s、8s、16s、30s、30s……"
                  "既避免 broker 宕机时被高频重连冲击，也避免日志刷屏。详细分析见 8.5 节。")
