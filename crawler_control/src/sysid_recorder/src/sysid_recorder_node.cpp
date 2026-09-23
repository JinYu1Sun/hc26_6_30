#include <ros/ros.h>
#include <std_msgs/String.h>                    // 配置话题消息
#include <mower_msgs/VehicleCmd.h>              // 控制指令
#include <minibus_msg_util/GpsPosition.h>       // 定位数据

#include <nlohmann/json.hpp>                    // 解析 JSON 配置

#include <algorithm>    // std::max / std::min（限幅）
#include <cmath>        // sin / cos / hypot / lround
#include <cstdlib>      // getenv
#include <ctime>        // 生成时间戳文件名
#include <fstream>      // ofstream 写 CSV
#include <sstream>      // ostringstream 拼一行数据
#include <string>

#include <sys/stat.h>   // mkdir() 建输出目录

// 每个 CSV 文件首行写入的列名，顺序必须与 buildRow() 严格对应
const char* kCsvHeader =
  "ros_time,t_run,mode,drive_cmd,turn_cmd,"
  "v_forward_mps,v_norm_mps,v_north_mps,v_east_mps,"
  "yaw_rate_degs,azimuth_deg,longitude,latitude,height_m,"
  "gaussX_cm,gaussY_cm,gps_flag,gps_confidence";

// 一个通道（前进或转向）的波形参数。
// 指令值 = offset + 波形(t)，三种波形：
//   const: 恒定 A（静态标定）
//   sine : A*sin(B*t + T)（动态/耦合辨识，B 是角频率 rad/s，周期=2π/B）
//   step : t>=T 时输出 A，否则 0（测时间常数的阶跃响应）
// 默认值设计成 A=0 时输出恒 0：没配置的通道不发指令，安全。
struct WaveformConfig
{
  std::string type = "const";
  double A = 0.0, B = 1.0, T = 0.0, offset = 0.0;

  double eval(double t) const
  {
    if (type == "sine") return offset + A * std::sin(B * t + T);
    if (type == "step") return offset + (t >= T ? A : 0.0);
    return offset + A;  // const
  }
};

// 从 JSON 里读一个通道的波形；key 不存在时返回"恒定0"（即该通道不发指令）
WaveformConfig parseWave(const nlohmann::json& j, const char* key)
{
  WaveformConfig w;
  if (j.contains(key)) {
    const auto& c = j[key];
    w.type   = c.value("type", "const");
    w.A      = c.value("A", 0.0);
    w.B      = c.value("B", 1.0);
    w.T      = c.value("T", 0.0);
    w.offset = c.value("offset", 0.0);
  }
  return w;
}

// CSV 记录器：所有文件操作封装在这里，节点主体不关心文件细节
class CsvLogger
{
public:
  // 开始一段新试验：关掉旧文件（数据落盘），在 dir 下新建 模式_时间戳.csv
  void openRun(const std::string& dir, const std::string& mode)
  {
    close();
    ::mkdir(dir.c_str(), 0755);            // 目录已存在会返回错误，忽略即可
    path_ = dir + "/" + mode + "_" + nowStamp() + ".csv";
    ofs_.open(path_, std::ios::out | std::ios::trunc);
    if (!ofs_) {
      ROS_ERROR("无法创建文件: %s", path_.c_str());
      return;
    }
    ofs_ << kCsvHeader << "\n";            // 首行写表头
    rows_ = 0;
    ROS_INFO("开始记录: %s", path_.c_str());
  }

  void log(const std::string& row)
  {
    if (!ofs_.is_open()) return;
    ofs_ << row << "\n";
    // 每攒 50 行（20Hz 下 2.5 秒数据）落盘一次：
    // 行行 flush 的磁盘 IO 会拖慢定时器回调，攒批是"丢得起"与"开销"的平衡
    if (++rows_ % 50 == 0) ofs_.flush();
  }

