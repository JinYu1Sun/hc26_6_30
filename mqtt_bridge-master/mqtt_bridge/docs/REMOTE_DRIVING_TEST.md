# 远程驾驶功能测试指南

## 功能概述

本次更新实现了完整的远程驾驶协议（基于 communication_protocol.cddl），包括：

### 1. 远程控制 (RemoteControl)
- **功能**：接管/释放/急停车辆控制权
- **消息类型**：Request/Response (通过 ROS2 服务)
- **MQTT Topics**：
  - 请求: `vehicle/remote/control/request`
  - 响应: `vehicle/remote/control/response`
- **ROS2 Service**：`/remote_control/control_service`

### 2. 远程驾驶指令 (RemoteCommand)
- **功能**：发送实时驾驶指令（油门、刹车、转向、档位）
- **消息类型**：Message (单向，MQTT → ROS2)
- **MQTT Topic**：`vehicle/remote/command`
- **ROS2 Topic**：`/remote_control/command`

### 3. 远程驾驶报告 (RemoteReport)
- **功能**：上报实时驾驶状态
- **消息类型**：Message (单向，ROS2 → MQTT)
- **ROS2 Topic**：`/remote_control/report`
- **MQTT Topic**：`vehicle/remote/report`

### 4. 远程驾驶状态 (RemoteStatus)
- **功能**：上报远程控制状态（是否接管、网络质量等）
- **消息类型**：Message (单向，ROS2 → MQTT)
- **ROS2 Topic**：`/remote_control/status`
- **MQTT Topic**：`vehicle/remote/status`

### 5. 车辆事件 (CarEvent)
- **功能**：上报车辆事件（警告、故障等）
- **消息类型**：Message (单向，ROS2 → MQTT)
- **ROS2 Topic**：`/car_event`
- **MQTT Topic**：`vehicle/event`

### 6. 视频流控制 (VideoStreaming) - 已实现接口
- **功能**：请求开启/停止视频流
- **消息类型**：Request/Response
- **状态**：框架已实现，业务逻辑需补充

### 7. 文件传输 (FileTransfer) - 已实现接口
- **功能**：文件上传/下载
- **消息类型**：Request/Response
- **状态**：框架已实现，业务逻辑需补充

### 8. 路径查询 (RouteQuery) - 已实现接口
- **功能**：查询当前行驶路径
- **消息类型**：Request/Response
- **状态**：框架已实现，业务逻辑需补充

---

## 编译项目

```bash
cd /home/ubuntu/trj/nanocargo_J10A

# 编译消息定义和桥接节点
colcon build --packages-select angmen_ad_msgs mqtt_bridge

# source 环境
source install/setup.bash
```

---

## 完整测试流程

### 步骤 1: 启动 Mosquitto MQTT Broker

```bash
# 终端 1
mosquitto -v
```

### 步骤 2: 启动 MQTT Bridge 节点

```bash
# 终端 2
cd /home/ubuntu/trj/nanocargo_J10A
source install/setup.bash
ros2 run mqtt_bridge mqtt_bridge_node
```

### 步骤 3: 启动远程控制服务节点

```bash
# 终端 3
cd /home/ubuntu/trj/nanocargo_J10A/src/mqtt_bridge/scripts
source /home/ubuntu/trj/nanocargo_J10A/install/setup.bash
./remote_control_service_node.py
```

此节点会：
- 提供 `/remote_control/control_service` 服务（处理接管/释放/急停）
- 定时发布 `RemoteReport`（每秒，车辆实时状态）
- 定时发布 `RemoteStatus`（每2秒，远程控制状态）

### 步骤 4: 运行测试脚本

#### 测试远程控制（接管/释放/急停）

```bash
# 终端 4
cd /home/ubuntu/trj/nanocargo_J10A/src/mqtt_bridge/scripts

# 快速测试（默认）
./test_remote_control.py --quick

# 批量测试（测试不同接管级别）
./test_remote_control.py --batch

# 交互测试
./test_remote_control.py --interactive
```

