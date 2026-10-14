#include <ros/ros.h>
#include <std_msgs/String.h>                    // 配置话题消息
#include <mower_msgs/VehicleCmd.h>              // 控制指令
#include <util/GpsPosition.h>                 // 定位数据

#include <nlohmann/json.hpp>                    // 解析 JSON 配置

#include <algorithm>    // std::max / std::min（限幅）
#include <cmath>        // sin / cos / fmod / hypot / lround
#include <cstdlib>      // getenv
#include <ctime>        // 生成日期/时间戳
#include <fstream>      // ofstream 写 CSV
#include <sstream>      // ostringstream 拼一行数据
#include <string>

#include <sys/stat.h>   // mkdir() 建输出目录

// 每个 CSV 文件首行写入的列名，顺序必须与 buildRow() 严格对应
// run 列：同一天同一文件里可能有多段试验，用 run（启动时刻 HHMMSS）区分
const char* kCsvHeader =
  "ros_time,t_run,run,mode,drive_cmd,turn_cmd,"
  "v_forward_mps,v_norm_mps,v_north_mps,v_east_mps,"
  "yaw_rate_degs,azimuth_deg,longitude,latitude,height_m,"
  "gaussX_cm,gaussY_cm,gps_flag,gps_confidence";

// 一个通道（前进或转向）的波形参数。
// 指令值 = offset + 波形(t)，五种波形：
//   const: 恒定 A（静态标定）
//   sine : A*sin(B*t + T)（动态/耦合辨识，B 是角频率 rad/s，周期=2π/B）
//   step : t>=T 时输出 A，否则 0（测时间常数的阶跃响应）
//   dwell: 带零位保持和极值保持的类正弦（参数 Tr/Th/Tz，见 dwellShape 注释）
//   multisine: 多频正弦 + 过零保持 + 极值保持 + 零阶保持（耦合辨识专用，
//              转向通道用 1/3/5 次奇谐波、前进通道用 2/4 次偶谐波，
//              奇偶分频使两通道频谱互不重叠，见 multisineValue 注释）
// 默认值设计成 A=0 时输出恒 0：没配置的通道不发指令，安全。
struct WaveformConfig
{
  std::string type = "const";
  double A = 0.0, B = 1.0, T = 0.0, offset = 0.0;
  double Tr = 2.0, Th = 2.0, Tz = 2.0;   // dwell 专用：过渡/极值保持/零位保持（秒）
  // multisine 专用
  double T0 = 60.0;              // 周期（秒）
  double fc = 50.0;              // 零阶保持频率 Hz：时间先量化到 1/fc 网格再算波形
  double pZero = 0.10;           // 过零保持比例：每半周期约 pZero 比例时间贴零（建议<=0.10）
  double pPeak = 0.12;           // 极值保持比例：每半周期约 pPeak 比例时间贴平在峰值
  double A1 = 0.0, A3 = 0.0, A5 = 0.0;   // 奇次谐波幅值（转向通道用，相位恒 0）
  double A2 = 0.0, A4 = 0.0;             // 偶次谐波幅值（前进通道用）
  double P2 = 4.8860, P4 = 4.5729;       // 偶次谐波相位 rad（离线寻优结果，使轨迹近似闭合）

  double eval(double t) const
  {
    if (type == "sine") return offset + A * std::sin(B * t + T);
    if (type == "step") return offset + (t >= T ? A : 0.0);
    if (type == "dwell") return offset + A * dwellShape(t + T, Tr, Th, Tz);
    if (type == "multisine") return offset + multisineValue(t + T);
    return offset + A;  // const
  }

