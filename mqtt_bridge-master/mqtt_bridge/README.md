# MQTT桥接模块

## 功能概述

实现云平台与ROS2的双向消息通信框架，支持TLS 1.3加密和mTLS双向认证。

## 当前实现

- ✅ MQTT客户端封装（TLS/mTLS支持）
- ✅ 自动重连机制（指数退避）
- ✅ 心跳机制（10s周期）
- ✅ 连接状态监控
- ✅ **CommonRequest/Response** - 通用请求/响应（CBOR格式）
- ✅ **MissionRequest/Response** - 任务请求/响应（CBOR格式）
- ✅ **ModeChangeRequest/Response** - 模式切换请求/响应（CBOR格式）
- ✅ **VideoStreamingRequest/Response** - 视频流控制请求/响应（CBOR格式）
- ✅ **FileTransferRequest/Response** - 文件传输请求/响应（CBOR格式）
- ✅ **RouteQueryRequest/Response** - 路径查询请求/响应（CBOR格式）
- ✅ **TwoWayAudioRequest/Response** - 双向对讲请求/响应（预留接口）
- ✅ **RemoteControl** - 远程会话控制（CBOR格式）
- ✅ **RemoteCommand** - 远程控制指令（CBOR格式）
- ✅ **CarEvent** - 车端事件上报（CBOR格式）
- ✅ **自动生成MQTT话题** - 基于基础格式自动生成话题映射

## 性能指标

- 消息转发延迟: < 50ms
- 支持QoS 0/1/2
- CPU占用: < 2%
- 内存占用: < 30MB

## 依赖安装

```bash
sudo apt install libpaho-mqtt-dev libpaho-mqttpp-dev libyaml-cpp-dev libssl-dev
```

## 编译运行

```bash
cd /home/ubuntu/trj/nanocargo_J10A
colcon build --packages-select mqtt_bridge
source install/setup.bash
ros2 launch mqtt_bridge mqtt_bridge.launch.py
```

## 话题映射规则

### 基础格式

MQTT话题格式：`cabr/j10a/{province}/{city}/{zone}/{vin}/{topic_type}`

**参数说明**:
- `province`: 省份代码 (如: hunan)
- `city`: 城市代码 (如: changsha)
- `zone`: 区域代码 (如: lugu)
- `vin`: 车辆VIN (如: LBV5S1000MS688888)
- `topic_type`: 话题类型

### ROS话题 ↔ MQTT话题对应关系

| ROS话题 | 消息类型 | MQTT话题类型 | 方向 | QoS | 说明 |
|---------|----------|--------------|------|-----|------|
| `/vehicle/car_status` | `angmen_ad_msgs/msg/CarStatus` | `car_status` | ROS2 → MQTT | 0 | 车辆状态报告 |
| `/vehicle/car_event` | `angmen_ad_msgs/msg/CarEvent` | `car_event` | ROS2 → MQTT | 2 | 车端事件 |
| `/mqtt_bridge/common_service` | `angmen_ad_msgs/srv/CommonRequest` | `cloud/request` | MQTT → ROS2 | 2 | 通用请求（包含所有请求类型） |
| `/mqtt_bridge/common_service` | `angmen_ad_msgs/srv/CommonResponse` | `cloud/response` | ROS2 → MQTT | 2 | 通用响应（包含所有响应类型） |
| `/mqtt_bridge/remote_control_service` | `angmen_ad_msgs/srv/RemoteControl` | `remote_session_control/request` | MQTT → ROS2 | 0 | 远程会话控制 |
| `/mqtt_bridge/remote_control_response` | `angmen_ad_msgs/srv/RemoteControl` | `remote_session_control/response` | ROS2 → MQTT | 2 | 会话控制响应 |
| `/remote_control/command` | `angmen_ad_msgs/msg/RemoteCommand` | `remote_command` | MQTT → ROS2 | 0 | 远程控制指令 |
| `/remote_control/report` | `angmen_ad_msgs/msg/RemoteReport` | `remote_report` | ROS2 → MQTT | 1 | 远程驾驶报告 |
| `/remote_control/status` | `angmen_ad_msgs/msg/RemoteStatus` | `remote_status` | ROS2 → MQTT | 1 | 远程驾驶状态 |

