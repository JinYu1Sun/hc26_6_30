#include "mqtt_bridge/mqtt_client.hpp"
#include <rclcpp/rclcpp.hpp>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace mqtt_bridge
{

MQTTClient::MQTTClient(const MQTTConfig& config)
  : config_(config)
{
  if (config_.sm2_enabled) {
    if (config_.sm2_mode != "c1c2c3") {
      throw std::invalid_argument("测试版SM2仅支持c1c2c3模式");
    }
    if (config_.sm2_encoding != "binary") {
      throw std::invalid_argument("测试版SM2仅支持binary编码");
    }
    sm2_crypto_ = std::make_unique<SM2Crypto>(
      config_.sm2_private_key, config_.sm2_public_key);
    std::cout << "[MQTTClient] SM2测试加密已启用: C1C2C3/binary" << std::endl;
  }

  // 根据 TLS 配置构建 URI
  std::string protocol = config_.enable_tls ? "ssl" : "tcp";
  std::string uri = protocol + "://" + config_.host + ":" + std::to_string(config_.port);

  try {
    client_ = std::make_unique<mqtt::client>(uri, config_.client_id);
    std::cout << "[MQTTClient] MQTT客户端创建成功: " << uri
              << " (TLS=" << (config_.enable_tls ? "on" : "off") << ")" << std::endl;
  } catch (const mqtt::exception& e) {
    std::cerr << "[MQTTClient] 创建MQTT客户端失败: " << e.what() << std::endl;
    throw;
  }
}

MQTTClient::~MQTTClient()
{
  disconnect();
}

bool MQTTClient::connect()
{
  try {
    mqtt::connect_options conn_opts;
    conn_opts.set_clean_session(true);
    conn_opts.set_connect_timeout(config_.connect_timeout);
    conn_opts.set_keep_alive_interval(config_.keep_alive_interval);

    // 设置用户名和密码认证
    if (!config_.username.empty() && !config_.password.empty()) {
      conn_opts.set_user_name(config_.username);
      conn_opts.set_password(config_.password);
      std::cout << "[MQTTClient] 使用认证: username=" << config_.username << std::endl;
    }

    // TLS/mTLS 配置
    if (config_.enable_tls) {
      mqtt::ssl_options ssl_opts;
      ssl_opts.set_trust_store(config_.ca_file);
      if (!config_.cert_file.empty() && !config_.key_file.empty()) {
        ssl_opts.set_key_store(config_.cert_file);
        ssl_opts.set_private_key(config_.key_file);
        std::cout << "[MQTTClient] 启用 mTLS: ca=" << config_.ca_file
                  << ", cert=" << config_.cert_file << std::endl;
      } else {
        std::cout << "[MQTTClient] 启用 TLS: ca=" << config_.ca_file << std::endl;
      }
      conn_opts.set_ssl(ssl_opts);
    }

    client_->connect(conn_opts);
    connected_ = true;

    // 设置MQTT消息回调函数
    client_->set_callback(*this);

    if (connection_callback_) {
      connection_callback_(true);
    }

    std::cout << "[MQTTClient] 连接成功: " << config_.host << ":" << config_.port << std::endl;
    return true;

  } catch (const mqtt::exception& e) {
    std::cerr << "[MQTTClient] 连接失败: " << e.what() << std::endl;
    connected_ = false;
    return false;
  }
}

void MQTTClient::disconnect()
{
  std::cout << "[MQTTClient] 断开连接, connected=" << connected_ << std::endl;

  // 先停止重连
  std::unique_ptr<std::thread> reconnect_thread;
  {
    std::lock_guard<std::mutex> lock(reconnect_mutex_);
    reconnect_enabled_ = false;
    reconnect_requested_ = false;
    reconnect_thread = std::move(reconnect_thread_);
  }
  reconnect_cv_.notify_all();
  if (reconnect_thread && reconnect_thread->joinable()) {
    reconnect_thread->join();
  }

  if (client_ && connected_) {
    try {
      std::cout << "[MQTTClient] 调用 client->disconnect()" << std::endl;
      client_->disconnect();
      std::cout << "[MQTTClient] client->disconnect() 完成" << std::endl;
    } catch (const mqtt::exception& e) {
      std::cerr << "[MQTTClient] 断开连接错误: " << e.what() << std::endl;
    }
    connected_ = false;
    std::cout << "[MQTTClient] 发送断开连接信号" << std::endl;
    if (connection_callback_) {
      std::cout << "[MQTTClient] 调用 connection_callback_" << std::endl;
      connection_callback_(false);
    }
    std::cout << "[MQTTClient] 断开连接完成" << std::endl;
  }

  // 清理MQTT客户端资源
  if (client_) {
    std::cout << "[MQTTClient] 删除MQTT客户端" << std::endl;
    client_.reset();
    std::cout << "[MQTTClient] MQTT客户端已删除" << std::endl;
  }

  std::cout << "[MQTTClient] 断开连接完成" << std::endl;
}

bool MQTTClient::isConnected() const
{
  return connected_ && client_ && client_->is_connected();
}

bool MQTTClient::publish(const std::string& topic, const std::string& payload, int qos, bool retained)
{
  if (!isConnected()) {
    std::cerr << "[MQTTClient] 未连接" << std::endl;
    return false;
  }

  try {
    std::string wire_payload = payload;
    if (sm2_crypto_) {
      wire_payload = sm2_crypto_->encryptC1C2C3(payload);
    }

    auto msg = mqtt::make_message(topic, wire_payload);
    msg->set_qos(qos);
    msg->set_retained(retained);
    client_->publish(msg);
    return true;
  } catch (const mqtt::exception& e) {
    std::cerr << "[MQTTClient] 发布失败: " << e.what() << std::endl;
    return false;
  } catch (const std::exception& e) {
    std::cerr << "[MQTTClient] SM2加密失败，消息未发送: " << e.what() << std::endl;
    return false;
  }
}

bool MQTTClient::subscribe(const std::string& topic, int qos)
{
  {
    std::lock_guard<std::mutex> lock(subscribe_mutex_);
    bool found = false;
    for (auto& [saved_topic, saved_qos] : subscribed_topics_) {
      if (saved_topic == topic) {
        saved_qos = qos;
        found = true;
        break;
      }
    }
    if (!found) {
      subscribed_topics_.emplace_back(topic, qos);
    }
  }

  if (!isConnected()) {
    std::cerr << "[MQTTClient] 未连接，已记录订阅，连接恢复后自动订阅: "
              << topic << std::endl;
    return false;
  }

  try {
    client_->subscribe(topic, qos);
    std::cout << "[MQTTClient] 订阅话题: " << topic << " (QoS=" << qos << ")" << std::endl;
    return true;
  } catch (const mqtt::exception& e) {
    std::cerr << "[MQTTClient] 订阅失败: " << e.what() << std::endl;
    return false;
  }
}

bool MQTTClient::unsubscribe(const std::string& topic)
{
  if (!isConnected()) {
    std::cerr << "[MQTTClient] 未连接" << std::endl;
    return false;
  }

  try {
    client_->unsubscribe(topic);
    {
      std::lock_guard<std::mutex> lock(subscribe_mutex_);
      subscribed_topics_.erase(
        std::remove_if(subscribed_topics_.begin(), subscribed_topics_.end(),
          [&topic](const auto& p) { return p.first == topic; }),
        subscribed_topics_.end());
    }
    return true;
  } catch (const mqtt::exception& e) {
    std::cerr << "[MQTTClient] 取消订阅失败: " << e.what() << std::endl;
    return false;
  }
}

void MQTTClient::setMessageCallback(MessageCallback callback)
{
  message_callback_ = callback;
}

void MQTTClient::setConnectionCallback(ConnectionCallback callback)
{
  connection_callback_ = callback;
}

std::string MQTTClient::generateTopic(const std::string& topic_type)
{
  if (vin_.empty() || province_.empty() || city_.empty() || zone_.empty()) {
    std::cerr << "[MQTTClient] 车辆配置未设置，使用默认话题" << std::endl;
    return topic_type;  // 返回原始话题作为fallback
  }

  if (topic_type == "remote_session_control") {
    return "cabr/j10a/" + province_ + "/" + city_ + "/" + zone_ + "/" + vin_ + "/remote_session_control/request";
  } else if (topic_type == "remote_session_control_response") {
    return "cabr/j10a/" + province_ + "/" + city_ + "/" + zone_ + "/" + vin_ + "/remote_session_control/response";
  } else if (topic_type == "remote_command") {
    return "cabr/j10a/" + province_ + "/" + city_ + "/" + zone_ + "/" + vin_ + "/remote_command";
  } else if (topic_type == "drive_cmd") {
    // 保持向后兼容
    return "cabr/j10a/" + province_ + "/" + city_ + "/" + zone_ + "/" + vin_ + "/drive_cmd";
  } else if (topic_type == "car_status") {
    return "cabr/j10a/" + province_ + "/" + city_ + "/" + zone_ + "/" + vin_ + "/car_status";
  }

  return topic_type;  // 默认返回原始话题
}

void MQTTClient::setVehicleConfig(const std::string& vin, const std::string& province,
                                 const std::string& city, const std::string& zone)
{
  vin_ = vin;
  province_ = province;
  city_ = city;
  zone_ = zone;
  std::cout << "[MQTTClient] 车辆配置已设置: vin=" << vin << ", location="
            << province << "/" << city << "/" << zone << std::endl;
}

// ============================================================================
// paho-mqtt-cpp 回调方法实现
// ============================================================================

void MQTTClient::message_arrived(mqtt::const_message_ptr msg)
{
  // 检查对象是否仍然有效
  if (!connected_) {
    std::cout << "[MQTTClient] 收到消息但客户端已断开，忽略" << std::endl;
    return;
  }

  try {
    // 将paho-mqtt消息转换为回调
    std::string topic = msg->get_topic();
    std::string payload = msg->to_string();
    if (sm2_crypto_) {
      payload = sm2_crypto_->decryptC1C2C3(payload);
    }

    std::cout << "[MQTTClient] MQTT消息到达: " << topic
              << " (" << payload.size() << " 字节)" << std::endl;

    // 调用消息回调
    if (message_callback_) {
      message_callback_(topic, payload);
    }
  } catch (const std::exception& e) {
    std::cerr << "[MQTTClient] MQTT消息解密或处理失败，消息已丢弃: "
              << e.what() << std::endl;
  }
}

void MQTTClient::connection_lost(const std::string& cause)
{
  std::cout << "[MQTTClient] 连接丢失: " << cause << std::endl;
  connected_ = false;
  if (connection_callback_) {
    connection_callback_(false);
  }

  startReconnect();
}

void MQTTClient::startReconnect()
{
  {
    std::lock_guard<std::mutex> lock(reconnect_mutex_);
    if (!reconnect_enabled_) {
      return;
    }
    reconnect_requested_ = true;
    if (!reconnect_thread_) {
      reconnect_thread_ = std::make_unique<std::thread>(&MQTTClient::attemptReconnect, this);
    }
  }
  reconnect_cv_.notify_all();
}

void MQTTClient::attemptReconnect()
{
  std::unique_lock<std::mutex> reconnect_lock(reconnect_mutex_);
  while (reconnect_enabled_) {
    reconnect_cv_.wait(reconnect_lock, [this]() {
      return !reconnect_enabled_ || reconnect_requested_;
    });
    if (!reconnect_enabled_) {
      break;
    }
    reconnect_requested_ = false;
    reconnect_lock.unlock();

    int delay = 1;
    while (reconnect_enabled_ && !connected_) {
      std::unique_lock<std::mutex> wait_lock(reconnect_mutex_);
      reconnect_cv_.wait_for(wait_lock, std::chrono::seconds(delay), [this]() {
        return !reconnect_enabled_;
      });
      if (!reconnect_enabled_ || connected_) {
        break;
      }
      wait_lock.unlock();

      std::cout << "[MQTTClient] 尝试重新连接 (延迟=" << delay << "s)..." << std::endl;
      if (connect()) {
        std::cout << "[MQTTClient] 重新连接成功" << std::endl;
        std::lock_guard<std::mutex> lock(subscribe_mutex_);
        for (const auto& [topic, qos] : subscribed_topics_) {
          try {
            client_->subscribe(topic, qos);
            std::cout << "[MQTTClient] 重新订阅话题: " << topic
                      << " (QoS=" << qos << ")" << std::endl;
          } catch (const mqtt::exception& e) {
            std::cerr << "[MQTTClient] 重新订阅失败: " << topic
                      << " - " << e.what() << std::endl;
          }
        }
        break;
      }
      delay = std::min(delay * 2, MAX_RECONNECT_DELAY);
    }
    reconnect_lock.lock();
  }
}

void MQTTClient::delivery_complete(mqtt::delivery_token_ptr token)
{
  // 消息投递完成回调（可选实现）
  if (token) {
    std::cout << "[MQTTClient] 消息投递完成: "
              << token->get_message()->get_topic() << std::endl;
  }
}

}  // namespace mqtt_bridge
