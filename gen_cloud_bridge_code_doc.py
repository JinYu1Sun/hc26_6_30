# -*- coding: utf-8 -*-
"""生成《cloud_bridge代码详解》Word 文档
用法: python gen_cloud_bridge_code_doc.py
产物: cloud_bridge代码详解.docx（当前目录）
"""
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
    add_para(doc, "cloud_bridge 是割草机车端的 ROS1 功能包，作用是把车端与云平台连接起来。"
                  "整个包只有一个可执行节点 cloud_bridge_node，源码共两个 C++ 文件、一个头文件：")
    for line in [
        "crawler_control/src/cloud_bridge/",
        "├── include/cloud_bridge/mqtt_client.h   MQTT 客户端封装（声明）",
        "├── src/mqtt_client.cpp                 MQTT 客户端封装（实现）",
        "├── src/cloud_bridge_node.cpp           主节点：指令处理 + 状态上报 + SRT 推流",
        "├── launch/cloud_bridge.launch          启动参数",
        "├── CMakeLists.txt / package.xml        构建配置",
    ]:
        add_code(doc, line)
    add_para(doc, "整体数据流有三条：", bold=True)
    add_para(doc, "1) MQTT 下行指令：云平台把 JSON 指令发到 MQTT broker 的 mower/{device_id}/cmd/xxx 主题，"
                  "MqttClient 收到后回调 CloudBridgeNode::onMqttMessage，按主题分发到 move/blade/task/video "
                  "四个处理函数。遥控类指令转成 ROS 消息发到 /mower/manual_driving_cmd；任务类指令转成字符串发到 /signal，"
                  "由 task_node / pure_pursuit 等原有节点消费。")
    add_para(doc, "2) ROS 状态 → MQTT 上行：节点订阅 /Mower/position、/vehicle/status、左右轮速、刀盘高度等话题，"
                  "用定时器按固定频率（定位 2Hz、车辆状态 1Hz）把最新值拼成 JSON，通过 MQTT 发布到 "
                  "mower/{device_id}/state/xxx 主题。")
    add_para(doc, "3) 视频：订阅 /camera/image_rect 图像话题，经 cv_bridge 转成 OpenCV 图像并缩放后，"
                  "写入 ffmpeg 子进程的 stdin 管道；ffmpeg 把裸视频编码成 H.264，以 SRT caller 模式推到云平台。")
    add_para(doc, "线程模型：ROS 侧用 ros::AsyncSpinner(2) 开两个线程跑订阅/定时器回调；"
                  "paho MQTT 库内部有自己的线程调用 message_arrived；MqttClient 另有一个重连线程；"
                  "任务启动流程还会临时 detach 一个顺序发信号的线程。因此代码里大量使用 "
                  "std::mutex 和 std::atomic 保护跨线程共享数据，这是阅读本包时最需要注意的一点。")


# ==================== 第2章 mqtt_client.h ====================

