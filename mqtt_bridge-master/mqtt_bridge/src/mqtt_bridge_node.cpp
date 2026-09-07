#include "mqtt_bridge/mqtt_bridge_node.hpp"
#include "mqtt_bridge/coordinate_converter.hpp"
#include "angmen_ad_msgs/msg/car_status.hpp"
#include <chrono>
#include <functional>
#include <nlohmann/json.hpp>
#include <random>
#include <stdexcept>

using json = nlohmann::json;
using namespace std::chrono_literals;

namespace mqtt_bridge
{

// 辅助函数：将16字节UUID转为字符串格式 xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
static std::string uuidBytesToString(const std::vector<uint8_t>& bytes)
{
  char uuid_str[37];
  snprintf(uuid_str, sizeof(uuid_str),
    "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    bytes[0], bytes[1], bytes[2], bytes[3],
    bytes[4], bytes[5], bytes[6], bytes[7],
    bytes[8], bytes[9], bytes[10], bytes[11],
    bytes[12], bytes[13], bytes[14], bytes[15]);
  return std::string(uuid_str);
}

static std::string generateUuidString()
{
  std::vector<uint8_t> bytes(16);
  std::random_device random;
  for (auto& value : bytes) {
    value = static_cast<uint8_t>(random());
  }
  bytes[6] = static_cast<uint8_t>((bytes[6] & 0x0f) | 0x40);
  bytes[8] = static_cast<uint8_t>((bytes[8] & 0x3f) | 0x80);
  return uuidBytesToString(bytes);
}

// VIN 校验：检查消息中的 VIN 是否与本地车辆 VIN 一致
bool MQTTBridgeNode::validateVIN(const std::string& vin) const
{
  const std::string& local_vin = config_manager_->getVIN();
  if (vin.empty()) {
    RCLCPP_WARN(this->get_logger(), "VIN校验失败: 消息中VIN为空");
    return false;
  }
  if (vin != local_vin) {
    RCLCPP_WARN(this->get_logger(), "VIN校验失败: 收到VIN=%s, 本地VIN=%s",
      vin.c_str(), local_vin.c_str());
    return false;
  }
  return true;
}

// 辅助函数：将固定长度数组转为UUID字符串
template<typename T>
static std::string uuidArrayToString(const T& arr)
{
  char uuid_str[37];
  snprintf(uuid_str, sizeof(uuid_str),
    "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    arr[0], arr[1], arr[2], arr[3],
    arr[4], arr[5], arr[6], arr[7],
    arr[8], arr[9], arr[10], arr[11],
    arr[12], arr[13], arr[14], arr[15]);
  return std::string(uuid_str);
}

MQTTBridgeNode::MQTTBridgeNode(const rclcpp::NodeOptions& options)
  : Node("mqtt_bridge_node", options),
    start_time_(std::chrono::steady_clock::now()),
    car_status_seq_(0)
{
  // 声明参数
  this->declare_parameter<std::string>("config_file", "");
  this->declare_parameter<std::string>("vin", "");
  
  std::string config_file;
  this->get_parameter("config_file", config_file);
  this->get_parameter("vin", vehicle_state_.vin);

  // 初始化车辆状态默认值
  initVehicleState();

  // 加载配置
  config_manager_ = std::make_unique<ConfigManager>();
  if (!config_manager_->loadFromFile(config_file)) {
    RCLCPP_ERROR(this->get_logger(), "配置文件加载失败");
    return;
  }
  if (vehicle_state_.vin.empty()) {
    RCLCPP_FATAL(this->get_logger(), "缺少必需的共享车辆VIN参数");
    throw std::invalid_argument("vin parameter is required");
  }
  config_manager_->setVIN(vehicle_state_.vin);
  
  setupMQTTClient();
  setupTopicMappings();
  if (!mqtt_client_->isConnected()) {
    RCLCPP_WARN(this->get_logger(), "MQTT首次连接失败，节点保持运行并在后台自动重连");
    mqtt_client_->startReconnect();
  }
  setupCarStatusTimer();
  setupMonitoringTimer();
  
  // 初始化远程驾驶功能
  initRemoteDriving();

  RCLCPP_INFO(this->get_logger(), "MQTT桥接节点启动成功");
}

MQTTBridgeNode::~MQTTBridgeNode()
{
  if (mqtt_client_) {
    mqtt_client_->disconnect();
  }
}

void MQTTBridgeNode::initVehicleState()
{
  vehicle_state_.longitude = 0.0;
  vehicle_state_.latitude = 0.0;
  vehicle_state_.altitude = 0.0;
  vehicle_state_.heading = 0.0;
  vehicle_state_.roll = 0.0;
  vehicle_state_.pitch = 0.0;
  vehicle_state_.gnss_status = 0;
  vehicle_state_.gear = 0;
  vehicle_state_.speed = 0.0;
  vehicle_state_.brake = 0.0;
  vehicle_state_.steer = 0.0;
  vehicle_state_.door = -1;
  vehicle_state_.parking_brake = -1;
  vehicle_state_.turn_signal = -1;
  vehicle_state_.low_beam = -1;
  vehicle_state_.high_beam = -1;
  vehicle_state_.position_light = -1;
  vehicle_state_.odometer = 0.0;
  vehicle_state_.trip_odometer = 0.0;
  vehicle_state_.estimated_range = 0;
  vehicle_state_.power_battery_soc = 0.0;
  vehicle_state_.low_voltage_battery_voltage = 0.0;
  vehicle_state_.init_status = 0;
  vehicle_state_.vehicle_mode_status = 0;
  vehicle_state_.mission_status = 0;
  vehicle_state_.positioning_status = 0;
  vehicle_state_.fault_status = 0;
  vehicle_state_.current_mission_id = "00000000-0000-0000-0000-000000000000";
}

void MQTTBridgeNode::setupMQTTClient()
{
  auto config = config_manager_->getMQTTConfig();
  config.client_id = config_manager_->getVIN();
  mqtt_client_ = std::make_unique<MQTTClient>(config);

  // 设置车辆配置（用于生成符合协议的话题）
  mqtt_client_->setVehicleConfig(
    config_manager_->getVIN(),
    config_manager_->getProvince(),
    config_manager_->getCity(),
    config_manager_->getZone()
  );

  mqtt_client_->setMessageCallback(
    std::bind(&MQTTBridgeNode::onMQTTMessage, this, std::placeholders::_1, std::placeholders::_2)
  );

  mqtt_client_->setConnectionCallback(
    std::bind(&MQTTBridgeNode::onConnectionChange, this, std::placeholders::_1)
  );

  if (!mqtt_client_->connect()) {
    RCLCPP_ERROR(this->get_logger(), "MQTT连接失败");
  }
}

void MQTTBridgeNode::setupTopicMappings()
{
  // 使用ConfigManager自动生成话题映射
  auto mappings = config_manager_->getTopicMappings();

  // 存储话题映射的查找表
  std::map<std::string, TopicMapping> topic_map;
  for (const auto& mapping : mappings) {
    topic_map[mapping.topic_name] = mapping;
  }

  // 设置车辆状态相关话题
  auto car_status_it = topic_map.find("cabr/j10a/" + config_manager_->getProvince() + "/" +
                                      config_manager_->getCity() + "/" + config_manager_->getZone() + "/" +
                                      config_manager_->getVIN() + "/car_status");
  if (car_status_it != topic_map.end()) {
    car_status_mqtt_topic_ = car_status_it->second.topic_name;
    car_status_qos_ = car_status_it->second.qos;
    RCLCPP_INFO(this->get_logger(), "配置车辆状态发布: %s (QoS: %d)",
      car_status_mqtt_topic_.c_str(), car_status_qos_);

    car_status_sub_ = this->create_subscription<angmen_ad_msgs::msg::CarStatus>(
      "/vehicle/car_status",
      10,
      std::bind(&MQTTBridgeNode::onCarStatusReceived, this, std::placeholders::_1)
    );
    RCLCPP_INFO(this->get_logger(), "订阅ROS2 CarStatus: /vehicle/car_status");
  }

  // 设置远程驾驶相关话题
  setupRemoteDrivingTopics(topic_map);

  // 设置服务相关话题
  setupServiceTopics(topic_map);

  RCLCPP_INFO(this->get_logger(), "MQTT桥接设置完成，共配置 %zu 个话题映射", mappings.size());
}

void MQTTBridgeNode::setupRemoteDrivingTopics(const std::map<std::string, TopicMapping>& topic_map)
{
  // RemoteControl请求话题
  auto remote_control_it = topic_map.find("cabr/j10a/" + config_manager_->getProvince() + "/" +
                                         config_manager_->getCity() + "/" + config_manager_->getZone() + "/" +
                                         config_manager_->getVIN() + "/remote_session_control/request");
  if (remote_control_it != topic_map.end()) {
    remote_control_request_mqtt_topic_ = remote_control_it->second.topic_name;
    if (mqtt_client_->subscribe(remote_control_request_mqtt_topic_, 2)) {
      RCLCPP_INFO(this->get_logger(), "订阅RemoteControl: %s (QoS: 2)",
        remote_control_request_mqtt_topic_.c_str());
    }
  }

  // RemoteControl响应话题
  auto remote_control_response_it = topic_map.find("cabr/j10a/" + config_manager_->getProvince() + "/" +
                                                   config_manager_->getCity() + "/" + config_manager_->getZone() + "/" +
                                                   config_manager_->getVIN() + "/remote_session_control/response");
  if (remote_control_response_it != topic_map.end()) {
    remote_control_response_mqtt_topic_ = remote_control_response_it->second.topic_name;
  }

  // RemoteCommand话题
  auto remote_command_it = topic_map.find("cabr/j10a/" + config_manager_->getProvince() + "/" +
                                         config_manager_->getCity() + "/" + config_manager_->getZone() + "/" +
                                         config_manager_->getVIN() + "/drive_cmd");
  if (remote_command_it != topic_map.end()) {
    remote_command_mqtt_topic_ = remote_command_it->second.topic_name;
    if (mqtt_client_->subscribe(remote_command_mqtt_topic_, 2)) {
      RCLCPP_INFO(this->get_logger(), "订阅RemoteCommand: %s (QoS: 2)",
        remote_command_mqtt_topic_.c_str());
    }
  }

  // RemoteReport话题
  auto remote_report_it = topic_map.find("cabr/j10a/" + config_manager_->getProvince() + "/" +
                                        config_manager_->getCity() + "/" + config_manager_->getZone() + "/" +
                                        config_manager_->getVIN() + "/remote_report");
  if (remote_report_it != topic_map.end()) {
    remote_report_mqtt_topic_ = remote_report_it->second.topic_name;
  }

  // RemoteStatus话题
  auto remote_status_it = topic_map.find("cabr/j10a/" + config_manager_->getProvince() + "/" +
                                        config_manager_->getCity() + "/" + config_manager_->getZone() + "/" +
                                        config_manager_->getVIN() + "/remote_status");
  if (remote_status_it != topic_map.end()) {
    remote_status_mqtt_topic_ = remote_status_it->second.topic_name;
  }
}

void MQTTBridgeNode::setupServiceTopics(const std::map<std::string, TopicMapping>& topic_map)
{
  // CommonRequest/Response
  auto common_request_it = topic_map.find("cabr/j10a/" + config_manager_->getProvince() + "/" +
                                         config_manager_->getCity() + "/" + config_manager_->getZone() + "/" +
                                         config_manager_->getVIN() + "/cloud/request");
  if (common_request_it != topic_map.end()) {
    common_request_mqtt_topic_ = common_request_it->second.topic_name;
    if (mqtt_client_->subscribe(common_request_mqtt_topic_, 2)) {
      RCLCPP_INFO(this->get_logger(), "订阅CommonRequest: %s (QoS: 2)",
        common_request_mqtt_topic_.c_str());
    }
  }

  auto common_response_it = topic_map.find("cabr/j10a/" + config_manager_->getProvince() + "/" +
                                          config_manager_->getCity() + "/" + config_manager_->getZone() + "/" +
                                          config_manager_->getVIN() + "/cloud/response");
  if (common_response_it != topic_map.end()) {
    common_response_mqtt_topic_ = common_response_it->second.topic_name;
  }


  // CarEvent
  auto car_event_it = topic_map.find("cabr/j10a/" + config_manager_->getProvince() + "/" +
                                    config_manager_->getCity() + "/" + config_manager_->getZone() + "/" +
                                    config_manager_->getVIN() + "/car_event");
  if (car_event_it != topic_map.end()) {
    car_event_mqtt_topic_ = car_event_it->second.topic_name;
    car_event_qos_ = car_event_it->second.qos;
    RCLCPP_INFO(this->get_logger(), "配置CarEvent发布: %s (QoS: %d)",
      car_event_mqtt_topic_.c_str(), car_event_qos_);
  }

  auto takeover_request_it = topic_map.find(
    "cabr/j10a/" + config_manager_->getProvince() + "/" +
    config_manager_->getCity() + "/" + config_manager_->getZone() + "/" +
    config_manager_->getVIN() + "/car/request");
  if (takeover_request_it != topic_map.end()) {
    takeover_request_mqtt_topic_ = takeover_request_it->second.topic_name;
    RCLCPP_INFO(this->get_logger(), "配置车辆主动接管发布: %s (QoS: 2)",
      takeover_request_mqtt_topic_.c_str());
  }
}

void MQTTBridgeNode::setupCarStatusTimer()
{
  // 定时发送车辆状态（如果没有收到ROS2消息，发送缓存的数据）
  car_status_timer_ = this->create_wall_timer(
    1s,
    [this]() {
      sendCarStatus();
    }
  );
}

void MQTTBridgeNode::setupMonitoringTimer()
{
  monitor_timer_ = this->create_wall_timer(
    1s,
    [this]() {
      publishStatistics();
    }
  );
}

void MQTTBridgeNode::onMQTTMessage(const std::string& topic, const std::string& payload)
{
  messages_received_++;
  
  // 检查是否是CommonRequest
  if (topic == common_request_mqtt_topic_) {
    handleCommonRequest(payload);
  } 
  // 远程驾驶相关消息处理
  else if (topic == remote_control_request_mqtt_topic_) {
    handleRemoteControl(payload);
  }
  else if (topic == remote_command_mqtt_topic_) {
    handleRemoteCommand(payload);
  }
  else {
    RCLCPP_DEBUG(this->get_logger(), "收到未处理的MQTT消息 [%s]: %zu bytes", 
      topic.c_str(), payload.size());
  }
}

void MQTTBridgeNode::onConnectionChange(bool connected)
{
  if (connected) {
    RCLCPP_INFO(this->get_logger(), "MQTT已连接");
  } else {
    RCLCPP_WARN(this->get_logger(), "MQTT连接断开");
    connection_lost_count_++;
  }
}

void MQTTBridgeNode::publishStatistics()
{
  auto now = std::chrono::steady_clock::now();
  auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_).count();
  
  RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 10000,
    "[MQTT统计] 上行=%lu 下行=%lu 断线=%lu CarStatus成功/失败=%lu/%lu "
    "RemoteStatus成功/失败=%lu/%lu 运行=%lds",
    messages_sent_.load(), messages_received_.load(), 
    connection_lost_count_.load(), car_status_sent_count_.load(),
    car_status_failed_count_.load(), remote_status_sent_count_.load(),
    remote_status_failed_count_.load(), uptime);
}

void MQTTBridgeNode::onCarStatusReceived(const angmen_ad_msgs::msg::CarStatus::SharedPtr msg)
{
  // 更新最新的车辆状态
  latest_car_status_ = *msg;
  
  RCLCPP_DEBUG(this->get_logger(), 
    "收到CarStatus: speed=%.2f, gear=%u", msg->speed, msg->gear);
}

void MQTTBridgeNode::sendCarStatus()
{
  if (car_status_mqtt_topic_.empty()) {
    car_status_failed_count_++;
    RCLCPP_ERROR_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
      "[MQTT发送失败] CarStatus topic未配置，累计失败=%lu",
      car_status_failed_count_.load());
    return;
  }
  
  std::vector<uint8_t> cbor_payload;
  try {
    cbor_payload = encodeCarStatusToCBOR(latest_car_status_);
  } catch (const std::exception& e) {
    car_status_failed_count_++;
    RCLCPP_ERROR_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
      "[MQTT发送失败] CarStatus编码失败: %s，累计失败=%lu",
      e.what(), car_status_failed_count_.load());
    return;
  }
  std::string payload_str(cbor_payload.begin(), cbor_payload.end());
  
  // 发送到MQTT
  if (mqtt_client_->publish(car_status_mqtt_topic_, payload_str, car_status_qos_)) {
    messages_sent_++;
    car_status_sent_count_++;
    RCLCPP_DEBUG(this->get_logger(), 
      "[MQTT发送] CarStatus已提交 [seq=%u]: topic=%s, qos=%d, size=%zu bytes",
      car_status_seq_ - 1, car_status_mqtt_topic_.c_str(), car_status_qos_,
      cbor_payload.size());
    RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
      "[MQTT发送] CarStatus持续发送: seq=%u, qos=%d, size=%zu bytes, 成功=%lu, 失败=%lu",
      car_status_seq_ - 1, car_status_qos_, cbor_payload.size(),
      car_status_sent_count_.load(), car_status_failed_count_.load());
  } else {
    car_status_failed_count_++;
    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
      "[MQTT发送失败] CarStatus未提交: seq=%u, 成功=%lu, 失败=%lu",
      car_status_seq_ - 1, car_status_sent_count_.load(),
      car_status_failed_count_.load());
  }
}

std::vector<uint8_t> MQTTBridgeNode::encodeCarStatusToCBOR(const angmen_ad_msgs::msg::CarStatus& status)
{
  // 获取当前时间戳
  auto now = std::chrono::system_clock::now();
  auto timestamp = std::chrono::duration<double>(now.time_since_epoch()).count();
  
  // UUID字符串转16字节
  std::vector<uint8_t> uuid_bytes(16, 0);
  if (status.current_mission_id.length() >= 36) {
    std::string uuid_str = status.current_mission_id;
    uuid_str.erase(std::remove(uuid_str.begin(), uuid_str.end(), '-'), uuid_str.end());
    
    for (size_t i = 0; i < 16 && i * 2 < uuid_str.length(); ++i) {
      std::string byte_str = uuid_str.substr(i * 2, 2);
      uuid_bytes[i] = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
    }
  }
  
  // 使用 CBOR 对象格式，字符串key按照remote_driving_sequence.md定义
  json j = json::object();
  j["ver"] = 0;  // ver
  j["timestamp"] = timestamp;  // timestamp
  j["seq"] = car_status_seq_++;  // seq
  j["vin"] = config_manager_->getVIN();  // vin
  const auto gcj02 = wgs84ToGcj02(status.latitude, status.longitude);
  j["longitude"] = gcj02.second;  // longitude
  j["latitude"] = gcj02.first;  // latitude
  j["altitude"] = status.altitude;  // altitude
  j["heading"] = status.heading;  // heading
  j["roll"] = status.roll;  // roll
  j["pitch"] = status.pitch;  // pitch
  j["gnss_status"] = status.gnss_status;  // gnss_status
  j["gear"] = status.gear;  // gear
  j["speed"] = status.speed;  // speed
  j["brake"] = status.brake;  // brake
  j["steer"] = status.steer;  // steer
  j["door"] = status.door;  // door
  j["parking_brake"] = status.parking_brake;  // parking_brake
  j["turn_signal"] = status.turn_signal;  // turn_signal
  j["low_beam"] = status.low_beam;  // low_beam
  j["high_beam"] = status.high_beam;  // high_beam
  j["position_light"] = status.position_light;  // position_light
  j["odometer"] = status.odometer;  // odometer
  j["trip_odometer"] = status.trip_odometer;  // trip_odometer
  j["estimated_range"] = status.estimated_range;  // estimated_range
  j["power_battery_soc"] = status.power_battery_soc;  // power_battery_soc
  j["low_voltage_battery_voltage"] = status.low_voltage_battery_voltage;  // low_voltage_battery_voltage
  j["init_status"] = status.init_status;  // init_status
  j["vehicle_mode_status"] = status.vehicle_mode_status;  // vehicle_mode_status
  j["mission_status"] = status.mission_status;  // mission_status
  j["positioning_status"] = status.positioning_status;  // positioning_status
  j["fault_status"] = status.fault_status;  // fault_status
  j["current_mission_id"] = status.current_mission_id;  // current_mission_id - UUID as string
  j["emergency_stop"] = status.emergency_stop;  // emergency_stop
  j["emergency_reason"] = status.emergency_reason;  // emergency_reason
  
  // 转换为 CBOR 格式
  std::vector<uint8_t> cbor_data = json::to_cbor(j);
  
  return cbor_data;
}

void MQTTBridgeNode::handleCommonRequest(const std::string& payload)
{
  try {
    // 从 CBOR 解析
    std::vector<uint8_t> cbor_bytes(payload.begin(), payload.end());
    json j = json::from_cbor(cbor_bytes);
    
    // 协议使用字符串key：ver, request_type, request_id, request_payload
    uint8_t ver = 0;
    uint32_t request_type = 0;
    
    // 尝试读取字段，支持字符串key格式
    // ver: 兼容数字和字符串
    if (j.contains("ver")) {
      if (j["ver"].is_number()) {
        ver = j["ver"].get<uint8_t>();
      } else if (j["ver"].is_string()) {
        ver = static_cast<uint8_t>(std::stoi(j["ver"].get<std::string>()));
      }
    }
    if (j.contains("request_type")) {
      request_type = j["request_type"].get<uint32_t>();
    }
    
    // 提取 request_id: UUID 字符串
    std::vector<uint8_t> request_id_bytes(16, 0);
    std::string request_id_str;
    if (j.contains("request_id") && j["request_id"].is_string()) {
      request_id_str = j["request_id"].get<std::string>();
      std::string hex = request_id_str;
      hex.erase(std::remove(hex.begin(), hex.end(), '-'), hex.end());
      for (size_t i = 0; i < 16 && i * 2 < hex.length(); ++i) {
        request_id_bytes[i] = static_cast<uint8_t>(std::stoul(hex.substr(i * 2, 2), nullptr, 16));
      }
    }

    
    // 提取 request_payload (字节数组)
    std::vector<uint8_t> request_payload_bytes;
    if (j.contains("request_payload")) {
      if (j["request_payload"].is_array()) {
        request_payload_bytes = j["request_payload"].get<std::vector<uint8_t>>();
      } else {
        // 如果不是数组，将其转换为CBOR
        request_payload_bytes = json::to_cbor(j["request_payload"]);
      }
    }
    
    // 日志使用 request_id_str（如果是UUID字符串）或转换后的hex
    char uuid_hex[37];
    snprintf(uuid_hex, sizeof(uuid_hex),
      "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
      request_id_bytes[0], request_id_bytes[1], request_id_bytes[2], request_id_bytes[3],
      request_id_bytes[4], request_id_bytes[5], request_id_bytes[6], request_id_bytes[7],
      request_id_bytes[8], request_id_bytes[9], request_id_bytes[10], request_id_bytes[11],
      request_id_bytes[12], request_id_bytes[13], request_id_bytes[14], request_id_bytes[15]);
    std::string log_id = request_id_str.empty() ? std::string(uuid_hex) : request_id_str;

    RCLCPP_INFO(this->get_logger(), 
      "收到CommonRequest - Ver: %u, Type: %u, ID: %s", ver, request_type, log_id.c_str());
    
    // 根据 request_type 分派到对应处理器
    // 各 handler 内部会调用对应 service 并发送响应回云端
    if (!request_payload_bytes.empty()) {
      std::string inner_payload(request_payload_bytes.begin(), request_payload_bytes.end());
      switch (request_type) {
        case 1: {  // MissionRequest
          RCLCPP_INFO(this->get_logger(), "分派到 MissionRequest 处理器");
          handleMissionRequest(ver, request_type, request_id_bytes, inner_payload);
          break;
        }
        case 2: {  // ModeChangeRequest
          RCLCPP_INFO(this->get_logger(), "分派到 ModeChangeRequest 处理器");
          handleModeChangeRequest(ver, request_type, request_id_bytes, inner_payload);
          break;
        }
        case 3: {  // VideoStreamingRequest
          RCLCPP_INFO(this->get_logger(), "分派到 VideoStreamingRequest 处理器");
          handleVideoStreamingRequest(ver, request_type, request_id_bytes, inner_payload);
          break;
        }
        case 4: {  // TwoWayAudioRequest（预留）
          RCLCPP_INFO(this->get_logger(), "TwoWayAudioRequest 暂为空实现");
          sendCommonResponse(ver, request_type, request_id_bytes, std::vector<uint8_t>{});
          break;
        }
        case 5: {  // FileTransferRequest
          RCLCPP_INFO(this->get_logger(), "分派到 FileTransferRequest 处理器");
          handleFileTransferRequest(ver, request_type, request_id_bytes, inner_payload);
          break;
        }
        case 6: {  // TakeoverRequest（预留）
          RCLCPP_INFO(this->get_logger(), "TakeoverRequest 暂为空实现");
          sendCommonResponse(ver, request_type, request_id_bytes, std::vector<uint8_t>{});
          break;
        }
        case 7: {  // RouteQueryRequest
          RCLCPP_INFO(this->get_logger(), "分派到 RouteQueryRequest 处理器");
          handleRouteQueryRequest(ver, request_type, request_id_bytes, inner_payload);
          break;
        }
        case 8: {  // VehicleFileManagementRequest（预留）
          RCLCPP_INFO(this->get_logger(), "VehicleFileManagementRequest 暂为空实现");
          sendCommonResponse(ver, request_type, request_id_bytes, std::vector<uint8_t>{});
          break;
        }
        case 10: {  // PathPlanningRequest
          RCLCPP_INFO(this->get_logger(), "分派到 PathPlanningRequest 处理器");
          handlePathPlanningRequest(ver, request_type, request_id_bytes, inner_payload);
          break;
        }
        default:
          RCLCPP_WARN(this->get_logger(), "未知 request_type: %u, 使用通用处理", request_type);
          callROS2Service(ver, request_type, request_id_bytes, request_payload_bytes);
          break;
      }
    } else {
      RCLCPP_WARN(this->get_logger(), "request_payload 为空, Type: %u", request_type);
      // 4/6/8 当前为预留空实现，即使请求本身没有 payload，也要返回空响应，
      // 避免落入通用服务后让云端等待超时。
      if (request_type == 4 || request_type == 6 || request_type == 8) {
        sendCommonResponse(ver, request_type, request_id_bytes, std::vector<uint8_t>{});
      } else {
        callROS2Service(ver, request_type, request_id_bytes, request_payload_bytes);
      }
    }
    
  } catch (const json::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "CBOR解析异常: %s", e.what());
  } catch (const std::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "处理CommonRequest异常: %s", e.what());
  }
}


