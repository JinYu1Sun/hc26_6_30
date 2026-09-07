#include "mqtt_bridge/config_manager.hpp"
#include <fstream>
#include <iostream>
#include <cstdlib>  // for std::getenv
#include <filesystem>

namespace mqtt_bridge
{

bool ConfigManager::loadFromFile(const std::string& config_file)
{
  try {
    YAML::Node config = YAML::LoadFile(config_file);

    // 加载MQTT配置（新格式）
    if (config["mqtt"]) {
      auto mqtt = config["mqtt"];
      std::string host = mqtt["host"].as<std::string>();
      int port = mqtt["port"].as<int>();
      mqtt_config_.host = host;
      mqtt_config_.port = port;
      mqtt_config_.client_id = mqtt["client_id"].as<std::string>();
      mqtt_config_.username = mqtt["username"].as<std::string>("");
      mqtt_config_.password = mqtt["password"].as<std::string>("");
      mqtt_config_.qos = mqtt["qos"].as<int>(1);
      mqtt_config_.keep_alive_interval = mqtt["keep_alive_interval"].as<int>(30);
      mqtt_config_.connect_timeout = mqtt["connect_timeout"].as<int>(5);

      // 加载 TLS/mTLS 配置（可选）
      if (mqtt["enable_tls"]) {
        mqtt_config_.enable_tls = mqtt["enable_tls"].as<bool>(false);
        if (mqtt_config_.enable_tls) {
          mqtt_config_.ca_file = mqtt["ca_file"].as<std::string>("");
          mqtt_config_.cert_file = mqtt["cert_file"].as<std::string>("");
          mqtt_config_.key_file = mqtt["key_file"].as<std::string>("");
        }
      }
    }

    if (config["sm2"]) {
      auto sm2 = config["sm2"];
      mqtt_config_.sm2_enabled = sm2["enabled"].as<bool>(false);
      mqtt_config_.sm2_mode = sm2["mode"].as<std::string>("c1c2c3");
      mqtt_config_.sm2_encoding = sm2["encoding"].as<std::string>("binary");

      if (mqtt_config_.sm2_enabled) {
        std::filesystem::path key_file = sm2["key_file"].as<std::string>("");
        if (key_file.empty()) {
          throw YAML::Exception(sm2.Mark(), "启用SM2时key_file不能为空");
        }
        if (key_file.is_relative()) {
          key_file = std::filesystem::path(config_file).parent_path() / key_file;
        }

        YAML::Node key_config = YAML::LoadFile(key_file.string());
        if (!key_config["sm2"] || !key_config["sm2"]["private_key"] ||
            !key_config["sm2"]["public_key"]) {
          throw YAML::Exception(key_config.Mark(), "SM2测试密钥文件缺少private_key或public_key");
        }
        mqtt_config_.sm2_private_key =
          key_config["sm2"]["private_key"].as<std::string>();
        mqtt_config_.sm2_public_key =
          key_config["sm2"]["public_key"].as<std::string>();
      }
    }

    // 环境变量覆盖配置（优先于 YAML 配置）
    const char* env_host = std::getenv("MQTT_HOST");
    if (env_host) {
      mqtt_config_.host = env_host;
      std::cout << "[Config] 环境变量覆盖: MQTT_HOST=" << env_host << std::endl;
    }
    const char* env_port = std::getenv("MQTT_PORT");
    if (env_port) {
      mqtt_config_.port = std::stoi(env_port);
      std::cout << "[Config] 环境变量覆盖: MQTT_PORT=" << env_port << std::endl;
    }
    const char* env_user = std::getenv("MQTT_USERNAME");
    if (env_user) {
      mqtt_config_.username = env_user;
      std::cout << "[Config] 环境变量覆盖: MQTT_USERNAME=" << env_user << std::endl;
    }
    const char* env_pass = std::getenv("MQTT_PASSWORD");
    if (env_pass) {
      mqtt_config_.password = env_pass;
      std::cout << "[Config] 环境变量覆盖: MQTT_PASSWORD=****" << std::endl;
    }

    // 加载车辆位置信息
    if (config["vehicle"]) {
      auto vehicle = config["vehicle"];
      province_ = vehicle["province"].as<std::string>("hunan");
      city_ = vehicle["city"].as<std::string>("changsha");
      zone_ = vehicle["zone"].as<std::string>("lugu");
    }

    // 初始化话题定义
    initializeTopicDefinitions();

    std::cout << "配置加载成功: " << config_file << std::endl;
    return true;
  } catch (const YAML::Exception& e) {
    std::cerr << "配置加载失败: " << e.what() << std::endl;
    return false;
  }
}

void ConfigManager::setVIN(const std::string& vin)
{
  vin_ = vin;
  topic_mappings_ = generateTopicMappings();
}

void ConfigManager::initializeTopicDefinitions()
{
  // 车端 → 云平台/驾驶舱
  // 与angmen_ad_cockpit_demo保持一致的话题格式
  topic_definitions_ = {
    // 车辆状态报告 (QoS 0 - 尽力交付)
    {"/vehicle/car_status", "car_status", "ros2_to_mqtt", "mqtt_bridge/msg/CarStatus", 0},

    // 远程驾驶相关
    // 会话控制请求 (QoS 2 - 精确一次)
    {"/mqtt_bridge/remote_control_service", "remote_session_control/request", "mqtt_to_ros2", "RemoteControl", 2},
    // 会话控制响应 (QoS 2 - 精确一次)
    {"/mqtt_bridge/remote_control_response", "remote_session_control/response", "ros2_to_mqtt", "RemoteControlResponse", 2},
    // 远程控制指令 (QoS 2 - 精确一次)
    // 远程控制指令 (QoS 2 - 精确一次)
    {"/remote_control/command", "drive_cmd", "mqtt_to_ros2", "RemoteCommand", 2},
    // 远程驾驶报告 (QoS 1 - 至少一次)
    {"/remote_control/report", "remote_report", "ros2_to_mqtt", "RemoteReport", 1},
    // 远程驾驶状态 (QoS 1 - 至少一次)
    {"/remote_control/status", "remote_status", "ros2_to_mqtt", "RemoteStatus", 1},

    // 云平台请求/响应 (QoS 2 - 精确一次)
    {"/mqtt_bridge/cloud_request", "cloud/request", "mqtt_to_ros2", "CommonRequest", 2},
    {"/mqtt_bridge/cloud_response", "cloud/response", "ros2_to_mqtt", "CommonResponse", 2},

    // 任务相关 (QoS 2 - 精确一次)
    {"/mqtt_bridge/mission_service", "cloud/request", "mqtt_to_ros2", "MissionRequest", 2},
    {"/mqtt_bridge/mission_response", "car/response", "ros2_to_mqtt", "MissionResponse", 2},

    // 模式切换相关 (QoS 2 - 精确一次)
    {"/mqtt_bridge/mode_change_service", "cloud/request", "mqtt_to_ros2", "ModeChangeRequest", 2},
    {"/mqtt_bridge/mode_change_response", "car/response", "ros2_to_mqtt", "ModeChangeResponse", 2},

    // 视频流相关 (QoS 2 - 精确一次)
    {"/mqtt_bridge/video_streaming_service", "cloud/request", "mqtt_to_ros2", "VideoStreamingRequest", 2},
    {"/mqtt_bridge/video_streaming_response", "car/response", "ros2_to_mqtt", "VideoStreamingResponse", 2},

    // 文件传输相关 (QoS 2 - 精确一次)
    {"/mqtt_bridge/file_transfer_service", "cloud/request", "mqtt_to_ros2", "FileTransferRequest", 2},
    {"/mqtt_bridge/file_transfer_response", "car/response", "ros2_to_mqtt", "FileTransferResponse", 2},

    // 路径查询相关 (QoS 2 - 精确一次)
    {"/mqtt_bridge/route_query_service", "cloud/request", "mqtt_to_ros2", "RouteQueryRequest", 2},
    {"/mqtt_bridge/route_query_response", "car/response", "ros2_to_mqtt", "RouteQueryResponse", 2},

    // 车端事件 (QoS 2 - 精确一次)
    {"/vehicle/car_event", "car_event", "ros2_to_mqtt", "CarEvent", 2},

    // 车辆主动接管请求 (QoS 2 - 精确一次)
    {"/vehicle/takeover_request", "car/request", "ros2_to_mqtt", "TakeoverRequest", 2},
  };
}

std::string ConfigManager::generateMQTTTopic(const std::string& topic_type) const
{
  // 基础格式: cabr/j10a/{province}/{city}/{zone}/{vin}/{topic_type}
  // 与angmen_ad_cockpit_demo保持一致
  std::string topic = "cabr/j10a/" + province_ + "/" + city_ + "/" + zone_ + "/" + vin_ + "/" + topic_type;
  return topic;
}

std::vector<TopicMapping> ConfigManager::generateTopicMappings() const
{
  std::vector<TopicMapping> mappings;

  for (const auto& def : topic_definitions_) {
    TopicMapping mapping;

    // 生成完整的MQTT话题
    mapping.topic_name = generateMQTTTopic(def.mqtt_topic_type);
    mapping.direction = def.direction;
    mapping.message_type = def.message_type;
    mapping.qos = def.qos;

    mappings.push_back(mapping);

    // 输出生成的话题信息
    std::cout << "生成话题映射 - ROS: " << def.ros_topic
              << " -> MQTT: " << mapping.topic_name
              << " (" << mapping.direction << ")" << std::endl;
  }

  return mappings;
}

}  // namespace mqtt_bridge