### 数据流向说明

**车端 → 云平台/驾驶舱 (ROS2 → MQTT):**
- `/vehicle/car_status` → `cabr/j10a/{province}/{city}/{zone}/{vin}/car_status`
- `/vehicle/car_event` → `cabr/j10a/{province}/{city}/{zone}/{vin}/car_event`
- `/remote_control/report` → `cabr/j10a/{province}/{city}/{zone}/{vin}/remote_report`
- `/remote_control/status` → `cabr/j10a/{province}/{city}/{zone}/{vin}/remote_status`
- 各种Response → `cabr/j10a/{province}/{city}/{zone}/{vin}/car/response`

**云平台/驾驶舱 → 车端 (MQTT → ROS2):**
- `cabr/j10a/{province}/{city}/{zone}/{vin}/cloud/request` → `/mqtt_bridge/common_service`
- `cabr/j10a/{province}/{city}/{zone}/{vin}/remote_session_control/request` → `/mqtt_bridge/remote_control_service`
- `cabr/j10a/{province}/{city}/{zone}/{vin}/remote_command` → `/remote_control/command`

## 通用请求/响应机制 (CommonRequest/Response)

### 请求类型映射

| request_type | 请求类型 | 请求载荷 | 响应载荷 | 说明 |
|-------------|---------|---------|---------|------|
| 1 | MissionRequest | MissionRequest | MissionResponse | 云端下发任务 |
| 2 | ModeChangeRequest | ModeChangeRequest | ModeChangeResponse | 切换驾驶模式 |
| 3 | VideoStreamingRequest | VideoStreamingRequest | VideoStreamingResponse | 控制视频流 |
| 4 | TwoWayAudioRequest | - | - | 双向对讲控制（预留空实现） |
| 5 | FileTransferRequest | FileTransferRequest | FileTransferResponse | OTA/文件传输 |
| 6 | TakeoverRequest | - | - | 远程接管（预留空实现） |
| 7 | RouteQueryRequest | RouteQueryRequest | RouteQueryResponse | 查询规划路径 |
| 8 | VehicleFileManagementRequest | - | - | 车端文件管理（预留空实现） |

### 请求数据结构

```json
{
  "ver": 0,                    // 版本
  "request_type": 1,           // 请求类型（见上表）
  "request_id": "uuid-12345",  // 请求ID (UUID)
  "request_payload": {...}     // 请求载荷（根据request_type变化）
}
```

### 响应数据结构

```json
{
  "ver": 0,                    // 版本
  "request_type": 1,           // 请求类型
  "request_id": "uuid-12345",  // 请求ID (UUID)
  "response_payload": {...}    // 响应载荷（根据request_type变化）
}
```

### 处理流程

```
云平台 → /cloud/request (CommonRequest CBOR)
  { ver, request_type, request_id, request_payload: {具体请求字段} }

车端 handleCommonRequest → switch(request_type) → 对应 handler
  → 解析具体请求 → 调用 ROS2 Service
  → 回调中构建具体 Response payload
  → sendCommonResponse(ver, request_type, request_id, payload)

车端 → /car/response (CommonResponse CBOR)
  { ver, request_type, request_id, response_payload: {具体响应字段} }
```

## CBOR数据结构

### CarStatus (车辆状态报告)

**话题**: `car_status`
**方向**: 车端 → 云平台
**频率**: 10 Hz
**数据类型**: **CBOR**

**数据字段** (31个字段):