void MQTTBridgeNode::callROS2Service(uint8_t ver, uint32_t request_type,
                                     const std::vector<uint8_t>& request_id,
                                     const std::vector<uint8_t>& request_payload_bytes)
{
  if (!common_service_client_) {
    RCLCPP_ERROR(this->get_logger(), "Service客户端未初始化");
    return;
  }
  
  // 等待服务可用（增加超时时间到5秒）
  RCLCPP_DEBUG(this->get_logger(), "等待CommonRequest服务...");
  if (!common_service_client_->wait_for_service(std::chrono::seconds(5))) {
    RCLCPP_WARN(this->get_logger(), "CommonRequest服务不可用 (超时5秒)");
    RCLCPP_WARN(this->get_logger(), "请确保服务节点正在运行: /mqtt_bridge/common_service");
    return;
  }
  
  RCLCPP_DEBUG(this->get_logger(), "CommonRequest服务可用，准备调用");
  
  // 创建服务请求
  auto request = std::make_shared<angmen_ad_msgs::srv::CommonRequest::Request>();
  request->ver = ver;
  request->request_type = request_type;
  request->request_id = request_id;
  request->request_payload = request_payload_bytes;
  
  // 异步调用服务
  auto future = common_service_client_->async_send_request(request,
    [this, ver, request_type, request_id](
      rclcpp::Client<angmen_ad_msgs::srv::CommonRequest>::SharedFuture future) {
      try {
        auto response = future.get();
        
        RCLCPP_INFO(this->get_logger(), 
          "Service调用成功, ver=%u, request_type=%u", response->ver, response->request_type);
        
        // 发送 CommonResponse
        sendCommonResponse(response->ver, response->request_type, response->request_id, response->response_payload);
        
      } catch (const std::exception& e) {
        RCLCPP_ERROR(this->get_logger(), 
          "Service调用异常: %s", e.what());
        
        // 发送错误响应
        json error_payload = {
          {"error", e.what()}
        };
        std::vector<uint8_t> error_bytes = json::to_cbor(error_payload);
        
        sendCommonResponse(ver, request_type, request_id, error_bytes);
      }
    }
  );
  
  RCLCPP_INFO(this->get_logger(), 
    "已发送Service请求 - Type: %u", request_type);
}

void MQTTBridgeNode::sendCommonResponse(uint8_t ver, uint32_t request_type, 
                                        const std::vector<uint8_t>& request_id,
                                        const std::vector<uint8_t>& response_payload_bytes)
{
  if (common_response_mqtt_topic_.empty()) {
    RCLCPP_ERROR(this->get_logger(), "CommonResponse topic未配置");
    return;
  }
  
  try {
    // 构建响应消息，使用字符串key：ver, request_type, request_id, response_payload
    json j = json::object();
    j["ver"] = ver;
    j["request_type"] = request_type;
    
    // request_id: byte array -> UUID string
    char uuid_str[37];
    snprintf(uuid_str, sizeof(uuid_str),
      "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
      request_id[0], request_id[1], request_id[2], request_id[3],
      request_id[4], request_id[5], request_id[6], request_id[7],
      request_id[8], request_id[9], request_id[10], request_id[11],
      request_id[12], request_id[13], request_id[14], request_id[15]);
    j["request_id"] = std::string(uuid_str);
    
    // response_payload: CBOR bytes -> json object
    if (!response_payload_bytes.empty()) {
      json payload_json = json::from_cbor(response_payload_bytes);
      j["response_payload"] = payload_json;
    } else {
      j["response_payload"] = json::object();
    }
    
    // 转换为 CBOR
    std::vector<uint8_t> cbor_data = json::to_cbor(j);
    std::string payload_str(cbor_data.begin(), cbor_data.end());
    
    // 发送响应 (QoS 2)
    if (mqtt_client_->publish(common_response_mqtt_topic_, payload_str, 2)) {
      messages_sent_++;
      RCLCPP_INFO(this->get_logger(), 
        "CommonResponse已发送 - Ver: %u, Type: %u, ID: %s, Size: %zu bytes", 
        ver, request_type, uuid_str, cbor_data.size());
    } else {
      RCLCPP_WARN(this->get_logger(), "CommonResponse发送失败");
    }
    
  } catch (const json::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "构建CommonResponse失败: %s", e.what());
  }
}

void MQTTBridgeNode::sendErrorResponse(uint8_t ver, uint32_t request_type,
                                       const std::vector<uint8_t>& request_id,
                                       const std::string& error_message)
{
  try {
    json j;
    j["success"] = false;
    j["error"] = error_message;
    std::vector<uint8_t> payload_bytes = json::to_cbor(j);
    sendCommonResponse(ver, request_type, request_id, payload_bytes);
  } catch (const std::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "构建错误响应失败: %s", e.what());
  }
}


void MQTTBridgeNode::handleMissionRequest(uint8_t ver, uint32_t request_type, const std::vector<uint8_t>& request_id, const std::string& payload)
{
  try {
    // 从 CBOR 解析 MissionRequest
    std::vector<uint8_t> cbor_bytes(payload.begin(), payload.end());
    json j = json::from_cbor(cbor_bytes);
    
    // MissionRequest 字段: timestamp, vin, mission_id, longitude, latitude, heading
    double timestamp = 0.0;
    if (j.contains("timestamp")) {
      timestamp = j["timestamp"].get<double>();
    }
    
    std::string vin;
    if (j.contains("vin")) {
      vin = j["vin"].get<std::string>();
    }
    
    // VIN 校验
    if (!validateVIN(vin)) {
      RCLCPP_ERROR(this->get_logger(), "MissionRequest VIN校验失败，拒绝处理");
      return;
    }
    
    // 提取 mission_id (UUID): 字符串格式
    std::vector<uint8_t> mission_id_bytes(16, 0);
    if (j.contains("mission_id") && j["mission_id"].is_string()) {
      std::string mission_id_str = j["mission_id"].get<std::string>();
      std::string hex = mission_id_str;
      hex.erase(std::remove(hex.begin(), hex.end(), '-'), hex.end());
      for (size_t i = 0; i < 16 && i * 2 < hex.length(); ++i) {
        mission_id_bytes[i] = static_cast<uint8_t>(std::stoul(hex.substr(i * 2, 2), nullptr, 16));
      }
    }

    double longitude = 0.0;
    if (j.contains("longitude")) {
      longitude = j["longitude"].get<double>();
    }
    
    double latitude = 0.0;
    if (j.contains("latitude")) {
      latitude = j["latitude"].get<double>();
    }

    const auto wgs84 = gcj02ToWgs84(latitude, longitude);
    latitude = wgs84.first;
    longitude = wgs84.second;
    
    double heading = 0.0;
    if (j.contains("heading")) {
      heading = j["heading"].get<double>();
    }
    
    // 转换UUID为字符串用于日志
    char uuid_str[37];
    snprintf(uuid_str, sizeof(uuid_str),
      "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
      mission_id_bytes[0], mission_id_bytes[1], mission_id_bytes[2], mission_id_bytes[3],
      mission_id_bytes[4], mission_id_bytes[5], mission_id_bytes[6], mission_id_bytes[7],
      mission_id_bytes[8], mission_id_bytes[9], mission_id_bytes[10], mission_id_bytes[11],
      mission_id_bytes[12], mission_id_bytes[13], mission_id_bytes[14], mission_id_bytes[15]);
    
    RCLCPP_INFO(this->get_logger(), "收到MissionRequest - VIN: %s, Mission ID: %s, Pos: (%.6f, %.6f), Heading: %.1f°", 
      vin.c_str(), uuid_str, longitude, latitude, heading);
    
    // 直接调用 MissionRequest 服务
    callMissionService(ver, request_type, request_id, timestamp, mission_id_bytes, longitude, latitude, heading);
    
  } catch (const json::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "CBOR解析MissionRequest异常: %s", e.what());
  } catch (const std::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "处理MissionRequest异常: %s", e.what());
  }
}

