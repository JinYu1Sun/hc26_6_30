# -*- coding: utf-8 -*-
"""生成《pure_pursuit代码详解》Word 文档
用法: python gen_pure_pursuit_code_doc.py
产物: 代码详解/pure_pursuit代码详解.docx
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
    add_para(doc, "pure_pursuit 是履带割草机车端 crawler_control 工作空间中的一个 ROS1 功能包，"
                  "是整个运动控制链路的最末端：它接收规划端下发的五维局部路径和定位端的车辆实时位姿，"
                  "用纯跟踪（Pure Pursuit）算法算出期望的线速度和角速度，"
                  "再打包成 mower_msgs/VehicleCmd 消息发到 /vehicle/cmd 话题，"
                  "由 agv_can_bridge 转成 CAN 报文下发给底盘单片机执行。"
                  "除了跟线控制，它还负责原地转向（掉头）、低速越障预警、紧急倒车、"
                  "割盘开关/高度随控制指令下发、卡死脱困、转弯完成反馈等一系列与安全相关的逻辑。")
    add_para(doc, "整个包只编译出一个可执行节点 pure_pursuit（节点名同为 pure_pursuit），"
                  "源码共 2 个头文件、2 个 .cpp 文件、1 个自定义服务、1 个 launch 文件：", bold=True)
    add_code(doc, """crawler_control/src/pure_pursuit/
├── include/
│   ├── pure_pursuit/pure_pursuit.h   PurePursuit 类声明、waypoint/Twist/状态枚举
│   └── geometry.h                    通用几何工具库（robot::geometry 命名空间，模板实现）
├── src/
│   ├── main.cpp                      节点入口（仅 11 行）
│   └── pure_pursuit.cpp              PurePursuit 类全部实现（约 590 行，核心所在）
├── srv/
│   └── TurnCompleted.srv             “某转弯点是否已转弯完成”查询服务
├── launch/
│   └── pure_pursuit.launch           启动节点并加载 5 个控制参数
├── CMakeLists.txt                    构建配置
└── package.xml                       包清单与依赖""")
    add_para(doc, "本包目录下没有带日期后缀或 _old 后缀的历史版本文件（历史备份主要存在于 "
                  "task、agv_can_bridge 等其它包中），CMakeLists.txt 编译的就是上述两个 .cpp，"
                  "本文档全部围绕实际编译的版本展开。")

    add_heading(doc, "1.1 整体数据流", 2)
    add_para(doc, "节点的数据流可以概括为“三入一出一服务”：")
    add_code(doc, """  定位模块  --/Mower/position (util/Position)----------+
  规划端   --/lawn_mower/global_path (util/LocalPath)---+        +--> /vehicle/cmd
  避障端   --/lawn_mower/avoid_state (std_msgs/UInt8)---+  状态机 |   (mower_msgs/VehicleCmd)
  避障端   --/lawn_mower/out_of_bounds (std_msgs/Bool)--+  20Hz   |        |
  task    --/mower/stop_car (std_msgs/Bool)------------+  循环   |   agv_can_bridge -> CAN -> 底盘
  云端     --/signal (std_msgs/String)------------------+        |
  遥控     --/PC_or_Remote (std_msgs/UInt8)-------------+        |
                                                          +--------+
  read_waypoints --/turn_completed (pure_pursuit/TurnCompleted 服务) <--+""")
    add_para(doc, "1) 输入侧：positionCallback 收到定位后，立刻把最新一条全局路径逐点转换到车体坐标系，"
                  "缓存为 local_waypoints_；localPathCallback 只负责缓存最新路径原文。"
                  "其余 5 个订阅回调分别更新停车标志、避障等级、越界标志、割盘配置、控制来源。"
                  "2) 计算侧：state_machine_run() 以 20Hz 循环跑一个四态状态机"
                  "（Stop / Follow / Turn / Reverse），每轮先检查各类安全条件，"
                  "再找最近点、算预瞄点、按状态产出 Twist。"
                  "3) 输出侧：publishCommand() 把 Twist 换算成底盘单片机需要的整型量纲并附带割盘状态发出。"
                  "4) 服务侧：/turn_completed 服务供规划端 read_waypoints 查询某个转弯点是否已经掉头完成，"
                  "用于决定何时裁掉已走过的路径段。")
    add_para(doc, "线程模型：本包没有显式创建额外线程，也没有用 AsyncSpinner。"
                  "构造函数最后直接调用 state_machine_run()，在该函数内以 ros::Rate(20) 循环，"
                  "每轮先 ros::spinOnce() 处理订阅/服务回调，再做控制计算。"
                  "也就是说回调和主控循环跑在同一线程里，但代码仍对跨回调共享的数据"
                  "（路径、越界、避障、割盘配置、转弯完成队列）加了互斥锁，"
                  "对标志量用了 std::atomic，保留了将来多线程化的余地，阅读时需要注意锁的配对。")


# ==================== 第2章 启动与构建 ====================

def chapter_launch_build(doc):
    add_heading(doc, "2. 启动与构建配置", 1)

    add_heading(doc, "2.1 launch/pure_pursuit.launch", 2)
    add_code(doc, """<launch>
    <node pkg="pure_pursuit" type="pure_pursuit" name="pure_pursuit" output="screen">
        <param name="v_min" value="0.08" />
        <param name="w_max" value="0.8" />
        <param name="min_lookahead_distance" value="1.5" />
        <param name="regulated_min_radius" value="1.8" />
        <param name="low_speed" value="0.3" />
    </node>
</launch>""")
    add_para(doc, "launch 文件启动一个节点，node 的 name 为 pure_pursuit，因此下面 5 个参数"
                  "在参数服务器上的全名是 /pure_pursuit/v_min 等形式，与代码里 "
                  "nh.param(\"pure_pursuit/v_min\", ...) 的私有命名空间读取方式一一对应。"
                  "output=\"screen\" 让 ROS_INFO/ROS_WARN 日志直接打到终端，方便车上调试。")
    add_para(doc, "5 个参数的含义与代码默认值对照：")
    add_table(doc,
        ["参数名（私有命名空间）", "launch 值", "代码默认值", "含义"],
        [
            ["v_min", "0.08", "0.2", "跟线线速度下限 (m/s)，纯跟踪输出不会低于该值，防止速度过小原地打滑"],
            ["w_max", "0.8", "0.15", "角速度上限 (rad/s)，纯跟踪输出的绝对值被钳制在该值以内"],
            ["min_lookahead_distance", "1.5", "1.5", "基础预瞄距离 (m)，实际预瞄距离 = 该值 + 0.7 × 期望速度"],
            ["regulated_min_radius", "1.8", "2.0", "限速转弯半径 (m)，转弯半径小于该值时按比例压低线速度，防止角速度过大"],
            ["low_speed", "0.3", "0.3", "低速预警模式下的期望速度 (m/s)，收到避障 2 级预警或越界信号时启用"],
        ])
    add_para(doc, "注意：launch 中 w_max=0.8 与代码默认值 0.15 差别很大，v_min=0.08 也小于默认值 0.2，"
                  "说明实车部署时用 launch 覆盖了默认值；阅读代码时不要把默认值当成实际运行值。")

    add_heading(doc, "2.2 CMakeLists.txt", 2)
    add_code(doc, """find_package(catkin REQUIRED COMPONENTS
  geometry_msgs
  roscpp
  std_msgs
  mower_msgs
  util
  message_generation
)