| 键 | 字段名 | 类型 | 必选 | 描述 |
|---|--------|------|------|------|
| 0 | ver | uint | ✅ | 协议版本 |
| 1 | timestamp | float | ✅ | UNIX时间戳(秒) |
| 2 | seq | uint | ✅ | 序列号 |
| 3 | vin | string | ✅ | 车辆VIN |
| 4 | longitude | float | ✅ | 经度(°) |
| 5 | latitude | float | ✅ | 纬度(°) |
| 6 | altitude | float | ✅ | 海拔(m) |
| 7 | heading | float | ✅ | 航向角(°) |
| 8 | roll | float | ✅ | 横滚角(°) |
| 9 | pitch | float | ✅ | 俯仰角(°) |
| 10 | gnss_status | int | ✅ | GNSS状态 |
| 11 | gear | int | ✅ | 挡位(0:P,1:R,2:N,3:D) |
| 12 | speed | float | ✅ | 速度(m/s) |
| 13 | brake | float | ✅ | 刹车(0-1) |
| 14 | steer | float | ✅ | 转向角(°) |
| 15 | door | int | ✅ | 车门状态 |
| 16 | parking_brake | int | ✅ | 驻车制动 |
| 17 | turn_signal | int | ✅ | 转向灯 |
| 18 | low_beam | int | ✅ | 近光灯 |
| 19 | high_beam | int | ✅ | 远光灯 |
| 20 | position_light | int | ✅ | 位置灯 |
| 21 | odometer | float | ✅ | 里程表(km) |
| 22 | trip_odometer | float | ✅ | 本次里程(km) |
| 23 | estimated_range | int | ✅ | 预估续航(km) |
| 24 | power_battery_soc | float | ✅ | 动力电池SOC(%) |
| 25 | low_voltage_battery_voltage | float | ✅ | 低压电池电压(V) |
| 26 | init_status | int | ✅ | 初始化状态 |
| 27 | vehicle_mode_status | int | ✅ | 车辆模式状态 |
| 28 | mission_status | int | ✅ | 任务状态 |
| 29 | positioning_status | int | ✅ | 定位状态 |
| 30 | fault_status | uint | ✅ | 故障状态 |
| 31 | current_mission_id | UUID | ✅ | 当前任务ID |
| 32 | emergency_stop | bool | ✅ | 紧急停止状态 |
| 33 | emergency_reason | string | ✅ | 紧急停止原因 |

### RemoteControl (远程会话控制)

**话题**: `remote_session_control/request` & `remote_session_control/response`
**方向**: 驾驶舱 ↔ 车端
**数据类型**: **CBOR**

**控制类型枚举**:
- `1`: 接管 (TAKEOVER)
- `2`: 释放 (RELEASE)
- `3`: 配对 (PAIRING)
- `4`: 解配 (DISCONNECT)
- `5`: 紧急停止 (EMERGENCY_STOP)

**请求数据结构**:
```json
{
  "control_type": 3,        // 控制类型
  "operator_id": "operator_001",  // 操作员ID
  "session_id": "session_12345",         // 会话ID
  "vehicle_id": "J10A-001"  // 车辆ID (配对时使用)
}
```

**响应数据结构**:
```json
{
  "success": true,          // 是否成功
  "message": "配对成功"     // 响应消息
  "session_id": "session_12345"
}
```

### RemoteCommand (远程控制指令)

**话题**: `remote_command`
**方向**: 驾驶舱 → 车端
**频率**: 20 Hz (50ms间隔)
**数据类型**: **CBOR**

**数据结构**:
```json
{
  "ver": 0,                    // 协议版本
  "vin": "LBV5S1000MS688888",  // 车辆VIN
  "timestamp": 1767148280.79,  // UNIX时间戳
  "seq": 12345,                // 序列号
  "enable": true,              // 启用远程驾驶
  "emergency_stop": false,     // 紧急停止
  "gear": 3,                   // 挡位 (0:P,1:R,2:N,3:D)
  "speed": 5.0,                // 目标速度 (km/h)
  "steer": 45.0,               // 转向角 (°)
  "brake": 0.0,                // 刹车 (0-1)
  "auto_brake_enable": true,   // 自动刹车启用
  "echo_timestamp": 1767148280790  // 回显时间戳 (ms)
}
```

### CarEvent (车端事件)

**话题**: `car_event`
**方向**: 车端 → 云平台
**数据类型**: **CBOR**

**数据结构**:
```json
{
  "ver": 0,                    // 版本
  "vin": "LBV5S1000MS688888",  // 车辆VIN
  "event_level": 1,            // 事件等级
  "event_type": 1,             // 事件类型
  "event_description": "故障描述",  // 事件描述
  "event_time": 1767148280.79, // 事件时间
  "event_longitude": 112.123,  // 事件经度
  "event_latitude": 28.123,    // 事件纬度
  "event_mission_id": "uuid-12345"  // 事件任务ID
}
```