void MQTTBridgeNode::callMissionService(uint8_t ver, uint32_t request_type, const std::vector<uint8_t>& request_id, double timestamp,
                                        const std::vector<uint8_t>& mission_id,
                                        double longitude, double latitude, double heading)
{
  if (!mission_service_client_) {
    RCLCPP_ERROR(this->get_logger(), "MissionService客户端未初始化");
    // 构建 MissionResponse payload
    json j;
    j["timestamp"] = timestamp;
    j["vin"] = config_manager_->getVIN();
    j["mission_id"] = uuidBytesToString(mission_id);
    j["response"] = 0;
    j["reason"] = 0;
    j["reason_description"] = "服务客户端未初始化";
    std::vector<uint8_t> payload_bytes = json::to_cbor(j);
    sendCommonResponse(ver, request_type, request_id, payload_bytes);
    return;
  }
  
  // 等待服务可用
  RCLCPP_DEBUG(this->get_logger(), "等待MissionRequest服务...");
  if (!mission_service_client_->wait_for_service(std::chrono::seconds(5))) {
    RCLCPP_WARN(this->get_logger(), "MissionRequest服务不可用 (超时5秒)");
    // 构建 MissionResponse payload
    json j;
    j["timestamp"] = timestamp;
    j["vin"] = config_manager_->getVIN();
    j["mission_id"] = uuidBytesToString(mission_id);
    j["response"] = 0;
    j["reason"] = 0;
    j["reason_description"] = "服务不可用";
    std::vector<uint8_t> payload_bytes = json::to_cbor(j);
    sendCommonResponse(ver, request_type, request_id, payload_bytes);
    return;
  }
  
  RCLCPP_DEBUG(this->get_logger(), "MissionRequest服务可用，准备调用");
  
  // 创建服务请求
  auto request = std::make_shared<angmen_ad_msgs::srv::MissionRequest::Request>();
  request->timestamp = timestamp;
  std::copy_n(mission_id.begin(), 16, request->mission_id.begin());
  request->longitude = longitude;
  request->latitude = latitude;
  request->heading = heading;
  
  // 异步调用服务
  auto future = mission_service_client_->async_send_request(request,
    [this, ver, request_type, request_id](
      rclcpp::Client<angmen_ad_msgs::srv::MissionRequest>::SharedFuture future) {
      try {
        auto response = future.get();
        
        RCLCPP_INFO(this->get_logger(), 
          "MissionService调用成功, response=%u", response->response);
        
        // 构建 MissionResponse payload (始终包含 reason 和 reason_description)
        json j;
        j["timestamp"] = response->timestamp;
        j["vin"] = config_manager_->getVIN();
        j["mission_id"] = uuidArrayToString(response->mission_id);
        j["response"] = response->response;
        j["reason"] = response->reason;
        j["reason_description"] = response->reason_description;
        std::vector<uint8_t> payload_bytes = json::to_cbor(j);
        
        // 发送 CommonResponse 到 cloud/response
        sendCommonResponse(ver, request_type, request_id, payload_bytes);
        
      } catch (const std::exception& e) {
        RCLCPP_ERROR(this->get_logger(), 
          "MissionService调用异常: %s", e.what());
        
        // 构建错误响应 payload
        auto now = std::chrono::system_clock::now();
        double error_timestamp = std::chrono::duration<double>(now.time_since_epoch()).count();
        json j;
        j["timestamp"] = error_timestamp;
        j["vin"] = config_manager_->getVIN();
        j["mission_id"] = "00000000-0000-0000-0000-000000000000";
        j["response"] = 0;
        j["reason"] = 0;
        j["reason_description"] = std::string(e.what());
        std::vector<uint8_t> payload_bytes = json::to_cbor(j);
        sendCommonResponse(ver, request_type, request_id, payload_bytes);
      }
    }
  );
  
  RCLCPP_INFO(this->get_logger(), 
    "已发送MissionService请求 - Mission ID: %02x%02x...", mission_id[0], mission_id[1]);
}

// =================  ModeChangeRequest 处理 =================

void MQTTBridgeNode::handleModeChangeRequest(uint8_t ver, uint32_t request_type, const std::vector<uint8_t>& request_id, const std::string& payload)
{
  try {
    // 从 CBOR 解码
    std::vector<uint8_t> cbor_data(payload.begin(), payload.end());
    json j = json::from_cbor(cbor_data);
    
    // 解析字段: timestamp, vin, mode, session_id（均为必填）
    if (!j.contains("timestamp") || !j.contains("vin") || !j.contains("mode") ||
        !j.contains("session_id")) {
      RCLCPP_ERROR(this->get_logger(), "ModeChangeRequest缺少必需字段");
      sendErrorResponse(ver, request_type, request_id, "ModeChangeRequest缺少必需字段");
      return;
    }
    
    double timestamp = j["timestamp"].get<double>();
    std::string vin = j["vin"].get<std::string>();
    uint32_t mode = j["mode"].get<uint32_t>();
    std::string session_id = j["session_id"].get<std::string>();
    if (session_id.empty()) {
      RCLCPP_ERROR(this->get_logger(), "ModeChangeRequest session_id不能为空");
      sendErrorResponse(ver, request_type, request_id,
        "ModeChangeRequest session_id不能为空");
      return;
    }

    // VIN 校验
    if (!validateVIN(vin)) {
      RCLCPP_ERROR(this->get_logger(), "ModeChangeRequest VIN校验失败，拒绝处理");
      sendErrorResponse(ver, request_type, request_id,
        "ModeChangeRequest VIN不匹配");
      return;
    }
    
    RCLCPP_INFO(this->get_logger(), 
      "收到ModeChangeRequest - VIN: %s, Mode: %u, SessionID: %s",
      vin.c_str(), mode, session_id.c_str());
    
    // 验证mode值 (0=不受控, 1=自动驾驶, 2=远程控制)
    if (mode > 2) {
      RCLCPP_WARN(this->get_logger(), "无效的mode值: %u", mode);
      sendErrorResponse(ver, request_type, request_id,
        "ModeChangeRequest mode必须为0、1或2");
      return;
    }
    
    // 调用ROS2服务
    callModeChangeService(ver, request_type, request_id, timestamp, mode, session_id);
    
  } catch (const json::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "解析ModeChangeRequest失败: %s", e.what());
    sendErrorResponse(ver, request_type, request_id,
      std::string("解析ModeChangeRequest失败: ") + e.what());
  }
}

void MQTTBridgeNode::callModeChangeService(uint8_t ver, uint32_t request_type, const std::vector<uint8_t>& request_id, double timestamp, uint32_t mode, const std::string& session_id)
{
  if (!mode_change_service_client_) {
    RCLCPP_ERROR(this->get_logger(), "ModeChangeService客户端未初始化");
    // 构建 ModeChangeResponse payload
    json j;
    j["timestamp"] = timestamp;
    j["vin"] = config_manager_->getVIN();
    j["response"] = 0;
    j["reason"] = 0;
    j["session_id"] = session_id;
    j["reason_description"] = "服务客户端未初始化";
    std::vector<uint8_t> payload_bytes = json::to_cbor(j);
    sendCommonResponse(ver, request_type, request_id, payload_bytes);
    return;
  }

  // 将服务调用放到后台线程，避免阻塞 MQTT consumer thread
  std::thread([this, ver, request_type, request_id, timestamp, mode, session_id]() {
    // 等待服务可用（保留5秒超时）
    RCLCPP_DEBUG(this->get_logger(), "等待ModeChangeRequest服务...");
    if (!mode_change_service_client_->wait_for_service(std::chrono::seconds(5))) {
      RCLCPP_WARN(this->get_logger(), "ModeChangeRequest服务不可用 (超时5秒)");
      // 构建 ModeChangeResponse payload
      json j;
      j["timestamp"] = timestamp;
      j["vin"] = config_manager_->getVIN();
      j["response"] = 0;
      j["reason"] = 0;
      j["session_id"] = session_id;
      j["reason_description"] = "服务不可用";
      std::vector<uint8_t> payload_bytes = json::to_cbor(j);
      sendCommonResponse(ver, request_type, request_id, payload_bytes);
      return;
    }

    RCLCPP_DEBUG(this->get_logger(), "ModeChangeRequest服务可用，准备调用");

    // 创建服务请求
    auto request = std::make_shared<angmen_ad_msgs::srv::ModeChangeRequest::Request>();
    request->timestamp = timestamp;
    request->mode = mode;
    request->session_id = session_id;

    // 异步调用服务
    auto future = mode_change_service_client_->async_send_request(request,
      [this, ver, request_type, request_id, mode, session_id](
        rclcpp::Client<angmen_ad_msgs::srv::ModeChangeRequest>::SharedFuture future) {
        try {
          auto response = future.get();

          RCLCPP_INFO(this->get_logger(),
            "ModeChangeService调用成功, response=%u", response->response);

          // 构建 ModeChangeResponse payload (包含 session_id)
          json j;
          j["timestamp"] = response->timestamp;
          j["vin"] = config_manager_->getVIN();
          j["response"] = response->response;
          j["reason"] = response->reason;
          j["session_id"] = response->session_id;
          j["reason_description"] = response->reason_description;
          std::vector<uint8_t> payload_bytes = json::to_cbor(j);
          
          // 发送 CommonResponse 到 cloud/response
          sendCommonResponse(ver, request_type, request_id, payload_bytes);

        } catch (const std::exception& e) {
          RCLCPP_ERROR(this->get_logger(),
            "ModeChangeService调用异常: %s", e.what());

          // 构建错误响应 payload（含 session_id）
          auto now = std::chrono::system_clock::now();
          double error_timestamp = std::chrono::duration<double>(now.time_since_epoch()).count();
          json j;
          j["timestamp"] = error_timestamp;
          j["vin"] = config_manager_->getVIN();
          j["response"] = 0;
          j["reason"] = 0;
          j["session_id"] = session_id;
          j["reason_description"] = std::string(e.what());
          std::vector<uint8_t> payload_bytes = json::to_cbor(j);
          sendCommonResponse(ver, request_type, request_id, payload_bytes);
        }
      }
    );

    RCLCPP_INFO(this->get_logger(),
      "已发送ModeChangeService请求 - Mode: %u", mode);
  }).detach();
}


// =============================================================================
// 远程驾驶功能初始化
// =============================================================================

void MQTTBridgeNode::initRemoteDriving()
{
  // 创建 CommonRequest 服务客户端
  common_service_client_ = this->create_client<angmen_ad_msgs::srv::CommonRequest>(
    "/mqtt_bridge/common_service");
  RCLCPP_INFO(this->get_logger(), "CommonRequest服务客户端已创建");

  // 创建 MissionRequest 服务客户端
  mission_service_client_ = this->create_client<angmen_ad_msgs::srv::MissionRequest>(
    "/mqtt_bridge/mission_service");
  RCLCPP_INFO(this->get_logger(), "MissionRequest服务客户端已创建");

  // 创建 ModeChangeRequest 服务客户端（指向 RemoteControlNode）
  mode_change_service_client_ = this->create_client<angmen_ad_msgs::srv::ModeChangeRequest>(
    "/remote_control/mode_change");
  RCLCPP_INFO(this->get_logger(), "ModeChangeRequest服务客户端已创建 -> /remote_control/mode_change");

  // 创建 RemoteControl 服务客户端
  remote_control_service_client_ = this->create_client<angmen_ad_msgs::srv::RemoteControl>(
    "/mqtt_bridge/remote_control_service");
  RCLCPP_INFO(this->get_logger(), "RemoteControl服务客户端已创建");
  
  // 创建 RemoteCommand 发布者 (MQTT -> ROS2)
  remote_command_pub_ = this->create_publisher<angmen_ad_msgs::msg::RemoteCommand>(
    "/remote_control/command", 10);
  RCLCPP_INFO(this->get_logger(), "RemoteCommand发布者已创建");

  video_stream_state_pub_ = this->create_publisher<std_msgs::msg::Bool>(
    "/remote_control/video_stream_active", 10);
  
  // 创建 RemoteReport 订阅者 (ROS2 -> MQTT)
  remote_report_sub_ = this->create_subscription<angmen_ad_msgs::msg::RemoteReport>(
    "/remote_control/report", 10,
    std::bind(&MQTTBridgeNode::onRemoteReportReceived, this, std::placeholders::_1));
  RCLCPP_INFO(this->get_logger(), "RemoteReport订阅者已创建");
  
  // 创建 RemoteStatus 订阅者 (ROS2 -> MQTT)
  remote_status_sub_ = this->create_subscription<angmen_ad_msgs::msg::RemoteStatus>(
    "/remote_control/status", 10,
    std::bind(&MQTTBridgeNode::onRemoteStatusReceived, this, std::placeholders::_1));
  RCLCPP_INFO(this->get_logger(), "RemoteStatus订阅者已创建");
  
  // 创建其他服务客户端
  video_streaming_service_client_ = this->create_client<angmen_ad_msgs::srv::VideoStreamingRequest>(
    "/mqtt_bridge/video_streaming_service");
  
  file_transfer_service_client_ = this->create_client<angmen_ad_msgs::srv::FileTransferRequest>(
    "/mqtt_bridge/file_transfer_service");
  
  route_query_service_client_ = this->create_client<angmen_ad_msgs::srv::RouteQueryRequest>(
    "/mqtt_bridge/route_query_service");

  path_planning_service_client_ = this->create_client<angmen_ad_msgs::srv::PathPlanningRequest>(
    "/mqtt_bridge/path_planning_service");
  
  // 创建 CarEvent 订阅者
  car_event_sub_ = this->create_subscription<angmen_ad_msgs::msg::CarEvent>(
    "/vehicle/car_event", 10,
    std::bind(&MQTTBridgeNode::onCarEventReceived, this, std::placeholders::_1));
  RCLCPP_INFO(this->get_logger(), "CarEvent订阅者已创建");

  takeover_request_sub_ = this->create_subscription<angmen_ad_msgs::msg::TakeoverRequest>(
    "/vehicle/takeover_request", 10,
    std::bind(&MQTTBridgeNode::onTakeoverRequestReceived, this, std::placeholders::_1));
  RCLCPP_INFO(this->get_logger(), "TakeoverRequest订阅者已创建");
}