def chapter_header(doc):
    add_heading(doc, "2. mqtt_client.h —— MQTT 客户端封装（声明）", 1)
    add_para(doc, "这个头文件定义了 MqttConfig 配置结构体和 MqttClient 类。"
                  "MqttClient 是对 Eclipse Paho MQTT C++ 库 mqtt::async_client 的一层薄封装，"
                  "对外只暴露 start/stop/publish/addSubscription/setMessageCallback 几个简单接口，"
                  "内部处理断线自动重连和重连后恢复订阅。")

    add_heading(doc, "2.1 头文件保护与包含", 2)
    add_code(doc, """#ifndef CLOUD_BRIDGE_MQTT_CLIENT_H
#define CLOUD_BRIDGE_MQTT_CLIENT_H

#include <mqtt/async_client.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>""")
    add_para(doc, "经典的 #ifndef/#define 头文件保护宏，防止同一翻译单元重复包含。"
                  "<mqtt/async_client.h> 是 Paho MQTT C++ 库的主头文件，提供 mqtt::async_client、"
                  "mqtt::callback、mqtt::connect_options 等。其余是 STL 头："
                  "atomic（原子变量）、functional（std::function）、mutex、thread、"
                  "utility（std::pair）、vector，对应下面类里用到的成员类型。")

    add_heading(doc, "2.2 MqttConfig 配置结构体", 2)
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
    add_para(doc, "纯数据聚合结构体，所有成员用 C++11 的类内初始值给出默认值，"
                  "调用方只改需要改的字段。keep_alive_interval 是 MQTT 保活秒数"
                  "（期间无报文时客户端自动发 PINGREQ）；connect_timeout 是 TCP/MQTT 连接超时秒数。"
                  "username 留空表示匿名连接，构造函数里会据此决定是否设置用户名密码。")

    add_heading(doc, "2.3 MqttClient 类声明：继承 paho 回调接口", 2)
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
      bool isConnected() const { return connected_.load(); }""")
    add_para(doc, "语法点1：class MqttClient : public virtual mqtt::callback —— 公有虚继承 paho 的回调接口类。"
                  "mqtt::callback 声明了 connection_lost / message_arrived / delivery_complete 三个虚函数，"
                  "paho 库的线程在事件发生时会回调这些函数；MqttClient 用 override 重写它们（见 2.5）。"
                  "用 virtual 继承是 paho 官方示例的写法，避免与 iaction_listener 等接口多继承时出现菱形继承问题。")
    add_para(doc, "语法点2：using MessageCallback = std::function<...> —— 类型别名，定义一个"
                  "“收到 (topic, payload) 就调用”的函数对象类型。std::function 可以装普通函数、lambda、"
                  "成员函数绑定等任何可调用对象，使用者（主节点）用 lambda 注入自己的处理逻辑，实现解耦。")
    add_para(doc, "语法点3：explicit 构造函数 —— 禁止 MqttConfig 到 MqttClient 的隐式转换，"
                  "防止写成 MqttClient c = config; 这种容易误用的代码。"
                  "~MqttClient() override 表示重写基类的虚析构函数（mqtt::callback 有虚析构）。")
    add_para(doc, "isConnected() 是头文件内联实现：connected_ 是 std::atomic<bool>，"
                  ".load() 做一次原子读，保证跨线程读到的是一致的值而不会读到写了一半的状态。")

    add_heading(doc, "2.4 发布与订阅接口", 2)
    add_code(doc, """      bool publish(const std::string& topic, const std::string& payload, int qos = 0);
      bool publish(const std::string& topic, const void* payload, size_t len, int qos = 0);

      void addSubscription(const std::string& topic, int qos = 1);
      void setMessageCallback(MessageCallback cb) { message_cb_ = std::move(cb); }""")
    add_para(doc, "publish 有两个重载：string 版本只是便利包装，内部调用 (void*, len) 版本；"
                  "二进制版本可以发任意字节。默认参数 qos = 0（最多一次送达，适合高频状态上报）。"
                  "addSubscription 默认 qos = 1（至少一次，适合下行指令）。"
                  "订阅先记入内部列表，断线重连后统一恢复，这是本封装的核心价值。")
    add_para(doc, "setMessageCallback 内联实现，std::move(cb) 把传入的函数对象移动进成员，"
                  "避免一次不必要的拷贝（std::function 内部可能有堆分配）。")

    add_heading(doc, "2.5 私有回调与成员变量", 2)
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
    add_para(doc, "三个 override 就是 paho 要求的回调：连接丢失、消息到达、投递完成。"
                  "delivery_complete 用 /*token*/ 注释掉形参名，避免“未使用参数”编译警告；"
                  "空函数体 {} 表示我们不关心 QoS1 的投递回执。")
    add_para(doc, "语法点：mqtt::const_message_ptr 是 paho 定义的共享指针类型"
                  "（内部是 std::shared_ptr<const message> 的别名），消息对象由库管理生命周期。")
    add_para(doc, "三个 std::atomic<bool>：connected_（当前是否在线）、stopping_（用户要求停止）、"
                  "reconnect_needed_（需要重连的标志）。它们会被 paho 回调线程、重连线程、"
                  "ROS 线程同时读写，用原子类型保证可见性和免锁安全。{false} 是类内默认初始化。")
    add_para(doc, "subscriptions_ 是 (topic, qos) 对的列表，addSubscription 和 resubscribeAll 都会访问它，"
                  "又可能跑在不同线程，所以配了一把 sub_mutex_。注意 std::mutex 放在成员里之后，"
                  "这个类就不可拷贝了（mutex 不可拷贝），本类也确实不需要拷贝语义。")


# ==================== 第3章 mqtt_client.cpp ====================

def chapter_client_cpp(doc):
    add_heading(doc, "3. mqtt_client.cpp —— MQTT 客户端实现", 1)

    add_heading(doc, "3.1 构造函数：成员初始化列表与连接选项", 2)
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
    add_para(doc, "语法点：冒号后面是成员初始化列表。config_ 直接拷贝；client_ 是 paho 的异步客户端，"
                  "它没有默认构造函数，必须在初始化列表里用“服务器URI + 客户端ID”构造，"
                  "URI 形如 tcp://127.0.0.1:1883（明文 TCP，无 TLS）。std::to_string 把 int 端口号转成字符串拼接。")
    add_para(doc, "函数体里配置连接选项：clean_session=true 表示 broker 不保留离线消息和旧订阅，"
                  "每次连接都是干净会话——配合断线后 resubscribeAll() 重新订阅即可；"
                  "keep_alive 用秒数直接传；connect_timeout 用 std::chrono::seconds 显式包装成时间段类型，"
                  "这是 paho 的类型安全接口。用户名非空才设置用户名密码，否则走匿名连接。")
    add_para(doc, "client_.set_callback(*this) 是关键一行：把自己（已经 override 了三个回调函数）"
                  "注册给 paho 客户端，之后连接丢失、消息到达等事件都会回到本对象。")

    add_heading(doc, "3.2 start()：首次连接", 2)
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
    add_para(doc, "第一步先启动重连线程：std::thread(成员函数指针, this) 是在新线程里跑成员函数的标准写法。"
                  "此时 reconnect_needed_ 还是 false，重连线程会空转等待（见 3.6）。")
    add_para(doc, "client_.connect() 返回的是异步 token（mqtt::token_ptr），->wait() 阻塞等待连接结果，"
                  "所以 start() 是“阻塞式首次连接”。失败时 paho 抛 mqtt::exception，"
                  "catch 后不报错退出，而是置 reconnect_needed_=true，把重试任务交给重连线程后台慢慢试，"
                  "并返回 false 让调用方知道首发失败。这样 broker 没起时节点仍能正常跑，等 broker 上线后自动连上。")
    add_para(doc, "connected_ = true 是对 std::atomic 的写操作（等价于 .store(true)），原子赋值。")

    add_heading(doc, "3.3 stop() 与析构", 2)
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
    add_para(doc, "析构函数只调 stop()，集中释放逻辑。stop 的顺序很重要：先置 stopping_=true，"
                  "让重连线程下一轮循环就退出；joinable() 判断线程对象是否还关联着一个未回收的线程"
                  "（stop 可能被调用多次，比如析构前手动 stop 过一次，所以要先判断再 join，否则 join 会抛异常）；"
                  "join() 阻塞等重连线程结束，保证之后没有线程再碰 client_。")
    add_para(doc, "确认没有别的线程在用 client_ 后，才在线时调用 disconnect()->wait() 优雅断开。"
                  "断开也可能抛异常（比如刚断网），空 catch 吞掉即可——对象都要销毁了，断不开也无所谓。")

    add_heading(doc, "3.4 publish：两个重载", 2)
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
    add_para(doc, "string 版本用 payload.data()/size() 取出底层字符缓冲转发给二进制版本，避免逻辑重复。"
                  "发布前先原子检查 connected_，断线期间直接返回 false 丢弃——上行状态数据高频且有时效性，"
                  "丢了等下一帧即可，不值得缓存重发。publish 的第五个参数 false 是 retained 标志："
                  "不使用保留消息。整个调用不 ->wait()，纯异步发出，不阻塞 ROS 回调线程。")

    add_heading(doc, "3.5 addSubscription 与 resubscribeAll", 2)
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
    add_para(doc, "语法点1：内层花括号 {} 人为制造作用域，std::lock_guard 在作用域结束析构时自动解锁"
                  "（RAII 手法）。这样锁只覆盖对 vector 的修改，不包含后面的网络操作，减小临界区。")
    add_para(doc, "语法点2：emplace_back(topic, qos) 直接在 vector 尾部原地构造 pair，"
                  "比 push_back(std::make_pair(...)) 少一次移动。")
    add_para(doc, "设计思路：订阅信息先落进列表，这一步无论在线与否都做；如果当前在线就立即真正订阅，"
                  "->wait() 阻塞等 broker 确认。断线时只记录不订阅，等重连成功后由 resubscribeAll 统一补订。")
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
    add_para(doc, "范围 for 循环遍历 (topic, qos) 列表逐个补订阅。单个订阅失败只跳过，不中断其它订阅。"
                  "注意这里持锁期间做了阻塞网络操作，临界区偏大，但只在连接建立时执行一次，可接受。")

    add_heading(doc, "3.6 paho 回调与重连循环（指数退避）", 2)
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
    add_para(doc, "connection_lost 由 paho 线程在检测到断线时调用：标记离线，并置 reconnect_needed_ 唤醒重连。"
                  "检查 stopping_ 是为了在主动 stop 的过程中不再触发重连。cause 参数不用，注释掉名字避免警告。")
    add_para(doc, "message_arrived 把 paho 消息对象转成 (topic, payload) 字符串转发给使用方注入的回调。"
                  "先 if (message_cb_) 判断 std::function 是否为空（未设置时调用会抛 std::bad_function_call）。"
                  "注意：这个函数跑在 paho 的线程里，所以上层回调里访问共享数据必须加锁或用原子变量。")
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
    add_para(doc, "重连线程主循环：不需要重连时每 200ms 醒一次检查标志（轮询而不是条件变量，代码简单够用）。"
                  "需要重连时尝试 connect 并阻塞等结果；成功后清标志、把退避 delay 重置回 1 秒、补订阅。")
    add_para(doc, "指数退避：失败后睡 delay 秒，然后 delay 翻倍并用 std::min 封顶 30 秒，"
                  "即重试间隔为 1s、2s、4s、8s、16s、30s、30s……避免 broker 宕机时被高频重连请求冲击，"
                  "也避免日志刷屏。std::min(a,b) 取两者较小值，是限幅（clamp 上界）的惯用写法。")


# ==================== 第4章 cloud_bridge_node.cpp ====================

def chapter_node_streamer(doc):
    add_heading(doc, "4. cloud_bridge_node.cpp —— 主节点", 1)
    add_para(doc, "主文件包含两个类：SrtStreamer（用 fork/pipe 拉起 ffmpeg 做 SRT 视频推流）和 "
                  "CloudBridgeNode（MQTT 指令处理、ROS 话题收发、看门狗、JSON 状态上报），最后是 main 函数。")

    add_heading(doc, "4.1 头文件包含", 2)
    add_code(doc, """#include <ros/ros.h>