## 配置文件

编辑 `config/mqtt_config.yaml`：

```yaml
mqtt:
  broker_address: "ssl://mqtt.example.com:8883"
  client_id: "nanocargo_j10a_001"
  username: "vehicle_client"
  password: "secure_password"
  qos: 1
  enable_tls: true
  ca_cert_path: "/etc/mqtt/certs/ca.crt"
  client_cert_path: "/etc/mqtt/certs/client.crt"
  client_key_path: "/etc/mqtt/certs/client.key"
  verify_hostname: true
  keep_alive_interval: 10
  reconnect_interval: 5
  max_reconnect_interval: 60

# 车辆位置信息（用于自动生成MQTT话题）
vehicle:
  province: "hunan"
  city: "changsha"
  zone: "lugu"
  vin: "LBV5S1000MS688888"

# 话题映射（可选，如果为空则使用自动生成）
# topic_mappings:
#   - topic_name: "cabr/j10a/hunan/changsha/lugu/LBV5S1000MS688888/car_status"
#     direction: "ros2_to_mqtt"
#     message_type: "mqtt_bridge/msg/CarStatus"
#     qos: 0
```

**说明**:
- `vehicle` 部分定义了车辆位置信息，用于自动生成MQTT话题
- `topic_mappings` 部分是可选的，如果为空则使用自动生成的话题映射
- 自动生成的话题格式：`cabr/j10a/{province}/{city}/{zone}/{vin}/{topic_type}`

## 扩展开发

### 添加新的请求类型

1. **定义服务类型**（如果需要新的ROS2服务）
2. **更新话题定义**：在 `ConfigManager::initializeTopicDefinitions()` 中添加新的话题定义
3. **实现处理逻辑**：在 `MQTTBridgeNode` 中添加相应的handler和service caller
4. **注册请求类型**：在 `handleCommonRequest()` 的switch-case中添加新的request_type分支
5. **更新配置文件**（可选）：如果需要自定义话题，可以在配置文件中添加

### 处理函数模板

```cpp
// 1. 在 hpp 中添加声明
void handleXxxRequest(uint8_t ver, uint32_t request_type, 
                      const std::vector<uint8_t>& request_id, 
                      const std::string& payload);
void callXxxService(uint8_t ver, uint32_t request_type,
                    const std::vector<uint8_t>& request_id,
                    /* 具体参数 */);

// 2. 在 cpp 中实现 handler
void MQTTBridgeNode::handleXxxRequest(uint8_t ver, uint32_t request_type, 
                                      const std::vector<uint8_t>& request_id, 
                                      const std::string& payload)
{
  try {
    std::vector<uint8_t> cbor_bytes(payload.begin(), payload.end());
    json j = json::from_cbor(cbor_bytes);
    
    // 解析字段
    double timestamp = j["timestamp"].get<double>();
    std::string vin = j["vin"].get<std::string>();
    // ... 其他字段
    
    // 调用服务
    callXxxService(ver, request_type, request_id, timestamp, vin, ...);
    
  } catch (const std::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "解析XxxRequest失败: %s", e.what());
    sendErrorResponse(ver, request_type, request_id, e.what());
  }
}

// 3. 在 cpp 中实现 service caller
void MQTTBridgeNode::callXxxService(uint8_t ver, uint32_t request_type,
                                    const std::vector<uint8_t>& request_id,
                                    double timestamp, const std::string& vin, ...)
{
  if (!xxx_service_client_) {
    RCLCPP_ERROR(this->get_logger(), "Xxx服务客户端未初始化");
    // 构建错误响应 payload
    json j;
    j["timestamp"] = timestamp;
    j["response"] = 0;
    // ...
    std::vector<uint8_t> payload_bytes = json::to_cbor(j);
    sendCommonResponse(ver, request_type, request_id, payload_bytes);
    return;
  }
  
  if (!xxx_service_client_->wait_for_service(std::chrono::seconds(5))) {
    RCLCPP_WARN(this->get_logger(), "Xxx服务不可用");
    // 构建错误响应 payload
    // ...
    return;
  }
  
  auto request = std::make_shared<angmen_ad_msgs::srv::XxxRequest::Request>();
  request->timestamp = timestamp;
  request->vin = vin;
  // ... 其他字段
  
  auto future = xxx_service_client_->async_send_request(request,
    [this, ver, request_type, request_id](rclcpp::Client<...>::SharedFuture future) {
      try {
        auto response = future.get();
        
        // 构建响应 payload
        json j;
        j["timestamp"] = response->timestamp;
        j["response"] = response->response;
        // ... 其他字段
        std::vector<uint8_t> payload_bytes = json::to_cbor(j);
        
        // 统一通过 sendCommonResponse 发送
        sendCommonResponse(ver, request_type, request_id, payload_bytes);
        
      } catch (const std::exception& e) {
        RCLCPP_ERROR(this->get_logger(), "Xxx调用异常: %s", e.what());
        // 构建错误响应 payload
        // ...
        sendCommonResponse(ver, request_type, request_id, payload_bytes);
      }
    }
  );
}
```

