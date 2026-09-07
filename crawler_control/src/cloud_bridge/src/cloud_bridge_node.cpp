#include <ros/ros.h>
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
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "cloud_bridge/mqtt_client.h"

namespace cloud_bridge
{

// ---------------- SRT 视频推流器 ----------------
// 订阅到的图像帧经缩放后以 rawvideo 写入 ffmpeg 子进程 stdin，
// ffmpeg 编码 H.264 后以 SRT caller 模式推送到云平台。
struct VideoCfg
{
  bool enable = false;
  double fps = 5.0;
  int width = 640;
  int height = 480;
  int bitrate = 800;  // kbps
};

class SrtStreamer
{
public:
  SrtStreamer() = default;
  ~SrtStreamer() { stopLocked(); }

  void setTarget(const std::string& target) { target_ = target; }

  // 参数变化时按需重启 ffmpeg
  void configure(const VideoCfg& cfg)
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
  }

  // 推入一帧 BGR 图像（必须已缩放到 cfg_.width x cfg_.height）
  void push(const cv::Mat& bgr)
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
  }

private:
  void startLocked()
  {
    if (target_.empty())
    {
      ROS_ERROR("srt_target not set, video streaming disabled");
      return;
    }
    int fds[2];
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
    child_pid_ = pid;
    ROS_INFO("SRT streaming started: %s %dx%d@%.1ffps %s", target_.c_str(),
             cfg_.width, cfg_.height, cfg_.fps, bitrate.c_str());
  }

  void stopLocked()
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
  }

  VideoCfg cfg_;
  std::string target_;
  int pipe_fd_ = -1;
  pid_t child_pid_ = -1;
  double last_restart_time_ = 0.0;
  std::mutex mutex_;
};

// ---------------- 主节点 ----------------

class CloudBridgeNode
{
public:
  CloudBridgeNode() : pnh_("~")
  {
    loadParams();
    streamer_.setTarget(srt_target_);
    streamer_.configure(video_cfg_);  // launch 里 video_enable=true 时立即起流

    pub_manual_ = nh_.advertise<mower_msgs::Manual_Driving_Cmd>(
        "/mower/manual_driving_cmd", 1);
    pub_signal_ = nh_.advertise<std_msgs::String>("/signal", 10);

    sub_position_ = nh_.subscribe("/Mower/position", 1,
                                  &CloudBridgeNode::positionCb, this);
    sub_vehicle_status_ = nh_.subscribe("/vehicle/status", 1,
                                        &CloudBridgeNode::vehicleStatusCb, this);
    sub_left_wheel_ = nh_.subscribe("/vehicle/left_wheel_speed", 1,
                                    &CloudBridgeNode::leftWheelCb, this);
    sub_right_wheel_ = nh_.subscribe("/vehicle/right_wheel_speed", 1,
                                     &CloudBridgeNode::rightWheelCb, this);
    sub_mower_height_ = nh_.subscribe("/vehicle/mower_height_to_app", 1,
                                      &CloudBridgeNode::mowerHeightCb, this);
    sub_image_ = nh_.subscribe(image_topic_, 1, &CloudBridgeNode::imageCb, this);

    const std::string prefix = topic_prefix_ + "/" + device_id_;
    topic_cmd_move_ = prefix + "/cmd/move";
    topic_cmd_blade_ = prefix + "/cmd/blade";
    topic_cmd_task_ = prefix + "/cmd/task";
    topic_cmd_video_ = prefix + "/cmd/video";
    topic_state_location_ = prefix + "/state/location";
    topic_state_vehicle_ = prefix + "/state/vehicle";

    mqtt_ = std::unique_ptr<MqttClient>(new MqttClient(mqtt_cfg_));
    mqtt_->setMessageCallback(
        [this](const std::string& topic, const std::string& payload) {
          onMqttMessage(topic, payload);
        });
    mqtt_->addSubscription(topic_cmd_move_, 1);
    mqtt_->addSubscription(topic_cmd_blade_, 1);
    mqtt_->addSubscription(topic_cmd_task_, 1);
    mqtt_->addSubscription(topic_cmd_video_, 1);

    if (!mqtt_->start())
      ROS_WARN("MQTT first connect failed, retrying in background");

    location_timer_ = nh_.createTimer(
        ros::Duration(1.0 / location_hz_), &CloudBridgeNode::locationTimerCb, this);
    status_timer_ = nh_.createTimer(
        ros::Duration(1.0 / status_hz_), &CloudBridgeNode::statusTimerCb, this);
    watchdog_timer_ = nh_.createTimer(
        ros::Duration(0.1), &CloudBridgeNode::watchdogTimerCb, this);

    ROS_INFO("cloud_bridge started, broker=%s:%d device_id=%s",
             mqtt_cfg_.host.c_str(), mqtt_cfg_.port, device_id_.c_str());
  }