#### 测试远程驾驶指令

```bash
# 快速测试（常见指令）
./test_remote_command.py --quick

# 批量测试（模拟连续驾驶场景）
./test_remote_command.py --batch

# 交互测试（手动输入指令）
./test_remote_command.py --interactive
```

---

## 预期结果

### 远程控制测试

**MQTT 测试脚本输出示例：**
```
✓ 已连接到MQTT broker
✓ 已订阅响应topic: vehicle/remote/control/response

📤 发送远程控制请求 [接管]
  timestamp: 1737974400000
  vin: TEST_VIN_00000001
  action: 1
  level: 50
  payload size: XX bytes
  ✓ 已发布到 vehicle/remote/control/request

📥 收到响应 [topic=vehicle/remote/control/response]
  timestamp: 1737974400123
  vin: TEST_VIN_00000001
  result: 0 (成功)
  message: 接管成功，级别: 50%
```

**ROS2 服务节点输出示例：**
```
[INFO] 收到远程控制请求:
[INFO]   timestamp: 1737974400000
[INFO]   vin: TEST_VIN_00000001
[INFO]   action: 1
[INFO]   level: 50
[INFO] 接管成功，级别: 50%
[INFO] 远程控制中: level=50%, speed=15.5km/h
```

**MQTT Bridge 节点输出示例：**
```
[INFO] 收到MQTT消息 [vehicle/remote/control/request]: 45 bytes
[INFO] 调用RemoteControl服务...
[INFO] RemoteControl服务调用成功
[INFO] 发送RemoteControl响应到 vehicle/remote/control/response
[INFO] 收到RemoteReport，发布到MQTT: vehicle/remote/report
[INFO] 收到RemoteStatus，发布到MQTT: vehicle/remote/status
```

### 远程驾驶指令测试

**MQTT 测试脚本输出示例：**
```
📤 发送驾驶指令
  timestamp: 1737974500000
  vin: TEST_VIN_00000001
  油门: 0.30
  刹车: 0.00
  转向: -0.30
  档位: 3 (D)
  payload size: XX bytes
  ✓ 已发布到 vehicle/remote/command
```

**MQTT Bridge 节点输出示例：**
```
[INFO] 收到MQTT消息 [vehicle/remote/command]: 52 bytes
[INFO] 发布RemoteCommand到ROS2: /remote_control/command
```

---

## 使用 ROS2 命令行测试

### 查看 topics

```bash
# 查看所有topics
ros2 topic list

# 应该能看到:
# /remote_control/control_service
# /remote_control/command
# /remote_control/report
# /remote_control/status
# /car_event
```

### 查看消息

```bash
# 查看远程驾驶报告
ros2 topic echo /remote_control/report

# 查看远程控制状态
ros2 topic echo /remote_control/status

# 查看远程驾驶指令
ros2 topic echo /remote_control/command
```

### 手动调用服务

```bash
# 接管控制（级别50%）
ros2 service call /remote_control/control_service angmen_ad_msgs/srv/RemoteControl \
  "{timestamp: 1737974400000, vin: 'TEST_VIN_00000001', action: 1, level: 50}"

# 释放控制
ros2 service call /remote_control/control_service angmen_ad_msgs/srv/RemoteControl \
  "{timestamp: 1737974400000, vin: 'TEST_VIN_00000001', action: 2, level: 0}"

# 急停
ros2 service call /remote_control/control_service angmen_ad_msgs/srv/RemoteControl \
  "{timestamp: 1737974400000, vin: 'TEST_VIN_00000001', action: 3, level: 0}"
```

---

## 监控 MQTT 消息

使用 `mosquitto_sub` 监控 MQTT 消息：

```bash
# 监控所有 vehicle/* topics
mosquitto_sub -v -t 'vehicle/#'

# 监控远程控制响应
mosquitto_sub -v -t 'vehicle/remote/control/response'

# 监控远程驾驶报告
mosquitto_sub -v -t 'vehicle/remote/report'

# 监控远程驾驶状态
mosquitto_sub -v -t 'vehicle/remote/status'

# 监控车辆事件
mosquitto_sub -v -t 'vehicle/event'
```