  void close()
  {
    if (ofs_.is_open()) { ofs_.flush(); ofs_.close(); }
  }

private:
  static std::string nowStamp()            // 20260706_153012 格式时间戳
  {
    std::time_t t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", std::localtime(&t));
    return buf;
  }

  std::ofstream ofs_;
  std::string path_;
  int rows_ = 0;
};

class SysidRecorder
{
public:
  SysidRecorder() : pnh_("~")   // 私有句柄：~参数 和 ~config 话题都靠它解析
  {
    // ---- 参数（launch 里可改，第二个实参是默认值）----
    pnh_.param<std::string>("gps_topic", gps_topic_, "/nanobot/gpsposition");
    pnh_.param<std::string>("cmd_topic", cmd_topic_, "/vehicle/cmd");
    pnh_.param<std::string>("output_dir", output_dir_,
                            std::string(getenv("HOME") ? getenv("HOME") : "/tmp")
                            + "/sysid_data");
    pnh_.param("sample_rate",  sample_rate_, 20.0);    // 发指令/记录频率 Hz
    pnh_.param("drive_limit",  drive_limit_, 10000.0); // 安全限幅
    pnh_.param("turn_limit",   turn_limit_, 12566.0);
    pnh_.param("mower_height", mower_height_, 11);

    // ---- 通信接口 ----
    // queue_size=1：指令只保留最新一条，防止积压发出过时指令
    pub_cmd_ = nh_.advertise<mower_msgs::VehicleCmd>(cmd_topic_, 1);
    sub_gps_ = nh_.subscribe(gps_topic_, 10, &SysidRecorder::gpsCb, this);
    // 用 pnh_ 订阅 "config"，完整话题名解析为 /sysid_recorder/config
    sub_cfg_ = pnh_.subscribe("config", 1, &SysidRecorder::configCb, this);

    // ---- 主循环定时器：采样率的载体，每 1/sample_rate 秒触发一次 ----
    // 指令节奏完全由定时器控制，不受 GPS 消息到达节奏影响
    timer_ = nh_.createTimer(ros::Duration(1.0 / sample_rate_),
                             &SysidRecorder::timerCb, this);

    ROS_INFO("sysid_recorder 已启动，向 %s 发 JSON 开始试验",
             pnh_.resolveName("config").c_str());
  }

  ~SysidRecorder() { logger_.close(); }   // 退出前确保数据落盘

private:
  // 三种测试各存一个文件夹；返回空串表示不认识的模式（即 stop）
  static std::string modeToDir(const std::string& mode)
  {
    if (mode == "static_forward" || mode == "static_turn")
      return "static_calibration";
    if (mode == "dynamic_forward" || mode == "dynamic_turn")
      return "dynamic_identification";
    if (mode == "coupling")
      return "coupling_identification";
    return "";
  }

  void gpsCb(const minibus_msg_util::GpsPosition::ConstPtr& msg)
  {
    gps_ = *msg;          // 只缓存最新一帧，处理留给定时器按固定节奏做
    have_gps_ = true;
  }

  // 配置回调：收到新 JSON = 切换测试
  void configCb(const std_msgs::String::ConstPtr& msg)
  {
    nlohmann::json j;
    try {
      j = nlohmann::json::parse(msg->data);
    } catch (const std::exception& e) {
      // 解析失败会抛异常，不 catch 节点会崩；兜住后保持当前状态继续跑
      ROS_WARN("config 不是合法 JSON: %s", e.what());
      return;
    }

    logger_.close();                          // 旧试验数据先落盘
    mode_      = j.value("mode", "stop");
    drive_cfg_ = parseWave(j, "drive");       // 前进通道波形
    turn_cfg_  = parseWave(j, "turn");        // 转向通道波形
    t0_        = ros::Time::now();            // 新试验 t_run 从 0 开始

    std::string subdir = modeToDir(mode_);
    if (!subdir.empty()) {
      logger_.openRun(output_dir_ + "/" + subdir, mode_);  // 新试验 = 新 CSV
    } else {
      ROS_INFO("收到 stop，停车");
    }
  }

