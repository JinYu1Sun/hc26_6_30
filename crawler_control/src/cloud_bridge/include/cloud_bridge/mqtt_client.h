#ifndef CLOUD_BRIDGE_MQTT_CLIENT_H
#define CLOUD_BRIDGE_MQTT_CLIENT_H

#include <mqtt/async_client.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace cloud_bridge
{

  struct MqttConfig
  {
    std::string host = "127.0.0.1";
    int port = 1883;
    std::string client_id = "cloud_bridge";
    std::string username;      // 留空表示不使用用户名密码
    std::string password;
    int keep_alive_interval = 30;
    int connect_timeout = 5;
  };

  // paho async_client 的简单封装：明文 TCP 连接，断线指数退避自动重连，
  // 重连成功后自动恢复全部订阅。
  class MqttClient : public virtual mqtt::callback
  {
    public:
      using MessageCallback =
          std::function<void(const std::string& topic, const std::string& payload)>;

      explicit MqttClient(const MqttConfig& config);
      ~MqttClient() override;

      bool start();   // 阻塞式首次连接，成功返回 true
      void stop();    // 停止重连线程并断开连接
      bool isConnected() const { return connected_.load(); }

      bool publish(const std::string& topic, const std::string& payload, int qos = 0);
      bool publish(const std::string& topic, const void* payload, size_t len, int qos = 0);

      void addSubscription(const std::string& topic, int qos = 1);
      void setMessageCallback(MessageCallback cb) { message_cb_ = std::move(cb); }

    private:
      void connection_lost(const std::string& cause) override;
      void message_arrived(mqtt::const_message_ptr msg) override;
      void delivery_complete(mqtt::delivery_token_ptr /*token*/) override {}

      void reconnectLoop();
      void resubscribeAll();

      MqttConfig config_;
      mqtt::async_client client_;
      mqtt::connect_options conn_opts_;
      MessageCallback message_cb_;

      std::atomic<bool> connected_{false};
      std::atomic<bool> stopping_{false};
      std::atomic<bool> reconnect_needed_{false};
      std::thread reconnect_thread_;

      std::mutex sub_mutex_;
      std::vector<std::pair<std::string, int>> subscriptions_;
  };

}  // namespace cloud_bridge

#endif  // CLOUD_BRIDGE_MQTT_CLIENT_H
