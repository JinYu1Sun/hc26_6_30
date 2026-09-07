#ifndef MQTT_BRIDGE__MQTT_CLIENT_HPP_
#define MQTT_BRIDGE__MQTT_CLIENT_HPP_

#include <mqtt/client.h>
#include <functional>
#include <memory>
#include <string>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>

#include "mqtt_bridge/sm2_crypto.hpp"

namespace mqtt_bridge
{

struct MQTTConfig
{
  std::string host;
  int port;
  std::string client_id;
  std::string username;
  std::string password;
  int qos = 1;
  int keep_alive_interval = 30;  // 心跳周期(秒)
  int connect_timeout = 5;       // 连接超时(秒)

  // TLS/mTLS 配置
  bool enable_tls = false;
  std::string ca_file;    // CA 证书文件路径 (mTLS 需要)
  std::string cert_file;  // 客户端证书文件路径
  std::string key_file;   // 客户端私钥文件路径

  bool sm2_enabled = false;
  std::string sm2_private_key;
  std::string sm2_public_key;
  std::string sm2_mode = "c1c2c3";
  std::string sm2_encoding = "binary";
};

class MQTTClient : public mqtt::callback
{
public:
  using MessageCallback = std::function<void(const std::string& topic, const std::string& payload)>;
  using ConnectionCallback = std::function<void(bool connected)>;

  explicit MQTTClient(const MQTTConfig& config);
  ~MQTTClient();

  bool connect();
  void startReconnect();
  void disconnect();
  bool isConnected() const;

  bool publish(const std::string& topic, const std::string& payload, int qos = 1, bool retained = false);
  bool subscribe(const std::string& topic, int qos = 1);
  bool unsubscribe(const std::string& topic);

  void setMessageCallback(MessageCallback callback);
  void setConnectionCallback(ConnectionCallback callback);

  // 话题生成方法（与angmen_ad_cockpit_demo一致）
  std::string generateTopic(const std::string& topic_type);

  // 设置车辆配置（用于生成符合协议的话题）
  void setVehicleConfig(const std::string& vin, const std::string& province,
                       const std::string& city, const std::string& zone);

private:
  // paho-mqtt-cpp 回调方法
  void message_arrived(mqtt::const_message_ptr msg) override;
  void connection_lost(const std::string& cause) override;
  void delivery_complete(mqtt::delivery_token_ptr token) override;

  MQTTConfig config_;
  std::unique_ptr<mqtt::client> client_;
  std::unique_ptr<SM2Crypto> sm2_crypto_;

  // 车辆配置（用于生成符合协议的话题）
  std::string vin_;
  std::string province_;
  std::string city_;
  std::string zone_;

  MessageCallback message_callback_;
  ConnectionCallback connection_callback_;

  std::atomic<bool> connected_{false};

  // 断线重连支持
  std::atomic<bool> reconnect_enabled_{true};
  std::unique_ptr<std::thread> reconnect_thread_;
  std::condition_variable reconnect_cv_;
  std::mutex reconnect_mutex_;
  bool reconnect_requested_ = false;
  static constexpr int MAX_RECONNECT_DELAY = 60;  // 最大重连间隔(秒)

  // 已订阅话题列表（用于重连后重新订阅）
  std::vector<std::pair<std::string, int>> subscribed_topics_;
  std::mutex subscribe_mutex_;

  void attemptReconnect();
};

}  // namespace mqtt_bridge

#endif  // MQTT_BRIDGE__MQTT_CLIENT_HPP_