  // 多频正弦 + 双边保持(zeroPeakHold) + 零阶保持(ZOH)，与离线 MATLAB 设计一致：
  //   s = A1 sin(wt) + A3 sin(3wt) + A5 sin(5wt)          <- 转向通道（奇次）
  //     + A2 sin(2wt+P2) + A4 sin(4wt+P4),  w = 2π/T0     <- 前进通道（偶次）
  // 过零保持：|s| < (Σ|Ai|)·sin(π·pZero/2) 时输出 0，波形每次过零在零值停留
  //   约 pZero 比例半周期，给车留出回正/稳定时间；前进通道谐波贴零即车速
  //   保持 offset。
  // 极值保持：s 超过 ±(Σ|Ai|)·cos(π·pPeak/2) 时钳在该电平，波形在峰值附近
  //   贴平约 pPeak 比例半周期，让车有时间进入稳态。
  // 两种保持都是逐点奇对称处理，不破坏奇/偶谐波分频特性（但会引入少量
  //   高次谐波，pZero 建议 <=0.10）。
  // 零阶保持：t 先向下量化到 1/fc 网格，输出与控制频率一致的阶梯波。
  double multisineValue(double t) const
  {
    const double T0l = T0 > 0.0 ? T0 : 60.0;         // 防除零
    const double fcl = fc > 0.0 ? fc : 50.0;
    const double tq  = std::floor(t * fcl + 1e-9) / fcl;   // ZOH 时间量化
    // （1e-9 容错：t 恰好落在段边界时，浮点误差不会让相邻时刻量化到不同段）
    const double w   = 2.0 * M_PI / T0l;
    const double s   = A1 * std::sin(1.0 * w * tq)
                     + A3 * std::sin(3.0 * w * tq)
                     + A5 * std::sin(5.0 * w * tq)
                     + A2 * std::sin(2.0 * w * tq + P2)
                     + A4 * std::sin(4.0 * w * tq + P4);
    const double Asum = std::fabs(A1) + std::fabs(A3) + std::fabs(A5)
                      + std::fabs(A2) + std::fabs(A4);
    return zeroPeakHold(s, Asum, pZero, pPeak);
  }

  // 双边保持：过零贴零 + 峰值贴平（与 MATLAB zeroPeakHold 一致）
  //   p_zero: 每半周期贴零的时间比例    p_peak: 每半周期贴峰值的时间比例
  // 判定都基于原始 s：s>=A_h 时即使 A_h 低于贴零阈值也输出 A_h
  static double zeroPeakHold(double s, double A, double p_zero, double p_peak)
  {
    if (p_peak >= 0.95) p_peak = 0.95;   // p_peak->1 时电平趋于 0，波形会被压没
    const double s_z = p_zero > 0.0 ? A * std::sin(M_PI * p_zero / 2.0) : 0.0;
    const double A_h = A * std::cos(M_PI * p_peak / 2.0);
    if (s >=  A_h) return  A_h;          // 正峰值段 -> 平顶
    if (s <= -A_h) return -A_h;          // 负峰值段 -> 平顶
    if (std::fabs(s) < s_z) return 0.0;  // 过零段 -> 0
    return s;
  }

  // 类正弦波的"形状"：返回值 ∈ [-1, 1]，外面乘 A 加 offset。
  // 一个周期 = 2*Tz + 2*Th + 4*Tr：
  //   零位保持Tz -> 余弦上升到+1(Tr) -> 极值保持Th -> 余弦降回0(Tr)
  //   -> 零位保持Tz -> 余弦降到-1(Tr) -> 极值保持Th -> 余弦升回0(Tr)
  // 过渡段用 (1-cos)/2 形状：端点斜率为 0，保持段与过渡段平滑衔接，
  // 不会像方波那样冲击底盘；保持段让车有时间进入稳态，一段波形
  // 同时包含稳态点（可做静态标定）和过渡段（可做动态辨识）。
  static double dwellShape(double t, double Tr, double Th, double Tz)
  {
    if (Tr <= 0.0) Tr = 0.05;              // 防除零：过渡时间最小 0.05s
    if (Th < 0.0) Th = 0.0;
    if (Tz < 0.0) Tz = 0.0;
    const double P = 2.0*Tz + 2.0*Th + 4.0*Tr;   // 周期
    double s = std::fmod(t, P);            // 折到一个周期内
    if (s < 0.0) s += P;                   // fmod 对负数保留符号，修正到 [0,P)

    double u;                              // 段内时间
    if (s < Tz) return 0.0;                // 零位保持
    s -= Tz;
    if (s < Tr) { u = s; return  (1.0 - std::cos(M_PI*u/Tr)) / 2.0; }  // 0 -> +1
    s -= Tr;
    if (s < Th) return 1.0;                // +A 极值保持
    s -= Th;
    if (s < Tr) { u = s; return  (1.0 + std::cos(M_PI*u/Tr)) / 2.0; }  // +1 -> 0
    s -= Tr;
    if (s < Tz) return 0.0;                // 零位保持
    s -= Tz;
    if (s < Tr) { u = s; return -(1.0 - std::cos(M_PI*u/Tr)) / 2.0; }  // 0 -> -1
    s -= Tr;
    if (s < Th) return -1.0;               // -A 极值保持
    s -= Th;
    u = s; return -(1.0 + std::cos(M_PI*u/Tr)) / 2.0;                  // -1 -> 0
  }
};