#include <std_msgs/Int16.h>
#include <std_msgs/String.h>
#include <std_msgs/UInt16.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/image_encodings.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

#include <mower_msgs/Manual_Driving_Cmd.h>
#include <mower_msgs/VehicleStatus.h>
#include <util/Position.h>

#include <nlohmann/json.hpp>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>""")
    add_para(doc, "分四组：1) ROS 与消息类型：ros/ros.h 是 ROS C++ 主头；std_msgs/sensor_msgs 是标准消息；"
                  "cv_bridge 负责 ROS 图像消息和 OpenCV cv::Mat 互转；opencv2 提供 resize 等图像处理。"
                  "2) 项目自定义消息：mower_msgs::Manual_Driving_Cmd（遥控指令）、VehicleStatus（车辆状态）、"
                  "util::Position（融合定位）。3) nlohmann/json 是现代 C++ JSON 库，一条语句完成解析/构造。"
                  "4) POSIX 系统头：signal.h（signal/SIGPIPE/SIGTERM）、sys/types.h（pid_t）、"
                  "sys/wait.h（waitpid）、unistd.h（fork/pipe/dup2/write/close），都是给 SrtStreamer 用的。")

    add_heading(doc, "4.2 VideoCfg 视频参数结构体", 2)
    add_code(doc, """struct VideoCfg
{
  bool enable = false;
  double fps = 5.0;
  int width = 640;
  int height = 480;
  int bitrate = 800;  // kbps
};""")
    add_para(doc, "推流参数聚合：默认关闭、5fps、640x480、800kbps H.264 码率。"
                  "类内初始值给出默认值，launch 参数和 cmd/video 指令都会更新这份配置。")

    add_heading(doc, "4.3 SrtStreamer::configure —— 参数变化时按需重启", 2)
    add_code(doc, """  void configure(const VideoCfg& cfg)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    const bool param_changed =
        cfg.fps != cfg_.fps || cfg.width != cfg_.width ||
        cfg.height != cfg_.height || cfg.bitrate != cfg_.bitrate;
    cfg_ = cfg;
    if (cfg_.enable)
    {
      if (child_pid_ <= 0 || param_changed)
      {
        stopLocked();
        startLocked();
      }
    }
    else
    {
      stopLocked();
    }
  }""")
    add_para(doc, "整个函数在锁内执行，因为它可能被 ROS 回调线程（cmd/video 指令）和构造函数先后调用，"
                  "与 push()（图像回调线程）竞争。先比较新旧参数算出 param_changed，再覆盖 cfg_。"
                  "逻辑：enable=true 时，如果 ffmpeg 还没起（child_pid_<=0）或参数变了，就先停后起（重启生效）；"
                  "enable=false 直接停。函数名带 Locked 后缀表示“调用前必须已持锁”，是常见的锁约定命名。")

    add_heading(doc, "4.4 SrtStreamer::push —— 写一帧给 ffmpeg", 2)
    add_code(doc, """  void push(const cv::Mat& bgr)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!cfg_.enable || pipe_fd_ < 0)
      return;
    cv::Mat cont = bgr.isContinuous() ? bgr : bgr.clone();
    const size_t len = cont.total() * cont.elemSize();
    if (::write(pipe_fd_, cont.data, len) < 0)
    {
      // ffmpeg 退出（如云端未监听），限频重启
      const double now = ros::Time::now().toSec();
      if (now - last_restart_time_ > 3.0)
      {
        last_restart_time_ = now;
        ROS_WARN("ffmpeg pipe broken, restarting SRT stream");
        stopLocked();
        startLocked();
      }
      else
      {
        stopLocked();
      }
    }
  }""")
    add_para(doc, "语法点：bgr.isContinuous() ? bgr : bgr.clone() 是条件运算符（三元表达式）。"
                  "cv::Mat 的像素内存可能不连续（比如是某大图的 ROI，行与行之间有间隔），"
                  "而 write 需要一块连续缓冲，所以不连续时 clone() 出一份连续拷贝，连续时直接用，零拷贝。")
    add_para(doc, "cont.total() 是像素总数（宽×高），elemSize() 是每像素字节数（bgr24 为 3），"
                  "相乘得到整帧字节数。::write 前面的 :: 明确调用全局命名空间的 POSIX write，防止名字被成员函数遮蔽。")
    add_para(doc, "写失败说明管道另一端（ffmpeg）已经退出，比如云端没开监听导致 ffmpeg 连接失败退出。"
                  "处理策略是限频重启：距上次重启超过 3 秒才真正重启，否则只停掉等下一帧再试，"
                  "避免云端长期不监听时每帧都 fork 一次 ffmpeg 拖垮系统。")

    add_heading(doc, "4.5 SrtStreamer::startLocked —— fork/pipe/dup2/execlp 拉起 ffmpeg", 2)
    add_code(doc, """    int fds[2];
    if (::pipe(fds) != 0)
    {
      ROS_ERROR("pipe() failed: %s", strerror(errno));
      return;
    }
    const std::string size =
        std::to_string(cfg_.width) + "x" + std::to_string(cfg_.height);
    const std::string fps = std::to_string(cfg_.fps);
    const std::string bitrate = std::to_string(cfg_.bitrate) + "k";

    const pid_t pid = ::fork();
    if (pid == 0)
    {
      // 子进程：stdin 接管读端，执行 ffmpeg
      ::setsid();
      ::dup2(fds[0], STDIN_FILENO);
      ::close(fds[0]);
      ::close(fds[1]);
      ::execlp("ffmpeg", "ffmpeg", "-loglevel", "error",
               "-f", "rawvideo", "-pix_fmt", "bgr24",
               "-s", size.c_str(), "-r", fps.c_str(), "-i", "pipe:0",
               "-c:v", "libx264", "-preset", "veryfast", "-tune", "zerolatency",
               "-b:v", bitrate.c_str(), "-pix_fmt", "yuv420p",
               "-f", "mpegts", target_.c_str(), (char*)nullptr);
      _exit(1);  // execlp 失败（未安装 ffmpeg 等）
    }
    ::close(fds[0]);
    if (pid < 0)
    {
      ::close(fds[1]);
      ROS_ERROR("fork() failed: %s", strerror(errno));
      return;
    }
    pipe_fd_ = fds[1];
    child_pid_ = pid;""")
    add_para(doc, "这是典型的 Unix “管道 + fork + exec”三件套：pipe(fds) 创建匿名管道，"
                  "fds[0] 读端、fds[1] 写端；fork() 把进程一分为二，子进程返回 0、父进程得到子进程 PID；"
                  "父子各自关闭用不到的一端。")
    add_para(doc, "子进程里：setsid() 脱离父进程会话成为新会话首进程，之后 kill 父进程不会误杀 ffmpeg；"
                  "dup2(fds[0], STDIN_FILENO) 把管道读端复制到文件描述符 0（标准输入），"
                  "这样 ffmpeg 从 stdin 读到的就是父进程写进管道的图像帧；随后关掉多余的 fd。"
                  "execlp 用 ffmpeg 程序替换子进程映像（p 表示按 PATH 搜索，l 表示参数逐个传递、"
                  "以 (char*)nullptr 结尾）。execlp 只有失败才会返回，此时 _exit(1) 立刻退出"
                  "（用 _exit 而非 exit，避免重复执行父进程的缓冲刷新和 atexit 钩子）。")
    add_para(doc, "ffmpeg 命令行含义：-f rawvideo -pix_fmt bgr24 -s 宽x高 -r 帧率 -i pipe:0 "
                  "表示从 stdin 读 BGR 裸视频；-c:v libx264 用 x264 编码 H.264，veryfast+zerolatency "
                  "是低延迟实时场景的标准preset/tune 组合；-b:v 800k 限码率；-pix_fmt yuv420p 转通用像素格式；"
                  "-f mpegts 用 MPEG-TS 封装后写到 srt://... URL，即以 SRT caller 模式主动推流。")
    add_para(doc, "父进程里：关掉读端，记下写端 pipe_fd_ 和子进程 PID 供 push()/stopLocked() 使用。"
                  "fork 失败时（pid<0）记得把写端也关掉，防止 fd 泄漏。")

    add_heading(doc, "4.6 SrtStreamer::stopLocked —— 关管道、杀子进程", 2)
    add_code(doc, """  void stopLocked()
  {
    if (pipe_fd_ >= 0)
    {
      ::close(pipe_fd_);  // ffmpeg 读到 EOF 自行退出
      pipe_fd_ = -1;
    }
    if (child_pid_ > 0)
    {
      ::kill(child_pid_, SIGTERM);
      ::waitpid(child_pid_, nullptr, 0);
      child_pid_ = -1;
    }
  }""")
    add_para(doc, "关闭写端后，ffmpeg 读 stdin 会得到 EOF，本来会自行退出；但为了保险再补一个 "
                  "SIGTERM 优雅终止信号。waitpid(pid, nullptr, 0) 阻塞等待子进程真正退出并回收它，"
                  "避免产生僵尸进程（zombie）。文件描述符和 PID 用完都复位成 -1，表示“无效”，"
                  "后面所有判断都靠 <0 / <=0 来区分状态。fd=-1、pid=-1 哨兵值 + 用完即复位，"
                  "是防止重复 close/double free 类 bug 的惯用手法。")