---

## 故障排查

### 1. 编译错误

**问题**：找不到消息类型定义

**解决**：
```bash
# 确保先编译 angmen_ad_msgs
colcon build --packages-select angmen_ad_msgs
source install/setup.bash

# 再编译 mqtt_bridge
colcon build --packages-select mqtt_bridge
```

### 2. MQTT 连接失败

**问题**：Bridge 节点无法连接到 MQTT broker

**检查**：
```bash
# 确认 mosquitto 正在运行
ps aux | grep mosquitto

# 测试连接
mosquitto_sub -t test
```

### 3. 服务调用超时

**问题**：RemoteControl 服务调用超时

**检查**：
```bash
# 确认服务节点正在运行
ros2 node list | grep remote_control

# 确认服务存在
ros2 service list | grep control_service

# 测试服务调用
ros2 service call /remote_control/control_service angmen_ad_msgs/srv/RemoteControl \
  "{timestamp: 1737974400000, vin: 'TEST', action: 1, level: 50}"
```

### 4. CBOR 解析错误

**问题**：C++ 端报错 "expected length specification (0x60-0x7B)"

**原因**：Python 脚本使用了整数键而不是字符串键

**解决**：确保所有 Python 测试脚本使用字符串键：
```python
# 错误 ❌
request = {
    0: timestamp,
    1: vin,
    2: action
}

# 正确 ✅
request = {
    "0": timestamp,
    "1": vin,
    "2": action
}
```

---

## 代码文件清单

### 新增消息定义
- `angmen_ad_msgs/srv/RemoteControl.srv`
- `angmen_ad_msgs/msg/RemoteCommand.msg`
- `angmen_ad_msgs/msg/RemoteReport.msg`
- `angmen_ad_msgs/msg/RemoteStatus.msg`
- `angmen_ad_msgs/srv/VideoStreamingRequest.srv`
- `angmen_ad_msgs/srv/FileTransferRequest.srv`
- `angmen_ad_msgs/srv/RouteQueryRequest.srv`
- `angmen_ad_msgs/msg/CarEvent.msg`

### 更新的文件
- `angmen_ad_msgs/CMakeLists.txt` - 添加新消息类型
- `mqtt_bridge/config/mqtt_config_test.yaml` - 添加 topic 映射
- `mqtt_bridge/include/mqtt_bridge/mqtt_bridge_node.hpp` - 添加声明
- `mqtt_bridge/src/mqtt_bridge_node.cpp` - 实现处理逻辑

### 测试脚本
- `mqtt_bridge/scripts/test_remote_control.py` - 测试接管/释放/急停
- `mqtt_bridge/scripts/test_remote_command.py` - 测试驾驶指令
- `mqtt_bridge/scripts/remote_control_service_node.py` - ROS2 服务节点

---

## 下一步工作

### 短期任务
1. ✅ 完成基础远程驾驶功能测试
2. ⏳ 实现 VideoStreamingRequest 的业务逻辑
3. ⏳ 实现 FileTransferRequest 的业务逻辑
4. ⏳ 实现 RouteQueryRequest 的业务逻辑

### 中期任务
1. 添加更多测试脚本（视频流、文件传输、路径查询）
2. 完善错误处理和异常情况
3. 添加单元测试和集成测试
4. 性能优化和延迟测试

### 长期任务
1. 增强安全性（认证、加密）
2. 添加日志记录和监控
3. 实现自动重连和故障恢复
4. 编写完整的开发文档

---

## 联系方式

如有问题，请查看：
- `mqtt_bridge/README.md` - 总体架构说明
- `mqtt_bridge/docs/MISSION_REQUEST_TEST.md` - 已有功能的测试说明
- ROS2 日志输出 - 详细的运行时信息