  void stop() { mqtt_->stop(); }

private:
  void loadParams()
  {
    pnh_.param<std::string>("broker_host", mqtt_cfg_.host, "127.0.0.1");
    pnh_.param("broker_port", mqtt_cfg_.port, 1883);
    pnh_.param<std::string>("client_id", mqtt_cfg_.client_id, "cloud_bridge");
    pnh_.param<std::string>("username", mqtt_cfg_.username, "");
    pnh_.param<std::string>("password", mqtt_cfg_.password, "");
    pnh_.param("keep_alive_interval", mqtt_cfg_.keep_alive_interval, 30);
    pnh_.param("connect_timeout", mqtt_cfg_.connect_timeout, 5);

    pnh_.param<std::string>("device_id", device_id_, "mower_001");
    pnh_.param<std::string>("topic_prefix", topic_prefix_, "mower");

    pnh_.param("location_hz", location_hz_, 2.0);
    pnh_.param("status_hz", status_hz_, 1.0);
    pnh_.param<std::string>("image_topic", image_topic_,
                            std::string("/camera/image_rect"));

    pnh_.param<std::string>("srt_target", srt_target_, std::string(""));
    pnh_.param("video_enable", video_cfg_.enable, false);
    pnh_.param("video_fps", video_cfg_.fps, 5.0);
    pnh_.param("video_width", video_cfg_.width, 640);
    pnh_.param("video_height", video_cfg_.height, 480);
    pnh_.param("video_bitrate", video_cfg_.bitrate, 800);

    pnh_.param("cmd_timeout", cmd_timeout_, 0.5);
    pnh_.param("drive_max", drive_max_, 10000);
    pnh_.param("turn_max", turn_max_, 12566);
    pnh_.param("auto_pause_on_manual", auto_pause_on_manual_, true);
    pnh_.param("task_step_delay", task_step_delay_, 0.5);
    pnh_.param("task_stop_delay", task_stop_delay_, 2.0);
    int mow_height = mow_height_.load();
    pnh_.param("mow_height", mow_height, 6);
    mow_height_ = mow_height;
  }

  // ---------------- MQTT 下行指令 ----------------

