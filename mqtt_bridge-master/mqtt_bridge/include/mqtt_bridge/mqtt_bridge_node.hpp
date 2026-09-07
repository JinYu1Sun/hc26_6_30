#ifndef MQTT_BRIDGE__MQTT_BRIDGE_NODE_HPP_
#define MQTT_BRIDGE__MQTT_BRIDGE_NODE_HPP_

#include <memory>
#include <string>
#include <mutex>
#include <atomic>
#include <vector>
#include <map>
#include <thread>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_msgs/msg/string.hpp"
#include "angmen_ad_msgs/msg/car_status.hpp"
#include "angmen_ad_msgs/srv/common_request.hpp"
#include "angmen_ad_msgs/srv/mission_request.hpp"
#include "angmen_ad_msgs/srv/mode_change_request.hpp"
#include "angmen_ad_msgs/srv/path_planning_request.hpp"
#include "angmen_ad_msgs/srv/remote_control.hpp"
#include "angmen_ad_msgs/srv/video_streaming_request.hpp"
#include "angmen_ad_msgs/srv/file_transfer_request.hpp"
#include "angmen_ad_msgs/srv/route_query_request.hpp"
#include "angmen_ad_msgs/msg/remote_command.hpp"
#include "angmen_ad_msgs/msg/remote_report.hpp"
#include "angmen_ad_msgs/msg/remote_status.hpp"
#include "angmen_ad_msgs/msg/car_event.hpp"
#include "angmen_ad_msgs/msg/takeover_request.hpp"
#include "mqtt_client.hpp"
#include "config_manager.hpp"

// 前向声明 cbor 类型
struct cbor_item_t;

