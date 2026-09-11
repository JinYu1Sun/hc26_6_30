#include <ros/ros.h>
#include <std_msgs/String.h>
#include <std_msgs/UInt16.h>

#include <mower_msgs/Manual_Driving_Cmd.h>
#include <mower_msgs/Position.h>
#include <mower_msgs/VehicleStatus.h>

#include <nlohmann/json.hpp>

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

// ---------------- 主节点 ----------------

class CloudBridgeNode
{
public:
  CloudBridgeNode() : pnh_("~")
  {
    loadParams();

    pub_manual_ = nh_.advertise<mower_msgs::Manual_Driving_Cmd>(
        "/mower/manual_driving_cmd", 1);
    pub_signal_ = nh_.advertise<std_msgs::String>("/signal", 10);

    sub_position_ = nh_.subscribe("/Mower/position", 1,
                                  &CloudBridgeNode::positionCb, this);
    sub_vehicle_status_ = nh_.subscribe("/vehicle/status", 1,
                                        &CloudBridgeNode::vehicleStatusCb, this);
    sub_mower_height_ = nh_.subscribe("/vehicle/mower_height_to_app", 1,
                                      &CloudBridgeNode::mowerHeightCb, this);

    const std::string prefix = topic_prefix_ + "/" + device_id_;
    topic_cmd_move_ = prefix + "/cmd/move";
    topic_cmd_blade_ = prefix + "/cmd/blade";
    topic_cmd_task_ = prefix + "/cmd/task";
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

  void positionCb(const mower_msgs::Position::ConstPtr& msg)
  {
    std::lock_guard<std::mutex> lock(position_mutex_);
    latest_position_ = *msg;
    has_position_ = true;
  }

  void locationTimerCb(const ros::TimerEvent&)
  {
    mower_msgs::Position pos;
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
           {"mower_height", mower_height_fb_}};
    }
    j["stamp"] = ros::Time::now().toSec();
    mqtt_->publish(topic_state_vehicle_, j.dump(), 0);
  }

  // ---------------- 成员 ----------------

  ros::NodeHandle nh_;
  ros::NodeHandle pnh_;

  ros::Publisher pub_manual_;
  ros::Publisher pub_signal_;
  ros::Subscriber sub_position_, sub_vehicle_status_, sub_mower_height_;
  ros::Timer location_timer_, status_timer_, watchdog_timer_;

  MqttConfig mqtt_cfg_;
  std::unique_ptr<MqttClient> mqtt_;
  std::string device_id_, topic_prefix_;
  std::string topic_cmd_move_, topic_cmd_blade_, topic_cmd_task_;
  std::string topic_state_location_, topic_state_vehicle_;

  double location_hz_ = 2.0, status_hz_ = 1.0;

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
  mower_msgs::Position latest_position_;
  bool has_position_ = false;

  std::mutex status_mutex_;
  bool has_status_ = false;
  int battery_soc_ = 0, warning_one_ = 0, warning_two_ = 0;
  int mower_height_fb_ = 0;
};

}  // namespace cloud_bridge

int main(int argc, char** argv)
{
  ros::init(argc, argv, "cloud_bridge");
  cloud_bridge::CloudBridgeNode node;
  ros::AsyncSpinner spinner(2);
  spinner.start();
  ros::waitForShutdown();
  node.stop();
  return 0;
}
