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

### 1.4 定位初始化 `mower/{id}/cmd/init_location`

```json
{"action": "request"}
{"action": "confirm"}
{"action": "cancel"}
```

| action | 说明 |
|---|---|
| request | 请求定位初始化，车端进入等待确认状态 |
| confirm | 确认初始化，车辆开始走 8 字形动作 |
| cancel | 取消初始化请求 |

流程说明：
1. 云平台下发 `request`，车端 `task_node` 进入初始化请求状态并上报 `/init_request=true`。
2. 云平台确认场地安全后下发 `confirm`，车辆才开始走 8 字形。
3. 初始化过程中可随时下发 `cancel` 取消。

### 1.5 建图控制 `mower/{id}/cmd/mapping`

建图 = 遥控车辆（`cmd/move`）沿区域边界/障碍物/路径行驶，车端按距离阈值采点，最终保存为矢量地图（YAML）。

```json
{"action": "enter"}
{"action": "start_boundary"}
{"action": "stop_boundary"}
{"action": "start_obstacle"}
{"action": "stop_obstacle"}
{"action": "start_parking"}
{"action": "stop_parking"}
{"action": "start_path"}
{"action": "stop_path"}
{"action": "save", "map_name": "map_0701"}
{"action": "delete", "map_name": "map_0701"}
{"action": "list"}
{"action": "reset"}
```

| action | 说明 |
|---|---|
| enter | 进入建图模式（车端发 `/signal = m_mode`）。雷达/RTK 收到后自行准备，随后 `/Mower/position` 坐标会被外部定位系统归零为 (0,0,0)，**必须等坐标归零后才能开始录边界** |
| start_boundary / stop_boundary | 开始 / 停止录制割草区域边界（每 1m 采一个点） |
| start_obstacle / stop_obstacle | 开始 / 停止录制障碍物（洞，每 0.1m 采一个点），需先录过边界 |
| start_parking / stop_parking | 开始 / 停止录制停车位（单点，取当前车位置） |
| start_path / stop_path | 开始 / 停止录制区域间连接路径（每 0.5m 采一个点） |
| save | 把已录制内容保存为 `map_name.yaml`（map_name 不能含 `/` 和 `..`），保存后本次建图会话结束 |
| delete | 删除指定地图文件 |
| list | 请求地图列表，结果通过 `state/map_list` 回传 |
| reset | 放弃当前录制内容并结束建图会话 |

典型建图流程：
1. 下发 `enter` 进入建图模式。
2. 观察 `state/mapping`（或 `state/location`），等定位状态有效（state 为 1/2/5）且 x、y 归零到 (0,0) 附近。
3. 下发 `start_boundary`，用 `cmd/move` 遥控车沿边界行驶——云平台用 `state/mapping` 的轨迹流实时描边。
4. 到终点后下发 `stop_boundary`；如有障碍物/停车位/连接路径，按需重复 start/stop。
5. 下发 `save` 保存地图。

## 二、上行：割草机 → 云平台（MQTT）

### 2.1 定位上报 `mower/{id}/state/location`

割草机开机（cloud_bridge 节点启动）后**持续上报**，默认 2Hz（车端可调）。

```json
{"x": 12.34, "y": 56.78, "z": 0.0, "roll": 0.01, "pitch": -0.02, "yaw": 1.57, "state": 4, "stamp": 1751356800.123}
```

| 字段 | 说明 |
|---|---|
| x / y / z | 局部平面坐标（米），来自车端融合定位 `/Mower/position`（`mower_msgs/Position`） |
| roll / pitch / yaw | 姿态（弧度） |
| state | 定位状态字（车端定位模块定义，0=无定位；1/2/5=有效融合定位） |

注：当前为局部坐标系；如需经纬度，需云平台与车端约定地图原点后换算，或后续版本增加原始 GNSS 转发。

### 2.2 车辆状态 `mower/{id}/state/vehicle`

默认 1Hz 上报。

```json
{"battery_soc": 85, "warning_state_one": 0, "warning_state_two": 0, "mower_height": 6, "stamp": 1751356800.5}
```

### 2.3 建图轨迹 `mower/{id}/state/mapping`

仅在**建图会话期间**（收到 `cmd/mapping enter` 起，到 `save` 或 `reset` 止）上报，频率与 `state/location` 相同（默认 10Hz），内容就是建图期间车辆走过的 `/Mower/position` 轨迹。云平台建图页面用它实时描出行驶轨迹（即正在录制的边界走向），并据此观察 enter 之后坐标是否已归零。

```json
{"x": 0.0, "y": 0.0, "z": 0.0, "roll": 0.01, "pitch": -0.02, "yaw": 1.57, "state": 4, "stamp": 1751356800.6}
```

字段含义与 `state/location` 完全一致。

### 2.4 地图列表 `mower/{id}/state/map_list`

收到 `cmd/mapping {"action":"list"}` 后回传。

```json
{"maps": ["map_0630", "map_0701"], "stamp": 1751356800.8}
```

### 2.5 地图原点 `mower/{id}/state/map_origin`

车端收到 `/signal` 选图信号 `use_map/<编号>` 时（云平台 `cmd/task start` 带 `map_name`、或安卓端选图，都会触发），从 `map_dir`（launch 可配）读取 `<编号>.mp` 原点文件后回传一次，云平台可据此把 `state/location` 的局部坐标换算成经纬度。

```json
{"map_name": "map_0630", "found": true, "map_index": 0,
 "latitude": 28.236557, "longitude": 112.876617, "height": 86.60,
 "gauss_yaw": 73.89, "stamp": 1751356800.9}
```

| 字段 | 说明 |
|---|---|
| map_name | 请求的地图编号 |
| found | 是否找到并解析成功；文件缺失时为 false（此时无以下字段） |
| map_index | 地图内部索引（int） |
| latitude / longitude / height | 地图原点 WGS84 纬度 / 经度 / 椭球高（米） |
| gauss_yaw | 建图时的高斯坐标系偏航角（度） |
| stamp | Unix 秒（double，车端 ROS 时间） |

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
# 请求定位初始化
mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/init_location' -m '{"action":"request"}'
# 确认定位初始化（车辆开始走 8 字形）
mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/init_location' -m '{"action":"confirm"}'
# 取消定位初始化
mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/init_location' -m '{"action":"cancel"}'
# 进入建图模式（等坐标归零后）开始录边界 → 遥控走边界 → 停止录边界 → 保存地图
mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/mapping' -m '{"action":"enter"}'
mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/mapping' -m '{"action":"start_boundary"}'
mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/mapping' -m '{"action":"stop_boundary"}'
mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/mapping' -m '{"action":"save","map_name":"map_0701"}'
# 请求地图列表
mosquitto_pub -h <broker> -t 'mower/mower_001/cmd/mapping' -m '{"action":"list"}'
```

## 四、安全约定

- 车端 0.5s 收不到 move 指令自动停车；云端断连不会导致车辆持续行驶。
- 车端原有的避障急停、侧翻保护、出边界保护与云端指令**并行生效**，云端无需处理。
- MQTT 断线后车端自动重连（指数退避，最长 30s），重连后自动恢复订阅。
