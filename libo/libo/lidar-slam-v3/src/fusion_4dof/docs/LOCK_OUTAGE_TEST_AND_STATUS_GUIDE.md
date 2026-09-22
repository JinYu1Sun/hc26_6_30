# 4DoF锁定、RTK缺帧与状态话题验证指南

## 1. 本轮修正的问题

### 1.1 2–3帧RTK缺失的速度基线问题

旧链路用一个`bool`同时表示“当前RTK可用”与“当前RTK为什么不可用”。
在10Hz附近，缺2帧后恢复的RTK与上一速度基线通常相隔约0.3s，超出
`0.05 < dt < 0.25s`速度差分范围。该帧本应只用于重建速度基线，却被当成
“已收到但质量/运动/创新冲突”，错误升级为22帧完整恢复。

现在拆分为以下结果：

- `Accepted`：真正通过速度/航向运动检查；
- `BaselineEstablished` / `TimestampDiscontinuity`：仅重建基线，不计稳定帧，
  也不进入硬故障；
- `Invalid` / `ExcessiveSpeed` / `HeadingMismatch`：拒绝当前坐标；
- 上层创新拒绝仍隔离当前RTK坐标，但同样不能由一帧触发切源。

系统已健康时，任意单帧拒绝都保持`state=1`并使用可信RTK锚点+同代LIO增量；
下一帧`Accepted`直接清零连续不可用计数。只有第4帧或真实时间上限到达，才进入
冻结4DoF/LIO和22帧完整恢复。

### 1.2 条件数的数学定义

水平内点坐标协方差的两个特征值为`λmax`和`λmin`，而`along_std`、
`cross_std`是特征值开方。因此条件数必须是：

```text
condition_number = λmax / λmin
                 = along_std² / cross_std²
```

旧实现的`along_std/cross_std <= 50`是标准差轴比，等价于允许协方差
条件数达2500。现在改为真实`λmax/λmin <= 50`，参数同步更名为：

```yaml
se2_lock_max_condition_number: 50.0
```

部署时必须删除旧键`se2_lock_max_axis_ratio`，不得两者并存后猜测哪个生效。

## 2. 自动回归测试

### 2.1 RTK速度基线与缺帧

`gps_motion_gate_test`覆盖：

- 首帧只建立基线；
- 0.1s正常连续样本；
- 0.3s间断只重建基线；
- 基线重建后的正常恢复；
- 超速与运动航向冲突仍为硬拒绝。

`gps_outage_policy_test`覆盖：

- 1帧缺失后下一Accepted直接恢复且不切源；
- 连续2～3帧缺失/基线/拒绝保持可信锚点延拓；
- 将`gps_grace_coast_output_enabled=false`后，前三帧仍累计宽限证据但不发布定位；
- 第4个连续不可用观测才进入22帧完整恢复；
- 基线重建不能进入硬故障；
- 真实硬拒绝要求22个连续合格样本；
- 硬故障恢复期再缺帧会清空候选窗并重置连续计数。

### 2.2 锁定默认门限

`global_fusion_recovery_test`现在额外覆盖：

- 14个鲁棒内点不锁，第15个才可锁；
- 18/26=69.23%内点不锁；
- 19/27=70.37%内点可进入锁定；
- 真实协方差条件数49可锁，51不锁；
- 静止±1°航向抖动不能累加为30°转弯；
- 锁定后`theta/tx/ty/tz`完整冻结；
- 三次确认必须包含新的时间和空间证据。

ROS1/catkin环境中执行：

```bash
catkin_make -DCATKIN_ENABLE_TESTING=ON
catkin_make run_tests_fusion_4dof
catkin_test_results build/test_results
```

## 3. 如何通过话题确认当前定位状态

### 3.1 权威状态话题

```bash
rostopic echo -n 1 /Mower/localization_status
```

`/Mower/localization_status`是`android_manager_4dof`汇总后的对外权威状态，优先供
调度、UI和控制使用。重点字段：

| 字段 | 判定 |
|---|---|
| `position_valid` | 当前是否有新鲜的生产定位；不能只看最后一个`position_state` |
| `position_source` | 1=RTK，2=已锁定LIO，3=仅播种LIO，4=受限IMU |
| `position_state` | 与`/Mower/position.position_state`一致 |
| `phase` | 当前生命周期阶段，见下表 |
| `alignment_ready` | 水平+高程4DoF已可用 |
| `alignment_locked` | 水平变换已通过鲁棒内点、几何和独立确认并冻结 |
| `gps_good` / `lio_good` | 最近的RTK/LIO数据是否通过各自门控且未超时 |
| `stop_required` | 是否必须保持停车 |
| `fault_code` / `reason` | 故障类型及当前原因 |
| `transitioning` / `restart_in_progress` | 是否处于地图切换或受控恢复 |

常用`phase`：

| phase | 含义 |
|---:|---|
| 4 | 等待稳定RTK |
| 5 | 等待完整4DoF对齐 |
| 6 | RTK生产定位就绪 |
| 7 | 已锁定4DoF/LIO回退就绪 |
| 8 | 仅播种、未锁定LIO回退 |
| 9 | 受限IMU，只可制动 |
| 10–13 | 地图切换/重启/恢复中 |
| 14 | 故障 |

仅查看fusion算法本身、不包含manager汇总判断时：

```bash
rostopic echo -n 1 /Mower/localization_algorithm_status
```

### 3.2 实际生产位置