// =============================================================================
// RemoteControl 实现
// =============================================================================

void MQTTBridgeNode::handleRemoteControl(const std::string& payload)
{
  try {
    // 从 CBOR 解析
    std::vector<uint8_t> cbor_bytes(payload.begin(), payload.end());
    json j = json::from_cbor(cbor_bytes);

    uint32_t control_type = 0;
    std::string operator_id;
    std::string session_id;
    std::string vehicle_id;

    // 支持数组格式和字符串key对象格式
    if (j.is_array()) {
      // 数组格式: [null, control_type, operator_id, vehicle_id, session_id]
      if (j.size() >= 3 && !j[1].is_null()) {
        control_type = j[1].get<uint32_t>();
        operator_id = j[2].get<std::string>();
        if (j.size() >= 4 && !j[3].is_null()) {
          vehicle_id = j[3].get<std::string>();
        }
        if (j.size() >= 5 && !j[4].is_null()) {
          session_id = j[4].get<std::string>();
        }
      } else {
        RCLCPP_ERROR(this->get_logger(), "RemoteControl数组格式缺少必需字段");
        return;
      }
    } else if (j.is_object()) {
      // 字符串key对象格式: {"control_type": value, "operator_id": value, "vehicle_id": value, "session_id": value}
      if (!j.contains("control_type") || !j.contains("operator_id")) {
        RCLCPP_ERROR(this->get_logger(), "RemoteControl字符串key格式缺少必需字段");
        return;
      }
      try {
        control_type = j["control_type"].get<uint32_t>();
        operator_id = j["operator_id"].get<std::string>();
        if (j.contains("vehicle_id")) {
          vehicle_id = j["vehicle_id"].get<std::string>();
        }
        if (j.contains("session_id")) {
          session_id = j["session_id"].get<std::string>();
        }
      } catch (const json::exception& e) {
        RCLCPP_ERROR(this->get_logger(), "RemoteControl字段解析失败: %s", e.what());
        return;
      }
    } else {
      RCLCPP_ERROR(this->get_logger(), "RemoteControl消息格式不支持");
      return;
    }

    // VIN 校验（vehicle_id 与本地 VIN 一致）
    if (!validateVIN(vehicle_id)) {
      RCLCPP_ERROR(this->get_logger(), "RemoteControl VIN校验失败，拒绝处理");
      sendRemoteControlResponse(false, "VIN mismatch: " + vehicle_id);
      return;
    }

    const char* control_names[] = {"接管", "释放", "急停"};
    RCLCPP_INFO(this->get_logger(),
      "收到RemoteControl - 类型: %u (%s), 操作员: %s, 会话: %s",
      control_type,
      control_type <= 2 ? control_names[control_type] : "未知",
      operator_id.c_str(),
      session_id.c_str());

    // 调用ROS2服务
    callRemoteControlService(control_type, operator_id, session_id, vehicle_id);

  } catch (const json::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "解析RemoteControl失败: %s", e.what());
  }
}

void MQTTBridgeNode::callRemoteControlService(uint32_t control_type, 
                                              const std::string& operator_id,
                                              const std::string& session_id,
                                              const std::string& vehicle_id)
{
  if (!remote_control_service_client_) {
    RCLCPP_ERROR(this->get_logger(), "RemoteControl服务客户端未初始化");
    sendRemoteControlResponse(false, "服务未初始化");
    return;
  }
  
  if (!remote_control_service_client_->wait_for_service(std::chrono::seconds(5))) {
    RCLCPP_WARN(this->get_logger(), "RemoteControl服务不可用");
    sendRemoteControlResponse(false, "服务不可用");
    return;
  }
  
  auto request = std::make_shared<angmen_ad_msgs::srv::RemoteControl::Request>();
  request->control_type = control_type;
  request->operator_id = operator_id;
  request->session_id = session_id;
  request->vehicle_id = vehicle_id;
  
  auto future = remote_control_service_client_->async_send_request(request,
    [this](rclcpp::Client<angmen_ad_msgs::srv::RemoteControl>::SharedFuture future) {
      try {
        auto response = future.get();
        RCLCPP_INFO(this->get_logger(), "RemoteControl调用成功: %s, session_id: %s",
          response->message.c_str(), response->session_id.c_str());
        sendRemoteControlResponse(response->success, response->message, response->session_id);
      } catch (const std::exception& e) {
        RCLCPP_ERROR(this->get_logger(), "RemoteControl调用异常: %s", e.what());
        sendRemoteControlResponse(false, std::string(e.what()), "");
      }
    }
  );
}

void MQTTBridgeNode::sendRemoteControlResponse(bool success, const std::string& message, const std::string& session_id)
{
  try {
    // 构建 CBOR Response: 使用字符串key
    json j = json::object();
    j["success"] = success;
    j["message"] = message;
    if (!session_id.empty()) {
      j["session_id"] = session_id;
    }

    std::vector<uint8_t> cbor_data = json::to_cbor(j);

    if (!remote_control_response_mqtt_topic_.empty()) {
      std::string payload(cbor_data.begin(), cbor_data.end());
      mqtt_client_->publish(remote_control_response_mqtt_topic_, payload, 2);
      messages_sent_++;

      RCLCPP_INFO(this->get_logger(), "发送RemoteControlResponse - Success: %s, SessionID: %s",
        success ? "true" : "false", session_id.c_str());
    }
  } catch (const json::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "构建RemoteControlResponse失败: %s", e.what());
  }
}

// =============================================================================
// RemoteCommand 实现 (MQTT -> ROS2 Topic)
// =============================================================================

void MQTTBridgeNode::handleRemoteCommand(const std::string& payload)
{
  try {
    // 从 CBOR 解析
    std::vector<uint8_t> cbor_bytes(payload.begin(), payload.end());
    json j = json::from_cbor(cbor_bytes);

    // 创建 RemoteCommand 消息
    auto msg = angmen_ad_msgs::msg::RemoteCommand();

    // 解析 DriveCmd 格式 - 字符串key格式
    // ver, vin, timestamp, seq, enable_remote, emergency_stop
    // gear, target_speed, steering_angle, brake, auto_brake_enable
    // operator_id, session_id, echo_timestamp

    msg.header.stamp = this->now();
    msg.header.frame_id = "base_link";

      // 字符串key对象格式解析 DriveCmd
    if (j.is_object()) {
      msg.vin = j.contains("vin") ? j["vin"].get<std::string>() : "";
      // VIN 校验
      if (!msg.vin.empty() && !validateVIN(msg.vin)) {
        RCLCPP_ERROR(this->get_logger(), "RemoteCommand VIN校验失败，忽略指令");
        return;
      }
      msg.operator_id = j.contains("operator_id") ? j["operator_id"].get<std::string>() : "";
      msg.session_id = j.contains("session_id") ? j["session_id"].get<std::string>() : "";

      // command_type: 4=CONTROL_CMD, 5=EMERGENCY_STOP
      // 兼容 bool 和 string 类型 (远程驾驶舱可能发送字符串 "true"/"false")
      auto parseBool = [](const json& val) -> bool {
        if (val.is_boolean()) return val.get<bool>();
        if (val.is_string()) {
          const auto& s = val.get<std::string>();
          return s == "true" || s == "1" || s == "True" || s == "TRUE";
        }
        if (val.is_number()) return val.get<int>() != 0;
        return false;
      };
      bool enable_remote = j.contains("enable_remote") ? parseBool(j["enable_remote"]) : false;
      bool emergency_stop = j.contains("emergency_stop") ? parseBool(j["emergency_stop"]) : false;
      msg.command_type = emergency_stop ? 5 : (enable_remote ? 4 : 0);

      msg.gear = j.contains("gear") ? static_cast<int8_t>(j["gear"].get<int>()) : 0;
      msg.target_speed = j.contains("target_speed") ? static_cast<float>(j["target_speed"].get<double>()) : 0.0f;
      msg.steering_angle = j.contains("steering_angle") ? static_cast<float>(j["steering_angle"].get<double>()) : 0.0f;
      msg.brake = j.contains("brake") ? static_cast<float>(j["brake"].get<double>()) : 0.0f;
      msg.auto_brake_enable = j.contains("auto_brake_enable") ? parseBool(j["auto_brake_enable"]) : true;

      // 信号灯和喇叭 - 解析远程驾驶舱发送的灯光字段
      // turn_signal: 0=无, 1=左转, 2=右转, 3=双闪
      int turn_signal = j.contains("turn_signal") ? j["turn_signal"].get<int>() : 0;
      msg.left_turn_signal = (turn_signal == 1 || turn_signal == 3);
      msg.right_turn_signal = (turn_signal == 2 || turn_signal == 3);
      msg.hazard_light = j.contains("hazard_flasher") ? parseBool(j["hazard_flasher"]) : false;
      msg.horn = j.contains("horn") ? parseBool(j["horn"]) : false;
      msg.target_acceleration = 0.0f;

      // echo_timestamp
      msg.echo_timestamp = j.contains("echo_timestamp") ? static_cast<uint64_t>(j["echo_timestamp"].get<int64_t>()) : 0;

      // timestamp (秒转换为纳秒)
      if (j.contains("timestamp")) {
        double ts_seconds = j["timestamp"].get<double>();
        msg.timestamp = static_cast<uint64_t>(ts_seconds * 1000000000.0);
      }

      // sequence_number
      msg.sequence_number = j.contains("seq") ? static_cast<uint64_t>(j["seq"].get<int64_t>()) : 0;
    }
    // 兼容数组格式
    else if (j.is_array()) {
      msg.vin = j.size() > 2 ? j[2].get<std::string>() : "";
      // VIN 校验
      if (!msg.vin.empty() && !validateVIN(msg.vin)) {
        RCLCPP_ERROR(this->get_logger(), "RemoteCommand VIN校验失败(数组格式)，忽略指令");
        return;
      }
      msg.operator_id = j.size() > 12 ? j[12].get<std::string>() : "";
      msg.session_id = j.size() > 13 ? j[13].get<std::string>() : "";

      bool enable_remote = j.size() > 5 ? j[5].get<bool>() : false;
      bool emergency_stop = j.size() > 6 ? j[6].get<bool>() : false;
      msg.command_type = emergency_stop ? 5 : (enable_remote ? 4 : 0);

      msg.gear = j.size() > 7 ? static_cast<int8_t>(j[7].get<int>()) : 0;
      msg.target_speed = j.size() > 8 ? static_cast<float>(j[8].get<double>()) : 0.0f;
      msg.steering_angle = j.size() > 9 ? static_cast<float>(j[9].get<double>()) : 0.0f;
      msg.brake = j.size() > 10 ? static_cast<float>(j[10].get<double>()) : 0.0f;
      msg.auto_brake_enable = j.size() > 11 ? j[11].get<bool>() : true;

      msg.left_turn_signal = false;
      msg.right_turn_signal = false;
      msg.hazard_light = false;
      msg.horn = false;
      msg.target_acceleration = 0.0f;

      msg.echo_timestamp = j.size() > 14 ? static_cast<uint64_t>(j[14].get<int64_t>()) : 0;

      if (j.size() > 3) {
        double ts_seconds = j[3].get<double>();
        msg.timestamp = static_cast<uint64_t>(ts_seconds * 1000000000.0);
      }

      msg.sequence_number = j.size() > 4 ? static_cast<uint64_t>(j[4].get<int64_t>()) : 0;
    }

    // 发布到 ROS2
    remote_command_pub_->publish(msg);

    RCLCPP_INFO(this->get_logger(), "发布RemoteCommand - seq:%lu, type:%d, echo_ts:%lu, speed:%.2f, steer:%.2f",
      msg.sequence_number, msg.command_type, msg.echo_timestamp, msg.target_speed, msg.steering_angle);

  } catch (const json::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "解析RemoteCommand失败: %s", e.what());
  }
}

