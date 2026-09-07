#ifndef MQTT_BRIDGE__CONFIG_MANAGER_HPP_
#define MQTT_BRIDGE__CONFIG_MANAGER_HPP_

#include <string>
#include <vector>
#include <map>
#include <yaml-cpp/yaml.h>
#include "mqtt_client.hpp"

namespace mqtt_bridge
{

struct TopicMapping
{
  std::string topic_name;
  std::string direction;  // "ros2_to_mqtt" or "mqtt_to_ros2"
  std::string message_type;  // "mqtt_bridge/msg/CarStatus", "CommonRequest", "CommonResponse"
  int qos = 1;
};

class ConfigManager
{
public:
  ConfigManager() = default;

  bool loadFromFile(const std::string& config_file);

  MQTTConfig getMQTTConfig() const { return mqtt_config_; }
  std::vector<TopicMapping> getTopicMappings() const { return topic_mappings_; }

  // 自动生成话题映射
  std::vector<TopicMapping> generateTopicMappings() const;

  // 获取车辆位置信息
  std::string getProvince() const { return province_; }
  std::string getCity() const { return city_; }
  std::string getZone() const { return zone_; }
  std::string getVIN() const { return vin_; }
  void setVIN(const std::string& vin);

private:
  MQTTConfig mqtt_config_;
  std::vector<TopicMapping> topic_mappings_;

  // 车辆位置信息
  std::string province_;
  std::string city_;
  std::string zone_;
  std::string vin_;

  // 话题类型定义
  struct TopicDefinition {
    std::string ros_topic;
    std::string mqtt_topic_type;
    std::string direction;
    std::string message_type;
    int qos;
  };

  std::vector<TopicDefinition> topic_definitions_;

  void initializeTopicDefinitions();
  std::string generateMQTTTopic(const std::string& topic_type) const;
};

}  // namespace mqtt_bridge

#endif  // MQTT_BRIDGE__CONFIG_MANAGER_HPP_