```bash
rostopic echo /Mower/position
rostopic hz /Mower/position
```

`position_state`的生产语义：

| state | 定位源与限制 |
|---:|---|
| 1 | RTK为x/y/yaw主源，z为LIO相对高程+冻结/ 鲁棒`tz` |
| 2 | 已锁定4DoF/LIO回退，是楼间遮挡时的目标状态 |
| 5 | 仅播种未锁定，只可作降级状态，不应允许进入已知严重遮挡区正常作业 |
| 6 | 受限IMU制动位姿，不得自动作业 |
| 0/9或无新消息 | 无有效生产定位 |

不能只看`state=1/2/5`，还必须同时确认消息频率/时间戳正常且
`/Mower/localization_status.position_valid=true`。

### 3.3 锁定门限的实时证据

```bash
rostopic echo /rosout | grep --line-buffered '\[4DOF-TUNE\]'
```

`[4DOF-TUNE]`现在输出：

- `window_n` / `xy_inliers`：窗口点数和鲁棒内点数；
- `outlier_ratio`：水平异常点比例；
- `along_std` / `cross_std`：主、次轴标准差；
- `condition_number`：真实协方差条件数；
- `turn`：解缠后的航向覆盖范围；
- `locked`：是否已锁定；
- `theta/tx/ty/tz`：当前生产变换；
- `gps_missing_cnt/transient/reacq`：缺帧宽限与短重获进度；
- `gps_failed_flag/gps_stable_cnt`：硬故障与22帧完整恢复进度。

默认锁定前应观察到：

```text
xy_inliers >= 15
xy_inliers / window_n >= 0.70
along_std >= 0.50 m
cross_std >= 0.10 m
condition_number <= 50
turn >= 30 deg
```

这些几何条件满足后，仍需要3组时间/位移独立验证才会看到
`locked=1`和`alignment_locked=true`。

### 3.4 RTK原始质量证据

```bash
rostopic echo -n 1 /nanobot/gpsposition
```

只使用当前1ant真实来源字段判定RTK质量：

- `gps_flag==4`；
- `INS_Status==3`；
- `INS_GpsFlag_Pos==56`；
- `INS_NumSV>=8`；
- `0<=INS_Gps_Age<=3`；
- 纬度/经度标准差均在(0, 0.30m]；
- 高程标准差在(0, 0.50m]。

航向要求`INS_Std_Heading`在(0,5°]；依赖车辆运动的`positionStatus`仅用于诊断，
不再参与4DoF冷启动门控。
`gps_confidence`、`INS_VehicleAlign`和`INS_GpsFlag_Heading`在当前1ant发布链中不是
可独立信任的接收机原始质量证据，不得单独用它们宣告RTK可用。

### 3.5 停车与故障确认

```bash
rostopic echo /mower/fusion_stop_car
rostopic echo /mower/manager_stop_car
rostopic echo /mower/stop_car
```

任一权威停车源为`true`时，最终汇总话题`/mower/stop_car`必须为`true`，车辆命令
与轮速也必须归零，才算完成端到端停车验证。同时查看
`/Mower/localization_status.stop_required/fault_code/reason`，不得仅依靠一个Bool的瞬时值猜测原因。

## 4. 实车与bag验收序列

### 4.1 缺帧序列

| 输入 | 预期生产状态 |
|---|---|
| 任意1帧不可用 | 保持`state=1`，发布可信RTK锚点+同代LIO增量；下一帧Accepted直接恢复 |
| 连续2或3帧不可用 | 保持`state=1`延拓；坏RTK坐标绝不发布 |
| 缺4帧 | 进入硬故障；候选窗清空；需22个连续Accepted |
| 连续4帧质量/运动/创新拒绝 | 第4帧进入完整失效；前三帧共享统一宽限 |
| 22帧恢复中又缺帧 | 重置回第0个恢复样本，不得保留非连续计数 |

### 4.2 楼间遮挡场景

1. 开阔区启动，确认`alignment_ready=true`且`alignment_locked=true`。
2. 记录入口处`theta/tx/ty/tz`。
3. 进入遮挡区，确认RTK缺失时切为`phase=7/state=2/source=2`。
4. 遮挡区内`theta/tx/ty/tz`必须保持不变。
5. 驶出时检查短重获或22帧恢复序列，以及切源前RTK与冻结LIO的同时间差。
6. 若差值超过安全切源门，应保持LIO并停车；该独立切源门仍在
   `SAFETY_TODO.md`中，当前不得将此项标记为已验收。

## 5. 采集要求

bag至少记录：

```bash
rosbag record -O localization_4d_validation.bag \
  /nanobot/gpsposition \
  /Mower/lio_slam_level /Mower/lio_level_transform \
  /Mower/lio_static_initialized /Mower/lio_static_calibration \
  /Mower/position /Mower/debug/gps /Mower/debug/lio /Mower/fusion \
  /Mower/localization_algorithm_status /Mower/localization_status \
  /mower/fusion_stop_car /mower/manager_stop_car /mower/stop_car \
  /vehicle/cmd /vehicle/status /vehicle/left_wheel_speed /vehicle/right_wheel_speed \
  /rosout
```

故障注入时必须同时保留未修改的原始RTK/LIO话题和独立的注入事件记录，
不得伪造“RTK fixed/INS good”等状态来代替真实设备输出。`/Mower/debug/*`的
`position_state`只是调试布尔语义，不得套用`/Mower/position`的生产状态定义。