// =============================================================================
// RemoteReport 实现 (ROS2 Topic -> MQTT)
// =============================================================================

void MQTTBridgeNode::onRemoteReportReceived(const angmen_ad_msgs::msg::RemoteReport::SharedPtr msg)
{
  try {

    json j = json::object();
    
    // header
    json header_json = json::array();
    header_json.push_back(msg->header.stamp.sec);
    header_json.push_back(msg->header.stamp.nanosec);
    header_json.push_back(msg->header.frame_id);
    j["header"] = header_json;
    
    // 其他字段
    j["vehicle_id"] = msg->vehicle_id;
    const auto gcj02 = wgs84ToGcj02(msg->latitude, msg->longitude);
    j["latitude"] = gcj02.first;
    j["longitude"] = gcj02.second;
    j["altitude"] = msg->altitude;
    j["heading"] = msg->heading;
    j["speed"] = msg->speed;
    j["steering_angle"] = msg->steering_angle;
    j["gear"] = msg->gear;
    j["actual_speed"] = msg->actual_speed;
    j["actual_steering"] = msg->actual_steering;
    j["control_source"] = msg->control_source;
    j["system_health_status"] = msg->system_health_status;
    j["obstacle_warning_front"] = msg->obstacle_warning_front;
    j["obstacle_warning_rear"] = msg->obstacle_warning_rear;
    j["obstacle_warning_left"] = msg->obstacle_warning_left;
    j["obstacle_warning_right"] = msg->obstacle_warning_right;
    j["nearest_obstacle_distance"] = msg->nearest_obstacle_distance;
    j["battery_soc"] = msg->battery_soc;
    j["last_received_sequence"] = msg->last_received_sequence;
    
    // timestamp
    json ts_json = json::array();
    ts_json.push_back(static_cast<uint32_t>(msg->timestamp / 1000000000ULL));
    ts_json.push_back(static_cast<uint32_t>(msg->timestamp % 1000000000ULL));
    j["timestamp"] = ts_json;
    
    j["echo_timestamp"] = msg->echo_timestamp;
    
    // 编码为 CBOR
    std::vector<uint8_t> cbor_data = json::to_cbor(j);
    
    // 发送到 MQTT
    if (!remote_report_mqtt_topic_.empty()) {
      std::string payload(cbor_data.begin(), cbor_data.end());
      mqtt_client_->publish(remote_report_mqtt_topic_, payload, 1);
      messages_sent_++;
      
      RCLCPP_DEBUG(this->get_logger(), "发送RemoteReport - VIN: %s, 位置: (%.6f, %.6f)", 
        msg->vehicle_id.c_str(), msg->longitude, msg->latitude);
    }
    
  } catch (const std::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "构建RemoteReport失败: %s", e.what());
  }
}

// =============================================================================
// RemoteStatus 实现 (ROS2 Topic -> MQTT)
// =============================================================================

void MQTTBridgeNode::onRemoteStatusReceived(const angmen_ad_msgs::msg::RemoteStatus::SharedPtr msg)
{
  try {
    // 构建 CBOR Payload - 字符串key格式
    // header, status, operator_id, video_stream_active, video_fps
    // video_latency, control_latency, network_quality, session_id
    // session_start_timestamp, echo_timestamp, rtt_latency

    json j = json::object();

    // header
    json header_json = json::array();
    header_json.push_back(msg->header.stamp.sec);
    header_json.push_back(msg->header.stamp.nanosec);
    header_json.push_back(msg->header.frame_id);
    j["header"] = header_json;

    // status
    j["status"] = msg->status;

    // operator_id
    j["operator_id"] = msg->operator_id;

    // 其他字段
    j["video_stream_active"] = msg->video_stream_active;
    j["video_fps"] = msg->video_fps;
    j["video_latency"] = msg->video_latency;
    j["control_latency"] = msg->control_latency;
    j["network_quality"] = msg->network_quality;
    j["vehicle_id"] = msg->vehicle_id;
    j["vin"] = config_manager_->getVIN();
    j["speed"] = msg->speed;
    j["steering_angle"] = msg->steering_angle;
    j["gear"] = msg->gear;
    j["battery_soc"] = msg->battery_soc;

    // session_id
    j["session_id"] = msg->session_id;

    // session_start_timestamp (纳秒转换为秒和纳秒)
    json start_ts_json = json::array();
    start_ts_json.push_back(static_cast<uint32_t>(msg->session_start_timestamp / 1000000000ULL));
    start_ts_json.push_back(static_cast<uint32_t>(msg->session_start_timestamp % 1000000000ULL));
    j["session_start_timestamp"] = start_ts_json;

    // echo_timestamp
    j["echo_timestamp"] = msg->echo_timestamp;

    // rtt_latency
    j["rtt_latency"] = msg->rtt_latency;

    // 编码为 CBOR
    std::vector<uint8_t> cbor_data = json::to_cbor(j);

    // 发送到 MQTT
    if (remote_status_mqtt_topic_.empty()) {
      remote_status_failed_count_++;
      RCLCPP_ERROR_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
        "[MQTT发送失败] RemoteStatus topic未配置，累计失败=%lu",
        remote_status_failed_count_.load());
      return;
    }

    std::string payload(cbor_data.begin(), cbor_data.end());
    if (mqtt_client_->publish(remote_status_mqtt_topic_, payload, 1)) {
      messages_sent_++;
      remote_status_sent_count_++;

      const char* status_names[] = {"待机","待机", "监看", "控制", "急停"};
      const char* status_name =
        msg->status >= 0 && msg->status <= 4 ? status_names[msg->status] : "未知";
      RCLCPP_DEBUG(this->get_logger(),
        "[MQTT发送] RemoteStatus已提交: status=%s, operator=%s, qos=1, size=%zu bytes, "
        "RTT=%lums, echo_ts=%lu",
        status_name,
        msg->operator_id.c_str(),
        cbor_data.size(),
        msg->rtt_latency,
        msg->echo_timestamp);
      RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
        "[MQTT发送] RemoteStatus持续发送: status=%s, qos=1, size=%zu bytes, "
        "成功=%lu, 失败=%lu",
        status_name, cbor_data.size(), remote_status_sent_count_.load(),
        remote_status_failed_count_.load());
    } else {
      remote_status_failed_count_++;
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
        "[MQTT发送失败] RemoteStatus未提交: 成功=%lu, 失败=%lu",
        remote_status_sent_count_.load(), remote_status_failed_count_.load());
    }

  } catch (const json::exception& e) {
    remote_status_failed_count_++;
    RCLCPP_ERROR(this->get_logger(), "构建RemoteStatus失败: %s", e.what());
  }
}

// =============================================================================
// CarEvent 实现 (ROS2 Topic -> MQTT)
// =============================================================================

void MQTTBridgeNode::onCarEventReceived(const angmen_ad_msgs::msg::CarEvent::SharedPtr msg)
{
  try {
    // 构建 CBOR Payload - 字符串key格式
    json j = json::object();
    j["ver"] = msg->ver;
    j["vin"] = config_manager_->getVIN();
    j["event_level"] = msg->event_level;
    j["event_type"] = msg->event_type;
    j["event_description"] = msg->event_description;
    j["event_time"] = msg->event_time;
    const auto gcj02 = wgs84ToGcj02(msg->event_latitude, msg->event_longitude);
    j["event_longitude"] = gcj02.second;
    j["event_latitude"] = gcj02.first;
    j["event_mission_id"] = uuidArrayToString(msg->event_mission_id);
    
    std::vector<uint8_t> cbor_data = json::to_cbor(j);
    
    if (!car_event_mqtt_topic_.empty()) {
      std::string payload(cbor_data.begin(), cbor_data.end());
      mqtt_client_->publish(car_event_mqtt_topic_, payload, car_event_qos_);
      messages_sent_++;
      
      RCLCPP_INFO(this->get_logger(), "发送CarEvent - 类型: %u, 级别: %u", 
        msg->event_type, msg->event_level);
    }
  } catch (const std::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "构建CarEvent失败: %s", e.what());
  }
}

void MQTTBridgeNode::onTakeoverRequestReceived(
  const angmen_ad_msgs::msg::TakeoverRequest::SharedPtr msg)
{
  if (msg->request > 2 || msg->reason > 6) {
    RCLCPP_ERROR(this->get_logger(),
      "车辆主动接管请求字段无效: request=%u, reason=%u", msg->request, msg->reason);
    return;
  }
  if (takeover_request_mqtt_topic_.empty()) {
    RCLCPP_ERROR(this->get_logger(), "车辆主动接管MQTT topic未配置");
    return;
  }

  try {
    json request_payload;
    request_payload["timestamp"] = msg->timestamp;
    request_payload["vin"] = config_manager_->getVIN();
    request_payload["request"] = msg->request;
    request_payload["reason"] = msg->reason;
    request_payload["reason_description"] = msg->reason_description;

    json envelope;
    envelope["ver"] = 0;
    envelope["request_type"] = 6;
    envelope["request_id"] = generateUuidString();
    envelope["request_payload"] = request_payload;

    std::vector<uint8_t> cbor_data = json::to_cbor(envelope);
    std::string payload(cbor_data.begin(), cbor_data.end());
    if (mqtt_client_->publish(takeover_request_mqtt_topic_, payload, 2)) {
      messages_sent_++;
      RCLCPP_INFO(this->get_logger(),
        "车辆主动接管请求已上报: request=%u, reason=%u, request_id=%s",
        msg->request, msg->reason, envelope["request_id"].get<std::string>().c_str());
    } else {
      RCLCPP_WARN(this->get_logger(), "车辆主动接管请求MQTT发送失败");
    }
  } catch (const std::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "构建车辆主动接管请求失败: %s", e.what());
  }
}

// =============================================================================
// VideoStreaming 实现 (MQTT -> ROS2 Service)
// =============================================================================

void MQTTBridgeNode::handleVideoStreamingRequest(uint8_t ver, uint32_t request_type,
                                                 const std::vector<uint8_t>& request_id,
                                                 const std::string& payload)
{
  try {
    // 从 CBOR 解析
    std::vector<uint8_t> cbor_bytes(payload.begin(), payload.end());
    json j = json::from_cbor(cbor_bytes);
    
    // 解析字段: timestamp, vin, cam_id, behavior, rtp_ip, rtp_port
    if (!j.contains("timestamp") || !j.contains("vin") || !j.contains("cam_id") ||
        !j.contains("behavior") || !j.contains("rtp_ip") || !j.contains("rtp_port")) {
      RCLCPP_ERROR(this->get_logger(), "VideoStreamingRequest缺少必需字段");
      sendErrorResponse(ver, request_type, request_id, "VideoStreamingRequest缺少必需字段");
      return;
    }

    if (!j["cam_id"].is_array() || !j["rtp_ip"].is_array() || !j["rtp_port"].is_array()) {
      RCLCPP_ERROR(this->get_logger(),
        "VideoStreamingRequest cam_id、rtp_ip、rtp_port必须为数组");
      sendErrorResponse(ver, request_type, request_id,
        "VideoStreamingRequest cam_id、rtp_ip、rtp_port必须为数组");
      return;
    }
    
    double timestamp = j["timestamp"].get<double>();
    std::string vin = j["vin"].get<std::string>();
    std::vector<uint32_t> cam_id = j["cam_id"].get<std::vector<uint32_t>>();
    uint32_t behavior = j["behavior"].get<uint32_t>();

    // VideoStreaming.behavior: 1=启动，2=停止
    if (behavior != 1 && behavior != 2) {
      RCLCPP_ERROR(this->get_logger(),
        "VideoStreamingRequest behavior无效: %u（仅支持1=启动、2=停止）", behavior);
      sendErrorResponse(ver, request_type, request_id,
        "VideoStreamingRequest behavior无效，仅支持1=启动、2=停止");
      return;
    }

    // VIN 校验
    if (!validateVIN(vin)) {
      RCLCPP_ERROR(this->get_logger(), "VideoStreamingRequest VIN校验失败，拒绝处理");
      sendErrorResponse(ver, request_type, request_id, "VIN mismatch: " + vin);
      return;
    }
    
    std::vector<std::string> rtp_ip = j["rtp_ip"].get<std::vector<std::string>>();
    std::vector<uint32_t> rtp_port = j["rtp_port"].get<std::vector<uint32_t>>();

    if (cam_id.empty() || rtp_ip.empty() || rtp_port.empty()) {
      RCLCPP_ERROR(this->get_logger(),
        "VideoStreamingRequest cam_id、rtp_ip、rtp_port不能为空数组");
      sendErrorResponse(ver, request_type, request_id,
        "VideoStreamingRequest cam_id、rtp_ip、rtp_port不能为空数组");
      return;
    }

    if (cam_id.size() != rtp_ip.size() || cam_id.size() != rtp_port.size()) {
      RCLCPP_ERROR(this->get_logger(),
        "VideoStreamingRequest数组长度不一致: cam_id=%zu, rtp_ip=%zu, rtp_port=%zu",
        cam_id.size(), rtp_ip.size(), rtp_port.size());
      sendErrorResponse(ver, request_type, request_id,
        "VideoStreamingRequest cam_id、rtp_ip、rtp_port数组长度必须一致");
      return;
    }
    
    const char* behavior_name = behavior == 1 ? "启动" : "停止";
    RCLCPP_INFO(this->get_logger(), 
      "收到VideoStreamingRequest - VIN: %s, 行为: %s, 摄像头数: %zu", 
      vin.c_str(), 
      behavior_name,
      cam_id.size());
    
    // 调用ROS2服务
    callVideoStreamingService(ver, request_type, request_id, timestamp, vin, cam_id, behavior, rtp_ip, rtp_port);
    
  } catch (const json::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "解析VideoStreamingRequest失败: %s", e.what());
    sendErrorResponse(ver, request_type, request_id, std::string("解析VideoStreamingRequest失败: ") + e.what());
  }
}