  void onMqttMessage(const std::string& topic, const std::string& payload)
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
  }

  static double clamp1(double v) { return std::max(-1.0, std::min(1.0, v)); }

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
  }

  void handleBladeCmd(const nlohmann::json& j)
  {
    const int state = j.value("state", 0);
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
    }
    ROS_INFO("blade %s, height=%d", blade_on_.load() ? "ON" : "OFF",
             mow_height_.load());
  }

  void handleTaskCmd(const nlohmann::json& j)
  {
    const std::string action = j.value("action", "");

    if (action == "start")
    {
      const std::string map_name = j.value("map_name", "");
      const std::string map_mode = j.value("map_mode", "");
      startTaskSequence(map_name, map_mode);
    }
    else if (action == "stop")
    {
      task_running_ = false;
      sendSignal("stop");
      std::thread([this]() {
        ros::Duration(task_stop_delay_).sleep();
        sendSignal("stop_execution");
      }).detach();
      ROS_INFO("auto task stop requested");
    }
    else if (action == "pause")
    {
      task_running_ = false;
      sendSignal("pause");
    }
    else if (action == "continue")
    {
      task_running_ = true;
      sendSignal("continue");
    }
    else
    {
      ROS_WARN("unknown task action: %s", action.c_str());
    }
  }

  void startTaskSequence(const std::string& map_name, const std::string& map_mode)
  {
    if (task_seq_running_.exchange(true))
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
      if (map_mode == "single_map" || map_mode == "multi_map")
      {
        sendSignal(map_mode);
        ros::Duration(task_step_delay_).sleep();
      }
      sendSignal("start_work");
      task_seq_running_ = false;
      ROS_INFO("auto task start sequence done (map=%s mode=%s)",
               map_name.c_str(), map_mode.c_str());
    }).detach();
  }

  void handleVideoCmd(const nlohmann::json& j)
  {
    VideoCfg cfg;
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
    if (j.contains("width"))
      cfg.width = std::max(16, std::min(1920, j["width"].get<int>()));
    if (j.contains("height"))
      cfg.height = std::max(16, std::min(1080, j["height"].get<int>()));
    if (j.contains("bitrate"))
      cfg.bitrate = std::max(100, std::min(8000, j["bitrate"].get<int>()));
    {
      std::lock_guard<std::mutex> lock(video_mutex_);
      video_cfg_ = cfg;
    }
    streamer_.configure(cfg);
    ROS_INFO("video cfg: enable=%d fps=%.1f %dx%d bitrate=%dkbps", cfg.enable,
             cfg.fps, cfg.width, cfg.height, cfg.bitrate);
  }

  // ---------------- ROS 侧收发 ----------------

  void publishManualCmd(double linear, double angular)
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
  }

  void sendSignal(const std::string& s)
  {
    std_msgs::String msg;
    msg.data = s;
    pub_signal_.publish(msg);
  }

  void watchdogTimerCb(const ros::TimerEvent&)
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
  }

  void positionCb(const util::Position::ConstPtr& msg)
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
        {"z", pos.position_z},     {"roll", pos.roll},
        {"pitch", pos.pitch},      {"yaw", pos.yaw},
        {"state", pos.position_state},
        {"stamp", ros::Time::now().toSec()}};
    mqtt_->publish(topic_state_location_, j.dump(), 0);
  }

  void vehicleStatusCb(const mower_msgs::VehicleStatus::ConstPtr& msg)
  {
    std::lock_guard<std::mutex> lock(status_mutex_);
    battery_soc_ = msg->battery_soc;
    warning_one_ = msg->warning_state_one;
    warning_two_ = msg->warning_state_two;
    has_status_ = true;
  }

  void leftWheelCb(const std_msgs::Int16::ConstPtr& msg)
  {
    std::lock_guard<std::mutex> lock(status_mutex_);
    left_wheel_ = msg->data;
  }

  void rightWheelCb(const std_msgs::Int16::ConstPtr& msg)
  {
    std::lock_guard<std::mutex> lock(status_mutex_);
    right_wheel_ = msg->data;
  }

  void mowerHeightCb(const std_msgs::UInt16::ConstPtr& msg)
  {
    std::lock_guard<std::mutex> lock(status_mutex_);
    mower_height_fb_ = msg->data;
  }

  void statusTimerCb(const ros::TimerEvent&)
  {
    nlohmann::json j;
    {
      std::lock_guard<std::mutex> lock(status_mutex_);
      if (!has_status_)
        return;
      j = {{"battery_soc", battery_soc_},
           {"warning_state_one", warning_one_},
           {"warning_state_two", warning_two_},
           {"left_wheel_speed", left_wheel_},
           {"right_wheel_speed", right_wheel_},
           {"mower_height", mower_height_fb_}};
    }
    j["stamp"] = ros::Time::now().toSec();
    mqtt_->publish(topic_state_vehicle_, j.dump(), 0);
  }

  void imageCb(const sensor_msgs::ImageConstPtr& msg)
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
  }

  // ---------------- 成员 ----------------

  ros::NodeHandle nh_;
  ros::NodeHandle pnh_;

  ros::Publisher pub_manual_;
  ros::Publisher pub_signal_;
  ros::Subscriber sub_position_, sub_vehicle_status_, sub_left_wheel_;
  ros::Subscriber sub_right_wheel_, sub_mower_height_, sub_image_;
  ros::Timer location_timer_, status_timer_, watchdog_timer_;

  MqttConfig mqtt_cfg_;
  std::unique_ptr<MqttClient> mqtt_;
  std::string device_id_, topic_prefix_;
  std::string topic_cmd_move_, topic_cmd_blade_, topic_cmd_task_, topic_cmd_video_;
  std::string topic_state_location_, topic_state_vehicle_;

  double location_hz_ = 2.0, status_hz_ = 1.0;
  std::string image_topic_;

  double cmd_timeout_ = 0.5;
  int drive_max_ = 10000, turn_max_ = 12566;
  bool auto_pause_on_manual_ = true;
  double task_step_delay_ = 0.5, task_stop_delay_ = 2.0;
  // 跨线程访问：MQTT 回调线程写，ROS 定时器线程读
  std::atomic<int> mow_height_{6};
  std::atomic<bool> blade_on_{false};

  std::mutex motion_mutex_;
  double last_linear_ = 0.0, last_angular_ = 0.0;
  ros::Time last_move_time_;

  std::atomic<bool> task_running_{false};
  std::atomic<bool> task_seq_running_{false};

  std::mutex position_mutex_;
  util::Position latest_position_;
  bool has_position_ = false;

  std::mutex status_mutex_;
  bool has_status_ = false;
  int battery_soc_ = 0, warning_one_ = 0, warning_two_ = 0;
  int left_wheel_ = 0, right_wheel_ = 0, mower_height_fb_ = 0;

  std::string srt_target_;
  VideoCfg video_cfg_;
  std::mutex video_mutex_;
  ros::Time last_frame_time_;
  SrtStreamer streamer_;
};

}  // namespace cloud_bridge

int main(int argc, char** argv)
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
}