// 从 JSON 里读一个通道的波形；key 不存在时返回"恒定0"（即该通道不发指令）
// is_turn 只影响 multisine 的默认谐波分配：转向=奇次(1/3/5)，前进=偶次(2/4)
WaveformConfig parseWave(const nlohmann::json& j, const char* key, bool is_turn)
{
  WaveformConfig w;
  if (j.contains(key)) {
    const auto& c = j[key];
    w.type   = c.value("type", "const");
    if (w.type == "multisine") {
      // 未显式给参数时的默认激励：幅值取 500~2500 指令单位，
      // 偶次谐波相位默认用离线寻优结果（轨迹近似闭合、外接框方正）
      if (is_turn) { w.A1 = 2500.0; w.A3 = 500.0; w.A5 = 500.0; }
      else         { w.offset = 2500.0; w.A2 = 1000.0; w.A4 = 500.0; }
    }
    w.A      = c.value("A", 0.0);
    w.B      = c.value("B", 1.0);
    w.T      = c.value("T", 0.0);
    w.offset = c.value("offset", w.offset);
    w.Tr     = c.value("Tr", 2.0);
    w.Th     = c.value("Th", 2.0);
    w.Tz     = c.value("Tz", 2.0);
    w.T0     = c.value("T0", 60.0);
    w.fc     = c.value("fc", 50.0);
    w.pZero  = c.value("P_ZERO", 0.10);
    w.pPeak  = c.value("P_PEAK", 0.12);
    w.A1     = c.value("A1", w.A1);
    w.A3     = c.value("A3", w.A3);
    w.A5     = c.value("A5", w.A5);
    w.A2     = c.value("A2", w.A2);
    w.A4     = c.value("A4", w.A4);
    w.P2     = c.value("P2", 4.8860);
    w.P4     = c.value("P4", 4.5729);
  }
  return w;
}

// CSV 记录器：所有文件操作封装在这里，节点主体不关心文件细节
class CsvLogger
{
public:
  // 开始一段新试验：同一天同一类测试共用一个文件 {dir}/{base}_{YYYYMMDD}.csv，
  // 在文件里追加一段新数据，用 run_id（HHMMSS）区分不同试验段
  void openRun(const std::string& dir, const std::string& base)
  {
    close();
    ::mkdir(dir.c_str(), 0755);            // 目录已存在会返回错误，忽略即可
    path_ = dir + "/" + base + "_" + dayStamp() + ".csv";
    bool exists = fileExists(path_);
    ofs_.open(path_, std::ios::out | std::ios::app);   // 追加，不覆盖当天已有数据
    if (!ofs_) {
      ROS_ERROR("无法创建文件: %s", path_.c_str());
      return;
    }
    if (!exists) ofs_ << kCsvHeader << "\n";           // 只有新文件才写表头
    run_id_ = timeStamp();
    rows_ = 0;
    ROS_INFO("开始记录: %s (run=%s)", path_.c_str(), run_id_.c_str());
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

  const std::string& runId() const { return run_id_; }

private:
  static bool fileExists(const std::string& path)
  {
    std::ifstream f(path.c_str());
    return f.good();
  }

  static std::string dayStamp()            // 20260706：文件名用的日期
  {
    std::time_t t = std::time(nullptr);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y%m%d", std::localtime(&t));
    return buf;
  }

  static std::string timeStamp()           // 153012：run_id 用的时刻
  {
    std::time_t t = std::time(nullptr);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%H%M%S", std::localtime(&t));
    return buf;
  }

  std::ofstream ofs_;
  std::string path_;
  std::string run_id_;
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

    ::mkdir(output_dir_.c_str(), 0755);    // 提前建好根目录（openRun 只建子目录）

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

  void gpsCb(const util::GpsPosition::ConstPtr& msg)
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
    drive_cfg_ = parseWave(j, "drive", false);    // 前进通道波形
    turn_cfg_  = parseWave(j, "turn", true);      // 转向通道波形
    t0_        = ros::Time::now();            // 新试验 t_run 从 0 开始
    run_id_.clear();

    std::string subdir = modeToDir(mode_);
    if (!subdir.empty()) {
      // 文件名按"测试类型_日期"：当天同类测试的数据都进同一个文件
      logger_.openRun(output_dir_ + "/" + subdir, subdir);
      run_id_ = logger_.runId();
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
       << ros::Time::now().toSec() << ',' << t_run << ',' << run_id_ << ','
       << mode_ << ','
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
  std::string run_id_;              // 本段试验在同日文件里的编号（HHMMSS）
  util::GpsPosition gps_;
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
