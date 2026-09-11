# 割草机车云接口协议

本文档是割草机车端 `cloud_bridge` 节点与云平台之间的接口约定。云平台开发以此为准。

## 总体架构

| 通道 | 协议 | 方向 | 用途 |
|---|---|---|---|
| 指令/状态通道 | MQTT over 明文 TCP，端口 1883 | 双向 | 遥控、刀盘、任务下发；定位、车辆状态上报 |

- 通道**无加密、无认证**（MQTT broker 如要求用户名密码，在车端 launch 中配置 `username`/`password` 即可，协议本身不变）。
- MQTT 主题前缀：`mower/{device_id}/`，`device_id` 在车端 launch 中配置，默认 `mower_001`。
- MQTT payload 均为 **UTF-8 JSON**；数值为 number 类型；时间戳 `stamp` 为 Unix 秒（double，车端 ROS 时间）。
- QoS：下行指令 QoS 1；上行状态/定位 QoS 0。

## 一、下行：云平台 → 割草机（MQTT）

### 1.1 遥控移动 `mower/{id}/cmd/move`

```json
{"linear": 0.5, "angular": -0.3}
```

| 字段 | 类型 | 范围 | 说明 |
|---|---|---|---|
| linear | number | -1.0 ~ 1.0 | 线速度比例：正=前进，负=后退，0=停 |
| angular | number | -1.0 ~ 1.0 | 角速度比例：正=左转，负=右转，0=直行 |

- 车端映射：`drive_value = linear × 10000`，`turn_value = -angular × 12566`（上限可在车端 launch 调）。
- **云平台需持续发送（建议 ≥5Hz）**：车端看门狗 0.5s 内没收到新的 move 指令会自动停车。松开摇杆请发 `{"linear":0,"angular":0}`。
- 自动任务运行中下发 move 指令，车端会先自动 pause 任务再接管底盘。

### 1.2 刀盘控制 `mower/{id}/cmd/blade`

```json
{"state": 1, "height": 6}
```

| 字段 | 类型 | 说明 |
|---|---|---|
| state | int | 1=开启刀盘，0=关闭刀盘 |
| height | int（可选） | 割草高度档位 2~11，不携带则保持当前档位 |

### 1.3 自动割草任务 `mower/{id}/cmd/task`

```json
{"action": "start", "map_name": "map_0630", "map_mode": "single_map"}
{"action": "stop"}
{"action": "pause"}
{"action": "continue"}
```

| 字段 | 说明 |
|---|---|
| action | `start` / `stop` / `pause` / `continue` |
| map_name | start 时携带，要使用的地图名；空则沿用当前已加载地图 |
| map_mode | start 时可选，`single_map`（默认）或 `multi_map` |

车端行为：
- `start`：依次执行 启动路径跟踪节点 → 复位 → 加载地图 → 选模式 → 开工，全程约 2~3 秒。
- `stop`：停止任务并复位，随后关闭路径跟踪节点。

## 二、上行：割草机 → 云平台（MQTT）

### 2.1 定位上报 `mower/{id}/state/location`

割草机开机（cloud_bridge 节点启动）后**持续上报**，默认 2Hz（车端可调）。

```json
{"x": 12.34, "y": 56.78, "z": 0.0, "roll": 0.01, "pitch": -0.02, "yaw": 1.57, "state": 4, "stamp": 1751356800.123}
```

| 字段 | 说明 |
|---|---|
| x / y / z | 局部平面坐标（米），来自车端融合定位 `/Mower/position` |
| roll / pitch / yaw | 姿态（弧度） |
| state | 定位状态字（车端定位模块定义，0=无定位；1/2/5=有效融合定位） |

注：当前为局部坐标系；如需经纬度，需云平台与车端约定地图原点后换算，或后续版本增加原始 GNSS 转发。

### 2.2 车辆状态 `mower/{id}/state/vehicle`

默认 1Hz 上报。

```json
{"battery_soc": 85, "warning_state_one": 0, "warning_state_two": 0, "left_wheel_speed": 120, "right_wheel_speed": 118, "mower_height": 6, "stamp": 1751356800.5}
```

## 三、联调示例（mosquitto 客户端模拟云平台）

```bash
# 订阅全部 MQTT 上行
mosquitto_sub -h <broker> -t 'mower/mower_001/#' -v

# 前进半速
mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/move' -m '{"linear":0.5,"angular":0}'
# 停车
mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/move' -m '{"linear":0,"angular":0}'
# 开刀盘
mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/blade' -m '{"state":1,"height":6}'
# 启动自动任务
mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/task' -m '{"action":"start","map_name":"map_0630","map_mode":"single_map"}'
# 停止任务
mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/task' -m '{"action":"stop"}'
```

## 四、安全约定

- 车端 0.5s 收不到 move 指令自动停车；云端断连不会导致车辆持续行驶。
- 车端原有的避障急停、侧翻保护、出边界保护与云端指令**并行生效**，云端无需处理。
- MQTT 断线后车端自动重连（指数退避，最长 30s），重连后自动恢复订阅。