def chapter_node_main(doc):
    add_heading(doc, "4.7 CloudBridgeNode 构造函数", 2)
    add_code(doc, """  CloudBridgeNode() : pnh_("~")
  {
    loadParams();
    streamer_.setTarget(srt_target_);
    streamer_.configure(video_cfg_);  // launch 里 video_enable=true 时立即起流

    pub_manual_ = nh_.advertise<mower_msgs::Manual_Driving_Cmd>(
        "/mower/manual_driving_cmd", 1);
    pub_signal_ = nh_.advertise<std_msgs::String>("/signal", 10);

    sub_position_ = nh_.subscribe("/Mower/position", 1,
                                  &CloudBridgeNode::positionCb, this);
    ...
    sub_image_ = nh_.subscribe(image_topic_, 1, &CloudBridgeNode::imageCb, this);""")
    add_para(doc, "成员初始化列表里 pnh_(\"~\") 创建私有 NodeHandle：读参数时自动在节点命名空间下查找"
                  "（launch 里 <node> 内的 <param> 都属私有参数）。nh_ 用默认构造，即全局命名空间。")
    add_para(doc, "advertise<消息类型>(话题, 队列长度) 创建发布者；subscribe(话题, 队列, 成员函数指针, this) "
                  "创建订阅者，消息到达时由 spinner 线程调用对应回调。队列长度 1 表示只保留最新一帧，"
                  "对图像和状态这种“新数据覆盖旧数据”的场景能防止积压延迟。")
    add_code(doc, """    const std::string prefix = topic_prefix_ + "/" + device_id_;
    topic_cmd_move_ = prefix + "/cmd/move";
    ...
    mqtt_ = std::unique_ptr<MqttClient>(new MqttClient(mqtt_cfg_));
    mqtt_->setMessageCallback(
        [this](const std::string& topic, const std::string& payload) {
          onMqttMessage(topic, payload);
        });
    mqtt_->addSubscription(topic_cmd_move_, 1);
    ...
    if (!mqtt_->start())
      ROS_WARN("MQTT first connect failed, retrying in background");""")
    add_para(doc, "MQTT 主题按 mower/{device_id}/cmd/xxx、/state/xxx 规则拼出来，存进成员供后续比较使用。")
    add_para(doc, "语法点：std::unique_ptr<MqttClient> 是独占所有权的智能指针，对象析构时自动 delete，"
                  "不用手动管理内存（C++17 起更推荐 std::make_unique，这里 new 写法等价）。"
                  "setMessageCallback 传入的 [this](...) { ... } 是 lambda 表达式：捕获 this 指针，"
                  "把 paho 线程收到的消息转交给成员函数 onMqttMessage 处理。"
                  "注意它跑在 paho 线程里，所以 onMqttMessage 里访问的共享状态都要同步。")
    add_para(doc, "首次连接失败仅警告不退出——重连线程已在后台工作，broker 上线后会自动连上并补订阅。")
    add_code(doc, """    location_timer_ = nh_.createTimer(
        ros::Duration(1.0 / location_hz_), &CloudBridgeNode::locationTimerCb, this);
    status_timer_ = nh_.createTimer(
        ros::Duration(1.0 / status_hz_), &CloudBridgeNode::statusTimerCb, this);
    watchdog_timer_ = nh_.createTimer(
        ros::Duration(0.1), &CloudBridgeNode::watchdogTimerCb, this);""")
    add_para(doc, "createTimer 创建周期定时器：定位上报 1/2=0.5s 一次、车辆状态 1s 一次、"
                  "遥控看门狗 0.1s 一次。定时器回调同样由 spinner 线程执行。")

    add_heading(doc, "4.8 loadParams —— 读取 launch 参数", 2)
    add_code(doc, """    pnh_.param<std::string>("broker_host", mqtt_cfg_.host, "127.0.0.1");
    pnh_.param("broker_port", mqtt_cfg_.port, 1883);
    ...
    int mow_height = mow_height_.load();
    pnh_.param("mow_height", mow_height, 6);
    mow_height_ = mow_height;""")
    add_para(doc, "pnh_.param(参数名, 输出变量, 默认值)：参数服务器上没有该参数时用默认值，有则覆盖。"
                  "模板参数 <std::string> 在字符串类型时需要显式给出（否则 \"...\" 会被推成 const char*），"
                  "数值类型可自动推导。")
    add_para(doc, "最后三行处理 mow_height_ 这个 std::atomic<int>：param 无法直接写原子变量，"
                  "所以先用 load() 读出当前值到普通 int，读参数，再原子赋值回去。这是 atomic 与"
                  "“需要引用/地址的接口”对接时的标准过墙手法。")

    add_heading(doc, "4.9 onMqttMessage —— JSON 解析与指令分发", 2)
    add_code(doc, """  void onMqttMessage(const std::string& topic, const std::string& payload)
  {
    nlohmann::json j;
    try
    {
      j = nlohmann::json::parse(payload);
    }
    catch (const nlohmann::json::exception& e)
    {
      ROS_WARN("bad json on %s: %s", topic.c_str(), e.what());
      return;
    }

    try
    {
      if (topic == topic_cmd_move_)
        handleMoveCmd(j);
      else if (topic == topic_cmd_blade_)
        handleBladeCmd(j);
      else if (topic == topic_cmd_task_)
        handleTaskCmd(j);
      else if (topic == topic_cmd_video_)
        handleVideoCmd(j);
    }
    catch (const nlohmann::json::exception& e)
    {
      ROS_WARN("bad field on %s: %s", topic.c_str(), e.what());
    }
  }""")
    add_para(doc, "nlohmann::json::parse 把字符串解析成 JSON 对象，语法错误抛 json::exception。"
                  "两段 try/catch 分开：第一段挡“整体不是合法 JSON”，第二段挡“字段类型不对”"
                  "（比如 linear 给了字符串）。按 topic 字符串比较分发到四个处理函数，简单直接的命令路由。"
                  "对云端发来的畸形数据只警告不崩溃，保证节点稳健。")

    add_heading(doc, "4.10 handleMoveCmd —— 遥控移动", 2)
    add_code(doc, """  static double clamp1(double v) { return std::max(-1.0, std::min(1.0, v)); }

  void handleMoveCmd(const nlohmann::json& j)
  {
    const double linear = clamp1(j.value("linear", 0.0));
    const double angular = clamp1(j.value("angular", 0.0));

    if (auto_pause_on_manual_ && task_running_.load())
    {
      sendSignal("pause");
      task_running_ = false;
      ROS_INFO("auto task paused by manual command");
    }

    std::lock_guard<std::mutex> lock(motion_mutex_);
    last_linear_ = linear;
    last_angular_ = angular;
    last_move_time_ = ros::Time::now();
    publishManualCmd(linear, angular);
  }""")
    add_para(doc, "语法点1：j.value(\"linear\", 0.0) 是 nlohmann::json 的取值接口——字段存在则取出并转成 "
                  "double，不存在返回默认值 0.0；比 j[\"linear\"] 安全（后者字段缺失会插入 null 再转换出错）。")
    add_para(doc, "语法点2：clamp1 用 std::max(-1.0, std::min(1.0, v)) 把数值夹在 [-1,1] 区间："
                  "内层 min 限上界、外层 max 限下界，是手写 clamp 的经典组合（C++17 也有 std::clamp 可用）。"
                  "static 成员函数不依赖对象状态。")
    add_para(doc, "如果自动任务正在跑（task_running_ 原子读）且配置了 auto_pause_on_manual，"
                  "先发 /signal=pause 让 pure_pursuit 暂停，再接管底盘——防止遥控和自动导航同时抢车。")
    add_para(doc, "最后持锁更新“最近一次运动指令”三件套（线速度、角速度、时刻）并立即下发。"
                  "这些变量同时被看门狗定时器（另一个线程）读写，所以必须加 motion_mutex_。"
                  "注意锁保护下才调用 publishManualCmd，而它又读 blade_on_/mow_height_ 原子量，不会死锁。")

    add_heading(doc, "4.11 handleBladeCmd —— 刀盘控制", 2)
    add_code(doc, """    const int state = j.value("state", 0);
    const int height = j.value("height", -1);
    blade_on_ = (state != 0);
    if (height >= 2 && height <= 11)
      mow_height_ = height;

    if (task_running_.load())
    {
      // 自动任务运行中：刀盘走 /signal 通道，由 pure_pursuit 接管
      sendSignal(blade_on_.load() ? "open" : "close");
      if (height >= 2 && height <= 11)
        sendSignal(std::to_string(mow_height_.load()));
    }
    else
    {
      std::lock_guard<std::mutex> lock(motion_mutex_);
      publishManualCmd(last_linear_, last_angular_);
    }""")
    add_para(doc, "刀盘开关和高度档位先记入原子成员。height 缺省 -1 表示“本条指令不改高度”，"
                  "合法档位 2~11 才更新。")
    add_para(doc, "关键设计：刀盘命令的下发通道随模式切换。自动任务中底盘控制器由 pure_pursuit 管理，"
                  "刀盘也要走 /signal 字符串通道（open/close 和档位数字字符串）；"
                  "手动模式则把刀盘状态搭在 Manual_Driving_Cmd 消息的 mover_bool/mow_height 字段里，"
                  "用上次的运动值重发一帧（底盘指令和刀盘指令在同一条消息里，必须整帧重发）。")

    add_heading(doc, "4.12 handleTaskCmd —— 自动任务 start/stop/pause/continue", 2)
    add_code(doc, """    else if (action == "stop")
    {
      task_running_ = false;
      sendSignal("stop");
      std::thread([this]() {
        ros::Duration(task_stop_delay_).sleep();
        sendSignal("stop_execution");
      }).detach();
      ROS_INFO("auto task stop requested");
    }""")
    add_para(doc, "pause/continue 很简单：更新 task_running_ 原子标志，发对应 /signal 字符串即可。"
                  "stop 稍微讲究：先发 stop 让规划节点复位，然后起一个临时线程睡 task_stop_delay_（默认2秒）"
                  "后再发 stop_execution 彻底关闭 pure_pursuit。"
                  "语法点：std::thread(lambda).detach() —— 创建线程并立即分离，线程跑完自动回收，"
                  "主流程不用 join 等待，实现“延时后执行一次”的效果；[this] 捕获本对象指针。")
    add_para(doc, "用延时线程而不是直接 sleep，是因为当前正跑在 paho 回调线程里，"
                  "阻塞 2 秒会卡住所有 MQTT 消息处理。")

    add_heading(doc, "4.13 startTaskSequence —— 任务启动序列", 2)
    add_code(doc, """    if (task_seq_running_.exchange(true))
    {
      ROS_WARN("task start sequence already running");
      return;
    }
    task_running_ = true;
    std::thread([this, map_name, map_mode]() {
      // start_execution 拉起 pure_pursuit，再依次完成选图与开工
      sendSignal("start_execution");
      ros::Duration(task_step_delay_).sleep();
      sendSignal("reset");
      ros::Duration(task_step_delay_).sleep();
      if (!map_name.empty())
      {
        sendSignal("use_map/" + map_name);
        ros::Duration(task_step_delay_).sleep();
      }
      ...
      sendSignal("start_work");
      task_seq_running_ = false;
      ...
    }).detach();""")
    add_para(doc, "语法点（重点）：task_seq_running_.exchange(true) 是 std::atomic 的原子交换——"
                  "一步完成“读出旧值并写入 true”，返回旧值。若旧值已是 true 说明上一条 start 序列还在跑，"
                  "直接拒绝。这是“检查并置位”防重入的标准原子操作，比 load()+store() 两步安全"
                  "（两步之间可能插入另一个线程）。")
    add_para(doc, "启动序列在 detach 线程里按固定节奏发一串 /signal：start_execution（task_node 拉起 "
                  "pure_pursuit）→ reset（复位状态机）→ use_map/地图名（可选）→ single_map/multi_map（可选）"
                  "→ start_work（开工），每步间隔 task_step_delay_ 给下游节点反应时间。"
                  "lambda 捕获列表 [this, map_name, map_mode] 按值拷贝两个字符串，"
                  "避免线程还没跑完局部变量已析构的悬垂引用。")

    add_heading(doc, "4.14 handleVideoCmd —— 视频参数动态调整", 2)
    add_code(doc, """    VideoCfg cfg;
    {
      std::lock_guard<std::mutex> lock(video_mutex_);
      cfg = video_cfg_;
    }
    if (j.contains("enable"))
    {
      const auto& e = j["enable"];
      cfg.enable = e.is_boolean() ? e.get<bool>() : (e.get<int>() != 0);
    }
    if (j.contains("fps"))
      cfg.fps = std::max(0.1, std::min(30.0, j["fps"].get<double>()));
    ...
    {
      std::lock_guard<std::mutex> lock(video_mutex_);
      video_cfg_ = cfg;
    }
    streamer_.configure(cfg);""")
    add_para(doc, "先在锁内拷贝当前配置，再在锁外逐字段修改（缩小临界区），最后锁内写回并调 "
                  "streamer_.configure 按需重启 ffmpeg。j.contains(\"enable\") 判断字段是否存在——"
                  "云端可以只下发想改的字段。")
    add_para(doc, "enable 字段做了类型兼容：is_boolean() 为真按 bool 取，否则按 int 取再 !=0，"
                  "云端发 true 或 1 都能用。fps/width/height/bitrate 都用 min/max 组合限幅到合理范围"
                  "（如 fps 0.1~30、码率 100~8000kbps），防止云端误发极端值拖垮车端。")