find_package(yaml-cpp CONFIG REQUIRED)  # 查找 yaml-cpp 库

## Generate services in the 'srv' folder
add_service_files(
  FILES
  TurnCompleted.srv
)

generate_messages(
  DEPENDENCIES
  std_msgs
)

catkin_package(
  CATKIN_DEPENDS geometry_msgs roscpp std_msgs message_runtime
)

 add_executable(${PROJECT_NAME} src/main.cpp src/pure_pursuit.cpp)

 target_link_libraries(${PROJECT_NAME}
   ${catkin_LIBRARIES}
   ${YAML_CPP_LIBRARIES}
 )""")
    add_para(doc, "要点有三个：")
    add_para(doc, "1) add_service_files + generate_messages 声明本包生成 TurnCompleted 服务，"
                  "生成的 C++ 头文件为 pure_pursuit/TurnCompleted.h，被 pure_pursuit.h 引用。"
                  "srv 里只有 float64 字段（属于基础类型），但 generate_messages 仍按惯例带上 std_msgs 依赖。")
    add_para(doc, "2) add_executable 只编译 src/main.cpp 和 src/pure_pursuit.cpp 两个文件，"
                  "geometry.h 是纯模板头文件，全部实现在头内，不需要单独编译。")
    add_para(doc, "3) 一个比较特别的地方：find_package 里的 util 并不是 crawler_control/src 下的独立目录，"
                  "它实际由同工作空间的 minibus_msg_util 包提供——该包 CMakeLists 里写的是 project(util)，"
                  "消息文件在 crawler_control/src/minibus_msg_util/msg/ 下，因此代码里 #include \"util/Position.h\" "
                  "对应的就是 minibus_msg_util/msg/Position.msg。这一点对排查“找不到 util 包”类问题很关键。"
                  "另外 yaml-cpp 被 find 并链接了，但当前两个 .cpp 里并没有直接 include yaml-cpp，"
                  "属于历史遗留依赖，去掉不影响编译（是否清理由维护者决定）。")
    add_para(doc, "文件末尾的 install 规则只安装了 launch/ 目录到包 share 目录，"
                  "没有安装可执行文件，说明部署方式是直接在 devel 空间运行（source devel/setup.bash 后 roslaunch）。")

    add_heading(doc, "2.3 package.xml 依赖", 2)
    add_para(doc, "package.xml 声明了 build/export/exec 三段依赖：roscpp（ROS C++ 客户端库）、"
                  "std_msgs 与 geometry_msgs（标准消息）、mower_msgs（车端自定义消息包，"
                  "提供 VehicleCmd）、util（即 minibus_msg_util，提供 Position、LocalPath）、"
                  "message_runtime（自定义服务运行期）。维护者填写为 rice。"
                  "与 CMakeLists 对照可见 message_generation 只在编译期需要，运行期由 message_runtime 承担，"
                  "这是 ROS 自定义消息/服务包的标准写法。")

    add_heading(doc, "2.4 srv/TurnCompleted.srv", 2)
    add_code(doc, """float64 x
float64 y
---
bool turn_finish""")
    add_para(doc, "请求是全局坐标系下的一个点 (x, y)，响应是 bool：该点是否已经完成转弯（掉头）。"
                  "调用方是规划端 read_waypoints 节点（Mower/src/planning/read_waypoints/），"
                  "它在裁剪已走过的路径前调用 /turn_completed 服务确认某个转弯点是否真的掉头完成，"
                  "避免把还没转完的掉头点从路径里删掉。pure_pursuit 侧的实现见第 6.3 节。")


# ==================== 第3章 geometry.h ====================

def chapter_geometry(doc):
    add_heading(doc, "3. include/geometry.h —— 通用几何工具库", 1)
    add_para(doc, "这是一个与 ROS 无关的纯模板头文件，放在 robot::geometry 命名空间下，"
                  "提供向量、角度、路径点等基础几何类型和运算。本包实际只用到了其中一小部分"
                  "（主要是 NormalizeAngle），但因为它被 pure_pursuit.cpp 包含，这里按功能分组介绍。")

    add_heading(doc, "3.1 标量工具函数", 2)
    add_code(doc, """template <class T>
