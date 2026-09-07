#include "cloud_bridge/mqtt_client.h"

#include <algorithm>
#include <chrono>

namespace cloud_bridge
{

MqttClient::MqttClient(const MqttConfig& config)
  : config_(config),
    client_("tcp://" + config.host + ":" + std::to_string(config.port),
            config.client_id)
{
  conn_opts_.set_clean_session(true);
  conn_opts_.set_keep_alive_interval(config.keep_alive_interval);
  conn_opts_.set_connect_timeout(std::chrono::seconds(config.connect_timeout));
  if (!config.username.empty())
  {
    conn_opts_.set_user_name(config.username);
    conn_opts_.set_password(config.password);
  }
  client_.set_callback(*this);
}

MqttClient::~MqttClient()
{
  stop();
}

bool MqttClient::start()
{
  reconnect_thread_ = std::thread(&MqttClient::reconnectLoop, this);
  try
  {
    client_.connect(conn_opts_)->wait();
    connected_ = true;
    resubscribeAll();
    return true;
  }
  catch (const mqtt::exception&)
  {
    connected_ = false;
    reconnect_needed_ = true;  // 交给重连线程继续尝试
    return false;
  }
}

void MqttClient::stop()
{
  stopping_ = true;
  reconnect_needed_ = false;
  if (reconnect_thread_.joinable())
    reconnect_thread_.join();
  if (connected_.load())
  {
    try
    {
      client_.disconnect()->wait();
    }
    catch (const mqtt::exception&)
    {
    }
  }
  connected_ = false;
}

bool MqttClient::publish(const std::string& topic, const std::string& payload, int qos)
{
  return publish(topic, payload.data(), payload.size(), qos);
}

bool MqttClient::publish(const std::string& topic, const void* payload, size_t len, int qos)
{
  if (!connected_.load())
    return false;
  try
  {
    client_.publish(topic, payload, len, qos, false);
    return true;
  }
  catch (const mqtt::exception&)
  {
    return false;
  }
}

void MqttClient::addSubscription(const std::string& topic, int qos)
{
  {
    std::lock_guard<std::mutex> lock(sub_mutex_);
    subscriptions_.emplace_back(topic, qos);
  }
  if (connected_.load())
  {
    try
    {
      client_.subscribe(topic, qos)->wait();
    }
    catch (const mqtt::exception&)
    {
    }
  }
}

void MqttClient::connection_lost(const std::string& /*cause*/)
{
  connected_ = false;
  if (!stopping_.load())
    reconnect_needed_ = true;
}

void MqttClient::message_arrived(mqtt::const_message_ptr msg)
{
  if (message_cb_)
    message_cb_(msg->get_topic(), msg->get_payload_str());
}

void MqttClient::resubscribeAll()
{
  std::lock_guard<std::mutex> lock(sub_mutex_);
  for (const auto& sub : subscriptions_)
  {
    try
    {
      client_.subscribe(sub.first, sub.second)->wait();
    }
    catch (const mqtt::exception&)
    {
    }
  }
}

void MqttClient::reconnectLoop()
{
  int delay = 1;
  while (!stopping_.load())
  {
    if (!reconnect_needed_.load())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      continue;
    }
    try
    {
      client_.connect(conn_opts_)->wait();
      connected_ = true;
      reconnect_needed_ = false;
      delay = 1;
      resubscribeAll();
    }
    catch (const mqtt::exception&)
    {
      std::this_thread::sleep_for(std::chrono::seconds(delay));
      delay = std::min(delay * 2, 30);
    }
  }
}

}  // namespace cloud_bridge