  // 主循环：算指令 -> 限幅 -> 发布 -> 记录
  void timerCb(const ros::TimerEvent&)
  {
    double drive = 0.0, turn = 0.0, t_run = 0.0;   // 默认停车发 0
    if (mode_ != "stop") {
      t_run = (ros::Time::now() - t0_).toSec();
      drive = clamp(drive_cfg_.eval(t_run), drive_limit_);
      turn  = clamp(turn_cfg_.eval(t_run),  turn_limit_);
    }

    // 组装并发布控制指令（字段含义参照 task_node 的 goStraight）
    mower_msgs::VehicleCmd cmd;
    cmd.header.stamp      = ros::Time::now();
    cmd.ad_control_enable = 1;      // 使能自动控制
    cmd.gear_model        = 3;      // D 前进档
    cmd.mover_bool        = 0;      // 不割草
    cmd.mower_height      = mower_height_;
    cmd.drive_value       = (int16_t)std::lround(drive);
    cmd.turn_value        = (int16_t)std::lround(turn);
    pub_cmd_.publish(cmd);

    // 记录条件：在试验中 且 收到过 GPS；同一行里指令与响应成对出现
    if (mode_ != "stop" && have_gps_)
      logger_.log(buildRow(t_run, cmd.drive_value, cmd.turn_value));
  }

  static double clamp(double v, double lim)
  {
    return std::max(-lim, std::min(lim, v));
  }

  std::string buildRow(double t_run, int drive, int turn)
  {
    double vn = gps_.northVelocity / 100.0;          // cm/s -> m/s
    double ve = gps_.eastVelocity  / 100.0;
    double az = gps_.azimuth * 0.01 * M_PI / 180.0;  // 0.01度 -> 弧度（东为零逆时针）
    // 速度投影到车头方向：有符号的前进速度（倒车为负）
    double v_fwd  = ve * std::cos(az) + vn * std::sin(az);
    double v_norm = std::hypot(vn, ve);              // 速度大小

    std::ostringstream os;
    os.precision(6);
    os << std::fixed
       << ros::Time::now().toSec() << ',' << t_run << ',' << mode_ << ','
       << drive << ',' << turn << ','
       << v_fwd << ',' << v_norm << ',' << vn << ',' << ve << ','
       << gps_.rot_z << ','                          // 横摆角速度 deg/s
       << gps_.azimuth * 0.01 << ','                 // 航向角 deg
       << gps_.longitude << ',' << gps_.latitude << ',' << gps_.height << ','
       << gps_.gaussX << ',' << gps_.gaussY << ','
       << gps_.gps_flag << ',' << gps_.gps_confidence;
    return os.str();
  }

  // ---- 成员变量 ----
  // 这些 ROS 句柄必须是成员变量：句柄销毁则订阅自动注销，
  // 若写成构造函数局部变量，构造函数一结束回调就永远不会触发
  ros::NodeHandle nh_, pnh_;
  ros::Publisher  pub_cmd_;
  ros::Subscriber sub_gps_, sub_cfg_;
  ros::Timer      timer_;

  std::string gps_topic_, cmd_topic_, output_dir_;
  double sample_rate_, drive_limit_, turn_limit_;
  int mower_height_;

  std::string mode_ = "stop";       // 上电默认停车，收到配置才动
  WaveformConfig drive_cfg_, turn_cfg_;
  ros::Time t0_;                    // 本段试验起始时刻
  minibus_msg_util::GpsPosition gps_;
  bool have_gps_ = false;
  CsvLogger logger_;
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "sysid_recorder");   // 节点名决定 /sysid_recorder/config
  SysidRecorder node;
  ros::spin();       // 单线程事件循环：回调排队执行互不并发，所以全程不需要锁
  return 0;          // Ctrl+C 时 spin 返回，node 析构、文件落盘，干净退出
}