def chapter_node_rest(doc):
    add_heading(doc, "4.15 publishManualCmd —— uint16 补码回绕", 2)
    add_code(doc, """  void publishManualCmd(double linear, double angular)
  {
    mower_msgs::Manual_Driving_Cmd cmd;
    cmd.ad_control_enable = 1;
    cmd.gear_model = 3;
    // uint16 字段赋负值会按二进制补码回绕，task_node 拷进 int16 后还原为负
    cmd.drive_value = static_cast<int>(linear * drive_max_);
    cmd.turn_value = static_cast<int>(-angular * turn_max_);
    cmd.mover_bool = blade_on_.load() ? 1 : 0;
    cmd.mow_height = mow_height_.load();
    pub_manual_.publish(cmd);
  }""")
    add_para(doc, "把 [-1,1] 的比例值放大成下位机量程：drive_value = linear×10000，turn_value = -angular×12566"
                  "（注意角速度取了负号，是车端坐标约定）。ad_control_enable=1 表示使能遥控，gear_model=3 为档位模式。")
    add_para(doc, "注释里的语法点：消息字段 drive_value/turn_value 是 uint16（无符号16位），"
                  "而后退/右转需要负数。C++ 里有符号转无符号按模 2^16 回绕（-1 变成 65535），"
                  "二进制位型恰好等于该负数的补码；task_node 收到后把同样的16位按 int16 解读，"
                  "就还原回负数。这是利用补码表示“免费”穿过无符号字段的技巧，"
                  "static_cast<int> 先把 double 截断成整数再赋给 uint16 字段。")

    add_heading(doc, "4.16 watchdogTimerCb —— 遥控看门狗", 2)
    add_code(doc, """  void watchdogTimerCb(const ros::TimerEvent&)
  {
    std::lock_guard<std::mutex> lock(motion_mutex_);
    if (last_linear_ == 0.0 && last_angular_ == 0.0)
      return;
    if ((ros::Time::now() - last_move_time_).toSec() > cmd_timeout_)
    {
      last_linear_ = 0.0;
      last_angular_ = 0.0;
      publishManualCmd(0.0, 0.0);
      ROS_WARN_THROTTLE(5, "move cmd timeout, auto stop");
    }
  }""")
    add_para(doc, "每 0.1 秒检查一次：当前指令是停车就不用管；若距最后一条 move 指令超过 cmd_timeout_"
                  "（默认 0.5s，云端需持续发指令维持行驶），自动把指令清零并发停车帧。"
                  "这是遥控安全兜底——云端断网、MQTT 断线都不会让车继续冲。")
    add_para(doc, "ROS_WARN_THROTTLE(5, ...) 是限频日志宏：同样内容 5 秒内最多打一条，防止日志刷屏。"
                  "ros::Time 相减得到 ros::Duration，.toSec() 转秒。形参 const ros::TimerEvent& 不写名字，"
                  "表示不用事件信息。")

    add_heading(doc, "4.17 定位上报：positionCb + locationTimerCb", 2)
    add_code(doc, """  void positionCb(const util::Position::ConstPtr& msg)
  {
    std::lock_guard<std::mutex> lock(position_mutex_);
    latest_position_ = *msg;
    has_position_ = true;
  }

  void locationTimerCb(const ros::TimerEvent&)
  {
    util::Position pos;
    {
      std::lock_guard<std::mutex> lock(position_mutex_);
      if (!has_position_)
        return;
      pos = latest_position_;
    }
    nlohmann::json j = {
        {"x", pos.position_x},     {"y", pos.position_y},
        ...
        {"stamp", ros::Time::now().toSec()}};
    mqtt_->publish(topic_state_location_, j.dump(), 0);
  }""")
    add_para(doc, "“订阅回调只存最新值，定时器按固定频率上报”的采样式结构：回调里加锁拷贝消息并置 "
                  "has_position_；定时器里锁内拷贝出来、锁外拼 JSON 发布——持锁时间最短，"
                  "网络操作不占用锁。ConstPtr 是 ROS 消息的常量共享指针，消息由 ROS 内存池管理，"
                  "拷贝 *msg 才得到实体。")
    add_para(doc, "语法点：nlohmann::json j = {{\"x\", v}, ...} 用初始化列表直接构造 JSON 对象，"
                  "键值对写法接近 Python 字典字面量；j.dump() 序列化成字符串；QoS 0 发布。"
                  "state 是定位状态字，stamp 是 ROS 时间转 Unix 秒。")

    add_heading(doc, "4.18 车辆状态回调与 statusTimerCb", 2)
    add_code(doc, """  void vehicleStatusCb(const mower_msgs::VehicleStatus::ConstPtr& msg)
  {
    std::lock_guard<std::mutex> lock(status_mutex_);
    battery_soc_ = msg->battery_soc;
    warning_one_ = msg->warning_state_one;
    warning_two_ = msg->warning_state_two;
    has_status_ = true;
  }
  // leftWheelCb / rightWheelCb / mowerHeightCb 结构相同，各存一个数值

  void statusTimerCb(const ros::TimerEvent&)
  {
    nlohmann::json j;
    {
      std::lock_guard<std::mutex> lock(status_mutex_);
      if (!has_status_)
        return;
      j = {{"battery_soc", battery_soc_}, ... };
    }
    j["stamp"] = ros::Time::now().toSec();
    mqtt_->publish(topic_state_vehicle_, j.dump(), 0);
  }""")
    add_para(doc, "四个订阅回调共用一把 status_mutex_ 保护一组松散状态（电量、报警字、左右轮速、刀盘高度反馈），"
                  "粒度粗一点但逻辑简单。statusTimerCb 同样锁内取值锁外发布，"
                  "j[\"stamp\"] = ... 演示 nlohmann::json 的 operator[] 赋值（键不存在则创建）。"
                  "has_status_ 由 VehicleStatus 回调置位——车辆状态话题起来了才开始上报，轮速等字段没收到就是 0。")

    add_heading(doc, "4.19 imageCb —— 图像回调：cv_bridge 转换与帧率节流", 2)
    add_code(doc, """  void imageCb(const sensor_msgs::ImageConstPtr& msg)
  {
    VideoCfg cfg;
    {
      std::lock_guard<std::mutex> lock(video_mutex_);
      cfg = video_cfg_;
    }
    if (!cfg.enable)
      return;

    // 帧率节流
    const ros::Time now = ros::Time::now();
    {
      std::lock_guard<std::mutex> lock(video_mutex_);
      if ((now - last_frame_time_).toSec() < 1.0 / cfg.fps)
        return;
      last_frame_time_ = now;
    }

    cv_bridge::CvImageConstPtr cv_ptr;
    try
    {
      // mono8 / bgr8 / yuv422 等统一转成 BGR8
      cv_ptr = cv_bridge::toCvShare(msg, sensor_msgs::image_encodings::BGR8);
    }
    catch (const cv_bridge::Exception& e)
    {
      ROS_WARN_THROTTLE(5, "cv_bridge convert failed: %s", e.what());
      return;
    }

    cv::Mat frame = cv_ptr->image;
    if (frame.cols != cfg.width || frame.rows != cfg.height)
      cv::resize(frame, frame, cv::Size(cfg.width, cfg.height));

    streamer_.push(frame);
  }""")
    add_para(doc, "相机帧率（如 30fps）通常高于推流帧率（默认 5fps），所以做帧率节流：距上次送帧不足 "
                  "1/fps 秒就直接丢弃。last_frame_time_ 在锁内检查并更新，避免多回调并发时双送。"
                  "video_cfg_ 先锁内拷贝出来用，与 handleVideoCmd 的写同步。")
    add_para(doc, "cv_bridge::toCvShare 把 ROS 图像消息转成 CvImage 智能指针并指定目标编码 BGR8——"
                  "toCvShare 在源编码相同的时候零拷贝共享原消息内存，不同则转换；mono8/yuv422 等都能统一转成 BGR。"
                  "转换失败（不支持的编码）抛 cv_bridge::Exception，限频警告后丢弃该帧。")
    add_para(doc, "尺寸不匹配就 cv::resize 缩放到推流分辨率（降低带宽），最后交给 streamer_.push 写入 ffmpeg 管道。"
                  "注意 push 内部有自己的锁，本函数此时不持 video_mutex_，无嵌套锁问题。")

    add_heading(doc, "4.20 成员变量总览", 2)
    add_code(doc, """  // 跨线程访问：MQTT 回调线程写，ROS 定时器线程读
  std::atomic<int> mow_height_{6};
  std::atomic<bool> blade_on_{false};

  std::mutex motion_mutex_;
  double last_linear_ = 0.0, last_angular_ = 0.0;
  ros::Time last_move_time_;

  std::atomic<bool> task_running_{false};
  std::atomic<bool> task_seq_running_{false};""")
    add_para(doc, "成员分几组：ROS 句柄（NodeHandle/Publisher/Subscriber/Timer）；MQTT 客户端与主题名；"
                  "参数缓存；以及跨线程共享状态。共享状态的保护策略分两类：单个简单值用 std::atomic"
                  "（mow_height_、blade_on_、task_running_、task_seq_running_，免锁）；"
                  "一组必须一致读写的值共用一把 mutex（运动指令 motion_mutex_、定位 position_mutex_、"
                  "车辆状态 status_mutex_、视频配置 video_mutex_）。选择标准是“要不要多个变量保持一致的快照”。")

    add_heading(doc, "4.21 main 函数", 2)
    add_code(doc, """int main(int argc, char** argv)
{
  // ffmpeg 子进程退出后管道写会触发 SIGPIPE，忽略之，由 write 返回值处理
  signal(SIGPIPE, SIG_IGN);
  ros::init(argc, argv, "cloud_bridge");
  cloud_bridge::CloudBridgeNode node;
  ros::AsyncSpinner spinner(2);
  spinner.start();
  ros::waitForShutdown();
  node.stop();
  return 0;
}""")
    add_para(doc, "signal(SIGPIPE, SIG_IGN)：向已关闭的管道 write 时内核默认会发 SIGPIPE 直接杀死进程；"
                  "ffmpeg 退出后我们还想靠 write 返回值做限频重启（见 4.4），所以必须忽略这个信号。"
                  "这是所有用管道写子进程的程序都要做的第一步。")
    add_para(doc, "ros::init 初始化节点；node 在栈上构造（构造函数里完成全部初始化）；"
                  "ros::AsyncSpinner spinner(2) 开 2 个后台线程处理订阅和定时器回调（默认单线程 "
                  "ros::spin() 的并发版），回调可能慢（图像处理）时不至于互相阻塞；"
                  "waitForShutdown 阻塞到 Ctrl+C/rosnode kill；退出前先 node.stop() 断开 MQTT，"
                  "然后栈展开自动析构 node（MqttClient 析构再兜底 stop、SrtStreamer 析构杀 ffmpeg）——"
                  "RAII 保证退出路径上资源都被清理。")