void MQTTBridgeNode::callVideoStreamingService(uint8_t ver, uint32_t request_type,
                                               const std::vector<uint8_t>& request_id,
                                               double timestamp, const std::string& vin,
                                               const std::vector<uint32_t>& cam_id, uint32_t behavior,
                                               const std::vector<std::string>& rtp_ip,
                                               const std::vector<uint32_t>& rtp_port)
{
  if (!video_streaming_service_client_) {
    RCLCPP_ERROR(this->get_logger(), "VideoStreaming服务客户端未初始化");
    // 构建 VideoStreamingResponse payload
    json j;
    j["timestamp"] = timestamp;
    j["vin"] = vin;
    j["response"] = 0;
    j["reason"] = 1;
    j["reason_description"] = "服务未初始化";
    std::vector<uint8_t> payload_bytes = json::to_cbor(j);
    sendCommonResponse(ver, request_type, request_id, payload_bytes);
    return;
  }
  
  if (!video_streaming_service_client_->wait_for_service(std::chrono::seconds(5))) {
    RCLCPP_WARN(this->get_logger(), "VideoStreaming服务不可用");
    json j;
    j["timestamp"] = timestamp;
    j["vin"] = vin;
    j["response"] = 0;
    j["reason"] = 2;
    j["reason_description"] = "服务不可用";
    std::vector<uint8_t> payload_bytes = json::to_cbor(j);
    sendCommonResponse(ver, request_type, request_id, payload_bytes);
    return;
  }
  
  auto request = std::make_shared<angmen_ad_msgs::srv::VideoStreamingRequest::Request>();
  request->timestamp = timestamp;
  request->vin = vin;
  request->cam_id = cam_id;
  request->behavior = behavior;
  request->rtp_ip = rtp_ip;
  request->rtp_port = rtp_port;
  
  auto future = video_streaming_service_client_->async_send_request(request,
    [this, ver, request_type, request_id, vin, behavior](rclcpp::Client<angmen_ad_msgs::srv::VideoStreamingRequest>::SharedFuture future) {
      try {
        auto response = future.get();
        RCLCPP_INFO(this->get_logger(), "VideoStreaming调用成功: response=%u", 
          response->response);
        if (response->response == 1) {
          std_msgs::msg::Bool stream_state;
          stream_state.data = behavior == 1;
          video_stream_state_pub_->publish(stream_state);
        }
        // 构建 VideoStreamingResponse payload
        json j;
        j["timestamp"] = response->timestamp;
        j["vin"] = vin;
        j["response"] = response->response;
        j["reason"] = response->reason;
        j["reason_description"] = response->reason_description;
        std::vector<uint8_t> payload_bytes = json::to_cbor(j);
        sendCommonResponse(ver, request_type, request_id, payload_bytes);
      } catch (const std::exception& e) {
        RCLCPP_ERROR(this->get_logger(), "VideoStreaming调用异常: %s", e.what());
        auto now = std::chrono::system_clock::now();
        double error_timestamp = std::chrono::duration<double>(now.time_since_epoch()).count();
        json j;
        j["timestamp"] = error_timestamp;
        j["vin"] = vin;
        j["response"] = 0;
        j["reason"] = 99;
        j["reason_description"] = std::string(e.what());
        std::vector<uint8_t> payload_bytes = json::to_cbor(j);
        sendCommonResponse(ver, request_type, request_id, payload_bytes);
      }
    }
  );
  
  RCLCPP_INFO(this->get_logger(), "已发送VideoStreaming服务请求");
}

// =============================================================================
// FileTransfer 实现 (MQTT -> ROS2 Service)
// =============================================================================

void MQTTBridgeNode::handleFileTransferRequest(uint8_t ver, uint32_t request_type,
                                                 const std::vector<uint8_t>& request_id,
                                                 const std::string& payload)
{
  try {
    // 从 CBOR 解析
    std::vector<uint8_t> cbor_bytes(payload.begin(), payload.end());
    json j = json::from_cbor(cbor_bytes);
    
    // 解析字段: timestamp, vin, task_id, action, file_type, file_name, size_bytes, checksum, description, download_url, expiry_seconds, content_type
    if (!j.contains("timestamp") || !j.contains("vin") || !j.contains("task_id") ||
        !j.contains("action") || !j.contains("file_type") ||
        !j.contains("file_name") || !j.contains("size_bytes") ||
        !j.contains("checksum")) {
      RCLCPP_ERROR(this->get_logger(), "FileTransferRequest缺少必需字段");
      sendErrorResponse(ver, request_type, request_id, "FileTransferRequest缺少必需字段");
      return;
    }
    
    double timestamp = j["timestamp"].get<double>();
    std::string vin = j["vin"].get<std::string>();
    // VIN 校验
    if (!validateVIN(vin)) {
      RCLCPP_ERROR(this->get_logger(), "FileTransferRequest VIN校验失败，拒绝处理");
      sendErrorResponse(ver, request_type, request_id, "VIN mismatch: " + vin);
      return;
    }
    std::string task_id = j["task_id"].get<std::string>();
    uint32_t action = j["action"].get<uint32_t>();
    if (action > 1) {
      RCLCPP_ERROR(this->get_logger(),
        "FileTransferRequest action无效: %u（仅支持0=下载、1=上传）", action);
      sendErrorResponse(ver, request_type, request_id,
        "FileTransferRequest action无效，仅支持0=下载、1=上传");
      return;
    }
    uint32_t file_type = j["file_type"].get<uint32_t>();
    if (file_type > 6) {
      RCLCPP_ERROR(this->get_logger(),
        "FileTransferRequest file_type无效: %u（仅支持0~6）", file_type);
      sendErrorResponse(ver, request_type, request_id,
        "FileTransferRequest file_type无效，仅支持0~6");
      return;
    }
    std::string file_name = j["file_name"].get<std::string>();
    uint64_t size_bytes = j["size_bytes"].get<uint64_t>();
    std::string checksum = j["checksum"].get<std::string>();
    if (file_name.empty() || size_bytes == 0 || checksum.empty()) {
      RCLCPP_ERROR(this->get_logger(),
        "FileTransferRequest file_name、size_bytes、checksum内容无效");
      sendErrorResponse(ver, request_type, request_id,
        "FileTransferRequest file_name、size_bytes、checksum不能为空");
      return;
    }
    std::string description = j.value("description", "");
    std::string download_url = j.value("download_url", "");
    uint32_t expiry_seconds = j.value("expiry_seconds", 0);
    std::string content_type = j.value("content_type", "");
    
    const char* action_names[] = {"下载", "上传"};
    const char* file_type_names[] = {
      "OTA软件包下发",
      "参数文件下发",
      "配置规则下发",
      "点云地图下发",
      "矢量地图下发",
      "日志数据上传",
      "视频数据上传"
    };
    
    RCLCPP_INFO(this->get_logger(), 
      "收到FileTransferRequest - 任务: %s, 动作: %s, 类型: %s, 文件: %s", 
      task_id.c_str(),
      action <= 1 ? action_names[action] : "未知",
      file_type_names[file_type],
      file_name.c_str());
    
    // 调用ROS2服务
    callFileTransferService(ver, request_type, request_id, timestamp, task_id, action, file_type, file_name,
                           size_bytes, checksum, description, download_url, 
                           expiry_seconds, content_type);
    
  } catch (const json::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "解析FileTransferRequest失败: %s", e.what());
    sendErrorResponse(ver, request_type, request_id, std::string("解析FileTransferRequest失败: ") + e.what());
  }
}

void MQTTBridgeNode::callFileTransferService(uint8_t ver, uint32_t request_type,
                                             const std::vector<uint8_t>& request_id,
                                             double timestamp, const std::string& task_id,
                                             uint32_t action, uint32_t file_type, 
                                             const std::string& file_name,
                                             uint64_t size_bytes, const std::string& checksum, 
                                             const std::string& description,
                                             const std::string& download_url, 
                                             uint32_t expiry_seconds, 
                                             const std::string& content_type)
{
  if (!file_transfer_service_client_) {
    RCLCPP_ERROR(this->get_logger(), "FileTransfer服务客户端未初始化");
    // 构建 FileTransferResponse payload
    json j;
    j["timestamp"] = timestamp;
    j["task_id"] = task_id;
    j["response"] = 0;
    std::vector<uint8_t> payload_bytes = json::to_cbor(j);
    sendCommonResponse(ver, request_type, request_id, payload_bytes);
    return;
  }
  
  if (!file_transfer_service_client_->wait_for_service(std::chrono::seconds(5))) {
    RCLCPP_WARN(this->get_logger(), "FileTransfer服务不可用");
    json j;
    j["timestamp"] = timestamp;
    j["task_id"] = task_id;
    j["response"] = 0;
    std::vector<uint8_t> payload_bytes = json::to_cbor(j);
    sendCommonResponse(ver, request_type, request_id, payload_bytes);
    return;
  }
  
  auto request = std::make_shared<angmen_ad_msgs::srv::FileTransferRequest::Request>();
  request->timestamp = timestamp;
  request->task_id = task_id;
  request->action = action;
  request->file_type = file_type;
  request->file_name = file_name;
  request->size_bytes = size_bytes;
  request->checksum = checksum;
  request->description = description;
  request->download_url = download_url;
  request->expiry_seconds = expiry_seconds;
  request->content_type = content_type;
  
  auto future = file_transfer_service_client_->async_send_request(request,
    [this, ver, request_type, request_id](rclcpp::Client<angmen_ad_msgs::srv::FileTransferRequest>::SharedFuture future) {
      try {
        auto response = future.get();
        RCLCPP_INFO(this->get_logger(), "FileTransfer调用成功: response=%u, upload_url=%s", 
          response->response, response->upload_url.c_str());
        // 构建 FileTransferResponse payload
        json j;
        j["timestamp"] = response->timestamp;
        j["task_id"] = response->task_id;
        j["response"] = response->response;
        j["upload_url"] = response->upload_url;
        j["method"] = response->method;
        j["expiry_seconds"] = response->expiry_seconds;
        j["headers"] = response->headers;
        std::vector<uint8_t> payload_bytes = json::to_cbor(j);
        sendCommonResponse(ver, request_type, request_id, payload_bytes);
      } catch (const std::exception& e) {
        RCLCPP_ERROR(this->get_logger(), "FileTransfer调用异常: %s", e.what());
        auto now = std::chrono::system_clock::now();
        double error_timestamp = std::chrono::duration<double>(now.time_since_epoch()).count();
        json j;
        j["timestamp"] = error_timestamp;
        j["task_id"] = "";
        j["response"] = 0;
        std::vector<uint8_t> payload_bytes = json::to_cbor(j);
        sendCommonResponse(ver, request_type, request_id, payload_bytes);
      }
    }
  );
  
  RCLCPP_INFO(this->get_logger(), "已发送FileTransfer服务请求");
}