## 测试

### CommonRequest/Response 测试
```bash
cd ~/trj/nanocargo_J10A/src/mqtt_bridge/scripts
python3 test_common_service.py
```

### MissionRequest/Response 测试
```bash
cd ~/trj/nanocargo_J10A/src/mqtt_bridge/scripts
python3 test_mission_request.py
```
详细文档: [MISSION_REQUEST_TEST.md](docs/MISSION_REQUEST_TEST.md)

### ModeChangeRequest/Response 测试

**启动服务节点:**
```bash
# 终端1: 启动 ModeChangeService
cd ~/trj/nanocargo_J10A/src/mqtt_bridge/scripts
./mode_change_service_node.py
```

**运行测试:**
```bash
# 终端2: 快速测试
./test_mode_change.sh quick

# 交互式模式
./test_mode_change.sh interactive

# 直接切换模式
./test_mode_change.sh 1  # 切换到自动驾驶
```

详细文档: [MODE_CHANGE_REQUEST_TEST.md](docs/MODE_CHANGE_REQUEST_TEST.md)

完整实现说明: [MODE_CHANGE_IMPLEMENTATION.md](MODE_CHANGE_IMPLEMENTATION.md)

## 话题自动生成说明

### 自动生成规则

1. **基础格式**: `cabr/j10a/{province}/{city}/{zone}/{vin}/{topic_type}`
2. **车辆信息**: 从配置文件的 `vehicle` 部分读取
3. **话题类型**: 根据消息类型和方向预定义

### 自动生成的优势

1. **减少配置**: 无需在配置文件中手动编写每个话题
2. **一致性**: 确保所有话题遵循统一的命名规范
3. **可维护性**: 话题定义集中管理，易于修改和扩展
4. **灵活性**: 支持通过配置文件覆盖自动生成的话题

### 自定义话题

如果需要自定义话题，可以在配置文件中添加 `topic_mappings` 部分：

```yaml
topic_mappings:
  - topic_name: "custom/topic/name"
    direction: "ros2_to_mqtt"
    message_type: "CustomMessage"
    qos: 1
```

系统会优先使用配置文件中定义的话题，未定义的则使用自动生成的话题。

## 更新日志

### v2.0 (2026-04-24)
- ✅ 重构 CommonRequest/Response 处理机制
- ✅ 统一所有请求类型通过 `handleCommonRequest` → `sendCommonResponse` 处理
- ✅ 支持 MissionRequest/ModeChangeRequest/VideoStreamingRequest/FileTransferRequest/RouteQueryRequest/TwoWayAudioRequest
- ✅ 清理 dead code（未使用的函数、变量、话题字符串）
- ✅ 所有响应统一通过 `/car/response` 话题发送

### v1.0 (2026-01-26)
- ✅ 基础 MQTT 桥接功能
- ✅ 支持 CarStatus/RemoteControl/RemoteCommand/RemoteReport/RemoteStatus
- ✅ 支持 CommonRequest/Response 基础框架
- ✅ 支持 MissionRequest/Response
- ✅ 支持 ModeChangeRequest/Response