# ==================== 第5章 launch / 第6章 构建 ====================

def chapter_launch(doc):
    add_heading(doc, "5. launch/cloud_bridge.launch —— 启动参数", 1)
    add_code(doc, """<launch>
  <node pkg="cloud_bridge" type="cloud_bridge_node" name="cloud_bridge" output="screen">

    <!-- ===== MQTT broker（明文 TCP，无加密无认证）===== -->
    <param name="broker_host" value="127.0.0.1"/>
    <param name="broker_port" value="1883"/>
    <param name="client_id" value="cloud_bridge_mower_001"/>
    <param name="username" value=""/>
    <param name="password" value=""/>
    <param name="keep_alive_interval" value="30"/>
    <param name="connect_timeout" value="5"/>""")
    add_para(doc, "<node pkg type name output>：启动 cloud_bridge 包里的 cloud_bridge_node 可执行文件，"
                  "节点名 cloud_bridge，output=\"screen\" 把 ROS_INFO/ROS_WARN 日志打到终端。"
                  "节点标签内的 <param> 是私有参数，对应代码里 pnh_(\"~\") 读取的那批，"
                  "名字与 loadParams() 中一一对应。broker 组：broker_host 默认 127.0.0.1 表示 broker 跑在车端本机，"
                  "实际部署时改成云平台地址；username/password 留空即匿名连接。")
    add_code(doc, """    <param name="device_id" value="mower_001"/>
    <param name="topic_prefix" value="mower"/>

    <param name="location_hz" value="2.0"/>
    <param name="status_hz" value="1.0"/>""")
    add_para(doc, "device_id + topic_prefix 拼出 MQTT 主题前缀 mower/mower_001/...；"
                  "location_hz/status_hz 控制两个上行定时器的频率。")
    add_code(doc, """    <param name="image_topic" value="/camera/image_rect"/>
    <param name="srt_target" value=""/>
    <param name="video_enable" value="false"/>
    <param name="video_fps" value="5.0"/>
    <param name="video_width" value="640"/>
    <param name="video_height" value="480"/>
    <param name="video_bitrate" value="800"/>""")
    add_para(doc, "视频组：image_topic 是图像源；srt_target 是云平台 SRT 接收地址"
                  "（形如 srt://云平台IP:9000?mode=caller），留空则推流功能禁用（startLocked 会报错返回）；"
                  "video_enable 默认 false，等云端 cmd/video 指令下发 enable 才起流；"
                  "fps/分辨率/码率是初始值，运行时均可远程改。")
    add_code(doc, """    <param name="cmd_timeout" value="0.5"/>
    <param name="drive_max" value="10000"/>
    <param name="turn_max" value="12566"/>
    <param name="auto_pause_on_manual" value="true"/>

    <param name="task_step_delay" value="0.5"/>
    <param name="task_stop_delay" value="2.0"/>

    <param name="mow_height" value="6"/>""")
    add_para(doc, "遥控组：cmd_timeout 是看门狗超时（云端停发 move 指令 0.5 秒自动停车）；"
                  "drive_max/turn_max 是比例值 1.0 映射到的下位机数值；auto_pause_on_manual 决定自动任务中"
                  "收到遥控是否先 pause。任务组：task_step_delay 是 start 序列各 /signal 间隔，"
                  "task_stop_delay 是 stop 到 stop_execution 的延时。mow_height 是默认割草高度档位（2~11）。")