// =============================================================================
// RouteQuery 实现 (MQTT -> ROS2 Service)
// =============================================================================

void MQTTBridgeNode::handleRouteQueryRequest(uint8_t ver, uint32_t request_type,
                                                 const std::vector<uint8_t>& request_id,
                                                 const std::string& payload)
{
  try {
    // 从 CBOR 解析
    std::vector<uint8_t> cbor_bytes(payload.begin(), payload.end());
    json j = json::from_cbor(cbor_bytes);
    
    // 解析字段: timestamp, vin, request
    if (!j.contains("timestamp") || !j.contains("vin") || !j.contains("request")) {
      RCLCPP_ERROR(this->get_logger(), "RouteQueryRequest缺少必需字段");
      sendErrorResponse(ver, request_type, request_id, "RouteQueryRequest缺少必需字段");
      return;
    }
    
    double timestamp = j["timestamp"].get<double>();
    std::string vin = j["vin"].get<std::string>();
    uint32_t request = j["request"].get<uint32_t>();

    // RouteQuery.request 当前协议仅定义 0：查询当前已规划路径
    if (request != 0) {
      RCLCPP_ERROR(this->get_logger(),
        "RouteQueryRequest request无效: %u（仅支持0=查询当前已规划路径）", request);
      sendErrorResponse(ver, request_type, request_id,
        "RouteQueryRequest request无效，仅支持0=查询当前已规划路径");
      return;
    }

    // VIN 校验
    if (!validateVIN(vin)) {
      RCLCPP_ERROR(this->get_logger(), "RouteQueryRequest VIN校验失败，拒绝处理");
      sendErrorResponse(ver, request_type, request_id, "VIN mismatch: " + vin);
      return;
    }
    
    RCLCPP_INFO(this->get_logger(), 
      "收到RouteQueryRequest - VIN: %s, 请求: %s", 
      vin.c_str(), 
      "查询当前已规划路径");
    
    // 调用ROS2服务
    callRouteQueryService(ver, request_type, request_id, timestamp, request);
    
  } catch (const json::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "解析RouteQueryRequest失败: %s", e.what());
    sendErrorResponse(ver, request_type, request_id, std::string("解析RouteQueryRequest失败: ") + e.what());
  }
}

void MQTTBridgeNode::callRouteQueryService(uint8_t ver, uint32_t request_type,
                                           const std::vector<uint8_t>& request_id,
                                           double timestamp, uint32_t request)
{
  if (!route_query_service_client_) {
    RCLCPP_ERROR(this->get_logger(), "RouteQuery服务客户端未初始化");
    // 构建 RouteQueryResponse payload
    json j;
    j["timestamp"] = timestamp;
    j["response"] = 0;
    j["route_longitude"] = std::vector<double>{};
    j["route_latitude"] = std::vector<double>{};
    std::vector<uint8_t> payload_bytes = json::to_cbor(j);
    sendCommonResponse(ver, request_type, request_id, payload_bytes);
    return;
  }
  
  if (!route_query_service_client_->wait_for_service(std::chrono::seconds(5))) {
    RCLCPP_WARN(this->get_logger(), "RouteQuery服务不可用");
    json j;
    j["timestamp"] = timestamp;
    j["response"] = 0;
    j["route_longitude"] = std::vector<double>{};
    j["route_latitude"] = std::vector<double>{};
    std::vector<uint8_t> payload_bytes = json::to_cbor(j);
    sendCommonResponse(ver, request_type, request_id, payload_bytes);
    return;
  }
  
  auto srv_request = std::make_shared<angmen_ad_msgs::srv::RouteQueryRequest::Request>();
  srv_request->timestamp = timestamp;
  srv_request->request = request;
  
  auto future = route_query_service_client_->async_send_request(srv_request,
    [this, ver, request_type, request_id](rclcpp::Client<angmen_ad_msgs::srv::RouteQueryRequest>::SharedFuture future) {
      try {
        auto response = future.get();
        RCLCPP_INFO(this->get_logger(), "RouteQuery调用成功: response=%u, 路点数=%zu", 
          response->response, response->route_longitude.size());
        // 构建 RouteQueryResponse payload
        json j;
        j["timestamp"] = response->timestamp;
        j["response"] = response->response;
        if (response->route_longitude.size() != response->route_latitude.size()) {
          throw std::runtime_error("RouteQuery响应经纬度数组长度不一致");
        }
        std::vector<double> route_longitude;
        std::vector<double> route_latitude;
        route_longitude.reserve(response->route_longitude.size());
        route_latitude.reserve(response->route_latitude.size());
        for (size_t index = 0; index < response->route_longitude.size(); ++index) {
          const auto gcj02 = wgs84ToGcj02(
            response->route_latitude[index], response->route_longitude[index]);
          route_longitude.push_back(gcj02.second);
          route_latitude.push_back(gcj02.first);
        }
        j["route_longitude"] = route_longitude;
        j["route_latitude"] = route_latitude;
        std::vector<uint8_t> payload_bytes = json::to_cbor(j);
        sendCommonResponse(ver, request_type, request_id, payload_bytes);
      } catch (const std::exception& e) {
        RCLCPP_ERROR(this->get_logger(), "RouteQuery调用异常: %s", e.what());
        auto now = std::chrono::system_clock::now();
        double error_timestamp = std::chrono::duration<double>(now.time_since_epoch()).count();
        json j;
        j["timestamp"] = error_timestamp;
        j["response"] = 0;
        j["route_longitude"] = std::vector<double>{};
        j["route_latitude"] = std::vector<double>{};
        std::vector<uint8_t> payload_bytes = json::to_cbor(j);
        sendCommonResponse(ver, request_type, request_id, payload_bytes);
      }
    }
  );
  
  RCLCPP_INFO(this->get_logger(), "已发送RouteQuery服务请求");
}

// =============================================================================
// PathPlanning 实现 (MQTT -> ROS2 Service)
// =============================================================================

void MQTTBridgeNode::handlePathPlanningRequest(uint8_t ver, uint32_t request_type,
                                               const std::vector<uint8_t>& request_id,
                                               const std::string& payload)
{
  try {
    std::vector<uint8_t> cbor_bytes(payload.begin(), payload.end());
    json j = json::from_cbor(cbor_bytes);

    if (!j.contains("timestamp") || !j.contains("vin") ||
        !j.contains("longitude") || !j.contains("latitude")) {
      RCLCPP_ERROR(this->get_logger(), "PathPlanningRequest缺少必需字段");
      sendErrorResponse(ver, request_type, request_id, "PathPlanningRequest缺少必需字段");
      return;
    }

    if (!j["longitude"].is_array() || !j["latitude"].is_array()) {
      RCLCPP_ERROR(this->get_logger(), "PathPlanningRequest经纬度必须为数组");
      sendErrorResponse(ver, request_type, request_id, "PathPlanningRequest经纬度必须为数组");
      return;
    }

    double timestamp = j["timestamp"].get<double>();
    std::string vin = j["vin"].get<std::string>();
    std::vector<double> longitude = j["longitude"].get<std::vector<double>>();
    std::vector<double> latitude = j["latitude"].get<std::vector<double>>();

    if (!validateVIN(vin)) {
      RCLCPP_ERROR(this->get_logger(), "PathPlanningRequest VIN校验失败，拒绝处理");
      sendErrorResponse(ver, request_type, request_id, "VIN mismatch: " + vin);
      return;
    }

    if (longitude.empty() || longitude.size() != latitude.size()) {
      RCLCPP_ERROR(this->get_logger(),
        "PathPlanningRequest站点数组无效: longitude=%zu, latitude=%zu",
        longitude.size(), latitude.size());
      sendErrorResponse(ver, request_type, request_id,
        "PathPlanningRequest站点数组不能为空且长度必须一致");
      return;
    }

    for (size_t index = 0; index < longitude.size(); ++index) {
      const auto wgs84 = gcj02ToWgs84(latitude[index], longitude[index]);
      longitude[index] = wgs84.second;
      latitude[index] = wgs84.first;
    }

    RCLCPP_INFO(this->get_logger(),
      "收到PathPlanningRequest - VIN: %s, 站点数: %zu",
      vin.c_str(), longitude.size());

    callPathPlanningService(
      ver, request_type, request_id, timestamp, longitude, latitude);
  } catch (const json::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "解析PathPlanningRequest失败: %s", e.what());
    sendErrorResponse(ver, request_type, request_id,
      std::string("解析PathPlanningRequest失败: ") + e.what());
  } catch (const std::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "处理PathPlanningRequest失败: %s", e.what());
    sendErrorResponse(ver, request_type, request_id,
      std::string("处理PathPlanningRequest失败: ") + e.what());
  }
}

void MQTTBridgeNode::callPathPlanningService(uint8_t ver, uint32_t request_type,
                                             const std::vector<uint8_t>& request_id,
                                             double timestamp, const std::vector<double>& longitude,
                                             const std::vector<double>& latitude)
{
  auto send_failure = [this, ver, request_type, request_id, timestamp]() {
    json j;
    j["timestamp"] = timestamp;
    j["response"] = 0;
    j["route_longitude"] = std::vector<double>{};
    j["route_latitude"] = std::vector<double>{};
    sendCommonResponse(ver, request_type, request_id, json::to_cbor(j));
  };

  if (!path_planning_service_client_) {
    RCLCPP_ERROR(this->get_logger(), "PathPlanning服务客户端未初始化");
    send_failure();
    return;
  }

  if (!path_planning_service_client_->wait_for_service(std::chrono::seconds(5))) {
    RCLCPP_WARN(this->get_logger(), "PathPlanning服务不可用");
    send_failure();
    return;
  }

  auto srv_request = std::make_shared<angmen_ad_msgs::srv::PathPlanningRequest::Request>();
  srv_request->timestamp = timestamp;
  srv_request->longitude = longitude;
  srv_request->latitude = latitude;

  path_planning_service_client_->async_send_request(
    srv_request,
    [this, ver, request_type, request_id, timestamp](
      rclcpp::Client<angmen_ad_msgs::srv::PathPlanningRequest>::SharedFuture future) {
      try {
        auto response = future.get();
        if (response->route_longitude.size() != response->route_latitude.size()) {
          throw std::runtime_error("PathPlanning响应经纬度数组长度不一致");
        }

        json j;
        j["timestamp"] = response->timestamp;
        j["response"] = response->response;
        std::vector<double> route_longitude;
        std::vector<double> route_latitude;
        route_longitude.reserve(response->route_longitude.size());
        route_latitude.reserve(response->route_latitude.size());
        for (size_t index = 0; index < response->route_longitude.size(); ++index) {
          const auto gcj02 = wgs84ToGcj02(
            response->route_latitude[index], response->route_longitude[index]);
          route_longitude.push_back(gcj02.second);
          route_latitude.push_back(gcj02.first);
        }
        j["route_longitude"] = route_longitude;
        j["route_latitude"] = route_latitude;
        sendCommonResponse(ver, request_type, request_id, json::to_cbor(j));

        RCLCPP_INFO(this->get_logger(),
          "PathPlanning调用成功: response=%u, 路点数=%zu",
          response->response, response->route_longitude.size());
      } catch (const std::exception& e) {
        RCLCPP_ERROR(this->get_logger(), "PathPlanning调用异常: %s", e.what());
        json j;
        j["timestamp"] = timestamp;
        j["response"] = 0;
        j["route_longitude"] = std::vector<double>{};
        j["route_latitude"] = std::vector<double>{};
        sendCommonResponse(ver, request_type, request_id, json::to_cbor(j));
      }
    });

  RCLCPP_INFO(this->get_logger(), "已发送PathPlanning服务请求");
}

}  // namespace mqtt_bridge

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<mqtt_bridge::MQTTBridgeNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
