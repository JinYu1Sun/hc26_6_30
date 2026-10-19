# -*- coding: utf-8 -*-
"""生成《udp_com_main代码详解》Word 文档
用法: python gen_udp_com_main_code_doc.py
产物: 代码详解/udp_com_main代码详解.docx
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
    """生成一个带表头的表格，表头加粗。"""
    t = doc.add_table(rows=1 + len(rows), cols=len(headers))
    t.style = "Table Grid"
    for j, h in enumerate(headers):
        cell = t.rows[0].cells[j]
        cell.text = ""
        r = cell.paragraphs[0].add_run(h)
        set_run_font(r, bold=True, size=Pt(10.5))
    for i, row in enumerate(rows):
        for j, v in enumerate(row):
            cell = t.rows[i + 1].cells[j]
            cell.text = ""
            r = cell.paragraphs[0].add_run(str(v))
            set_run_font(r, size=Pt(10.5))
    return t


# ==================== 第1章 包总体介绍 ====================

def chapter_overview(doc):
    add_heading(doc, "1. 包总体介绍", 1)
    add_para(doc, "udp_com_main 是履带割草机车端的 ROS1 功能包，职责是充当车端 ROS 系统与下位机"
                  "（底盘控制器，IP 192.168.1.30:8888）之间的 UDP 通信桥梁："
                  "下行方向把 ROS 话题上的车辆控制指令（/vehicle/cmd）打包成 17 字节的 UDP 帧发给下位机；"
                  "上行方向用独立线程接收下位机回传的 UDP 状态帧（电量、左右轮转速、割盘高度、手自动状态），"
                  "按帧头 0xAA 0x55 + 消息 ID 分发解析后转成 ROS 话题发布出去，"
                  "供 cloud_bridge（上云）、pure_pursuit（导航）等节点消费。")
    add_para(doc, "包名 udp_com_main 的由来：本包最初拷自 Continental 开源的通用 UDP 通信包 udp_com"
                  "（README.md、CHANGELOG.rst 中的版本号为 1.1.2），后来在包内重写了面向割草机业务的"
                  " udp_async_node.cpp，并把 CMakeLists.txt 整体改写为只编译这一个节点。"
                  "原始 udp_com 的文件（include/ 下的 boost::asio 封装、msg/srv、test/ 等）仍保留在目录里"
                  "作为历史备份，但当前 CMakeLists.txt 完全不编译它们，详解以 udp_async_node 为主。", )
    add_para(doc, "节点/可执行文件清单：", bold=True)
    add_table(doc,
              ["可执行文件", "源文件", "节点名", "说明"],
              [["udp_async_node", "src/udp_async_node.cpp", "udp_async_node",
                "唯一实际编译运行的节点：UDP 异步收发（带接收缓冲区与独立接收线程）"]])
    add_para(doc, "目录结构树：")
    add_code(doc, """crawler_control/src/udp_com_main/
├── src/udp_async_node.cpp          主节点源码（当前唯一编译的 cpp，494 行）
├── launch/udp_async_node.launch    启动文件：remote_ip / remote_port / local_port / max_queue_size
├── config/udp_vehicle_config.yaml  旧版 udp_onlysend_node 的配置备份（当前节点不读取）
├── CMakeLists.txt                  只编译 udp_async_node；文件上半部分是原 udp_com 的注释备份
├── package.xml                     包清单；文件尾部注释块是原 udp_com 1.1.2 的清单备份
├── include/udp_com.h               【历史备份】原 udp_com 的 boost::asio UDP 封装类声明
├── include/udp_com_nodelet.h       【历史备份】原 udp_com 的 nodelet 服务声明
├── msg/UdpPacket.msg               【历史备份】原 udp_com 的 UDP 包消息定义
├── srv/UdpSend.srv                 【历史备份】原 udp_com 的发送服务定义
├── srv/UdpSocket.srv               【历史备份】原 udp_com 的建套接字服务定义
├── test/                           【历史备份】原 udp_com 的 gtest/rostest 单元测试
├── README.md / CHANGELOG.rst / Doxyfile / rosdep.yaml / LICENSE.txt / NOTICES.txt
│                                  均来自原 Continental udp_com 开源包
└── height_msg.txt                  运行时生成：割盘高度持久化文件（在包目录下，路径硬编码）""")
    add_para(doc, "整体数据流：", bold=True)
    add_para(doc, "1) 下行（ROS → 下位机）：task_node（遥控/任务仲裁）或 pure_pursuit（自动导航）把 "
                  "mower_msgs/VehicleCmd 发到 /vehicle/cmd；udp_async_node 订阅后在回调里调用 "
                  "createUdpPacket() 组出 17 字节 VEHC 帧（驱动量 int32 大端 + 转向量 int32 大端 + "
                  "刀盘开关/高度/使能 + 16 位发送计数器），sendto() 发给下位机 8888 端口。")
    add_para(doc, "2) 上行（下位机 → ROS）：接收线程用 poll+recvfrom 收 5 字节固定帧 "
                  "（帧头 AA 55 + 消息 ID + 2 字节大端数据）：ID 0x01 电量 → /vehicle/status"
                  "（mower_msgs/VehicleStatus，cloud_bridge 订阅后上云）；ID 0x02/0x03 左右轮转速 → "
                  "/vehicle/right_wheel_speed、/vehicle/left_wheel_speed（std_msgs/Int16）；"
                  "ID 0x04 割盘高度脉冲数 → 写入 height_msg.txt 持久化；ID 0x05 手/自动状态 → "
                  "/vehicle/pc_or_remote（std_msgs/UInt16）。")
    add_para(doc, "3) 高度回环（定时器）：0.5 秒定时器读 height_msg.txt 的最后一条完整记录，"
                  "把脉冲数换算成档位（round(11 - 9.0×脉冲/906)）发布到 /vehicle/mower_height_to_app"
                  "（cloud_bridge 订阅后上云显示），同时把原始脉冲值按 AA 55 06 帧回发给下位机做应答。")
    add_para(doc, "线程模型：主线程跑 ros::spin()，处理 /vehicle/cmd 订阅回调和高度回环定时器；"
                  "另起一个 std::thread 跑 receiveLoop() 专门收 UDP。两个执行流之间用 "
                  "rx_mutex_ 保护的 std::deque 环形缓冲区交换原始包，tx_counter_、running_ "
                  "用 std::atomic 保证跨线程安全。详见第 6 章。")


# ==================== 第2章 启动与构建 ====================

def chapter_launch(doc):
    add_heading(doc, "2. 启动与构建", 1)

    add_heading(doc, "2.1 launch/udp_async_node.launch —— 启动参数", 2)
    add_code(doc, """<launch>
    <!-- UDP 异步收发节点（带接收缓冲区） -->
    <node pkg="udp_com_main"
          type="udp_async_node"
          name="udp_async_node"
          output="screen">

        <!-- 目标设备IP地址 -->
        <param name="remote_ip" value="192.168.1.30"/>

        <!-- 目标端口号 -->
        <param name="remote_port" value="8888"/>

        <!-- 本地绑定端口（收发节点必须绑定） -->
        <param name="local_port" value="8888"/>

        <!-- 接收缓冲区最大长度 -->
        <param name="max_queue_size" value="100"/>
    </node>
</launch>""")
    add_para(doc, "节点标签内的 4 个 <param> 是私有参数，与 loadParameters() 中的读取一一对应：")
    add_table(doc,
              ["参数名", "类型/默认值", "用途"],
              [["remote_ip", "string / 192.168.1.30", "下位机（底盘控制器）IP，inet_pton 解析后作为 sendto 目标地址"],
               ["remote_port", "int / 8888", "下位机 UDP 端口，htons 转网络字节序后填入 remote_addr_"],
               ["local_port", "int / 8888", "本机绑定端口。收发一体的节点必须 bind，否则下位机回包没有稳定的目的端口"],
               ["max_queue_size", "int / 100", "接收环形缓冲区 rx_queue_ 的最大长度，超出时丢弃最旧一帧"]])
    add_para(doc, "output=\"screen\" 把 ROS_INFO/ROS_WARN 打到终端，方便在车上直接观察收发日志。"
                  "launch 注释里还留了 /vehicle/cmd、/vehicle/status 的 remap 示例，默认不重映射。")
    add_para(doc, "注意：config/udp_vehicle_config.yaml 里保存的是旧版 udp_onlysend_node 的参数"
                  "（local_address、scale_factor、timeout_ms 等），当前 udp_async_node 不读取它，"
                  "只作历史备份，launch 也没有 <rosparam> 加载该文件。")

    add_heading(doc, "2.2 CMakeLists.txt —— 构建配置", 2)
    add_code(doc, """cmake_minimum_required(VERSION 3.0.2)