namespace mqtt_bridge
{

// 车辆状态数据结构
struct VehicleState
{
  std::string vin;
  double longitude = 0.0;
  double latitude = 0.0;
  double altitude = 0.0;
  double heading = 0.0;
  double roll = 0.0;
  double pitch = 0.0;
  int gnss_status = 0;
  int gear = 0;
  double speed = 0.0;
  double brake = 0.0;
  double steer = 0.0;
  int door = 0;
  int parking_brake = 0;
  int turn_signal = 0;
  int low_beam = 0;
  int high_beam = 0;
  int position_light = 0;
  double odometer = 0.0;
  double trip_odometer = 0.0;
  int estimated_range = 0;
  double power_battery_soc = 0.0;
  double low_voltage_battery_voltage = 0.0;
  int init_status = 0;
  int vehicle_mode_status = 0;
  int mission_status = 0;
  int positioning_status = 0;
  uint32_t fault_status = 0;
  std::string current_mission_id;
};

class MQTTBridgeNode : public rclcpp::Node
{
public:
  explicit MQTTBridgeNode(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
  ~MQTTBridgeNode();

private:
  // 初始化方法
  void initVehicleState();
  void setupMQTTClient();
  void setupTopicMappings();
  void setupRemoteDrivingTopics(const std::map<std::string, TopicMapping>& topic_map);
  void setupServiceTopics(const std::map<std::string, TopicMapping>& topic_map);
  void setupCarStatusTimer();
  void setupMonitoringTimer();

  // 回调方法
  void onMQTTMessage(const std::string& topic, const std::string& payload);
  void onConnectionChange(bool connected);
  void onCarStatusReceived(const angmen_ad_msgs::msg::CarStatus::SharedPtr msg);
  
  // 车辆状态发送
  void sendCarStatus();
  std::vector<uint8_t> encodeCarStatusToCBOR(const angmen_ad_msgs::msg::CarStatus& status);
  
  // CommonRequest/Response处理
  void handleCommonRequest(const std::string& payload);
  void callROS2Service(uint8_t ver, uint32_t request_type,
                      const std::vector<uint8_t>& request_id,
                      const std::vector<uint8_t>& request_payload_bytes);
  void sendCommonResponse(uint8_t ver, uint32_t request_type,
                         const std::vector<uint8_t>& request_id,
                         const std::vector<uint8_t>& response_payload_bytes);

  // 发送错误响应辅助函数
  void sendErrorResponse(uint8_t ver, uint32_t request_type,
                         const std::vector<uint8_t>& request_id,
                         const std::string& error_message);
  
  // MissionRequest/Response处理
  void handleMissionRequest(uint8_t ver, uint32_t request_type, const std::vector<uint8_t>& request_id, const std::string& payload);
  void callMissionService(uint8_t ver, uint32_t request_type, const std::vector<uint8_t>& request_id, double timestamp,
                          const std::vector<uint8_t>& mission_id,
                          double longitude, double latitude, double heading);
  
  // ModeChangeRequest/Response处理
  void handleModeChangeRequest(uint8_t ver, uint32_t request_type, const std::vector<uint8_t>& request_id, const std::string& payload);
  void callModeChangeService(uint8_t ver, uint32_t request_type, const std::vector<uint8_t>& request_id, double timestamp, uint32_t mode, const std::string& session_id);
  
  // RemoteControl处理
  void initRemoteDriving();
  void handleRemoteControl(const std::string& payload);
  void callRemoteControlService(uint32_t control_type, const std::string& operator_id,
                                const std::string& session_id, const std::string& vehicle_id);
  void sendRemoteControlResponse(bool success, const std::string& message, const std::string& session_id = "");

  // RemoteCommand处理 (MQTT -> ROS2 Topic发布)
  void handleRemoteCommand(const std::string& payload);
  
  // RemoteReport处理 (ROS2 Topic订阅 -> MQTT发布)
  void onRemoteReportReceived(const angmen_ad_msgs::msg::RemoteReport::SharedPtr msg);
  
  // RemoteStatus处理 (ROS2 Topic订阅 -> MQTT发布)
  void onRemoteStatusReceived(const angmen_ad_msgs::msg::RemoteStatus::SharedPtr msg);
  
  // VideoStreamingRequest处理
  void handleVideoStreamingRequest(uint8_t ver, uint32_t request_type,
                                   const std::vector<uint8_t>& request_id,
                                   const std::string& payload);
  void callVideoStreamingService(uint8_t ver, uint32_t request_type,
                                 const std::vector<uint8_t>& request_id,
                                 double timestamp, const std::string& vin,
                                 const std::vector<uint32_t>& cam_id, uint32_t behavior,
                                 const std::vector<std::string>& rtp_ip,
                                 const std::vector<uint32_t>& rtp_port);
  // FileTransferRequest处理
  void handleFileTransferRequest(uint8_t ver, uint32_t request_type,
                                 const std::vector<uint8_t>& request_id,
                                 const std::string& payload);
  void callFileTransferService(uint8_t ver, uint32_t request_type,
                               const std::vector<uint8_t>& request_id,
                               double timestamp, const std::string& task_id, uint32_t action,
                               uint32_t file_type, const std::string& file_name,
                               uint64_t size_bytes, const std::string& checksum,
                               const std::string& description, const std::string& download_url,
                               uint32_t expiry_seconds, const std::string& content_type);
  // RouteQueryRequest处理
  void handleRouteQueryRequest(uint8_t ver, uint32_t request_type,
                               const std::vector<uint8_t>& request_id,
                               const std::string& payload);
  void callRouteQueryService(uint8_t ver, uint32_t request_type,
                             const std::vector<uint8_t>& request_id,
                             double timestamp, uint32_t request);
  void handlePathPlanningRequest(uint8_t ver, uint32_t request_type,
                                 const std::vector<uint8_t>& request_id,
                                 const std::string& payload);
  void callPathPlanningService(uint8_t ver, uint32_t request_type,
                               const std::vector<uint8_t>& request_id,
                               double timestamp, const std::vector<double>& longitude,
                               const std::vector<double>& latitude);
  // CarEvent处理 (ROS2 Topic订阅 -> MQTT发布)
  void onCarEventReceived(const angmen_ad_msgs::msg::CarEvent::SharedPtr msg);
  void onTakeoverRequestReceived(const angmen_ad_msgs::msg::TakeoverRequest::SharedPtr msg);
  
  // VIN 校验
  bool validateVIN(const std::string& vin) const;
  
  // CBOR辅助函数
  
  // 统计信息
  void publishStatistics();

  // 成员变量
  std::unique_ptr<ConfigManager> config_manager_;
  std::unique_ptr<MQTTClient> mqtt_client_;

  // ROS2发布者和订阅者
  rclcpp::Subscription<angmen_ad_msgs::msg::CarStatus>::SharedPtr car_status_sub_;
  rclcpp::Client<angmen_ad_msgs::srv::CommonRequest>::SharedPtr common_service_client_;
  rclcpp::Client<angmen_ad_msgs::srv::MissionRequest>::SharedPtr mission_service_client_;
  rclcpp::Client<angmen_ad_msgs::srv::ModeChangeRequest>::SharedPtr mode_change_service_client_;
  
  // 远程驾驶相关
  rclcpp::Client<angmen_ad_msgs::srv::RemoteControl>::SharedPtr remote_control_service_client_;
  rclcpp::Publisher<angmen_ad_msgs::msg::RemoteCommand>::SharedPtr remote_command_pub_;
  rclcpp::Subscription<angmen_ad_msgs::msg::RemoteReport>::SharedPtr remote_report_sub_;
  rclcpp::Subscription<angmen_ad_msgs::msg::RemoteStatus>::SharedPtr remote_status_sub_;
  
  // 视频和文件
  rclcpp::Client<angmen_ad_msgs::srv::VideoStreamingRequest>::SharedPtr video_streaming_service_client_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr video_stream_state_pub_;
  rclcpp::Client<angmen_ad_msgs::srv::FileTransferRequest>::SharedPtr file_transfer_service_client_;
  
  // 路径查询
  rclcpp::Client<angmen_ad_msgs::srv::RouteQueryRequest>::SharedPtr route_query_service_client_;
  rclcpp::Client<angmen_ad_msgs::srv::PathPlanningRequest>::SharedPtr path_planning_service_client_;
  
  // 事件上报
  rclcpp::Subscription<angmen_ad_msgs::msg::CarEvent>::SharedPtr car_event_sub_;
  rclcpp::Subscription<angmen_ad_msgs::msg::TakeoverRequest>::SharedPtr takeover_request_sub_;

  // 定时器
  rclcpp::TimerBase::SharedPtr car_status_timer_;
  rclcpp::TimerBase::SharedPtr monitor_timer_;

  // 车辆状态数据
  VehicleState vehicle_state_;
  angmen_ad_msgs::msg::CarStatus latest_car_status_;
  std::mutex state_mutex_;
  
  // MQTT topic配置
  std::string car_status_mqtt_topic_;
  std::string common_request_mqtt_topic_;
  std::string common_response_mqtt_topic_;
  std::string remote_control_request_mqtt_topic_;
  std::string remote_control_response_mqtt_topic_;
  std::string remote_command_mqtt_topic_;
  std::string remote_report_mqtt_topic_;
  std::string remote_status_mqtt_topic_;
  std::string car_event_mqtt_topic_;
  std::string takeover_request_mqtt_topic_;
  int car_status_qos_ = 0;
  int car_event_qos_ = 2;
  
  // 统计信息
  std::atomic<uint64_t> messages_sent_{0};
  std::atomic<uint64_t> messages_received_{0};
  std::atomic<uint64_t> connection_lost_count_{0};
  std::atomic<uint64_t> car_status_sent_count_{0};
  std::atomic<uint64_t> car_status_failed_count_{0};
  std::atomic<uint64_t> remote_status_sent_count_{0};
  std::atomic<uint64_t> remote_status_failed_count_{0};
  std::chrono::steady_clock::time_point start_time_;
  
  // 序列号
  std::atomic<uint32_t> car_status_seq_{0};
};

}  // namespace mqtt_bridge

#endif  // MQTT_BRIDGE__MQTT_BRIDGE_NODE_HPP_