def chapter_build(doc):
    add_heading(doc, "6. CMakeLists.txt 与 package.xml —— 构建配置", 1)

    add_heading(doc, "6.1 CMakeLists.txt", 2)
    add_code(doc, """cmake_minimum_required(VERSION 3.0.2)
project(cloud_bridge)

add_compile_options(-std=c++17)

find_package(catkin REQUIRED COMPONENTS
  roscpp std_msgs sensor_msgs mower_msgs util cv_bridge
)
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)""")
    add_para(doc, "catkin 工作空间的标准开头。add_compile_options(-std=c++17) 启用 C++17"
                  "（代码用到类内初始化、if 初始化语句风格等现代特性）。"
                  "find_package(catkin ... COMPONENTS) 声明依赖的 ROS 包，其中 mower_msgs/util 是本项目自定义消息包；"
                  "OpenCV（cv_bridge 的底层）和 Threads（std::thread 需要 -lpthread）单独找。")
    add_code(doc, """find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
  find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp)
  if(NOT NLOHMANN_JSON_INCLUDE_DIR)
    message(FATAL_ERROR "nlohmann/json.hpp not found. Install: sudo apt install nlohmann-json3-dev")
  endif()
endif()""")
    add_para(doc, "nlohmann_json 用 QUIET 方式先按 CMake config 包找，找不到退化为 find_path 只找头文件"
                  "（该库是 header-only，只需头文件路径）；两级都找不到就 FATAL_ERROR 终止配置并提示安装命令。"
                  "这种“找库 + 兜底 + 友好报错”的三段式写法对非 ROS 第三方依赖很典型。")
    add_code(doc, """find_library(PAHO_MQTT_CPP_LIB NAMES paho-mqttpp3
  PATHS /usr/local/lib /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu)
...
find_library(PAHO_MQTT_C_LIB NAMES paho-mqtt3as paho-mqtt3a
  PATHS /usr/local/lib /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu)""")
    add_para(doc, "Paho 没有官方 CMake config 时只能 find_library 按库文件名搜："
                  "paho-mqttpp3 是 C++ 库，paho-mqtt3as/paho-mqtt3a 是底层 C 库（s=静态，a=异步），"
                  "PATHS 覆盖源码安装（/usr/local）和 apt 安装的 x86/ARM 两种架构目录。"
                  "C++ 库内部依赖 C 库，两个都必须显式链接，否则报 undefined reference。")
    add_code(doc, """catkin_package(
  INCLUDE_DIRS include
  CATKIN_DEPENDS roscpp std_msgs sensor_msgs mower_msgs util cv_bridge
)

add_executable(cloud_bridge_node
  src/cloud_bridge_node.cpp
  src/mqtt_client.cpp
)

add_dependencies(cloud_bridge_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

target_link_libraries(cloud_bridge_node
  ${catkin_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${PAHO_MQTT_CPP_LIB}
  ${PAHO_MQTT_C_LIB}
  Threads::Threads
)""")
    add_para(doc, "catkin_package 声明本包对外的头文件目录和消息依赖（下游包 find_package 时用）。"
                  "add_executable 把两个 .cpp 编成一个节点。"
                  "add_dependencies 保证先生成自定义消息（mower_msgs/util 的头）再编本节点，"
                  "这是使用自定义消息时必须写的一行。"
                  "target_link_libraries 链接 catkin 库、OpenCV、两个 paho 库和线程库。"
                  "文件末尾的 install 规则把可执行文件和 launch 目录装进 devel/install 空间，"
                  "供 catkin_make install 部署。")

    add_heading(doc, "6.2 package.xml", 2)
    add_code(doc, """<package format="2">
  <name>cloud_bridge</name>
  <version>1.0.0</version>
  <description>MQTT cloud bridge for the mower: remote driving, blade control,
    auto task start/stop, video streaming and location reporting.</description>

  <buildtool_depend>catkin</buildtool_depend>

  <depend>roscpp</depend>
  <depend>std_msgs</depend>
  <depend>sensor_msgs</depend>
  <depend>mower_msgs</depend>
  <depend>util</depend>
  <depend>cv_bridge</depend>
</package>""")
    add_para(doc, "ROS 包清单：format=\"2\" 是较新的格式版本，<depend> 一条同时声明编译和运行依赖"
                  "（format 1 要分 build_depend/run_depend 写两遍）。"
                  "这里只列 ROS 包；OpenCV、nlohmann_json、paho 是系统库，不走 package.xml，"
                  "只在 CMakeLists.txt 里 find。rosdep 可根据本文件自动安装 ROS 依赖。")


# ==================== 主流程 ====================

def build_docx(path):
    doc = Document()
    title = doc.add_heading("", level=0)
    r = title.add_run("cloud_bridge 代码详解")
    set_run_font(r, bold=True, size=Pt(22))
    title.alignment = WD_ALIGN_PARAGRAPH.CENTER
    add_para(doc, "ROS1 功能包 cloud_bridge 逐段代码讲解（C++ / ROS / MQTT / SRT 推流）", size=Pt(10.5))

    chapter_overview(doc)
    chapter_header(doc)
    chapter_client_cpp(doc)
    chapter_node_streamer(doc)
    chapter_node_main(doc)
    chapter_node_rest(doc)
    chapter_launch(doc)
    chapter_build(doc)

    doc.save(path)
    print("Word 文档已生成:", path)


if __name__ == "__main__":
    build_docx("cloud_bridge代码详解.docx")