project(udp_com_main)

# 设置C++11标准
add_compile_options(-std=c++11)

# 查找依赖包
find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  mower_msgs
)

catkin_package(
  CATKIN_DEPENDS roscpp std_msgs mower_msgs
)

include_directories(
  ${catkin_INCLUDE_DIRS}
)

# 创建可执行文件
add_executable(udp_async_node
  src/udp_async_node.cpp
)

# 链接库
target_link_libraries(udp_async_node
  ${catkin_LIBRARIES}
  pthread
)""")
    add_para(doc, "文件第 1~228 行是原 Continental udp_com 包的 CMakeLists（message_generation、"
                  "nodelet 库、rostest、CPack 打 deb 包等），全部被注释掉，仅作备份。"
                  "第 229 行起才是当前生效的配置：catkin 标准三段式 —— find_package 声明依赖、"
                  "catkin_package 导出依赖、add_executable + target_link_libraries 编译节点。")
    add_para(doc, "要点1：-std=c++11。代码用到 std::thread、std::atomic、lambda 等 C++11 特性，"
                  "这是最低要求（不像 cloud_bridge 用 C++17）。")
    add_para(doc, "要点2：依赖只有 roscpp / std_msgs / mower_msgs。mower_msgs 是项目自定义消息包，"
                  "提供 VehicleCmd（下行指令）和 VehicleStatus（上行车辆状态）两种消息。"
                  "UDP 通信用的是 Linux 原生 socket API（sys/socket.h），不走 boost::asio，"
                  "所以不需要 Boost 依赖；线程用 std::thread，链接时显式加 pthread。")
    add_para(doc, "要点3：没有写 add_dependencies(udp_async_node ${catkin_EXPORTED_TARGETS})。"
                  "严格来说用自定义消息时应该加这一行保证先生成 mower_msgs 头文件；"
                  "本工作空间里能编译通过，是因为 mower_msgs 总是先于 udp_com_main 被构建，"
                  "属于隐式依赖，单独编译本包时可能报头文件缺失。")
    add_code(doc, """install(TARGETS udp_async_node
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
install(DIRECTORY launch/
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/launch
)
install(DIRECTORY config/
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/config
)""")
    add_para(doc, "install 规则把可执行文件、launch 目录、config 目录装进 install 空间，"
                  "供 catkin_make install 部署。")

    add_heading(doc, "2.3 package.xml —— 包清单", 2)
    add_code(doc, """<package format="2">
  <name>udp_com_main</name>
  <version>1.0.0</version>
  <description>UDP-based vehicle command sender for ROS</description>
  <buildtool_depend>catkin</buildtool_depend>
  <build_depend>roscpp</build_depend>
  <build_depend>std_msgs</build_depend>
  <build_depend>mower_msgs</build_depend>
  <exec_depend>roscpp</exec_depend>
  <exec_depend>std_msgs</exec_depend>
  <exec_depend>mower_msgs</exec_depend>
  <exec_depend>message_runtime</exec_depend>
</package>
<!-- 文件尾部注释块：原 udp_com 1.1.2 的 package.xml（含 nodelet 插件导出） -->""")
    add_para(doc, "format=\"2\" 格式。编译依赖与运行依赖与 CMakeLists 一致；exec_depend 里多了 "
                  "message_runtime（历史遗留，本包现在不生成自己的消息，实际用不到，留着无害）。"
                  "文件尾部的注释块完整保留了原 udp_com 包的清单（含 nodelet 插件导出声明），"
                  "同样是备份用途。")


# ==================== 第3章 udp_async_node.cpp（上） ====================

def chapter_node_head(doc):
    add_heading(doc, "3. udp_async_node.cpp —— 主节点（上）：初始化", 1)
    add_para(doc, "主文件约 494 行，只含一个类 UdpAsyncNode（UDP 收发、帧解析、高度持久化）"
                  "外加 main 函数。文件头部的注释块已经准确概括了节点职责，与代码行为一致。")

    add_heading(doc, "3.1 头文件与宏", 2)
    add_code(doc, """#include <ros/ros.h>
#include <fstream>    // 核心：文件流
#include <mower_msgs/VehicleCmd.h>
#include <mower_msgs/VehicleStatus.h>
#include <std_msgs/UInt8MultiArray.h>
#include <std_msgs/Int16.h>
#include <std_msgs/UInt16.h>
#include <std_msgs/String.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
...
#include <deque>
#include <mutex>
#include <thread>
#include <atomic>
#include <algorithm>

#define DATA_SIZE 17""")
    add_para(doc, "分三组：1) ROS 与消息：mower_msgs::VehicleCmd（订阅的下行指令）、"
                  "mower_msgs::VehicleStatus（发布的电量/报警）、std_msgs::Int16（轮速）、"
                  "std_msgs::UInt16（割盘高度、手自动状态）。std_msgs/UInt8MultiArray.h 和 String.h "
                  "包含后实际未使用，是历史遗留。")
    add_para(doc, "2) Linux 原生 socket API：sys/socket.h（socket/sendto/recvfrom）、"
                  "netinet/in.h（sockaddr_in、htons/ntohs）、arpa/inet.h（inet_pton/inet_ntoa）、"
                  "unistd.h（close/usleep）、poll.h（poll 多路复用）。"
                  "本节点没有用原 udp_com 包的 boost::asio，而是直接裸写 POSIX socket，更轻量。")
    add_para(doc, "3) STL：deque（环形缓冲区）、mutex、thread、atomic、algorithm（std::min 限幅）。"
                  "DATA_SIZE 17 是下行 VEHC 帧的固定长度，vehicleCmdCallback 用它校验发送是否完整。")

    add_heading(doc, "3.2 UdpPacket 结构体与成员变量", 2)
    add_code(doc, """struct UdpPacket {
    std::vector<uint8_t> data;
    std::string src_ip;
    int src_port;
    ros::Time stamp;
};""")
    add_para(doc, "接收缓冲区的元素类型：原始字节 + 来源 IP/端口 + 到达时间戳。"
                  "注意它与历史备份 msg/UdpPacket.msg 结构相似但不是同一个东西——"
                  "这是 .cpp 内私有的 POD，不进 ROS 消息系统。")
    add_code(doc, """    ros::NodeHandle nh_;
    ros::NodeHandle private_nh_;
    ros::Subscriber vehicle_cmd_sub_;
    ros::Publisher recv_pub_;          // /vehicle/status
    ros::Publisher left_speed_pub_;    // /vehicle/left_wheel_speed
    ros::Publisher right_speed_pub_;   // /vehicle/right_wheel_speed
    ros::Publisher height_pub_;        // /vehicle/mower_height_to_app
    ros::Publisher PC_or_Remote_pub_;  // /vehicle/pc_or_remote
    ros::Timer height_echo_timer_;

    int udp_socket_;
    struct sockaddr_in remote_addr_;

    // 接收线程与缓冲区
    std::thread recv_thread_;
    std::atomic<bool> running_;
    std::deque<UdpPacket> rx_queue_;
    std::mutex rx_mutex_;

    // 发送计数器,vehicleCmdCallback 每发一次 +1,0-65535 循环
    std::atomic<uint16_t> tx_counter_;""")
    add_para(doc, "成员分四组：ROS 句柄（双 NodeHandle：nh_ 全局命名空间做话题，private_nh_(\"~\") "
                  "读私有参数）、socket 状态（描述符 + 预解析好的下位机地址）、"
                  "接收线程三件套（线程对象、原子运行标志、带锁的 deque 缓冲区）、"
                  "以及 16 位发送序号 tx_counter_（atomic 是因为接收线程的组帧函数不碰它、"
                  "但将来多线程发送时免锁）。")

    add_heading(doc, "3.3 构造函数", 2)
    add_code(doc, """    UdpAsyncNode() : udp_socket_(-1), private_nh_("~"), running_(false),
                     max_queue_size_(100), tx_counter_(0) {
        loadParameters();

        if (!initializeUdpSocket()) {
            ROS_ERROR("Failed to initialize UDP socket");
            ros::shutdown();
            return;
        }

        // 订阅车辆控制话题
        vehicle_cmd_sub_ = nh_.subscribe<mower_msgs::VehicleCmd>(
            "/vehicle/cmd", 10, &UdpAsyncNode::vehicleCmdCallback, this);

        // 创建接收数据发布者
        recv_pub_ = nh_.advertise<mower_msgs::VehicleStatus>("/vehicle/status", 100);
        left_speed_pub_ = nh_.advertise<std_msgs::Int16>("/vehicle/left_wheel_speed", 10);
        right_speed_pub_ = nh_.advertise<std_msgs::Int16>("/vehicle/right_wheel_speed", 10);
        height_pub_ = nh_.advertise<std_msgs::UInt16>("/vehicle/mower_height_to_app", 10);
        PC_or_Remote_pub_ = nh_.advertise<std_msgs::UInt16>("/vehicle/pc_or_remote", 10);

        // 启动接收线程
        running_ = true;
        recv_thread_ = std::thread(&UdpAsyncNode::receiveLoop, this);

        height_echo_timer_ = nh_.createTimer(ros::Duration(0.5),
                                             &UdpAsyncNode::heightEchoCallback, this);
    }""")
    add_para(doc, "初始化顺序很关键：先读参数，再初始化 socket——失败时 ros::shutdown() + return，"
                  "此时后续的话题/线程/定时器都没创建，对象处于“半构造”状态，"
                  "靠 main 里的 isInitialized() 判断拦住（见 3.10）。"
                  "udp_socket_(-1) 先置无效值，isInitialized() 就靠它判断。")
    add_para(doc, "socket 就绪后才 subscribe/advertise：订阅 /vehicle/cmd 队列 10（指令帧，"
                  "下位机控制周期快，允许小缓冲）；/vehicle/status 队列 100（历史遗留的大队列，"
                  "实际发布频率取决于下位机回帧频率）；其余状态话题队列 10。")
    add_para(doc, "running_ = true 必须先于 std::thread 创建，否则接收线程第一轮循环可能读到 "
                  "false 直接退出（虽然概率极小，但顺序写对是习惯）。"
                  "std::thread(&UdpAsyncNode::receiveLoop, this) 是在新线程跑成员函数的标准写法。"
                  "最后创建 0.5 秒周期的高度回环定时器，回调由 ros::spin() 所在的主线程执行。")

    add_heading(doc, "3.4 loadParameters —— 读 launch 参数", 2)
    add_code(doc, """    void loadParameters() {
        private_nh_.param<std::string>("remote_ip", remote_ip_, "192.168.1.30");
        private_nh_.param<int>("remote_port", remote_port_, 8888);
        private_nh_.param<int>("local_port", local_port_, 8888);
        private_nh_.param<int>("max_queue_size", max_queue_size_, 100);
    }""")
    add_para(doc, "private_nh_.param(参数名, 输出变量, 默认值)：参数服务器没有该参数就用默认值。"
                  "模板参数 <std::string>/<int> 显式给出。四个参数的语义见 2.1 的表。"
                  "默认值与 launch 文件里写的值一致，所以即使不用 launch 直接 rosrun 也能连到默认下位机。")

    add_heading(doc, "3.5 initializeUdpSocket —— 建套接字、绑定、配目标", 2)
    add_code(doc, """    bool initializeUdpSocket() {
        udp_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_socket_ < 0) {
            ROS_ERROR("Failed to create socket: %s", strerror(errno));
            return false;
        }

        // 允许地址重用
        int reuse = 1;
        if (setsockopt(udp_socket_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            ROS_WARN("Failed to set SO_REUSEADDR: %s", strerror(errno));
        }

        // 绑定本地端口（收发节点必须绑定，否则无法稳定接收）
        struct sockaddr_in local_addr;
        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = INADDR_ANY;
        local_addr.sin_port = htons(local_port_);

        if (bind(udp_socket_, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
            ROS_ERROR("Failed to bind port %d: %s", local_port_, strerror(errno));
            close(udp_socket_);
            udp_socket_ = -1;
            return false;
        }""")
    add_para(doc, "AF_INET + SOCK_DGRAM 创建 IPv4 UDP 套接字。SO_REUSEADDR 允许端口处于 "
                  "TIME_WAIT 等状态时立刻重新绑定——节点崩溃重启后不用等系统回收端口；"
                  "设置失败只警告不终止（非致命）。")
    add_para(doc, "bind 到 INADDR_ANY:local_port_：INADDR_ANY（0.0.0.0）表示监听本机所有网卡，"
                  "下位机回包无论从哪个接口进来都能收到。bind 失败（典型原因：8888 被别的进程占用）"
                  "是致命错误，close 并把描述符复位为 -1，返回 false 让构造函数走 shutdown 分支。"
                  "htons() 把主机字节序转网络字节序（大端），端口字段必须转。")
    add_code(doc, """        // 配置远程地址
        memset(&remote_addr_, 0, sizeof(remote_addr_));
        remote_addr_.sin_family = AF_INET;
        remote_addr_.sin_port = htons(remote_port_);
        if (inet_pton(AF_INET, remote_ip_.c_str(), &remote_addr_.sin_addr) <= 0) {
            ROS_ERROR("Invalid remote IP: %s", remote_ip_.c_str());
            close(udp_socket_);
            udp_socket_ = -1;
            return false;
        }

        // 设置发送超时
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if (setsockopt(udp_socket_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
            ROS_WARN("Failed to set send timeout: %s", strerror(errno));
        }
        return true;
    }""")
    add_para(doc, "remote_addr_ 一次性解析好存为成员，之后每次 sendto 直接用，省去逐包解析开销。"
                  "inet_pton 把点分十进制字符串转成 in_addr 二进制，返回值 <=0 表示格式非法"
                  "（比如把 IP 写成了主机名），同样按致命错误处理。")
    add_para(doc, "SO_SNDTIMEO 给 sendto 设 1 秒发送超时。UDP 发送通常不会阻塞，"
                  "但内核协议栈缓冲满时可能卡住，超时兜底防止车辆控制回调被拖死；"
                  "设置失败依然只警告。三段错误处理的分级很清晰：建 socket/bind/解析 IP 失败 → "
                  "致命返回 false；setsockopt 失败 → 警告继续。")


# ==================== 第4章 udp_async_node.cpp（中）：接收线程 ====================

def chapter_node_recv(doc):
    add_heading(doc, "4. udp_async_node.cpp —— 主节点（中）：接收线程 receiveLoop", 1)

    add_heading(doc, "4.1 poll 等待与 recvfrom 接收", 2)
    add_code(doc, """    void receiveLoop() {
        std::vector<uint8_t> buf(1500);
        struct sockaddr_in sender_addr;
        socklen_t addr_len = sizeof(sender_addr);

        while (running_) {
            struct pollfd pfd;
            pfd.fd = udp_socket_;
            pfd.events = POLLIN;

            // 100ms 超时，便于及时检查 running_ 标志退出线程
            int ret = poll(&pfd, 1, 100);
            if (ret < 0) {
                if (errno == EINTR) continue;
                ROS_WARN("poll error: %s", strerror(errno));
                usleep(10000);
                continue;
            }
            if (ret == 0) continue; // 超时，继续循环

            ssize_t n = recvfrom(udp_socket_, buf.data(), buf.size(), 0,
                                 (struct sockaddr*)&sender_addr, &addr_len);
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
                ROS_WARN("recvfrom error: %s", strerror(errno));
                continue;
            }""")
    add_para(doc, "缓冲区 1500 字节 = 以太网 MTU，一个 UDP 数据报最大也就这么大，不会截断。"
                  "接收模型是 poll + 阻塞 recvfrom 的组合：poll 等 POLLIN（有数据可读），"
                  "超时 100ms——这是“可中断的阻塞等待”的经典写法。如果直接裸调阻塞 recvfrom，"
                  "节点退出时线程会一直卡在 recvfrom 里 join 不掉；poll 每 100ms 醒一次检查 "
                  "running_ 标志，shutdown() 置 false 后最迟 100ms 线程就能退出。")
    add_para(doc, "错误分支三级处理：poll 返回 <0 且 errno==EINTR（被信号打断）属正常，直接重试；"
                  "其它 poll 错误警告后睡 10ms 防刷屏再重试；poll 返回 0 是超时，continue 回循环头检查 "
                  "running_。recvfrom 返回 <0 时 EAGAIN/EWOULDBLOCK（无数据，竞态下可能发生）忽略，"
                  "其它错误警告后继续。任何错误都不会让线程退出——接收线程是全节点的生命线，"
                  "设计上要求它“永远不死”。")

    add_heading(doc, "4.2 入队：环形缓冲区", 2)
    add_code(doc, """            UdpPacket pkt;
            pkt.data.assign(buf.begin(), buf.begin() + n);
            pkt.src_ip = inet_ntoa(sender_addr.sin_addr);
            pkt.src_port = ntohs(sender_addr.sin_port);
            pkt.stamp = ros::Time::now();

            {
                std::lock_guard<std::mutex> lock(rx_mutex_);
                rx_queue_.push_back(pkt);
                if (rx_queue_.size() > max_queue_size_) {
                    rx_queue_.pop_front();
                }
            }""")
    add_para(doc, "收到一帧后先封装成 UdpPacket：data 按实际长度 n 拷贝（不是整个 1500 字节缓冲），"
                  "inet_ntoa 把来源地址转回点分十进制字符串（用于日志），ntohs 端口转主机序，"
                  "stamp 打 ROS 时间戳（后面填进消息 header）。")
    add_para(doc, "入队时内层花括号人为制造作用域，std::lock_guard 出作用域自动解锁（RAII），"
                  "锁只覆盖对 deque 的两次操作。push_back 后若超过 max_queue_size_（默认 100）"
                  "就 pop_front 丢最旧的——典型的有界环形缓冲区（FIFO 覆盖），防止下位机狂发时内存暴涨。"
                  "getRxQueue()/getRxQueueSize() 两个公有接口也加同一把锁对外提供快照，"
                  "当前文件内没有调用者，是预留的调试接口。")

    add_heading(doc, "4.3 帧头校验与分发", 2)
    add_code(doc, """            // ===== 帧头校验：AA 55 + 消息ID + 2字节数据，每帧固定5字节 =====
            if (pkt.data.size() < 5 || pkt.data[0] != 0xAA || pkt.data[1] != 0x55) {
                ROS_WARN("bad frame header, drop [%zu bytes] from %s:%d",
                         pkt.data.size(), pkt.src_ip.c_str(), pkt.src_port);
                continue;   // 不是我们的帧或长度不够，直接丢弃
            }

            uint8_t msg_id = pkt.data[2];
            // 有效数据为第4、5字节，大端拼装（带符号 int16）

            switch (msg_id) {
            case 0x01: { ... }   // 电量帧
            case 0x02: { ... }   // 右轮转速帧
            case 0x03: { ... }   // 左轮转速帧
            case 0x04: { ... }   // 割盘高度帧
            case 0x05: { ... }   // 割草机状态帧
            default:
                ROS_WARN("unknown msg id: 0x%02X", msg_id);
                break;
            }""")
    add_para(doc, "上行协议是固定 5 字节短帧：字节0=0xAA、字节1=0x55 为帧头魔数，字节2=消息 ID，"
                  "字节3、4 为 16 位数据（大端，高字节在前）。校验只看“长度≥5 + 两个魔数字节”，"
                  "不满足就带来源信息警告并丢弃——UDP 是无连接协议，任何发到 8888 端口的垃圾包"
                  "都会走到这里，帧头校验就是防火墙。"
                  "注意协议没有长度字段和校验和：5 字节定长帧不需要长度；UDP 自带 CRC16 校验，"
                  "应用层就省了（车内有线局域网，误码率极低，是可接受的简化）。")
    add_para(doc, "大端拼装写法 pkt.data[3] << 8 | pkt.data[4]：uint8_t 会先整型提升为 int 再移位或运算，"
                  "所以结果天然是 int；要无符号语义就 static_cast<uint16_t>，要有符号就 "
                  "static_cast<int16_t>（16 位补码自动还原负数），下面各 case 分别演示了两种用法。")

    add_heading(doc, "4.4 case 0x01 —— 电量帧 → /vehicle/status", 2)
    add_code(doc, """            case 0x01: {   // ===== 电量帧 =====
                mower_msgs::VehicleStatus status_msg;
                status_msg.header.stamp = pkt.stamp;
                const uint16_t MAX_SOC_RAW = 0xF9F0;
                uint8_t warning_state_one = 0;
                uint8_t warning_state_two = 0;
                uint8_t battery_soc = 0;

                // 电量按无符号解析，计算百分比，只保留整数部分
                uint16_t raw = static_cast<uint16_t>(pkt.data[3] << 8 | pkt.data[4]);
                if (raw > MAX_SOC_RAW) {
                    battery_soc = 100;
                    // 数据大于 F9F0：警报2的 bit0 置1
                    warning_state_two |= 0x01;
                } else {
                    battery_soc = static_cast<uint8_t>((raw * 100U) / MAX_SOC_RAW);
                }
                status_msg.battery_soc = battery_soc;
                status_msg.warning_state_one = warning_state_one;
                status_msg.warning_state_two = warning_state_two;

                recv_pub_.publish(status_msg);
                break;
            }""")
    add_para(doc, "下位机上报的电量原始值 raw 是 0~0xF9F0（63984，对应满电电压采样值的标定上限）"
                  "之间的无符号 16 位数。百分比换算公式：battery_soc = raw × 100 / 63984，"
                  "整数除法自动截断只保留整数部分。raw * 100U 中的 U 后缀把 100 变成 unsigned int，"
                  "使乘法在 32 位无符号域进行——若按 uint16 直接乘 100 会溢出回绕，这是常见的整型提升陷阱，"
                  "代码里写法是正确的。")
    add_para(doc, "raw 超过上限（采样异常或过压）时不按公式算，直接置 100%，同时把报警字二的 bit0 置 1，"
                  "让云端能看到“电量数据异常”的标志。warning_state_one 当前恒为 0，是预留字段。"
                  "填好 header.stamp（用收包时刻而不是发包时刻，更接近真实采样时间）后发布到 "
                  "/vehicle/status，cloud_bridge 节点订阅这个话题把电量和报警字拼 JSON 上云。")

    add_heading(doc, "4.5 case 0x02 / 0x03 —— 左右轮转速帧", 2)
    add_code(doc, """            case 0x02: {   // ===== 右轮转速帧 =====
                std_msgs::Int16 speed_msg;
                speed_msg.data = static_cast<int16_t>(pkt.data[3] << 8 | pkt.data[4]);
                ROS_INFO("rightspeed=%d,data3=%x,data4=%x",speed_msg.data,pkt.data[3],pkt.data[4]);
                right_speed_pub_.publish(speed_msg);
                break;
            }
            case 0x03: {   // ===== 左轮转速帧 =====
                std_msgs::Int16 speed_msg;
                speed_msg.data = static_cast<int16_t>(pkt.data[3] << 8 | pkt.data[4]);
                ROS_INFO("leftspeed=%d,data3=%x,data4=%x",speed_msg.data,pkt.data[3],pkt.data[4]);
                left_speed_pub_.publish(speed_msg);
                break;
            }""")
    add_para(doc, "两个 case 结构完全对称，只是发布的话题不同。转速按有符号 int16 解析——"
                  "负值表示反转（倒车），static_cast<int16_t> 让 16 位补码还原出负数。"
                  "ROS_INFO 每帧都打印原始字节和解析值，这是调试期留下的日志："
                  "下位机高频回帧时这行会刷得比较快，但 output=\"screen\" 的终端上正好用来肉眼验证"
                  "“轮子转没转、方向对不对”。")

    add_heading(doc, "4.6 case 0x04 —— 割盘高度帧与文件持久化", 2)
    add_code(doc, """            case 0x04: {   // ===== 割盘高度帧 =====
                std_msgs::UInt16 height_msg;
                height_msg.data = std::min<int>(906,
                    static_cast<uint16_t>(pkt.data[3] << 8 | pkt.data[4]));

                // 每收到一帧就把最新值追加到文件，只保留最近 kMaxHeightLines 条
                const std::string height_path =
                    "/home/nvidia/crawler_control/src/udp_com_main/height_msg.txt";
                const size_t kMaxHeightLines = 100;   // 想保存多少条就改这里

                std::deque<std::string> lines;
                std::ifstream fin(height_path);
                std::string line;
                while (std::getline(fin, line)) {
                    lines.push_back(line);
                }
                fin.close();

                while (lines.size() >= kMaxHeightLines) {
                    lines.pop_front();
                }
                lines.push_back(std::to_string(height_msg.data));

                std::ofstream fout(height_path, std::ios::trunc);
                if (!fout) {
                    ROS_ERROR("open height_msg.txt for write failed");
                    break;
                }
                for (const std::string &l : lines) {
                    fout << l << '\\n';
                }
                fout.close();
                break;
            }""")
    add_para(doc, "割盘高度以脉冲数（0~906）上报，先用 std::min<int>(906, ...) 限幅封顶——"
                  "906 是丝杠行程的脉冲标定上限，超出的视为异常采样直接截断。"
                  "注意这个 case 不发布任何 ROS 话题，只做一件事：把脉冲值追加写进 "
                  "height_msg.txt。发布动作由 0.5 秒定时器统一做（见 5.2），"
                  "这样设计把“高频接收”与“低频上报/回环”解耦。")
    add_para(doc, "文件维护策略：每帧都全量读出所有行 → 队首弹到不足 100 行 → 追加新值 → "
                  "整体重写（std::ios::trunc）。相当于在磁盘上维护一个最多 100 行的滚动窗口。"
                  "每帧重写整个文件效率不高，但换来两个好处：一是文件内容永远是“最近 100 条有效记录”，"
                  "二是重写语义简单可靠。文件存在的目的在 5.2 揭晓——断电重启后恢复割盘高度。")
    add_para(doc, "文件路径 /home/nvidia/... 是硬编码的绝对路径（车端工控机是 NVIDIA Jetson），"
                  "换部署环境时需要改代码重新编译，这是本文件里少数几个不够工程化的点之一。"
                  "写文件打开失败时报错并 break，本帧数据丢弃但不影响后续接收。")

    add_heading(doc, "4.7 case 0x05 与 default —— 手/自动状态帧", 2)
    add_code(doc, """            case 0x05: {   // ===== 割草机状态帧 =====
                std_msgs::UInt16 PC_or_Remote_msg;

                PC_or_Remote_msg.data = pkt.data[3];

                PC_or_Remote_pub_.publish(PC_or_Remote_msg);
                break;
            }
            default:
                ROS_WARN("unknown msg id: 0x%02X", msg_id);
                break;""")
    add_para(doc, "0x05 帧上报车端当前是 PC（自动/上位机控制）还是 Remote（遥控器手动接管）状态。"
                  "与其它帧不同，它只取第 3 字节一个字节（data[4] 忽略），赋给 UInt16 消息的 data "
                  "（uint8 → uint16 零扩展）发布到 /vehicle/pc_or_remote，供上层判断控制权归属。"
                  "未知消息 ID 走 default 打警告，便于协议升级时在老节点日志里发现新帧。")


# ==================== 第5章 udp_async_node.cpp（下）：发送与收尾 ====================

def chapter_node_send(doc):
    add_heading(doc, "5. udp_async_node.cpp —— 主节点（下）：发送路径与收尾", 1)

    add_heading(doc, "5.1 vehicleCmdCallback —— /vehicle/cmd 订阅回调", 2)
    add_code(doc, """    void vehicleCmdCallback(const mower_msgs::VehicleCmd::ConstPtr &msg) {
        std::vector<uint8_t> data = createUdpPacket(
            msg->mover_bool, msg->drive_value, msg->turn_value,
            msg->mower_height, msg->ad_control_enable);
        // 每次通过本回调发送数据,计数器 +1
        tx_counter_.fetch_add(1);

        ssize_t sent = sendto(udp_socket_, data.data(), data.size(), 0,
                              (struct sockaddr*)&remote_addr_, sizeof(remote_addr_));
        if (sent < 0) {
            ROS_WARN("Send failed: %s", strerror(errno));
        } else if (sent != DATA_SIZE) {
            ROS_WARN("Partial send: %zd/%d", sent, DATA_SIZE);
        } else {
            ROS_INFO("TX: drive=%d, turn=%d,ad_control=%d",
                     msg->drive_value, msg->turn_value, msg->ad_control_enable);
        }
    }""")
    add_para(doc, "这是下行链路（ROS → 下位机）的入口，由 ros::spin() 所在的主线程执行。"
                  "流程三步：1) 调 createUdpPacket() 把 VehicleCmd 消息的 5 个有效字段组出 17 字节帧；"
                  "2) tx_counter_.fetch_add(1) 把 16 位发送序号加一（注意：先组帧后加一，"
                  "所以本帧里装的是加一之前的旧值，序号语义是“本帧是第 N 帧”，下位机可用它检测丢包）；"
                  "3) sendto() 发给预先解析好的 remote_addr_（192.168.1.30:8888）。")
    add_para(doc, "发送结果三级判断：sent<0 是真正失败（网络不可达等），打警告；"
                  "sent != DATA_SIZE 是短写（UDP 理论上要么整个数据报发出去要么失败，短写几乎不会发生，"
                  "但代码仍防御性检查）；成功则打一行 INFO 日志记录本次驱动量/转向量/使能位，"
                  "方便在车上用 rostopic echo 对照实际运动。")
    add_para(doc, "本回调不做任何限频：上游 /vehicle/cmd 发多快，UDP 就发多快。"
                  "整车链路里 task_node / pure_pursuit 通常以 10~20Hz 发指令，对 UDP 来说毫无压力；"
                  "SO_SNDTIMEO 的 1 秒超时（见 3.5）保证即使内核缓冲异常，回调也不会被永久阻塞。")

    add_heading(doc, "5.2 heightEchoCallback —— 0.5 秒高度回环定时器", 2)
    add_code(doc, """    void heightEchoCallback(const ros::TimerEvent&) {
        const std::string height_path =
            "/home/nvidia/crawler_control/src/udp_com_main/height_msg.txt";

        // 以二进制+末尾定位方式打开，先判断最后一个字节是不是 '\\n'
        std::ifstream fin(height_path, std::ios::binary | std::ios::ate);
        if (!fin) {
            return;   // 文件还不存在
        }
        if (fin.tellg() == 0) {
            return;   // 空文件
        }

        fin.seekg(-1, std::ios::end);
        char last_char = 0;
        fin.get(last_char);
        const bool last_line_complete = (last_char == '\\n');

        // 回到开头，逐行读，同时记住最后一行和倒数第二行
        fin.seekg(0);
        std::string line, last, prev;
        while (std::getline(fin, line)) {
            if (line.empty()) continue;   // 跳过空行
            prev = last;
            last = line;
        }
        fin.close();

        // 最后一行完整就用它；不完整（写了一半断电）说明损坏，退回上一条
        std::string chosen;
        if (last_line_complete) {
            chosen = last;
        } else {
            chosen = prev;
            ROS_WARN("height_msg.txt last line incomplete, use previous value");
        }
        if (chosen.empty()) {
            return;   // 没有可用的完整数据（比如文件里只有一条残行）
        }

        uint16_t height = static_cast<uint16_t>(std::atoi(chosen.c_str()));
        std_msgs::UInt16 height_to_app_;
        height_to_app_.data = round(11 - 9.0 * height / 906.0);
        height_pub_.publish(height_to_app_);
        // 组帧回发：AA 55 + 0x06 + 2字节数据（大端）
        uint8_t tx[5] = {0xAA, 0x55, 0x06,
                        static_cast<uint8_t>(height >> 8),
                        static_cast<uint8_t>(height & 0xFF)};
        ssize_t sent = sendto(udp_socket_, tx, sizeof(tx), 0,
                            (struct sockaddr*)&remote_addr_, sizeof(remote_addr_));
        if (sent < 0) {
            ROS_WARN("height echo send failed: %s", strerror(errno));
        }
    }""")
    add_para(doc, "这个定时器回调是本文件里设计得最讲究的一段，它把 4.6 写进 height_msg.txt 的脉冲数"
                  "以 2Hz 的频率做两件事：发布给云端、回发给下位机。理解它的前提是理解"
                  " height_msg.txt 存在的意义——断电恢复：割盘高度由丝杠电机驱动，下位机掉电后"
                  "编码器脉冲计数可能丢失，车端工控机把最近的高度值持久化到磁盘，"
                  "重启后继续按最后已知高度上报/应答，避免割盘高度状态跳变。")
    add_para(doc, "读文件分两轮。第一轮：std::ios::ate 直接定位到文件末尾，读最后一个字节判断"
                  "最后一行是否以换行符结尾——如果最后一个字节不是换行，说明上一次写文件写到一半"
                  "断电/崩溃了，最后一行是残行，不可用。文件不存在（首次运行）或为空时直接 return，"
                  "本周期静默跳过，不算错误。")
    add_para(doc, "第二轮：seekg 回开头逐行读，同时维护 last（最后一行）和 prev（倒数第二行）。"
                  "最后一行完整就用 last；不完整就退回 prev 并打警告。极端情况（文件里只有一条残行）"
                  "prev 也是空，chosen 为空直接 return。这套“残行回退”机制是针对掉电场景的"
                  "典型防御式编程：宁可上报旧一拍的值，也不把损坏数据发出去。")
    add_para(doc, "高度换算公式：档位 = round(11 - 9.0 × 脉冲 / 906)。脉冲 0 对应档位 11（割盘最高），"
                  "脉冲 906 对应档位 2（最低附近），即脉冲数与档位线性反比，906 个脉冲分 9 个档位跨度。"
                  "round() 四舍五入取整，结果发布到 /vehicle/mower_height_to_app，"
                  "cloud_bridge 订阅后上云，手机 App 上显示的就是这个档位。")
    add_para(doc, "回发给下位机的是原始脉冲值（不是档位），帧格式与上行短帧一致："
                  "AA 55 + ID 0x06 + 2 字节大端脉冲数，固定 5 字节。这相当于对 0x04 高度帧的"
                  "周期性应答/心跳，下位机可以据此确认车端 ROS 在线且高度链路正常。"
                  "发送失败只警告不处理——0.5 秒后下一周期会重发，天然具备重试语义。")

    add_heading(doc, "5.3 createUdpPacket —— 17 字节 VEHC 帧组帧", 2)
    add_code(doc, """    std::vector<uint8_t> createUdpPacket(int mover_bool, int drive_value,
                                         int turn_value, int mower_height,
                                         int ad_control_enable) {
        std::vector<uint8_t> data(DATA_SIZE);
        data[0] = 'V'; data[1] = 'E'; data[2] = 'H'; data[3] = 'C';

        int32_t drive_scaled = static_cast<int32_t>(drive_value);
        data[4] = (drive_scaled >> 24) & 0xFF;
        data[5] = (drive_scaled >> 16) & 0xFF;
        data[6] = (drive_scaled >> 8) & 0xFF;
        data[7] = drive_scaled & 0xFF;

        int32_t turn_scaled = static_cast<int32_t>(turn_value);
        data[8] = (turn_scaled >> 24) & 0xFF;
        data[9] = (turn_scaled >> 16) & 0xFF;
        data[10] = (turn_scaled >> 8) & 0xFF;
        data[11] = turn_scaled & 0xFF;

        data[12] = static_cast<uint8_t>(mover_bool);
        data[13] = static_cast<uint8_t>(mower_height);
        data[14] = static_cast<uint8_t>(ad_control_enable);

        // 16位发送计数器,大端,溢出自动回绕
        uint16_t cnt = tx_counter_.load();
        data[15] = (cnt >> 8) & 0xFF;
        data[16] = cnt & 0xFF;
        return data;
    }""")
    add_para(doc, "下行 VEHC 帧布局（固定 17 字节，全部大端）：", bold=True)
    add_table(doc,
              ["字节偏移", "字段", "含义"],
              [["0~3", "'V' 'E' 'H' 'C'", "帧头魔数（ASCII \"VEHC\"，Vehicle Control 缩写）"],
               ["4~7", "drive_value (int32 大端)", "驱动量：前后速度指令，负值倒车"],
               ["8~11", "turn_value (int32 大端)", "转向量：差速转向指令，履带车左右轮速差"],
               ["12", "mover_bool (uint8)", "割盘（刀盘）开关：0 停 / 1 转"],
               ["13", "mower_height (uint8)", "割盘目标高度档位"],
               ["14", "ad_control_enable (uint8)", "自动驾驶使能：0 手动 / 1 上位机接管"],
               ["15~16", "tx_counter_ (uint16 大端)", "发送序号，每帧 +1，0~65535 循环，供下位机检丢包"]])
    add_para(doc, "注意 VehicleCmd.msg 里 drive_value / turn_value 声明为 int16，"
                  "组帧时却按 int32 扩展成 4 字节大端写入——协议给驱动/转向预留了 32 位量程，"
                  "当前实际只用 int16 范围，属于协议的前向兼容设计。位移 + 掩码的手工拆字节写法"
                  "（>>24 & 0xFF …）不依赖 htonl，语义清晰且对负数的补码处理天然正确。")
    add_para(doc, "计数器用 tx_counter_.load() 读取（atomic 的显式读），uint16 溢出自动回绕，"
                  "无需手动取模。帧没有校验和与帧尾：UDP 自带 CRC16，且 17 字节定长帧靠帧头魔数"
                  "对齐即可，与上行 5 字节短帧的简化思路一致（见 4.3）。")

    add_heading(doc, "5.4 sendStopCommand 与 shutdown —— 安全停车与析构", 2)
    add_code(doc, """    void sendStopCommand() {
        std::vector<uint8_t> stop_data = createUdpPacket(0, 0, 0, 0, 0);
        stop_data[0] = '1'; stop_data[1] = '2';
        stop_data[2] = '3'; stop_data[3] = '4';
        for (int i = 0; i < 3; ++i) {
            sendto(udp_socket_, stop_data.data(), stop_data.size(), 0,
                   (struct sockaddr*)&remote_addr_, sizeof(remote_addr_));
            usleep(10000);
        }
    }

    void shutdown() {
        running_ = false;
        if (recv_thread_.joinable()) {
            recv_thread_.join();
        }
        if (udp_socket_ >= 0) {
            sendStopCommand();
            close(udp_socket_);
            udp_socket_ = -1;
        }
    }

    ~UdpAsyncNode() {
        shutdown();
    }""")
    add_para(doc, "sendStopCommand() 复用 createUdpPacket 组一帧“全零”指令（驱动 0、转向 0、"
                  "刀盘关、使能 0），然后把帧头魔数从 \"VEHC\" 改成 \"1234\"——这是与下位机约定的"
                  "紧急停车帧标识，下位机见到 1234 帧头立即停车，不解析其余字段。"
                  "连发 3 次、每次间隔 10ms：UDP 不保证送达，三发冗余把丢包导致“停不住车”的"
                  "概率降到极低，这是无连接协议上做可靠停车的标准土办法，简单但有效。")
    add_para(doc, "shutdown() 的收尾顺序经过仔细设计：1) 先置 running_ = false，通知接收线程退出——"
                  "接收线程最迟 100ms（poll 超时周期）后离开循环；2) join() 等接收线程真正结束，"
                  "此后没有任何执行流在用 socket；3) 发停车帧、close socket、描述符复位为 -1。"
                  "如果顺序反过来（先 close 再停线程），接收线程可能在已关闭的描述符上 poll/recvfrom，"
                  "属于典型的 use-after-close。析构函数直接调 shutdown()，"
                  "所以无论是 Ctrl+C 触发 ros::shutdown 还是进程正常退出，对象销毁时都会自动停车，"
                  "这是整车的最后一道安全兜底。")
    add_para(doc, "幂等性：shutdown() 可能被调两次（main 返回后析构 + 构造失败路径），"
                  "靠 udp_socket_ >= 0 和 joinable() 双重判断保证重复调用安全。")

    add_heading(doc, "5.5 isInitialized 与 main —— 进程入口", 2)
    add_code(doc, """    bool isInitialized() const {
        return udp_socket_ >= 0;
    }

int main(int argc, char** argv) {
    ros::init(argc, argv, "udp_async_node");

    UdpAsyncNode node;
    if (!node.isInitialized()) {
        ROS_ERROR("Initialization failed");
        return 1;
    }

    ros::spin();
    return 0;
}""")
    add_para(doc, "main 极简：ros::init 注册节点名 udp_async_node → 栈上构造 UdpAsyncNode → "
                  "检查 isInitialized()（即 socket 是否建好，3.3 里构造失败时 udp_socket_ 保持 -1，"
                  "这里就是拦住“半构造”对象的那道闸）→ ros::spin() 进入回调循环直到收到退出信号。"
                  "spin 返回后 main 结束，栈上对象析构触发 5.4 的自动停车收尾。"
                  "所有实际工作都在回调里：主线程处理 /vehicle/cmd 订阅和 0.5 秒高度定时器，"
                  "接收线程独立收 UDP，两个执行流的分工见第 6 章。")


# ==================== 第6章 线程模型与并发安全 ====================

def chapter_threads(doc):
    add_heading(doc, "6. 线程模型与并发安全", 1)
    add_para(doc, "节点内共有两个执行流，分工如下：", bold=True)
    add_table(doc,
              ["执行流", "入口", "承担的回调/工作"],
              [["主线程（ros::spin）", "main()", "/vehicle/cmd 订阅回调 vehicleCmdCallback；"
                "0.5s 定时器 heightEchoCallback；shutdown 时的停车帧发送"],
               ["接收线程（std::thread）", "receiveLoop()", "poll + recvfrom 收 UDP；入队 rx_queue_；"
                "帧头校验、switch 分发；发布 5 类上行话题；写 height_msg.txt"]])
    add_para(doc, "共享数据与保护手段：", bold=True)
    add_table(doc,
              ["共享资源", "访问方", "保护机制"],
              [["rx_queue_（接收环形缓冲区）", "接收线程写 / getRxQueue 等读", "std::mutex rx_mutex_，lock_guard RAII 加锁"],
               ["running_（线程退出标志）", "主线程写 / 接收线程每 100ms 读", "std::atomic<bool>"],
               ["tx_counter_（发送序号）", "主线程 +1 / 组帧读取", "std::atomic<uint16_t>，fetch_add/load"],
               ["udp_socket_", "主线程发 / 接收线程收 / shutdown 关闭", "UDP socket 本身允许一个线程收一个线程发；"
                "close 只发生在接收线程 join 之后，无竞态"],
               ["height_msg.txt", "接收线程写 / 主线程定时器读", "无锁——靠“整文件重写 + 残行回退”机制容错"
                "（读到写了一半的文件时退回上一条完整记录）"]])
    add_para(doc, "三个值得注意的并发设计点：")
    add_para(doc, "1) 可中断的阻塞等待：接收线程不用裸 recvfrom 阻塞，而是 poll 100ms 超时轮询，"
                  "保证 shutdown 置 running_=false 后线程最迟 100ms 退出，join 不会卡死。")
    add_para(doc, "2) 文件跨线程访问不加锁：height_msg.txt 被接收线程（0x04 帧到达时写）和主线程"
                  "（0.5 秒定时器读）并发访问，代码没有用文件锁，而是用 5.2 的“检查末尾换行符 + "
                  "残行回退”机制把损坏读的影响降到一条旧记录——配合写端“读全量→改→整文件重写”的"
                  "短窗口，实际风险可接受，属于用协议化思路替代锁的取舍。")
    add_para(doc, "3) 回调里没有耗时阻塞：vehicleCmdCallback 只做组帧 + sendto（有 1 秒超时兜底），"
                  "heightEchoCallback 文件读写量上限 100 行，都不会拖累 ros::spin 的回调调度。")


# ==================== 第7章 接口汇总：话题/参数/协议帧 ====================

def chapter_interfaces(doc):
    add_heading(doc, "7. 接口汇总：话题 / 参数 / 协议帧", 1)

    add_heading(doc, "7.1 订阅话题", 2)
    add_table(doc,
              ["话题", "消息类型", "队列", "来源节点", "用途"],
              [["/vehicle/cmd", "mower_msgs/VehicleCmd", "10",
                "task_node / pure_pursuit", "车辆控制指令，回调里组 VEHC 帧 UDP 下发下位机"]])
    add_para(doc, "mower_msgs/VehicleCmd 关键字段（crawler_control/src/mower_msgs/msg/VehicleCmd.msg）："
                  "mover_bool（uint8，刀盘开关）、ad_control_enable（uint8，自动驾驶使能）、"
                  "gear_model（uint8，档位，本节点未使用）、turn_value / drive_value（int16，转向/驱动量）、"
                  "mower_height（uint8，割盘高度档位）。本节点使用其中 5 个字段组帧，"
                  "gear_model 与 left/right_wheel_speed、speed_cmd 字段未使用。")

    add_heading(doc, "7.2 发布话题", 2)
    add_table(doc,
              ["话题", "消息类型", "队列", "触发源", "消费方"],
              [["/vehicle/status", "mower_msgs/VehicleStatus", "100", "0x01 电量帧",
                "cloud_bridge（拼 JSON 上云）"],
               ["/vehicle/right_wheel_speed", "std_msgs/Int16", "10", "0x02 右轮转速帧", "上层监控/调试"],
               ["/vehicle/left_wheel_speed", "std_msgs/Int16", "10", "0x03 左轮转速帧", "上层监控/调试"],
               ["/vehicle/mower_height_to_app", "std_msgs/UInt16", "10", "0.5s 定时器（脉冲→档位换算）",
                "cloud_bridge（App 显示割盘档位）"],
               ["/vehicle/pc_or_remote", "std_msgs/UInt16", "10", "0x05 手/自动状态帧",
                "上层判断控制权归属"]])
    add_para(doc, "mower_msgs/VehicleStatus 字段：header + battery_soc（uint8，电量百分比）、"
                  "warning_state_one / warning_state_two（uint8，报警字一/二）。"
                  "本节点只填 battery_soc 和 warning_state_two 的 bit0（电量采样超限标志），"
                  "warning_state_one 恒 0 为预留。")

    add_heading(doc, "7.3 服务", 2)
    add_para(doc, "本节点不提供也不调用任何 ROS 服务。目录下 srv/UdpSend.srv、srv/UdpSocket.srv "
                  "是原 Continental udp_com 包的历史备份，当前 CMakeLists 不生成、代码不使用。")

    add_heading(doc, "7.4 参数汇总", 2)
    add_table(doc,
              ["参数（私有）", "类型", "默认值", "launch 值", "说明"],
              [["~remote_ip", "string", "192.168.1.30", "192.168.1.30", "下位机 IP"],
               ["~remote_port", "int", "8888", "8888", "下位机 UDP 端口"],
               ["~local_port", "int", "8888", "8888", "本机绑定端口（INADDR_ANY）"],
               ["~max_queue_size", "int", "100", "100", "接收环形缓冲区长上限"]])

    add_heading(doc, "7.5 UDP 协议帧格式汇总", 2)
    add_para(doc, "下行（车端 → 下位机 192.168.1.30:8888）：", bold=True)
    add_table(doc,
              ["帧", "长度", "格式", "说明"],
              [["VEHC 控制帧", "17 字节", "\"VEHC\" + drive(int32 BE) + turn(int32 BE) + "
                "mover_bool + mower_height + ad_control_enable + tx_counter(uint16 BE)",
                "每收到一条 /vehicle/cmd 发一帧"],
               ["1234 停车帧", "17 字节", "同 VEHC 布局但帧头改为 \"1234\"，全零指令",
                "节点退出时连发 3 次、间隔 10ms"],
               ["高度应答帧", "5 字节", "AA 55 06 + 脉冲数(uint16 BE)",
                "0.5s 定时器周期回发，作为 0x04 帧的应答/心跳"]])
    add_para(doc, "上行（下位机 → 车端 8888 端口，均为 5 字节定长帧：AA 55 + ID + 2 字节大端数据）：",
           bold=True)
    add_table(doc,
              ["消息 ID", "含义", "数据解析", "去向"],
              [["0x01", "电量", "uint16，raw×100/0xF9F0 得百分比；超上限置 100% 并报警字二 bit0",
                "/vehicle/status"],
               ["0x02", "右轮转速", "int16（负值反转）", "/vehicle/right_wheel_speed"],
               ["0x03", "左轮转速", "int16（负值反转）", "/vehicle/left_wheel_speed"],
               ["0x04", "割盘高度脉冲", "uint16，min(906, raw) 限幅", "写 height_msg.txt（不直接发话题）"],
               ["0x05", "手/自动状态", "仅取数据高字节 data[3]", "/vehicle/pc_or_remote"],
               ["其它", "未知 ID", "—", "ROS_WARN 告警后丢弃"]])
    add_para(doc, "协议整体特征：无长度字段（全部定长）、无应用层校验和（依赖 UDP 自带 CRC16）、"
                  "大端字节序、帧头魔数对齐。上下行帧头风格不同（下行 ASCII \"VEHC\"，上行 AA 55），"
                  "说明两侧协议是分别演化的，本节点负责在中间做翻译。")


# ==================== 第8章 系统角色与总结 ====================

def chapter_summary(doc):
    add_heading(doc, "8. 在整车系统中的角色与总结", 1)

    add_heading(doc, "8.1 系统位置与包间交互", 2)
    add_para(doc, "在履带割草机车端 ROS 系统中，udp_com_main 处于最底层：它是 ROS 话题网络与"
                  "下位机底盘控制器之间唯一的 UDP 网关。上行（遥控/云端指令流）和下行（状态遥测流）"
                  "都经过它，系统数据流如下：")
    add_code(doc, """云端 App/遥控器
      │  (MQTT / 云平台)
      ▼
cloud_bridge ──/vehicle/cmd────────────────┐
                                           │
task_node ────/vehicle/cmd (遥控/任务仲裁)──┤  mower_msgs/VehicleCmd
                                           ▼
pure_pursuit ─/vehicle/cmd (自动导航)──► udp_async_node ══UDP══► 下位机(192.168.1.30:8888)
                                           │                      VEHC 控制帧 / 1234 停车帧
                                           │◄══UDP════════════════  AA 55 状态帧(0x01~0x05)
                                           │
              ┌────────────────────────────┼────────────────────────┐
              ▼                            ▼                        ▼
      /vehicle/status           /vehicle/mower_height_to_app   /vehicle/pc_or_remote
      (电量+报警→cloud_bridge)  (割盘档位→cloud_bridge→App)    + 左右轮速话题""")
    add_para(doc, "与其他包的具体交互：", bold=True)
    add_table(doc,
              ["对方包/节点", "交互内容", "方向"],
              [["task / task_node", "遥控与任务仲裁后的整车指令 /vehicle/cmd", "→ udp_com_main"],
               ["pure_pursuit", "自动导航跟踪轨迹时输出的 /vehicle/cmd", "→ udp_com_main"],
               ["cloud_bridge", "订阅 /vehicle/status（电量、报警）、/vehicle/mower_height_to_app（割盘档位）拼 JSON 上云",
                "udp_com_main → cloud_bridge"],
               ["mower_msgs", "提供 VehicleCmd / VehicleStatus 消息定义", "编译期依赖"],
               ["下位机底盘控制器", "UDP 8888 端口双向收发（VEHC 帧 / AA 55 帧）", "⇄ 网络链路"]])
    add_para(doc, "与 Mower 工作区里 agv_can_bridge（CAN 总线桥）的关系：两者都是“车端 ↔ 底盘”的"
                  "通信桥，但走不同物理链路——本项目 crawler_control 方案用 UDP（本包），"
                  "agv_can_bridge 是另一套 CAN 方案，二者不同时装车使用。")

    add_heading(doc, "8.2 历史版本说明", 2)
    add_para(doc, "包目录里保留了大量原 Continental udp_com 1.1.2 开源包的文件："
                  "include/udp_com.h（boost::asio 的 UDP 封装类声明）、include/udp_com_nodelet.h"
                  "（nodelet 服务声明）、msg/UdpPacket.msg、srv/UdpSend.srv、srv/UdpSocket.srv、"
                  "test/ 下的 gtest/rostest，以及 CMakeLists.txt 与 package.xml 中被整段注释的"
                  "原始构建配置。这些文件当前一律不参与编译和运行，仅为历史备份，"
                  "详解以 CMakeLists.txt 实际编译的 src/udp_async_node.cpp 为准。"
                  "config/udp_vehicle_config.yaml 是更早的 udp_onlysend_node（只发不收版本）的"
                  "参数备份，当前节点不读取。")

    add_heading(doc, "8.3 设计要点与已知不足总结", 2)
    add_para(doc, "设计要点：", bold=True)
    add_para(doc, "1) 双线程异步收发：poll 可中断等待 + 有界环形缓冲区，接收线程“永远不死”，"
                  "shutdown 顺序（停线程→join→发停车帧→关 socket）杜绝 use-after-close。")
    add_para(doc, "2) 安全兜底完整：退出时 1234 停车帧三连发；电量超上限置报警位；"
                  "高度数据 906 限幅；发送超时 1 秒兜底；帧头魔数过滤垃圾包。")
    add_para(doc, "3) 断电恢复：height_msg.txt 磁盘滚动窗口（100 条）+ 残行回退机制，"
                  "保证割盘高度状态跨重启连续。")
    add_para(doc, "4) 协议化容错：用“整文件重写 + 换行符完整性检查”替代文件锁，"
                  "用定长帧 + UDP 自带 CRC 简化协议，都是在车载有线内网环境下的合理取舍。")
    add_para(doc, "已知不足（维护时值得改进的点）：", bold=True)
    add_para(doc, "1) height_msg.txt 路径硬编码为 /home/nvidia/... 绝对路径，换部署环境需改码重编，"
                  "宜改为 launch 参数。")
    add_para(doc, "2) 每收到一帧 0x04 就全量重写 height_msg.txt，下位机高频上报时磁盘 I/O 偏频繁，"
                  "可改为降频写或追加+定期截断。")
    add_para(doc, "3) CMakeLists 未写 add_dependencies(... ${catkin_EXPORTED_TARGETS})，"
                  "单独编译本包时可能因子定义消息头文件未生成而失败。")
    add_para(doc, "4) 0x02/0x03 帧每帧 ROS_INFO 打印，高频下刷屏，量产宜降级为 DEBUG 或节流。")


# ==================== main：组装文档 ====================

def main():
    doc = Document()

    # 封面标题
    title = doc.add_paragraph()
    title.alignment = WD_ALIGN_PARAGRAPH.CENTER
    r = title.add_run("udp_com_main 代码详解")
    set_run_font(r, bold=True, size=Pt(22))
    sub = doc.add_paragraph()
    sub.alignment = WD_ALIGN_PARAGRAPH.CENTER
    r = sub.add_run("履带割草机车端 ROS ↔ 下位机 UDP 通信桥接节点")
    set_run_font(r, size=Pt(14))
    doc.add_paragraph()

    chapter_overview(doc)
    chapter_launch(doc)
    chapter_node_head(doc)
    chapter_node_recv(doc)
    chapter_node_send(doc)
    chapter_threads(doc)
    chapter_interfaces(doc)
    chapter_summary(doc)

    os.makedirs("代码详解", exist_ok=True)
    out_path = os.path.join("代码详解", "udp_com_main代码详解.docx")
    doc.save(out_path)
    print("saved:", out_path)


if __name__ == "__main__":
    main()