T clamp(const T& v, const T& lo, const T& hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

template <typename T>
T sqr(const T& x) {
  return x * x;
}

template <typename T>
T hypotFast(T x, T y) {
  return sqrt(sqr(x) + sqr(y));
}

template <typename T>
T NormalizeAngle(const T angle) {
  T a = std::fmod(angle + M_PI, 2.0 * M_PI);
  if (a < 0.0) {
    a += (2.0 * M_PI);
  }
  return a - M_PI;
}""")
    add_para(doc, "四个模板函数：clamp 把值钳制在 [lo, hi]；sqr 求平方；hypotFast 求直角三角形斜边"
                  "（即平面两点距离，注释里明确写了“适用车身坐标系求点距”）；"
                  "NormalizeAngle 把任意弧度角归一化到 (-π, π] 区间——先把 angle+π 对 2π 取余，"
                  "负数再加 2π 保证非负，最后减 π 搬回原区间。"
                  "NormalizeAngle 是 pure_pursuit.cpp 中 positionCallback 计算 local_yaw 时必用的函数，"
                  "保证角度差不因 ±π 处跳变而出错。")

    add_heading(doc, "3.2 Vec2 二维向量", 2)
    add_code(doc, """template <typename T>
struct Vec2 {
  T x, y;
  Vec2 operator+(const Vec2& other) const { return {x + other.x, y + other.y}; }
  Vec2 operator-(const Vec2& other) const { return {x - other.x, y - other.y}; }
  Vec2 operator*(T scale) const { return {x * scale, y * scale}; }
  Vec2 operator/(T scale) const { return {x / scale, y / scale}; }
  bool operator==(const Vec2& other) const { return x == other.x && y == other.y; }
  T heading() const { return std::atan2(y, x); }
  T length() const { return hypotFast<T>(x, y); }
  T cross(const Vec2& other) const { return x * other.y - y * other.x; }
  T dot(const Vec2& other) const { return x * other.x + y * other.y; }
  T distance(const Vec2& other) const { return hypotFast<T>(x - other.x, y - other.y); }
  Vec2 normalize() const { return *this / length(); }
};
using Vec2f = Vec2<double>;
using Vec2d = Vec2<double>;""")
    add_para(doc, "完整的二维向量类，重载了加减乘除和相等比较，提供朝向角 heading、模长 length、"
                  "叉积 cross（注释指出：结果正负表示 other 相对于本向量的旋转方向，逆时针为正）、"
                  "点积 dot、点距 distance、单位化 normalize。"
                  "有个小瑕疵：Vec2f 和 Vec2d 都被定义成 Vec2<double>，名字里的 f 通常表示 float，"
                  "属于历史遗留笔误，但因为 pure_pursuit 没有使用这两个别名，不影响功能。")

    add_heading(doc, "3.3 路径与车辆状态结构体", 2)
    add_code(doc, """template <typename T>
struct Vec3{ T x, y, z; };

struct PathPoint{
    Vec2f position;
    float curvature;  //曲率
    float speed;      //期望速度
    float s;    //离第一个点的路径长度（单位：m）
    float t;
};

struct RobotState{  //实时定位信息
    Vec2f position;
    float yaw; float roll; float pitch;
    float linearSpeed;
    float goalS;   //目标点s值
};

struct PoseC{  //定位信息
    double x; double y;
    float z; float roll; float yaw; float pitch; float vel;
};

using Path = std::deque<PathPoint>;""")
    add_para(doc, "一组面向轨迹跟踪场景的数据结构：PathPoint 是带曲率、期望速度、里程 s 的路径点；"
                  "RobotState 是车辆实时状态；PoseC 是简化位姿；Path 是基于 deque 的路径类型。"
                  "这些类型在 pure_pursuit 的 .cpp 中都没有直接使用，它们更像是从上层规划模块"
                  "沿用过来的公共定义，供同工程其它模块共用。")
    add_para(doc, "文件末尾还声明了 squaredDistance 和 latError 两个非模板函数"
                  "（点到线段的平方距离、横向误差），但本包内没有对应的 .cpp 实现，"
                  "pure_pursuit 也没有调用它们；若其它模块引用需要自行提供实现，属于“声明未用”状态。")


# ==================== 第4章 main.cpp 与类声明 ====================

def chapter_main_and_header(doc):
    add_heading(doc, "4. main.cpp 与 pure_pursuit.h", 1)

    add_heading(doc, "4.1 src/main.cpp —— 节点入口", 2)
    add_code(doc, """#include <ros/ros.h>
#include <pure_pursuit/pure_pursuit.h>

int main(int argc, char** argv)
{
    setlocale(LC_ALL,"");
    ros::init(argc, argv, "pure_pursuit");
    ros::NodeHandle nh;
    PurePursuit pp(nh);
    return 0;
}""")
    add_para(doc, "入口只有四件事：setlocale 让中文日志在终端正常显示；ros::init 注册节点名 pure_pursuit；"
                  "创建全局命名空间的 NodeHandle；构造 PurePursuit 对象。"
                  "注意没有 ros::spin()——构造函数内部直接调用了 state_machine_run() 阻塞式循环，"
                  "控制循环结束（ros::ok() 为假）后才析构退出。这种“构造函数即主循环”的写法比较少见，"
                  "意味着对象构造永远不会在正常运营期间返回。")

    add_heading(doc, "4.2 include/pure_pursuit/pure_pursuit.h —— 类声明", 2)
    add_para(doc, "头文件定义了三个关键类型和 PurePursuit 类：")
    add_code(doc, """struct waypoint
{
    float global_x; // 全局坐标系下的x坐标
    float global_y;
    float global_yaw;
    float local_x;  // 小车坐标系下的x坐标
    float local_y;
    float local_yaw;
    int gear;
    float speed;
    bool operator==(const waypoint& other) const { ... }
};

typedef struct { double linear; double angular; } Twist;

enum class RunStateValue : uint8_t { Stop, Follow, Turn, Reverse };""")
    add_para(doc, "waypoint 是本包最核心的数据结构：同一个路径点同时保存全局坐标（global_*）和"
                  "车体坐标（local_*）两套表示，外加 gear（档位/点类型，2 表示掉头转弯点）和 speed（期望速度）。"
                  "纯跟踪算法全部在车体坐标系下进行，全局坐标则用于跨帧识别“这是不是同一个点”"
                  "（例如转弯完成点记录、卡死检测）。")
    add_para(doc, "RunStateValue 是状态机的四个状态：Stop（停车）、Follow（跟线）、Turn（原地转向）、"
                  "Reverse（紧急倒车），详见第 7 章。")
    add_para(doc, "PurePursuit 类的私有成员可按职责分四组：")
    add_table(doc,
        ["分组", "成员", "说明"],
        [
            ["输入缓存", "car_position_ / last_car_position_ / last_pos_time_", "最新/上一次车辆位姿及其接收时间，用于卡死检测和定位超时判断"],
            ["", "last_local_path_ / last_local_path_time_（配 local_path_mutex_）", "最新五维局部路径原文及接收时间"],
            ["", "local_waypoints_（配 local_waypoints_mutex_）", "路径转换到车体坐标系后的 waypoint 列表，由 positionCallback 重建"],
            ["状态量", "running_state_（atomic）", "状态机当前状态"],
            ["", "stop_car_（atomic）", "task 节点下发的停车标志"],
            ["", "avoid_state_（配 avoid_state_mutex_）", "避障等级：2=普通预警低速，3=紧急停车，4=紧急倒车"],
            ["", "outboundary_flag_（配 outboundary_mutex_）", "是否超出电子边界，越界则低速"],
            ["", "PC_or_Remote_", "控制来源：0=PC，1=遥控接管，2=Auto；非 0 即停车"],
            ["", "low_speed_flag_（atomic）/ low_speed_", "低速模式标志与低速值"],
            ["预瞄/转弯", "lookahead_waypoint_ / last_lookahead_waypoint_ / lookahead_distance_ / last_lookahead_distance_", "当前与上一次预瞄点及预瞄距离，用于卡死检测"],
            ["", "turn_completed_points_（配 turn_completed_mutex_）/ turn_count_ / follow_count_", "已完成掉头点队列（最多保留 3 个）、转向计数、跟线卡死计数"],
            ["参数", "v_min_ / w_max_ / min_lookahead_distance_ / regulated_min_radius_ / low_speed_", "launch 注入的 5 个控制参数，见 2.1 节"],
            ["ROS 接口", "sub_position_ 等 7 个 Subscriber、pub_command_、turn_completed_srv_", "订阅 7 路话题、发布 1 路控制指令、提供 1 个服务，见第 8 章"],
        ])
    add_para(doc, "割盘控制相关：mover_bool_cfg_（割盘开关）与 mower_height_cfg_（割盘高度 2~11）"
                  "由 /signal 话题更新，随每条 VehicleCmd 一起下发给底盘（见 6.5 节）。")


# ==================== 第5章 回调函数 ====================

def chapter_callbacks(doc):
    add_heading(doc, "5. 订阅与服务回调详解", 1)
    add_para(doc, "构造函数在初始化参数后订阅 7 路话题、发布 /vehicle/cmd、提供 /turn_completed 服务，"
                  "然后进入 state_machine_run()。本章逐个讲解回调。")

    add_heading(doc, "5.1 positionCallback —— 定位回调（核心）", 2)
    add_code(doc, """void PurePursuit::positionCallback(const util::PositionConstPtr &msg_carposition)
{
    std::lock_guard<std::mutex> lock_local_waypoints(local_waypoints_mutex_);
    util::LocalPath last_local_path;
    {
        std::lock_guard<std::mutex> lock_local_path(local_path_mutex_);
        last_local_path = last_local_path_;
    }
    car_position_ = *msg_carposition;
    last_pos_time_ = ros::Time::now();

    float theta = car_position_.yaw;
    float cos_val = cosf(theta);
    float sin_val = sinf(theta);
    local_waypoints_.clear();
    local_waypoints_.reserve(last_local_path.point_num);
    for (int i = 0; i < last_local_path.point_num; i++)
    {
        waypoint waypoint;
        waypoint.global_x = last_local_path.x[i];
        waypoint.global_y = last_local_path.y[i];
        waypoint.global_yaw = last_local_path.heading[i];
        double dx = last_local_path.x[i] - car_position_.position_x;
        double dy = last_local_path.y[i] - car_position_.position_y;
        // 转换到车辆坐标系
        waypoint.local_x = dx * cos_val + dy * sin_val;
        waypoint.local_y = dx * (-sin_val) + dy * cos_val;
        waypoint.local_yaw = robot::geometry::NormalizeAngle(last_local_path.heading[i] - theta);
        waypoint.gear = last_local_path.gear[i];
        waypoint.speed = last_local_path.speed[i];
        local_waypoints_.push_back(waypoint);
    }
}""")
    add_para(doc, "每次收到定位就做两件大事：1) 更新 car_position_ 和 last_pos_time_"
                  "（后者是状态机判断“定位丢失”的依据，超过 1 秒没更新就停车）；"
                  "2) 把缓存的全局路径逐点做刚体变换转到车体坐标系：平移（dx, dy）后用车体 yaw 旋转，"
                  "朝向角差用 NormalizeAngle 归一化到 (-π, π]。"
                  "转换在定位回调里做而不是在控制循环里做，好处是控制循环拿到的 local_waypoints_ 永远是最新位姿下的结果，"
                  "且 20Hz 主循环内不必再做整段路径的三角运算。")
    add_para(doc, "锁的使用值得注意：函数开头先锁 local_waypoints_mutex_（保护输出），"
                  "再在小作用域里锁 local_path_mutex_ 拷贝路径原文后立即释放——锁嵌套顺序固定，避免死锁。")

    add_heading(doc, "5.2 localPathCallback —— 路径缓存", 2)
    add_code(doc, """void PurePursuit::localPathCallback(const util::LocalPathConstPtr &msg_planning_result)
{
    std::lock_guard<std::mutex> lock(local_path_mutex_);
    last_local_path_time_ = ros::Time::now();
    if (last_local_path_ != *msg_planning_result)
    {
        last_local_path_ = *msg_planning_result;
        ROS_INFO("五维路径更新");
    }
    else
    {
        ROS_INFO("五维路径未更新");
    }
}""")
    add_para(doc, "只负责缓存规划端（read_waypoints）下发的五维路径（x/y/heading/gear/speed 五个数组 + point_num），"
                  "并刷新 last_local_path_time_（状态机据此判断“路径丢失”，超过 3 秒停车）。"
                  "借助 util/LocalPath 自带的 operator!= 比较内容是否变化，只打日志不做额外处理。"
                  "注意：即使路径内容没变，接收时间也会刷新——也就是说“路径话题还活着”比“路径内容变了”更重要。")

    add_heading(doc, "5.3 stopSignalCallback / avoidstateCallback / OutBoundaryCallBack —— 安全信号", 2)
    add_code(doc, """void PurePursuit::stopSignalCallback(const std_msgs::BoolConstPtr &msg_stop_signal)
{
    if (msg_stop_signal->data) { stop_car_.store(true);  ROS_INFO("收到停车信号"); }
    else                       { stop_car_.store(false); }
}
void PurePursuit::avoidstateCallback(const std_msgs::UInt8ConstPtr &avoid_msg)
{
    std::lock_guard<std::mutex> lock(avoid_state_mutex_);
    avoid_state_ = avoid_msg->data;
}
void PurePursuit::OutBoundaryCallBack(const std_msgs::BoolConstPtr &outboundary_msg)
{
    std::lock_guard<std::mutex> lock(outboundary_mutex_);
    outboundary_flag_ = outboundary_msg->data;
}""")
    add_para(doc, "三个回调都只是把外部状态原样落库：/mower/stop_car 来自 task 任务调度节点；"
                  "/lawn_mower/avoid_state 来自避障模块，取值含义为 0 无障碍、1（代码未特殊处理）、"
                  "2 普通预警（低速）、3 紧急预警（停车）、4 紧急预警（倒车）；"
                  "/lawn_mower/out_of_bounds 表示是否超出电子边界，越界同样触发低速。"
                  "真正的处理动作全部集中在状态机里做（见 7.1 节），回调保持“只记录、不决策”，"
                  "这是本包清晰的分层设计。")

    add_heading(doc, "5.4 signalCallback —— 云端/上位机指令", 2)
    add_code(doc, """void PurePursuit::signalCallback(const std_msgs::StringConstPtr &msg_signal)
{
    std::lock_guard<std::mutex> lock(mover_mutex_);
    try {
        int height = std::stoi(msg_signal->data);
        if (height >= 2 && height <= 11) {
            mower_height_cfg_ = height;
            ROS_INFO("mower_height = %d", mower_height_cfg_);
        }
    } catch (const std::exception &) {
        // 非数字指令（open/close/stop 等），跳过高度设置
    }

    if (msg_signal->data == "open")  { mover_bool_cfg_ = 1; }
    if (msg_signal->data == "close") { mover_bool_cfg_ = 0; }
    if (msg_signal->data == "stop" || msg_signal->data == "mowing_finished" || msg_signal->data == "reset")
    {
        std::lock_guard<std::mutex> lock1(avoid_state_mutex_);
        std::lock_guard<std::mutex> lock5(outboundary_mutex_);
        std::lock_guard<std::mutex> lock2(local_path_mutex_);
        std::lock_guard<std::mutex> lock3(mover_mutex_);
        std::lock_guard<std::mutex> lock4(turn_completed_mutex_);
        low_speed_flag_.store(false);
        stop_car_.store(false);
        avoid_state_ = 0;
        outboundary_flag_ = 0;
        last_local_path_.point_num = 0;
        turn_count_ = 0;
        follow_count_ = 0;
        mover_bool_cfg_ = 0;
        mower_height_cfg_ = 0;
        turn_completed_points_.clear();
        ROS_INFO("Pure Pursuit node reset, msg is %s", msg_signal->data.c_str());
    }
}""")
    add_para(doc, "/signal 是一条“复用”的字符串指令通道（来自云端经 cloud_bridge 或 task 转发），"
                  "承载三类语义：1) 纯数字字符串（2~11）表示割盘高度档位，std::stoi 解析，"
                  "非数字会抛异常被 catch 吞掉——用异常做字符串分类，写法取巧但有效；"
                  "2) \"open\"/\"close\" 控制割盘电机开关；"
                  "3) \"stop\"/\"mowing_finished\"/\"reset\" 触发节点软复位：清所有安全标志、清空路径"
                  "（point_num 置 0，等规划端重新下发）、清转弯记录、关割盘。")
    add_para(doc, "复位分支有一个细节：函数开头已持有 mover_mutex_，复位块里 lock3 又锁了一次"
                  "mover_mutex_——std::mutex 不可重入，这理论上是未定义行为（多数实现下会死锁）。"
                  "因为五个锁在同一作用域内按 lock_guard 构造顺序加锁、析构顺序解锁，"
                  "其中 lock3 是重复加锁。这是代码中真实存在的一个隐患，阅读和维护时需要注意，"
                  "如需修复可把函数开头的锁改为小作用域或改用 recursive_mutex。")

    add_heading(doc, "5.5 PC_or_RemoteCallback —— 控制来源", 2)
    add_code(doc, """void PurePursuit::PC_or_RemoteCallback(const std_msgs::UInt8ConstPtr &PC_or_Remote_msg)
{
    if (PC_or_Remote_msg->data == 0)      { PC_or_Remote_= 0; ROS_INFO("Pure Pursuit node: PC control"); }
    else if (PC_or_Remote_msg->data == 1) { PC_or_Remote_= 1; ROS_INFO("Pure Pursuit node: Remote control"); }
    else if (PC_or_Remote_msg->data == 2) { PC_or_Remote_= 2; ROS_INFO("Pure Pursuit node: Auto control"); }
}""")
    add_para(doc, "记录当前控制权归属：0=PC（上位机/自主控制）、1=遥控器手动接管、2=Auto。"
                  "状态机里只要该值非 0（遥控或 Auto 模式）就强制 Stop——即本节点只在 PC 自主模式下输出控制量，"
                  "遥控器随时能夺走控制权，保证人工优先。")

    add_heading(doc, "5.6 turnCompletedCallback —— 转弯完成查询服务", 2)
    add_code(doc, """bool PurePursuit::turnCompletedCallback(pure_pursuit::TurnCompleted::Request &req,
                                        pure_pursuit::TurnCompleted::Response &res)
{
    std::lock_guard<std::mutex> lock(turn_completed_mutex_);
    auto it = std::find_if(turn_completed_points_.begin(), turn_completed_points_.end(),
            [&](const waypoint& pt)
            {
                return fabs(req.x - pt.global_x) < 1e-4f &&
                       fabs(req.y - pt.global_y) < 1e-4f;
            });
    if (it != turn_completed_points_.end()) { res.turn_finish = true;  return true; }
    else                                    { res.turn_finish = false; return true; }
}""")
    add_para(doc, "规划端 read_waypoints 在裁剪路径前调用 /turn_completed 服务，询问“全局坐标 (x,y) 处"
                  "的掉头点是否已经转完”。本函数在 turn_completed_points_ 队列里按坐标近似匹配"
                  "（1e-4 容差，约 0.1mm）查找，找到则回复 turn_finish=true。"
                  "该队列由 Turn 状态在转弯完成时写入，最多保留最近 3 个（见 7.3 节）。"
                  "返回值恒为 true：服务调用本身总是成功的，业务结果通过 res.turn_finish 表达。")


# ==================== 第6章 核心算法函数 ====================

def chapter_algorithm(doc):
    add_heading(doc, "6. 核心算法函数", 1)

    add_heading(doc, "6.1 findNearestPoint —— 找最近路径点", 2)
    add_code(doc, """int PurePursuit::findNearestPoint(const std::vector<waypoint> &local_waypoints)
{
    float min_distance = std::numeric_limits<float>::max();
    int nearest_waypoint_index = -1;
    for (int i = 0; i < local_waypoints.size(); i++)
    {
        float distance = hypot(local_waypoints[i].local_x, local_waypoints[i].local_y);
        if (distance < min_distance)
        {
            min_distance = distance;
            nearest_waypoint_index = i;
        }
    }
    ...
    return nearest_waypoint_index;
}""")
    add_para(doc, "在车体坐标系下线性扫描全部路径点，找离原点（即车辆当前位置）最近的点，返回其索引。"
                  "车体坐标系下点距直接用 hypot(local_x, local_y) 计算，无需开方以外的处理。"
                  "路径为空时返回 -1，状态机据此停车。找最近点是纯跟踪的第一步：从最近点开始向前找预瞄点，"
                  "避免跟踪到车辆后方的路径。")

    add_heading(doc, "6.2 calculateLookaheadPoint —— 计算预瞄点", 2)
    add_code(doc, """waypoint PurePursuit::calculateLookaheadPoint(const std::vector<waypoint> &local_waypoints,
                                              const int &nearest_waypoint_index,
                                              const float &v_expect)
{
    waypoint lookahead_waypoint;
    lookahead_waypoint.gear = -1;
    float lookahead_distance = min_lookahead_distance_ + v_expect * 0.7f;
    int point_num = std::ceil(lookahead_distance / 0.25f);  // 中间点数量
    int sum_points_num = nearest_waypoint_index + point_num;
    if (sum_points_num >= local_waypoints.size())  // 防止越界
        sum_points_num = local_waypoints.size() - 1;

    lookahead_waypoint = local_waypoints[sum_points_num];
    for (int i = nearest_waypoint_index; i <= sum_points_num; i++)
    {
        if (local_waypoints[i].gear == 2)
        {
            // 如果中间有转向点，则转向点就是预瞄点
            lookahead_waypoint = local_waypoints[i];
            break;
        }
    }
    return lookahead_waypoint;
}""")
    add_para(doc, "预瞄距离 = 基础预瞄距离（1.5m）+ 0.7 × 期望速度——速度越快看得越远，"
                  "这是纯跟踪的经典调参手法，能兼顾低速精度与高速平滑。"
                  "由于路径点间距固定为 0.25m，预瞄距离除以 0.25 向上取整得到间隔点数，"
                  "从最近点往后数这么多点即为预瞄点，并对索引做了越界钳制。")
    add_para(doc, "关键的特殊规则：如果最近点到预瞄点之间夹着 gear==2 的掉头转弯点，"
                  "则优先把该转弯点作为预瞄点。这保证车辆遇到掉头段时先精确开到转弯点再原地转向，"
                  "而不是按圆弧提前切过去——割草机要求覆盖作业不重不漏，掉头点必须走准。")

    add_heading(doc, "6.3 calculate_PurePursuit —— 纯跟踪控制律", 2)
    add_code(doc, """Twist PurePursuit::calculate_PurePursuit(const float &v_expect, const waypoint &lookahead_point)
{
    Twist cmd;
    float linear_vel = v_expect;
    // 前瞄距离的平方
    float ld2 = lookahead_point.local_x * lookahead_point.local_x
              + lookahead_point.local_y * lookahead_point.local_y;
    if (ld2 < 1e-4) { cmd.linear = 0.0; cmd.angular = 0.0; return cmd; }  // 防止除零
    // 如果接近预瞄点则减速
    if (ld2 < 0.25 * 0.25)
        linear_vel = std::min(linear_vel, 0.1f);

    // Pure Pursuit 曲率
    float curvature = 2.0 * lookahead_point.local_y / ld2;
    if (fabs(curvature) < 1e-4) { cmd.linear = linear_vel; cmd.angular = 0; return cmd; }

    const float radius = std::fabs(1.0 / curvature);
    if (radius < regulated_min_radius_)
        cmd.linear = linear_vel * (radius / regulated_min_radius_);
    else
        cmd.linear = linear_vel;

    cmd.linear = std::max(cmd.linear, static_cast<double>(v_min_)); // 最小速度限制
    if (cmd.linear * curvature >= 0)
        cmd.angular = std::min(cmd.linear * curvature, static_cast<double>(w_max_));
    else
        cmd.angular = std::max(cmd.linear * curvature, static_cast<double>(-w_max_));

    // 对小的角速度进行放大，避免草地上转不动
    if (fabs(cmd.angular) < 0.09 && fabs(cmd.angular) > 0.04)
        cmd.angular = std::copysign(fabs(cmd.angular) + 0.05, cmd.angular);

    return cmd;
}""")
    add_para(doc, "这是整个包的数学核心。纯跟踪的几何原理：车辆后轴中心沿圆弧行驶到预瞄点，"
                  "该圆弧曲率 κ = 2·y / Ld²，其中 y 是预瞄点在车体坐标系下的横向偏移，Ld 是预瞄距离。"
                  "代码里的 local_y 即 y，ld2 即 Ld²，curvature 即 κ。")
    add_para(doc, "围绕这条控制律做了五层工程化处理：")
    add_para(doc, "1) 防除零：预瞄点几乎与车体重合（Ld² < 1e-4）时直接输出零速度；")
    add_para(doc, "2) 近点减速：距预瞄点小于 0.25m 时线速度压到 0.1m/s，精确停靠转弯点；")
    add_para(doc, "3) 曲率限速：转弯半径小于 regulated_min_radius_（1.8m）时按半径比例压低线速度，"
                  "使角速度 ω = v·κ 不致过大，防止履带打滑侧移；")
    add_para(doc, "4) 双向钳制：线速度不低于 v_min_（0.08m/s，防蠕动卡死），角速度不超过 ±w_max_（0.8rad/s）；"
                  "符号判断 cmd.linear*curvature >= 0 保证倒车时角速度方向也正确；")
    add_para(doc, "5) 小角速度死区补偿：|ω| 在 (0.04, 0.09) 区间时加 0.05——草地摩擦力大，"
                  "太小的角速度指令根本转不动，这一手是实车调出来的经验值。")

    add_heading(doc, "6.4 calculate_rotate_vel —— 原地转向速度", 2)
    add_code(doc, """float PurePursuit::calculate_rotate_vel(const float &yaw)
{
    float cur_yaw = yaw;
    // 如果角度差在180度附近就取180度，防止正负左右横跳
    if (fabs(fabs(yaw) - M_PI) < 5.0 * M_PI / 180.0)
        cur_yaw = M_PI;
    float angular_vel;
    if (fabs(cur_yaw) > M_PI_2/2)       angular_vel = cur_yaw > 0 ? 0.5 : -0.5;
    else if (fabs(cur_yaw) > M_PI_2/6)  angular_vel = cur_yaw > 0 ? 0.4 : -0.4;
    else                                angular_vel = cur_yaw > 0 ? 0.3 : -0.3;
    return angular_vel;
}""")
    add_para(doc, "Turn 状态下按与目标朝向的角度差分段给定旋转角速度：>45° 用 0.5 rad/s 快转，"
                  "15°~45° 用 0.4 rad/s，<15° 用 0.3 rad/s 慢速逼近——大角度快、小角度慢，"
                  "减少过冲。角度差在 ±180° 附近（差 5° 以内）时强制取 +π：因为 180° 处左右转等距，"
                  "角度噪声会让符号在正负间抖动、车辆左右横跳，固定方向后行为确定。"
                  "最终转向完成的判据（|角度差| < 5°）在状态机里（见 7.3 节）。")

    add_heading(doc, "6.5 publishCommand —— 指令打包下发", 2)
    add_code(doc, """void PurePursuit::publishCommand(const Twist &cmd)
{
    mower_msgs::VehicleCmd cmd_msg;
    cmd_msg.header.stamp = ros::Time::now();
    cmd_msg.drive_value = static_cast<int>(cmd.linear * 10000); // 转化成cm/s
    // 单片机端角速度方向是左手坐标系，对转速除以10000，因此这里需乘10000
    cmd_msg.turn_value = static_cast<int>(cmd.angular * 10000);
    cmd_msg.turn_value = -cmd_msg.turn_value;
    cmd_msg.ad_control_enable = 1;
    if (fabs(cmd.linear) < 1e-4f && fabs(cmd.angular) < 1e-4f)
        cmd_msg.gear_model = cmd_msg.p_Gear;
    else
        cmd_msg.gear_model = cmd_msg.D_Gear;
    {
        std::lock_guard<std::mutex> lock(mover_mutex_);
        cmd_msg.mover_bool = mover_bool_cfg_;
        cmd_msg.mower_height = mower_height_cfg_;
    }
    if (stop_car_.load())
        cmd_msg.gear_model = cmd_msg.p_Gear;
    pub_command_.publish(cmd_msg);
    ...
}""")
    add_para(doc, "把 Twist 换算成底盘单片机协议量：线速度 ×10000 取整（对应 0.1mm/s 分辨率），"
                  "角速度 ×10000 后取负——因为单片机端是左手坐标系，与 ROS 右手坐标系转向相反。"
                  "ad_control_enable 恒置 1 表示自动驾驶控制使能。"
                  "速度为零时档位给 P（驻车），否则给 D（前进）；若 stop_car_ 标志置位，"
                  "无论算什么速度都强制 P 档——这是停车信号的最后一道保险，即使状态机还没来得及切状态，"
                  "这一帧指令也一定不会让车动。"
                  "割盘开关与高度随每条指令一起下发，底盘侧不需要单独维护割盘状态。")


# ==================== 第7章 状态机 ====================

def chapter_state_machine(doc):
    add_heading(doc, "7. state_machine_run —— 20Hz 主控状态机", 1)
    add_para(doc, "state_machine_run() 是节点的主循环：ros::Rate(20) 控制 20Hz 频率，"
                  "每轮先 ros::spinOnce() 处理回调，再依次做“安全检查 → 路径处理 → 状态执行”三段，"
                  "最后 loop_rate.sleep()。安全检查不满足时用 goto STATEMACHINE 跳过中间段直接进 switch，"
                  "这是本函数最显眼的结构特征——goto 只向后跳转到同一函数内的标号，逻辑上等价于 "
                  "if-else 链，但让“任一条件失败立即落入状态执行”的流程一目了然。")

    add_heading(doc, "7.1 安全检查链（状态切换的优先级）", 2)
    add_code(doc, """    low_speed_flag_.store(false);
    if (ros::Time::now() - last_pos_time_ > ros::Duration(1.0))
    { ROS_WARN("车辆位置信息丢失"); running_state_.store(RunStateValue::Stop); goto STATEMACHINE; }

    {
        std::lock_guard<std::mutex> lock1(avoid_state_mutex_);
        std::lock_guard<std::mutex> lock2(outboundary_mutex_);
        if (avoid_state_ == 4)
        { ROS_WARN("紧急预警，需倒车"); running_state_.store(RunStateValue::Reverse); goto STATEMACHINE; }
        if (avoid_state_ == 3 || PC_or_Remote_ == 1 || PC_or_Remote_ == 2 )
        { ROS_WARN("紧急预警，需停车"); running_state_.store(RunStateValue::Stop); goto STATEMACHINE; }
        if (avoid_state_ == 2 || outboundary_flag_ == 1)
        { ROS_WARN("有较远障碍物，普通预警，需低速"); low_speed_flag_.store(true); }
    }

    if (ros::Time::now() - last_local_path_time_ > ros::Duration(3.0))
    { ROS_WARN("五维路径信息丢失"); running_state_.store(RunStateValue::Stop); goto STATEMACHINE; }
    if (stop_car_.load())
    { running_state_.store(RunStateValue::Stop); goto STATEMACHINE; }""")
    add_para(doc, "按优先级从高到低排列的六道闸：")
    add_table(doc,
        ["优先级", "条件", "动作", "说明"],
        [
            ["1", "定位超时 > 1s", "→ Stop", "定位都没了，一切免谈"],
            ["2", "avoid_state_ == 4", "→ Reverse", "最高级别避障：先倒车 0.15m/s 一个控制周期（Reverse 态执行一次后立即转 Stop，见 7.3）"],
            ["3", "avoid_state_ == 3 或遥控/Auto 接管", "→ Stop", "紧急障碍或人工接管"],
            ["4", "avoid_state_ == 2 或越界", "置低速标志", "不停车，期望速度降为 low_speed_（0.3m/s）；注意该标志每轮开头先清 false，是“本轮有效”的瞬时量"],
            ["5", "路径超时 > 3s", "→ Stop", "规划端失联"],
            ["6", "stop_car_ 标志", "→ Stop", "task 节点下发的任务级停车"],
        ])

    add_heading(doc, "7.2 路径处理段", 2)
    add_para(doc, "通过安全检查后，主循环把 local_waypoints_ 拷到局部变量，然后按顺序做：")
    add_para(doc, "1) Turn 态特殊处理：若当前正在转弯，只更新预瞄点的 local_yaw（因为车在转，"
                  "目标角度差实时变化），在最新路径里按全局坐标找回原预瞄点；找不到说明该转弯点已被规划端裁掉"
                  "（转弯完成确认后），直接回落 Follow 态。此分支随后 goto STATEMACHINE 跳过找点。")
    add_para(doc, "2) findNearestPoint 找最近点，失败（-1）则 Stop。")
    add_code(doc, """    if (running_state_.load() == RunStateValue::Follow && local_waypoints[nearest_waypoint_index].gear == 2)
    {
        std::lock_guard<std::mutex> lock(turn_completed_mutex_);
        auto it = std::find_if(turn_completed_points_.begin(), turn_completed_points_.end(), ...);
        if (it != turn_completed_points_.end())
        {
            if (nearest_waypoint_index == local_waypoints.size() - 1)
            { ROS_INFO("小车已到达路径终点，等待新路径......"); running_state_.store(RunStateValue::Stop); goto STATEMACHINE; }
            else
            { ROS_INFO("... 跳过转弯点"); nearest_waypoint_index++; }
        }
    }""")
    add_para(doc, "3) 已完成转弯点去重：如果最近点是个掉头点且它已在“已完成”队列里，说明这个掉头做过了——"
                  "若它是路径最后一个点就停车等新任务，否则最近点索引 +1 跳过它，防止车辆对已完成的掉头点"
                  "反复原地转圈。")
    add_para(doc, "4) 取期望速度：低速模式用 low_speed_；最近点是转弯点时固定 0.25m/s（慢速精确就位）；"
                  "否则用该路径点自带的 speed 字段（规划端按路段给的期望速度）。")
    add_para(doc, "5) calculateLookaheadPoint 算预瞄点（见 6.2 节）。")

    add_heading(doc, "7.3 四态执行逻辑", 2)
    add_para(doc, "switch(running_state_) 的四个分支：", bold=True)
    add_para(doc, "Stop：清两个计数器，发零速度指令，然后立即把状态置回 Follow——Stop 是“一次性”状态，"
                  "只发一帧停车；下一轮若危险条件仍在，安全检查链会再次把它打进 Stop，形成持续的停车输出。")
    add_para(doc, "Follow：正常跟线。先判断到达预瞄点（local_x < 0.07，即预瞄点已几乎在车身正后方/脚下）"
                  "且该点是转弯点（gear==2），则停车并切 Turn 态；否则调 calculate_PurePursuit 算速度发布。")
    add_code(doc, """        if (lookahead_waypoint_.global_x == last_lookahead_waypoint_.global_x
            && lookahead_waypoint_.global_y == last_lookahead_waypoint_.global_y
            && hypot(last_car_position_.position_x - car_position_.position_x,
                     last_car_position_.position_y - car_position_.position_y) < 0.01
            && abs(last_lookahead_waypoint_.local_yaw - lookahead_waypoint_.local_yaw) < 0.01)
        {
            follow_count_++;
            if (follow_count_ > 300)
            {
                uint forward_count = 10;
                while (forward_count--)
                {
                    twist_cmd.linear = 1.0;
                    twist_cmd.angular = 0.0;
                    publishCommand(twist_cmd);
                    loop_rate.sleep();
                }
                follow_count_ = 0;
                break;
            }
        }
        else { follow_count_ = 0; }""")
    add_para(doc, "Follow 态内嵌“卡死脱困”：如果预瞄点没变、车身位置 20 帧内移动不到 1cm、角度差也没变，"
                  "follow_count_ 累加；超过 300 帧（15 秒）判定卡住，执行 10 帧（0.5 秒）1.0m/s 的猛冲前进。"
                  "草地打滑或被草堆顶住时用这个冲量脱困，冲完计数清零重新观察。")
    add_para(doc, "Turn：原地转向。角度差 |local_yaw| < 5°（M_PI_4/9）视为转弯完成：若预瞄点是转弯点则"
                  "记入 turn_completed_points_（队列超 3 个弹队首，只留最近 3 个供 /turn_completed 服务查询），"
                  "发零速、回 Follow 态。未完成时若 turn_count_ > 200（10 秒还没转过去，可能被草堆顶住），"
                  "先执行 10 帧 0.5m/s 后退脱困，再按 calculate_rotate_vel 给旋转速度。")
    add_para(doc, "Reverse：紧急倒车。发一帧 -0.15m/s 后退，清计数器，然后立即置回 Stop；"
                  "下一轮若 avoid_state_ 仍为 4 会再次进入 Reverse——效果是以 20Hz 断续后退，"
                  "一旦避障模块解除最高级别预警，车立刻停住，不会盲目倒远。")
    add_para(doc, "default：状态值异常（理论上不会发生）时打错误日志并发零速，相当于看门狗。")

    add_heading(doc, "7.4 状态转换图", 2)
    add_code(doc, """                    +------------------ 安全检查链（每轮最高优先） ------------------+
                    |  定位超时/避障3级/遥控接管/stop_car → Stop    避障4级 → Reverse   |
                    v                                                                 |
   +--------+   到达转弯预瞄点   +--------+   |角度差|<5°   +--------+                  |
   |  Stop  | -----------------> | Follow | -------------> |  Turn  |                  |
   | (发1帧 | <----------------- | (跟线) | <------------- | (原地转)|                  |
   |  零速) |   发完即回Follow   +--------+   转弯完成记入   +--------+                  |
   +--------+        ^            |  |         完成队列         |  ^                   |
        ^            |            |  |                          |  |                   |
        |            |   卡住15s  |  | 找不到预瞄点(已被裁掉)    |  | 10s未转完          |
        +------------+            |  +--------------------------+  | 后退0.5s脱困       |
        |                         v                                 v                   |
        |                    冲1m/s×0.5s 脱困                 后退-0.5m/s×0.5s          |
        |                                                                            |
   +----------+    发1帧-0.15m/s后退即回Stop                                          |
   | Reverse  | ---------------------------------------------------------------------+
   +----------+""")


# ==================== 第8章 通信接口 ====================

def chapter_interfaces(doc):
    add_heading(doc, "8. 通信接口汇总", 1)

    add_heading(doc, "8.1 订阅话题", 2)
    add_table(doc,
        ["话题", "消息类型", "来源", "用途"],
        [
            ["/Mower/position", "util/Position", "定位模块（Mower 工作空间）", "车辆实时位姿；驱动路径坐标转换；超时 1s 停车"],
            ["/lawn_mower/global_path", "util/LocalPath", "规划端 read_waypoints", "五维局部路径（x/y/heading/gear/speed）；超时 3s 停车"],
            ["/mower/stop_car", "std_msgs/Bool", "task 任务调度节点", "任务级停车/继续"],
            ["/lawn_mower/avoid_state", "std_msgs/UInt8", "避障模块", "避障等级：2 低速、3 停车、4 倒车"],
            ["/lawn_mower/out_of_bounds", "std_msgs/Bool", "避障/边界模块", "越界则低速"],
            ["/signal", "std_msgs/String", "cloud_bridge / task 转发", "割盘高度（2~11）、open/close、stop/mowing_finished/reset 软复位"],
            ["/PC_or_Remote", "std_msgs/UInt8", "udp_com_main（底盘状态转发）", "控制来源：0 PC 自主、1 遥控、2 Auto；非 0 停车"],
        ])

    add_heading(doc, "8.2 发布话题", 2)
    add_table(doc,
        ["话题", "消息类型", "去向", "说明"],
        [
            ["/vehicle/cmd", "mower_msgs/VehicleCmd", "agv_can_bridge → CAN → 底盘单片机",
             "drive_value/turn_value（×10000 整型）、gear_model（P/D）、ad_control_enable、mover_bool、mower_height"],
        ])

    add_heading(doc, "8.3 服务", 2)
    add_table(doc,
        ["服务", "类型", "调用方", "说明"],
        [
            ["/turn_completed", "pure_pursuit/TurnCompleted", "规划端 read_waypoints",
             "查询某个全局坐标的掉头点是否已转弯完成，用于安全裁剪已走路径"],
        ])

    add_heading(doc, "8.4 参数", 2)
    add_para(doc, "5 个私有参数已在 2.1 节列出（v_min、w_max、min_lookahead_distance、"
                  "regulated_min_radius、low_speed），均由 launch 注入，运行期不动态重读。")


# ==================== 第9章 总结 ====================

def chapter_summary(doc):
    add_heading(doc, "9. 总结：在整车系统中的角色", 1)
    add_para(doc, "pure_pursuit 是履带割草机车端控制链路的“最后一公里”：", bold=True)
    add_code(doc, """  云平台 ──MQTT──> cloud_bridge ──/signal 等──> task（任务调度）
                                                    │ 路径/停车
  定位模块 ──/Mower/position──┐                     v
  规划端 ──/lawn_mower/global_path──> pure_pursuit ──/vehicle/cmd──> agv_can_bridge
  避障模块 ──avoid_state/out_of_bounds──>  （本包）                    │ CAN
  底盘状态 ──udp_com_main ──/PC_or_Remote──>                          v
                                                              底盘单片机（执行）""")
    add_para(doc, "它的上游是定位、规划、避障、任务调度、云端指令五路信息，下游只有一条 /vehicle/cmd。"
                  "设计上体现了三个鲜明特点：")
    add_para(doc, "1) 安全冗余层层设防：定位/路径超时、三级避障分级处置、遥控优先、stop 标志、"
                  "P 档兜底、边界低速——任何一环异常都有对应的降级动作，且决策全部集中在状态机一处，便于审查。")
    add_para(doc, "2) 算法为工艺服务：标准纯跟踪控制律之外，掉头点优先预瞄、转弯点精确就位后原地转向、"
                  "小角速度死区补偿、卡死猛冲/后退脱困等大量规则，都是“割草覆盖作业 + 草地低附着路面”"
                  "这一具体场景逼出来的工程化改造。")
    add_para(doc, "3) 与规划端双向配合：不只是被动跟线，还通过 turn_completed_points_ 队列和 "
                  "/turn_completed 服务把“哪些掉头做完了”反馈给规划端，让路径裁剪有依据，"
                  "形成规划-控制的闭环。")
    add_para(doc, "已识别的代码瑕疵（供维护参考）：signalCallback 复位分支对 mover_mutex_ 重复加锁"
                  "（std::mutex 不可重入，存在死锁风险，见 5.4 节）；PC_or_Remote_ 未初始化即参与比较"
                  "（构造时未赋初值，首次回调前是不确定值）；geometry.h 中 Vec2f/Vec2d 均定义为 "
                  "Vec2<double>（见 3.2 节）；yaml-cpp 链接为历史遗留（见 2.2 节）。"
                  "这些问题不影响当前主流程运行，但建议排期清理。")


# ==================== 组装 ====================

def main():
    doc = Document()
    title = doc.add_heading("", level=0)
    r = title.add_run("pure_pursuit 包代码详解")
    set_run_font(r, bold=True, size=Pt(22))
    r.font.color.rgb = RGBColor(0, 0, 0)
    title.alignment = WD_ALIGN_PARAGRAPH.CENTER
    add_para(doc, "—— 履带割草机车端 crawler_control 工作空间 · 纯跟踪运动控制节点").alignment = \
        WD_ALIGN_PARAGRAPH.CENTER

    chapter_overview(doc)
    chapter_launch_build(doc)
    chapter_geometry(doc)
    chapter_main_and_header(doc)
    chapter_callbacks(doc)
    chapter_algorithm(doc)
    chapter_state_machine(doc)
    chapter_interfaces(doc)
    chapter_summary(doc)

    os.makedirs("代码详解", exist_ok=True)
    path = os.path.join("代码详解", "pure_pursuit代码详解.docx")
    doc.save(path)
    print("已生成:", path)


if __name__ == "__main__":
    main()
