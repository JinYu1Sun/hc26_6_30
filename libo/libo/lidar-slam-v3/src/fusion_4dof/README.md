# fusion_4dof —— 重力约束4DoF定位流程与实地调参文档

> 统一定位状态、manager 状态机、地图热切换、地图 bundle、`.lio4d` v2、基站边界与本次完整改动清单，见仓库 `docs/fusion_4dof_complete_change_summary.md`。本 README 仍专注算法原理与现场调参。

GPS(RTK)/LIO 融合定位（SE(2) 在线对齐版）。由 fast_lio 的 ceres 版 `fast_fusion`
演进而来，作为独立软件包整体替换部署，配套 `android_manager_4dof`（见 §10）。
节点名为 `fusion_4dof`（即 `/fusion_4dof`），话题名与 ceres 版完全相同
（`/Mower/position` 等），下游无需改动。**不再与旧版 `fast_fusion`/`android_manager`
同时运行**——旧版能拿到的“用 RTK 位置+航向求 LIO→ENU”的结果，本身就是本版
“播种(seed)”档位（§4.2 判据 4）的能力，已被本版覆盖，见 §10。

---

## 0. 术语表（先读这个）

- **LIO**：Lidar-Inertial Odometry，激光雷达+惯性里程计。靠点云匹配推算自身位姿，
  短期很准、长期会缓慢漂移；不依赖卫星，遮挡/室内也能用。由 `/laserMapping` 节点产出。
- **RTK / INS**：RTK 是载波相位差分卫星定位（厘米级绝对位置）；INS 是惯性导航
  （陀螺+加速度计）。本机是 RTK/INS 组合（GNSS+惯导），开阔处绝对位置准、且初始化
  后能持续输出航向；遮挡处卫星少则变差。
- **ENU 坐标系**：East-North-Up（东-北-天）的本地平面直角系。x=正东、y=正北，单位米，
  以某个原点为 (0,0)。本版位置一律用 ENU。
- **航向 (yaw / heading)**：车头朝向角。本版用“从正东起、逆时针为正”的 ENU 约定。
- **重力约束4DoF**：估计一个绕竖直轴的旋转 `θ` 和三个平移
  `(tx,ty,tz)`，即 `x/y/z/yaw`；`roll/pitch`不由GPS自由拟合，仍由重力水平
  系与IMU姿态给出。其中水平部分是SE(2)：
  `p_ENU.xy=Rz(θ)·p_H.xy+(tx,ty)`；竖直部分是
  `z_ENU=z_H+tz`。这比全局SE(3)少两个会与重力相争的自由度。
- **Umeyama（一种点集对齐算法）**：给两组一一对应的点（这里是同一时刻的“LIO 位置”
  与“GPS 的 ENU 位置”），用闭式公式（直接套公式、非迭代）求出让两组点最吻合的那个
  旋转+平移。**“全 Umeyama”** = 用窗口内全部配对点做这个拟合（数据多、最准）。
- **滑窗 (sliding window)**：只保留“最近一段时间/一定数量”的配对点参与计算，旧的丢弃。
- **锁定 (lock) / 播种 (seed)**：本版对“旋转角 θ 是否已被可靠估出”的两档置信度。
  **播种**=有了一个粗略可用的 θ（够回退用）；**锁定**=车已拐过弯、θ 被二维分布充分
  约束、很可靠。锁定后在长直线段会“冻结”θ 不再被直线带偏（详见 §4.2）。
- **沿迹向 / 横迹向**：沿迹向=车行进方向；横迹向=垂直于行进方向（左右）。只有车
  拐弯才会产生横迹向的分布展开，这正是判断旋转是否可靠（可“锁定”）的依据。
- **杆臂 (lever arm)**：传感器安装点与车体参考点 base_link 之间的固定位置偏移
  （本机雷达在 base_link 前 0.9m、上 0.26m），需做几何补偿。
- **base_link**：车体参考坐标系原点（一般在车体中心/轮轴），对外定位都换算到它。
- **header.seq**：ROS 消息头里的递增序号。节点重启后会从 0 重新计数，故可用“序号
  突然变小”判断 `/laserMapping` 是否重启过。
- **牛耕式 (boustrophedon)**：割草机像耕地一样“一行一行来回”的覆盖路径。

---

## 1. 设计要点（与 ceres 版的差异）

| 方面 | ceres 版 | 本版 fusion_4dof |
|---|---|---|
| LIO→地图对齐 | ceres 位姿图，每帧重建整图求解 | **水平滑窗SE(2)+稳健高程偏移**的4DoF，同步、轻量 |
| 位置帧 | 绕 `origin_gauss_yaw` 旋转的地图系 | **纯 ENU**（East/North），无单点航向杠杆 |
| GPS 失效切换 | ceres 输出回退 | 冻结 SE(2) 后 LIO 航位推算，无跳变 |
| 冷启动 | 单点保存航向 | **RTK 航向种子**（节点启动后实时航向），无需保存航向 |
| LIO 重启识别 | 无显式 | 自触发标志 + `header.seq` 回退 + 跳变兜底 |
| 故障重启 | 一律联合重启 | 分级：RESET(节点内) / RECOVER(仅 /laserMapping) / FAULT(停车) |

**纯 ENU 后果**：本版记录的地图/边界与 ceres 旋转帧**不兼容，须用本版重新记录**；
下游 pure_pursuit 按 ENU 消费 `/Mower/position`（横向跟踪误差与帧约定无关）。

---

## 2. 坐标系（纯 ENU）

- 位置：`geoConverter`（GeographicLib LocalCartesian）以地图原点为基准的 ENU
  East/North，**不做任何航向旋转**。定位模式原点 = 地图文件保存的原点；建图模式
  原点 = 系统初始化时的首个稳定 GPS（建议**长期静态后**再开始记录以获可靠原点）。
- 航向：由 GpsPosition 的 `gauss_yaw` 换算到 ENU（从 East 起、逆时针）：
  `yaw_enu = wrap(gauss_yaw·π/180 + π/2)`，范围 (-π, π]。该式由旧代码自身一致性
  导出（旧 `Is_GPS_GOOD` 中 `published_yaw == atan2(Δy,Δx)` 恒成立），**不含
  `origin_gauss_yaw`**，故航向也无单点航向杠杆。
- **⚠ 航向约定必须实地核对（本版新增的敏感点）**：因 `gauss_yaw = azimuth − 90°`，
  故 `yaw_enu = azimuth`（弧度）。这只有在 INS 的 azimuth 本就是“正东=0、逆时针”
  （GpsPosition.msg 所述）时才对。**旧版航向是“相对起点”，对 azimuth 约定的常量偏差
  免疫（会抵消）；本版是绝对 ENU，不再抵消。** 因此上电后务必核对：车头朝正东→发布
  `yaw≈0`、朝正北→`≈+π/2`。若实测为“正北=0、顺时针”，把 `GPS2Local` 的换算改成
  `π/2 − azimuth`。注意：**位置(x,y)不受影响**（来自经纬度），受影响的只有绝对航向，
  以及冷启动“RTK 航向种子”（锁定后 θ 由位置拟合得出，与航向约定无关）。
- **GPS 输出帧 与 SE(2) 回退帧严格同一帧**：估计器不再自带 GeographicLib，
  `InputGPS` 直接接收 `GPS2Local` 算好的 ENU 平面位置 → 二者帧完全一致，
  GPS↔回退切换无系统性偏移（这也消除了早期“双 converter 不同原点”的 create
  模式切换跳变）。

### 2.1 FAST-LIO重力水平局部系H

- `/Mower/lio_slam`：保留的原始FAST-LIO IMU位姿，表达在每代进程的W系；
  内部ikd-tree、`/Odometry`和点云均不改坐标定义。
- `/Mower/lio_slam_level`：供fusion使用的IMU位姿，表达在重力水平系H。
  FAST-LIO IMU初始化后用状态重力计算一次 `R_H_W`，使
  `R_H_W*g_W=[0,0,-|g|]`，随后在该`/laserMapping`进程代际内永久冻结。
- 位姿变换是 `p_HI=R_H_W*p_WI`、`R_HI=R_H_W*R_WI`；不使用逐帧加速度
  更新世界系，不改FAST-LIO内部估计。
- fusion再右乘IMU->base_link的6DoF外参并做杆臂补偿。不以fusion进程
  收到的第一帧重新定义原点；H的任意原点由`tx/ty/tz`一次性吸收，
  从而仅重启fusion不会二次归零。
- H的z轴与ENU Up平行，因此只需高程平移
  `tz=robust_mean(z_GPS_ENU-z_LIO_H)`，无需让GPS估计roll/pitch。生产z是
  `z_LIO_H+tz`；GPS高程只用于稳健更新`tz`，不逐帧直接替换输出z。
- `/Mower/lio_level_transform`是latched话题，旋转部分记录当前代际
  `R_H_W`，`header.stamp`是FAST-LIO进程代际标识。仅当代际、地图索引和
  ENU原点完全一致时，`fusion_only.launch`才恢复落盘的4DoF。
- 初始化时必须停车静止；否则线加速度会污染FAST-LIO的初始重力，这是算法前提。

---

## 3. 数据流与状态机

节点由 **LioCallBack 驱动**（LIO 作时钟）。每个 LIO 帧：

```
LioCallBack(lio_msg):
  1. 取 /Mower/lio_slam_level 的重力扶正IMU位姿 lio_t/lio_q
  2. [RESET] LIO 帧代际检测（flag / header.seq 回退 / 跳变兜底）→ 命中则 ResetLioCoupled
  3. LIOPrecess: 轴对齐 + 6DoF外参/杆臂补偿 → H中的base_link位姿
     （不按fusion首帧归零；调试xyy_lio与生产回退使用同一个4DoF）
  4. Is_LIO_GOOD（速度/漂移）→ 若好则 globalEstimator_.InputOdom(lio_stamp_, lio_t, lio_q)
  5. 从队列取与本帧时间最近的 GPS；RTK质量+速度/航向一致性门控→ location_gps(ENU)
  6. 若 gps 好且已初始化：正常态提交 InputGPS；失效恢复期仅缓存候选配对，
     达到 gps_stable_threshold 的同一帧才提交完整候选窗
  7. GetPosition() → fusion_pos_（4DoF回退x/y/z/yaw，姿态矩阵为Rz(θ)·R_H_base）
  8. 输出选择（状态机）：
       gps 好           → N_RTK : 发布 GPS(ENU)                     pos_state=1
       gps单帧不可用     → N_RTK_COAST: 可信RTK锚点+同代LIO增量     pos_state=1
       gps连续/超时不可用 & 对齐好 → N_LIO: 发布4DoF回退(ENU)      pos_state=2/5
       否则             → FAULT : reboot=true 停车
```

N_LIO 用的 4DoF 把 LIO 映射到与 GPS 相同的 ENU 帧；GPS 一断即冻结当前生产
`(θ,tx,ty,tz)`，LIO 经它航位推算。GPS 恢复确认期间候选样本不会修改回退位姿；
达到恢复门限时，生产变换提交与 `position_state: 2→1` 在同一回调发生。若断流期间
LIO 与 GPS 已产生真实偏差，切到状态 1 时仍可能出现源切换差值，但不会再提前污染
状态 2 输出。

所有不可用RTK观测统一遵循“不因一帧切源”：同步未收到、时间基线、质量、运动、
方向或创新拒绝的当前RTK坐标一律隔离；连续不可用的前3帧保持状态1，以最后一次
通过最终连续性门的RTK输出为锚点，叠加同代LIO相对该测量时刻的增量来延拓位姿。
`gps_grace_coast_output_enabled`控制是否实际发布该延拓结果：默认`true`；设为
`false`时宽限期仍照常计数和隔离坏RTK，但不发布定位帧，第4次连续不可用后仍按
正常状态机进入显式LIO回退。
任一Accepted都会清零计数；第4帧或真实时间上限先到时才切冻结4DoF/LIO并进入
22帧完整恢复。`gps_motion_max_dt_sec=0.45s`适配正常10Hz、偶发6～7Hz，并允许
10Hz下连续3个空洞后的约0.4s恢复帧；2Hz明确作为异常工况。

---

## 4. 重力约束4DoF在线对齐估计器（核心，`global_fusion.*`）

目标：估计 `p_ENU.xy ≈ Rz(θ)·p_H_base.xy+(tx,ty)` 与
`z_ENU ≈ z_H_base+tz`。水平仍用闭式Umeyama；竖直方向利用H与ENU都由
重力定义“向上”的前提，只估计一个高程零点偏移。该变换是刚体的，
不含尺度自由度。

### 4.1 配对
`InputGPS(gps_stamp, …)` 保留 GPS 的真实测量时间，`InputOdom(lio_stamp_, …)` 保留
LIO 的真实测量时间；估计器在统一的 `se2_match_tol_sec` 容差内按真实时间做最近邻，
只有合格的 (LIO,ENU) 对才允许入窗。（见 4.4。）

### 4.2 旋转来源（按精度优先级，高→低）
每帧 GPS 到来后重估，**优先用更精确的来源**：
1. **横迹展布充分**（`cross_std ≥ se2_lock_cross_std`）**且**鲁棒内点至少
   `se2_lock_min_samples`个、内点率不低于70%、沿迹展布至少0.50m、解缠航向覆盖至少
   30°、协方差条件数 `λmax/λmin`不超过50，**且**本帧数据可信（见
   §4.2b）**且** `se2_lock_confirm_frames` 组在时间和位移上都独立的观测
   都满足上述全部条件 → 全 Umeyama
   二维拟合，**锁定**（最精确，用鲁棒内点）。各个条件缺一不可：仅
   `cross_std` 达标不再足以锁定，防止小样本或单帧 GPS 噪声/多径瞬时抬高
   `cross_std` 造成误锁定（且不可撤销）。真实拐弯的横迹展布会持续覆盖窗口内
   多帧。第一次达标时冻结一版候选变换；后两次用不参与该候选生成的新RTK/LIO点
   检验它，且每次至少间隔1s、LIO位移至少0.30m。这样不是对三个高度重叠的滑窗
   简单重复计数；任一新证据与冻结候选不一致都会清零确认。
2. **已锁定** → 生产用完整 `(theta,tx,ty,tz)` **全部冻结**；后续RTK不得
   改写状态2/5使用的变换。若未来需要缓慢跟踪LIO长期漂移，应增加与生产变换
   隔离的shadow estimator和显式交易，不能原地改写。
3. **沿迹基线达标**（`along_std ≥ se2_seed_along_std`）→ 窗口平均的运动方向
   （比单点航向稳）。
4. **RTK 航向种子** → `θ = wrap(enu_yaw − lio_yaw)`（ENU 航向减 LIO 航向，即两坐标系
   的夹角）。**冷启动桥接**：仅当RTK位置原始质量通过、DRPVA航向标准差达标时，
   才允许单点航向播种，单帧即可
   给出可用回退，短时短距精度足够；随后被 3/1 的更精确来源取代。**注意**：此档
   精度明显低于判据 1（锁定），若长直线段上恰好长时间丢 RTK，误差会随行驶距离
   累积——运行时若回退正处于此档（未 `IsLocked()`），会打印节流告警
   `[4DOF] fallback in use but NOT locked yet ...`，供排查/评估风险用。
5. 否则 → 本帧暂不可对齐（`IsAligned()=false`）。

平移 `(tx,ty)` 始终用当前 θ 对齐窗口质心（鲁棒重拟合已生效时即内点质心；n=1 时
质心即当前点，等价锚到当前 GPS）。

**锁定的准确语义是生产变换不可变。** `locked_` 不再只是历史质量标志，
也不是只冻结theta；锁定后水平和竖直更新函数都直接保持上次提交值。

### 4.2a RTK字段可信来源与入窗前创新门

当前1ant发布器只在DRPVA校验通过、解算有效且RTK fixed，同时新鲜GGA为
quality=4、新鲜IMUATTA为INS good时发布。4DoF仍做独立的fail-closed复核：
`gps_flag`、`INS_Status`、`INS_GpsFlag_Pos`、星数、差分龄期及DRPVA位置/
高程标准差必须全部达标；标准差为0按“旧发布器未填写”处理，不冒充完美观测。
航向还必须满足DRPVA航向标准差。1ant派生的运动航向确认
`positionStatus`仅保留作诊断，不参与4DoF启动门控；否则控制器停车等待定位时会与
“车辆先运动才能确认航向”形成循环依赖。

`gps_confidence`当前固定为10，`INS_GpsFlag_Heading`只是由INS good推导，
`INS_VehicleAlign`固定为0，均不得参与质量裁决。与此同时，
`positionStatus`是工程派生状态而非接收机原始状态，不能替代RTK位置或航向质量字段。

已有提交变换后，每个新RTK点必须先与同测量时刻的
`T_committed(LIO)`比较XY、yaw、z创新；未锁定用较宽阈值，锁定/恢复用较紧阈值。
不通过的点不会更新单点种子元数据，也不会进入拟合窗口。RTK自身相邻帧速度检查
仍保留为辅助门：断流后的第一帧只建立基准、不计稳定帧；真正判断与当前定位是否
跳变，以同时间4DoF-LIO创新为主，不能先入窗再发现窗口被污染。

### 4.2b 异常值鲁棒拟合与拟合质量自检
每次重估分两步：
1. 先用全窗口做**参考拟合**（闭式 Umeyama）；
2. 用参考变换算出窗口内每个点的残差，剔除残差 `> se2_outlier_residual_thresh`
   的点；若剩余内点占多数（`≥ 3` 且 `≥ 半数`），**用内点重新拟合**，覆盖参考值
   作为本帧工作估计——单次 GPS 多径/野点不再能直接拉偏整窗结果。若内点不足半数
   （数据系统性异常，例如时间同步错位、杆臂标定有较大误差，而非孤立野点），本帧
   **拒绝更新生产变换**（θ/tx/ty 保持上次可信值），且不计入判据 1 的连续确认
   计数，并在日志打印 `majority of window flagged as outliers ...`。

内点残差 RMS 与本帧异常值占比作为**运行时质量自检**输出（`GetFitResidualRms()` /
`GetOutlierRatio()`），fusion 节点在两者持续偏大（`>0.3m` / `>30%`）时打印节流告警
`[4DOF] fit quality degraded: ...`。这是纯粹的可观测量，**不用于门控** `aligned_`/
`locked_`；但“多数异常”会直接拒绝本次变换提交——用于运行时/排障时发现“数据本身内部不自洽”
（GPS 多径、时间同步跑偏、杆臂标定错误等），并非用来验证 §2 中航向符号约定
（那属于绝对航向解释问题，不体现在位置残差上，仍需实地核对）。

### 4.3 冻结、恢复候选与回退
GPS 失效时 `(θ,tx,ty,tz)` 整体冻结；`InputOdom` 仍每帧用已提交变换映射 LIO。
缺消息、时间基线、质量差、速度/方向异常或创新冲突均共享3帧/真实时间双限制宽限；
坏RTK样本绝不进入输出，宽限期只用可信RTK锚点+同代LIO增量。第4个连续不可用
观测才进入完整失效。默认保留
`gps_recovery_floor=28`、`gps_stable_threshold=50`，即需要 22 个连续合格样本恢复。
这些样本通过 `InputGPS(..., commit_alignment=false)` 进入候选配对窗；任一不合格帧
会把计数恢复到 28 并清空候选窗。第 22 个样本调用
`InputGPS(..., commit_alignment=true)`。若变换尚未锁定，可一次性用完整候选窗重估；
若已锁定，仍保持完整4DoF不变，只允许恢复RTK作为输出源。

源切换还有最后一道发布门：直接检查本帧实际准备发布的 `xyy_p`，并与上一条已成功
发布的 `/Mower/position` 比较XY速度/绝对跳变量、yaw变化和z跳变量。RTK→LIO或
LIO→RTK都经过同一规则；超限时不发布、不前移可信基准、立即停车并报告算法故障。
这避免了旧实现“检查 `fusion_pos_` 的速度、实际却发布 `xyy_p`”的对象错位。

### 4.3a 竖直方向如何抑制GPS高程噪声

窗内每个同步样本形成 `dz_i=z_GPS_i-z_LIO_i`。先取`dz`中位数，剔除与
中位数相差超过`vertical_outlier_threshold`的样本；然后要求内点数不少于
`vertical_min_samples`、内点严格过半、内点标准差不超过`vertical_max_std`，
才把内点均值提交为`tz`。否则：尚未成功过就不发布有效map位姿；已有
可信`tz`则冻结上次值。`vertical_require_ins_height_std=true`时还会使用组导
`INS_Std_LocatHeight`做单帧门控；启用前须确认该字段单位为米。

这只能抵抗野值和短期噪声，不能证明GPS高程是真值。如果组导高程存在
平滑的系统偏差，全局z零点会继承它；但后续高程变化主要由LIO连续性给出。

### 4.4 GPS/LIO 时间键与最近邻配对
- **队列选帧**：节点以 LIO 回调驱动，在 `se2_match_tol_sec` 容差内检查当前已到达的
  全部 GPS 候选，选取与当前 LIO 时间戳绝对差最小的一帧；过旧帧丢弃，窗口上界以外
  的未来帧保留给下一次回调。上层队列和估计器使用同一个容差参数。
- **保留真实时间**：选中的 GPS 以自身 `gps_stamp` 送入估计器。估计器再从带真实时间
  的 LIO 缓存中查找最近邻，只有时间差不超过同一容差才允许进入 4DoF 窗口。不能在未
  插值的情况下把 GPS 重标成 `lio_stamp_`；那会把 `v·Δt` 的沿迹误差写进拟合样本。
- **输出语义**：`position_state=1` 的 `/Mower/position` 以 GPS 为 x/y/yaw 主来源，消息、
  RViz 输出和 map→base_link TF 使用该 GPS 的真实时间戳；状态 2/5 的 LIO 回退继续使用
  LIO 时间戳。正常 GPS 状态下 z 仍按既有设计采用连续的 LIO+tz，避免跟随 GNSS 高程
  噪声，因此该时间戳表示主定位观测时刻。
- **可选增强**：若未来需要亚厘米级动态同步，可在 GPS 真实时刻对相邻 LIO 位姿进行
  插值；当前实现采用带容差的最近邻，不做基于速度模型的 GPS 外推。

> 当前1ant把消息 `header.stamp` 写成主机收到并发布该帧时的 `ros::Time::now()`；
> 接收机原始 `gps_week/gps_millisecond`虽被原样发布，但尚未转换成与LIO同一时基的
> ROS测量时间。因此这里“真实时间”准确地说是**未经4DoF篡改的上游消息时间**，不是
> 接收机硬件测量时刻。若网络/串口延迟抖动接近50ms容差，应在1ant统一时钟后再使用
> 接收机时间，不能由4DoF猜测或伪造时间戳。

尚未完成的慢漂检测、RTK/LIO故障归因、独立源切换门和受控重对齐见
[`docs/SAFETY_TODO.md`](docs/SAFETY_TODO.md)。这些事项不得通过简单放宽创新或
最终输出阈值规避。

---

## 5. LIO 帧代际检测与重启分级

### 5.1 代际检测（识别 /laserMapping 重启 → 节点内 RESET 局部复位）

权威判据是`/Mower/lio_level_transform.header.stamp`：它由FAST-LIO在进程启动时用
墙上时钟生成，且只在2秒连续静态初始化成功后才latched发布。token变化立即
`ResetLioCoupled()`（清滑窗和4DoF，保留已读取的地图ENU原点）。
`header.seq`回退、自触发`lio_restart_pending_`和“大跳回原点”仍作为姿态流侧的
兜底判据，但不再独自承担fusion_only连续性识别。

### 5.1a 仅重启fusion为什么可以连续

4DoF有效后会以“同目录临时文件+原子rename”保存到`map_path+.lio4d`（可用
`alignment_state_path`覆盖）。v2文件含版本、校验和、LIO代际token、地图索引、
经纬高/航向原点、`theta/tx/ty/tz/locked`、地图UUID、坐标系版本与
原点校验和。旧v1缓存安全拒绝并重新对齐。`fusion_only.launch`固定以定位
模式读取建图原点；新fusion只在文件完整、校验通过、地图标识逐值一致
且FAST-LIO token相同时恢复。因此：

- 只重启fusion、FAST-LIO未动：恢复原4DoF，无GPS时也能保持坐标连续；
- FAST-LIO也重启：token不同，严禁恢复旧变换，必须重新用同步GPS/LIO对齐；
- 地图文件被换或损坏：拒绝恢复，定位处理保持故障封闭和停车，
  但服务节点保持在线以允许用完整历史图修复，绝不发布“看似有效”的错帧位姿。

### 5.2 重启分级（2×2：RTK 好/坏 × LIO 好/坏）
| | RTK 好 | RTK 坏 |
|---|---|---|
| LIO 好 | 正常 N_RTK | N_LIO 回退（用冻结 SE(2)） |
| LIO 坏 | **RECOVER**：仅重启 /laserMapping（`/fusion_4dof` 存活、走 RESET 重对齐） | **FAULT**：停车报障（无参考可重锚） |

- **RECOVER** 在节点内通过
  `rosrun fusion_4dof restart_ros_nodes.sh /laserMapping -- roslaunch fusion_4dof laserMapping_only.launch`
  触发。
- **FAULT** 置 `reboot=true` → 发 `/Mower/reboot` → android_manager_4dof 联合重启兜底。
- **android_manager_4dof 看门狗**：按本机墙钟接收时间计时，只有有效
  `position_state=1/2/5`才刷新位置存活时间；仍收到 LIO 但有效 position
  断流 >1s 立即停车，>10s 时
  诊断为 `/fusion_4dof` 卡死 → **仅重启 /fusion_4dof**（`fusion_only.launch`，保留健康
  LIO）；低频看门狗会再检查manager实收LIO是否1秒内新鲜、`lio_state=true`
  且静态初始化就绪，三门都满足时只重启fusion，任一缺失才联合重启。
  位置与LIO同时断流、算法全恢复请求和显式模式切换仍采用联合重启
  （`/fusion_4dof /laserMapping`）。`save_map/use_map`是节点内地图事务，
  不重启fusion或FAST-LIO。
- **自动重启风暴保护**：FAULT、低频和 fusion 断流三类自动路径共享
  计数，120s 窗口内最多执行3次。第4次起暂停自动重启并保持停车，
  直到控制器/安卓明确发出`use_map`、`m_mode`或`true`模式指令才解除。

### 5.3 重启期间的停车（独立请求 + /mower/stop_car 汇总）
**判据是”会不会重启 /fusion_4dof”，而不是”会不会重启 /laserMapping”**：/fusion_4dof
进程重启期间（旧进程已退出、新进程尚未起来），不论是否连带重启 /laserMapping，都
**没有任何位置输出**——不能假设下游会因为位置断流而自行停车，显式停车是唯一可靠的
安全兜底。Bool 话题的多发布者不具有“逻辑 OR”语义，一个发布者的
`false`会覆盖另一个仍在要求的`true`。因此现在由 fusion 锁存发布
`/mower/fusion_stop_car`，manager 锁存发布`/mower/manager_stop_car`。manager明确
订阅fusion请求、做逻辑OR，并作为唯一定位停车汇总者锁存发布底层实际订阅的
`/mower/stop_car`；不能依赖ROS多发布者实现OR。manager刚启动且尚未收到fusion的
latched状态时按`true`处理，避免连接建立期间误放行。任一来源未解除时都不会放行：
- **RECOVER（fusion_4dof 内，仅重启 /laserMapping，/fusion_4dof 本身不重启）**：
  `/fusion_4dof` 进程存活、代码可控，采用”精确”释放：发起重启前发 `true`；待新 LIO
  回来（代际复位完成）**且 `globalEstimator_.IsAligned()` 已重新对齐**、且连续
  `stop_car_resume_frames` (默认5) 帧通过时间戳、姿态、速度、同步、4DoF和输出连续性
  全部门控并实际发布的生产位姿后才发 `false` 解除。三个条件缺一不可——仅凭 LIO
  自身健康标志 `lio_state_`不能说明此刻确有可用于发布的位置来源：
  `ResetLioCoupled()` 已清空对齐状态，需等下一次好的 GPS↔LIO 配对重新播种（见
  §4.2 判据 4 冷启动种子）。若在此之前仅凭 `lio_state_` 健康就解除停车，一旦此时
  RTK 恰好也不可用，会出现”已放行但本帧其实无位置可发”的空窗（随后虽会被 FAULT
  分支追发停车，但那是滞后补救而非事先确认）——故已改为三条件联合门控，先确认有
  位置来源、再解除停车。
- **联合重启（android_manager_4dof：`m_mode` / `true`、fusion全恢复请求、有效位置
  低频看门狗、位置与LIO同时断流）**：均重启 `/fusion_4dof` 和 `/laserMapping`。
  `save_map/use_map`不会触发全量重启。重启前发 `true`；
  新 `/fusion_4dof` 的 `/Mower/position` 恢复有效若干帧后发 `false`。
- **仅重启 /fusion_4dof（android_manager_4dof `LioSlamCallback`：LIO 健在但 position
  断 >10s）**：**同样重启前发 `true`**——虽不动 /laserMapping，但 /fusion_4dof 进程
  仍会经历”旧进程退出→新进程未起来”的空窗，逻辑与联合重启一致，不能因为”LIO 健在”
  就假设可以不停车。重启完成、`/Mower/position` 恢复有效若干帧后发 `false`。

---

## 6. 参数总表（改后**重启节点即生效，无需重编译**）

**所有运行参数集中在唯一文件 `config/fusion_4dof_params.yaml`**，三种 launch
（`location_mode_4dof` / `create_mode_4dof` / `fusion_only`）都用
`<rosparam command="load" .../>` 载入它 —— 不在各 launch 重复，避免多处不一致。
缺省以**精度为主**。改完该 YAML 重启对应 launch 即生效。

### SE(2) 在线对齐
| 参数 | 默认 | 含义 |
|---|---|---|
| `se2_max_window` | 400 | 配对窗最大帧数，≈ `span × GPS频率` |
| `se2_window_span_sec` | 45.0 | 窗时间跨度(s)：大→更平滑、长直线保住上次锁定；过大→跟随 LIO 漂移慢 |
| `se2_match_tol_sec` | 0.08 | 上层GPS队列选帧与估计器GPS↔LIO最近邻共用的时间容差(s)；0915数据表明50ms会周期性漏配 |
| `se2_seed_along_std` | 0.50 | 沿迹标准差(m) 达此值才用窗口平均方向（不足时用 RTK 航向种子） |
| `se2_lock_cross_std` | 0.10 | 横迹标准差(m)：按本机0.4m行距与实测噪声地板设定（见§13） |

> 注：原 `se2_seed_yaw_min_speed`（航向种子车速门限）已**移除**——RTK/INS 组合惯导
> 在手动操控完成初始化后航向持续可用，无需车速门控。

### 锁定稳健性 + 异常值鲁棒拟合（见 §4.2/§4.2b）
| 参数 | 默认 | 含义 |
|---|---|---|
| `se2_lock_min_samples` | 15 | 鲁棒内点至少这么多才允许锁定，不再使用窗口总数冒充 |
| `se2_lock_min_inlier_ratio` | 0.70 | 锁定所需最小水平内点率 |
| `se2_lock_min_along_std_m` | 0.50 | 锁定所需最小沿迹展布 |
| `se2_lock_max_condition_number` | 50 | 二维位置协方差条件数 `λmax/λmin` 上限 |
| `se2_lock_min_accumulated_turn_deg` | 30 | 鲁棒内点窗口所需解缠航向覆盖范围（非抖动累加） |
| `se2_lock_confirm_frames` | 3 | 需这么多组独立观测达标才锁定，不等同于三个相邻回调 |
| `se2_lock_confirmation_min_interval_sec` | 1.0 | 两次锁定证据的最小时间间隔 |
| `se2_lock_confirmation_min_displacement_m` | 0.30 | 两次锁定证据的最小LIO位移 |
| `se2_outlier_residual_thresh` | 0.5 | 鲁棒重拟合的残差剔除阈值(m)：正常 RTK/LIO 噪声远小于此，仅用于剔除多径/野点 |

### LIO 帧代际检测
| 参数 | 默认 | 含义 / 约束 |
|---|---|---|
| `lio_epoch_jump_thresh` | 2.0 | 单帧位移>此=非物理跳变；须 > 最大车速×LIO周期、< 跳回原点位移 |
| `lio_epoch_far_thresh` | 3.0 | 开出此距离才“算曾远离原点”；须 > near |
| `lio_epoch_near_thresh` | 0.5 | 回到此半径内+跳变=判重启；须 > LIO 静止噪声、< 最小作业半径 |

---

## 7. 实地调参流程

1. **静态验证读入**：启动看日志 `[4DOF] window=… seed_along=… epoch …` 是否=所设
   （即 `config/fusion_4dof_params.yaml` 已被正确载入）。
2. **冷启动可用性**：确认 RTK/INS 已完成初始化（航向有效）后，RTK 好时从起点直行，
   RViz 中 `/Mower/debug/lio` 应**立即**贴合 `/Mower/debug/gps`（RTK 航向种子生效，
   无需先达到某车速）。若贴合不上 → 检查 INS 航向是否已初始化/有效。
   `/Mower/debug/gps`只表示通过质量门且与当前LIO成功时间配对的RTK观测；无可用RTK时
   不发布，不能把它当作GPS驱动存活心跳，存活性应监视`/nanobot/gpsposition`及状态话题。
3. **过渡到窗口平均**：行驶 `se2_seed_along_std` 量级距离后转用窗口平均方向；
   要更早转入可调小 `se2_seed_along_std`。
4. **锁定与航向稳定**：走完一个**调头**后进入长直线，若 `lio` 航向在直线上仍乱动
   → 调小 `se2_lock_cross_std` 让拐弯后尽快锁定，或调大 `se2_window_span_sec`
   让拐弯留在窗内。精度优先：宁可多等一个清晰拐弯再锁。
5. **回退切换验收**：对齐稳定后**遮挡 RTK 天线**几秒，看 `/Mower/position` 是否
   平滑切到 `state=2` 且无位置跳变 —— 最终验收。跳变大 → 回到 3/4 收紧对齐。
6. **代际检测验收**：RTK 好时 `rosnode kill /laserMapping`，看 fusion 日志是否打印
   `Detected LIO frame reset (flag/seq/jump)` 并随后重新对齐；误触发频繁 → 抬高
   `lio_epoch_jump_thresh`。

**精度优先的取舍**：因冷启动由 RTK 航向种子兜底、且不要求快速回退，参数默认偏向
“多平均、清晰拐弯才锁定”。若反而希望回退尽快可用，可调小 `se2_seed_along_std` /
`se2_lock_cross_std`，但单次估计噪声更大。

---

## 8. 可观测量（调参/排障）
- **RViz**（Fixed Frame=`map`）：`/Mower/viz/{gps,lio,fusion,output}/path`。对齐好时
  `lio` 应与 `gps` 轨迹重合。
- `/Mower/debug/lio`：仅在`IsAligned()`为真后发布的SE(2) ENU映射结果；
  未对齐期间不发布，防止把局部H系误标成`map`系。
- `/Mower/position` 的 `position_state`（完整定义见 `util/msg/Position.msg` 顶部注释）：
  1=GPS、**2=融合回退且 SE(2) 已锁定（精度最高）、5=融合回退但仅播种未锁定（沿迹
  平均/RTK 航向种子，精度较低）**。1/2/5 对下游而言都是“有效可用位置”，判断“是否
  有位置”须写 `==1||==2||==5`，只写 `==1||==2` 会把仅播种阶段误判为“无定位”。
  **⚠ 5 是本次新增的状态值**：已同步更新本仓库内的 `android_manager_4dof` 与
  `crawler_control/src/task/src/task_node.cpp`（`FusionMapCallBack`，该仓库原
  `==1||==2` 判断已一并改为 `==1||==2||==5`）。
- 启动日志 `[4DOF] …` 打印生效参数（含新增的 `lock_min_samples` / `lock_confirm_frames`
  / `outlier_thresh`）。
- **`[4DOF-TUNE] …`（周期性诊断日志，1Hz 节流，实地测试后离线复盘参数用）**：
  每条打印当前完整状态——`gps_good/gps_failed_cnt/gps_failed_flag/gps_stable_cnt`、
  `lio_state/lio_failed_cnt/lio_drift`、`aligned/locked`、`theta(rad/deg)/tx/ty`、
  `along_std/cross_std/window_n/xy_inliers/condition_number/turn`、
  `res_rms/outlier_ratio`、最终 `pos_state` 与
  发布的 `xyy`。**这是交给我分析调参时最主要的信息来源**：把 along_std/cross_std
  的时间序列跟 `se2_seed_along_std`/`se2_lock_cross_std`/`se2_lock_min_samples`
  对比，能看出播种/锁定的时机是否合理；`res_rms`/`outlier_ratio` 能看出数据质量；
  `aligned/locked/pos_state` 的跳变能定位每次状态切换发生在哪一刻。
  **如何拿到日志文件交给我**：
  - 方式一（推荐）：使用 ROS 默认的分节点日志
    `~/.ros/log/latest/fusion_4dof-*.log`
    （`latest` 是软链接，指向最近一次 `roslaunch`/`rosrun` 的运行目录），测完直接
    copy 这个文件；
  - 短时、人工看守的单次测试可以另存终端输出，但禁止让多个长期运行的
    `roslaunch`持续追加同一个`ros_restart.log`；这会失去进程边界且无界占用磁盘；
  - 若想要完整时序（含 rosbag 方式而非纯文本日志），也可以 `rosbag record /rosout`，
    但纯文本日志对本轮的参数分析已经够用，不必多此一举。
- 其余运行时告警（均为节流打印，不影响功能，用于排障/评估）：
  - `[4DOF] fallback in use but NOT locked yet (position_state=5) ...`：当前回退
    的 θ 只到“已播种”档（沿迹平均或 RTK 航向种子），尚未经拐弯锁定，长直线段长时间
    丢 RTK 时精度可能随距离下降。
  - `[4DOF] fit quality degraded: residual_rms=...m outlier_ratio=...`：SE(2) 拟合
    内点残差 RMS 或异常值占比持续偏高，提示 GPS 多径/时间同步/杆臂标定可能有系统性
    问题，值得排查（不是航向符号约定问题，那不体现在位置残差里）。
  - `[global_fusion] majority of window flagged as outliers (...)`：本帧窗口内多数
    点被判为异常值，本次生产变换提交被拒绝、θ/tx/ty 保持上次可信值，且未计入
    锁定确认计数；偶发可忽略，持续出现应排查 GPS/LIO 数据质量。

---

## 9. 已知限制
- **死角**：RTK 坏且 LIO 坏（或 LIO 帧刚重置又无 RTK 可重锚）→ FAULT 停车。
  要消除需不依赖 RTK 的绝对参考（先验地图重定位 / UWB / 双天线）。
- **跨重建图一致性**：原点取本次保存原点，频繁重建图会换原点 → 地图不叠合；
  “建一次、复用同一保存原点定位多次”成立。
- `pub_fusion==3`是纯LIO局部模式：输出base_link在H中的相对XYZ和姿态，
  其roll/pitch与重力自洽，但没有ENU原点和绝对yaw，不能直接用于历史地图跟踪。
- `GetSeedOnlyTransform()`（§11）在窗口只有 1 个配对点时，与 §4.2 判据 4 的种子
  分支数值上重合（都是单帧 RTK 位置+航向），但两者是完全独立的两条计算路径——
  前者纯只读、不写任何生产状态，后者是状态机的一部分。窗口点数变多后二者会分叉
  （前者永远只用“最近一帧”，后者一旦满足判据 1/2/3 会切到窗口拟合或冻结）。
- **统一生命周期状态机**：`fusion_4dof`在锁存话题
  `/Mower/localization_algorithm_status`发布其亲自掌握的INS、FAST-LIO代际、地图
  原点、4DoF对齐、位置来源和恢复请求；`android_manager_4dof`作为唯一生命周期
  所有者，在锁存话题`/Mower/localization_status`补齐请求/生效模式、当前/待切地图、
  重启范围、看门狗、停车和故障原因。fusion内部不再执行节点重启；LIO质量或同步
  异常只提出`RESTART_LIO`，由manager与Android指令、FAULT和看门狗事件一起经过
  同一显式状态机、限流和唯一命令执行入口处理。
- **地图热切换（4–6）**：`save_map` 保存新图后直接沿用建图时的
  4DoF 对齐；`use_map` 切到不同原点时只清地图耦合对齐、GPS 队列和
  IMU 全局锚点，不重启 FAST-LIO。每张命名图配套 v1 YAML 元数据，
  持久保存 UUID、`ENU_V1`、逻辑名和原点校验和。完整设计、故障语义、
  验证结果与剩余风险见 [`docs/localization_lifecycle_456.md`](../../docs/localization_lifecycle_456.md)。

### 9.1 `LocalizationStatus`语义

- `requested_mode/active_mode`作为兼容字段描述进程模式；新消费者应同时读取
  `workflow_mode`和`requested_runtime_mode/active_runtime_mode`。前者表示用户仍在
  建图还是已进入使用地图定位，后者表示fusion正在创建原点还是读取已提交原点。
  建图中故障恢复为`TRACK_SAVED_ORIGIN`时，`workflow_mode`仍为`WORKFLOW_MAPPING`。
  manager独立重启后若没有新的显式模式/地图命令，`workflow_mode`保持
  `WORKFLOW_UNKNOWN`，不得由当前runtime反推；否则会把建图中的
  `TRACK_SAVED_ORIGIN`恢复错误标成定位工作流。
- `map_origin_committed=true`仅表示canonical原点bundle已经完整落盘/加载，不表示
  最终命名地图或边界文件已经保存。
- `active_map`是实际生效的逻辑地图名，`pending_map`是正在切换但尚未通过就绪门控的
  地图；匿名运行槽`my_map.mp`不冒充逻辑地图名。老系统冷启动时若没有持久化地图名，
  `active_map`为空是“身份未知”的诚实表达。
- `ins_solution_good`严格表示接收机`INS_Status==3`；兼容字段`ins_initialized`现在采用
  相同语义，不再混入需要车辆运动的`positionStatus`。原始`INS_Status/
  INS_VehicleAlign/INS_GpsFlag_Heading/positionStatus`仍同时发布用于诊断。
- `position_valid`仅对生产状态1/2/5成立且会在输出超过1秒未刷新后自动变false；
  只要`stop_required/transitioning/fault/restart_scope`任一生效也必须为false。
  状态6仅为制动期受限IMU积分，不会被标成可自动作业定位。
- 恢复后解除fusion停车请求前显示`PHASE_WAIT_TRUSTED_OUTPUT`；计数对象是通过全部
  门控并实际发布的连续生产位姿，不是尚未经过速度/同步/连续性检查的原始LIO回调。
  fusion解除自身请求后，manager执行自己的连续5帧验证时也保持该phase；只有最终
  `/mower/stop_car`解除后才对外显示READY，不会出现READY同时要求停车的组合。
- 两个状态话题均为latched并以5Hz心跳刷新。manager若超过2秒收不到算法状态，先
  进入`PHASE_WAIT_ALGORITHM`并锁存停车；心跳恢复后还需5帧新鲜生产位姿才放行。
  这5帧只在同一代算法状态已经同时满足地图身份、FAST-LIO静态初始化、4DoF对齐、
  IMU里程计就绪且无停车/故障/重启时才计数；任一状态退回等待会立即清零，旧进程或
  转换期间仅凭`position_state=1/2/5`发布的帧不能提前积攒放行额度。
  若主定位已就绪而只剩IMU里程计门槛，manager明确显示
  `PHASE_WAIT_IMU_ODOMETRY`，不会再显示READY同时又要求停车。
  持续超过转换超时或确认进程故障才进入`FAULT_ALGORITHM`，避免一次短暂调度抖动
  造成永远无法自动退出的粘滞故障。
- **4DoF不校正LIO尺度**：它是刚体变换。若卷尺证明FAST-LIO的直线位移有
  稳定比例误差，必须从时间同步、LiDAR-IMU外参和FAST-LIO估计本身修复，
  不应给4DoF添一个经验尺度因子掩盖。
- **全局z继承组导高程基准**：稳健统计能拒绝野值，不能消除缓慢系统偏差。
  若需测量学意义的绝对高程，需另行标定大地高/正高基准。

### 9.2 静态初始化与实时roll/pitch

- FAST-LIO启动后必须先取得一段连续2秒的静态IMU窗口。逐样本检查陀螺模长、
  加速度模长和时间间隔，整窗检查陀螺均值。上述任一条件失败都会清空整窗重新
  计时，不能把运动前后的两段静态数据拼接起来。逐轴加速度方差仍被统计、发布，
  但默认不作为硬门槛；它强烈依赖IMU型号、安装刚度、振动、
  带宽和采样率。只有完成具体硬件验证后，才通过正值参数显式启用方差硬门槛。
- 静态确认前`/Mower/lio_slam_level`只允许发布`lio_state=false`的无效消息；
  `fusion_4dof`不会把它发布为有效位置。FAST-LIO将同一权威窗口的均值、方差、
  样本数、持续时间和LIO代际锁存发布到`/Mower/lio_static_calibration`；fusion
  不再运行第二套独立静止检测。`android_manager_4dof`在自身启动时发布并锁存
  `/mower/manager_stop_car=true`停车请求，只有`/Mower/imu_odometry/ready=true`且
  连续收到5帧有效融合位置后才解除。manager将它与
  `/mower/fusion_stop_car`做OR并锁存发布底层实际订阅的`/mower/stop_car`；此外，
  底层对`/Mower/position`断流的独立超时停车仍须保留。两条安全链路不能互相替代。
- 正常融合模式的`/Mower/position.roll/pitch`实时输出。默认优先组合导航：根据
  `GpsPosition.msg`的轴定义，将`roll(绕前向轴)`变为ROS roll，将
  `pitch(绕右向轴)`取反后变为ROS pitch，并从0.01度换算为弧度。组合导航姿态
  无效或超过0.30秒时，当帧自动使用重力扶正、外参补偿后的LIO roll/pitch。
  供应商若改变欧拉角轴序/符号，必须实测并修改`ins_*_scale_rad`，禁止原值直传。
- `positionStatus`是1ant在车辆运动时对“位移航向与组导航向一致”的诊断结果，
  不是GPS位置、4DoF启动或roll/pitch有效标志；停车时它可能为0。因此默认
  `ins_attitude_require_position_status=false`，姿态仍受有限值、±45°、0.30秒
  新鲜度以及与重力LIO相差不超过15°的联合门控。只有旧数据源把
  `positionStatus`明确定义为姿态有效位时，才应开启该选项。

#### 9.2a 三种“roll/pitch”不是三个可随意混合的量

1. **INS直接值**：组导自己的GNSS/IMU滤波器估计，是另一套传感器、偏置和
   初始对准结果。它可用于实时对外车体姿态，但必须先把供应商轴、单位、
   符号和INS->base_link安装角统一。当前代码按`GpsPosition.msg`文档做
   `roll=+raw_roll`、`pitch=-raw_pitch`的小角度换算，并检查状态、时效、最大值及
   与LIO的差异；不符合就回退LIO。
2. **LIO静态初始化后的值**：“静态初始化”不是只产生两个固定角。它用
   2秒静态IMU估计重力和陀螺零偏，固定`R_H_W`；之后每帧的
   `R_H_base=R_H_W·R_W_IMU·R_IMU_base`才是实时LIO姿态。它局部连续、
   不依赖GNSS，也是点云及杆臂几何的同源姿态；但会继承FAST-LIO的IMU偏置、
   外参误差与长时漂移。
3. **从最终旋转矩阵提取的值**：这不是第三个传感器观测，而是“最终要
   发布的旋转”的ZYX欧拉角表示。LIO路径中矩阵为
   `R_ENU_base=Rz(theta)·R_H_base`；理想非奇异姿态下，左乘一个纯yaw不改变
   roll/pitch物理值，但重新提取可保证欧拉角分支、角度包装和四元数完全一致。
   INS路径中则先用选中的INS roll/pitch与最终yaw构造四元数，再反提取标准
   RPY；因此`Position.roll/pitch/yaw`、RViz和TF永远表示同一个最终旋转。

恰当用法是：**LIO四元数永远用于FAST-LIO、点云、杆臂和4DoF内部几何；
INS roll/pitch只是可配置的对外姿态源；最后必须通过一个归一化四元数/旋转矩阵
统一发布。**不直接对两套欧拉角做加权平均，也不用INS欧拉角反过来修改LIO点云几何。

---

## 10. 部署与启动（android_manager_4dof 配套）

**本版是整体替换部署，不是与旧版并行的可选项**：旧版 ceres `fast_fusion` 能拿到的
“用 RTK 位置+航向直接求 LIO→ENU”的结果，正是本版 §4.2 判据 4（RTK 航向种子）的
能力子集，已被本版覆盖——本版额外还有滑窗 Umeyama 精化（判据 1/2/3）。故部署时
只启动本版全套，不再启动旧版。

**lidar-slam-v2工作空间内的三个核心catkin包**：
| 包 | 节点名 | 作用 |
|---|---|---|
| `fast_lio` | `/laserMapping` | FAST-LIO、2秒静态初始化、重力水平位姿与初始化状态 |
| `fusion_4dof` | `/fusion_4dof` | 定位融合（本文档描述的算法） |
| `android_manager_4dof` | `/android_manager_4dof` | 安卓指令响应 + 看门狗重启 |

**完整运行还依赖**：

- `libo/1ant`工作空间的`serial_reader`包：解析UM981的DRPVA/IMUATTA/GNGGA，
  以20Hz发布`/nanobot/gpsposition`；其协议依据为`libo/UM981-惯导命令.pdf`。
- 本工作空间的`util`消息包：至少提供`GpsPosition`、`Position`、`LIOPose`
  和`ImuStaticCalibration`。
- `fast_lio`的ROS依赖：`livox_ros_driver2`、PCL/`pcl_ros`、Eigen、ROS消息、
  `tf`；雷达/IMU外参和静态初始化门限来自
  `fast-lio/config/mid360_config.yaml`及对应FAST-LIO源码。
- `fusion_4dof`的系统/库依赖：yaml-cpp、Eigen，以及仓库内
  `fast-lio/Thirdparty/GeographicLib`的头文件和`libGeographic.so`。
- 自动恢复使用本包自带的`scripts/restart_ros_nodes.sh`，调用方式为
  `rosrun fusion_4dof restart_ros_nodes.sh ...`；不再依赖目标机`PATH`中额外拷贝
  的脚本或硬编码工作空间`setup.bash`。重启摘要默认写入
  `${ROS_HOME:-$HOME/.ros}/fusion_4dof_restart.log`，每次事务和新进程输出分别写入
  `fusion_4dof_restarts/restart_<时间>_<PID>/transaction.log`与`process.log`。
  摘要默认1 MiB并保留2份备份，单个进程日志默认8 MiB并保留2份备份，只保留最近
  12个事务；停止/启动窗口还由文件锁串行化，多个事务不会混写同一进程文件。
  脚本会等待旧节点从 master
  注销，并在报告成功前确认所有目标节点持续注册至少1秒、稳定窗两端
  XML-RPC可达；仅成功启动一个后台`roslaunch`进程不再被误报为“重启成功”。
  验证失败时日志会逐节点写入`registered`/`reachable`，用于区分master
  注册缺失与节点已注册但不可达。退出码20/21表示启动或停止失败，22/23表示
  新进程仍存活但注册稳定窗/最终可达性验证未完成；任一非零退出码均保持停车并
  明确进入重启失败，只有脚本成功后才进入generation/readiness验证。LIO进程身份由
  `/Mower/lio_static_calibration`的启动代次标记提前确认，不再等待静态窗口；
  `fast_lio_initialized`仍必须等同代`/Mower/lio_level_transform`到达才为true。
  因而“重启进程已换代”和“LIO已经静态初始化、可输出位姿”是两个独立事实。
- 目标环境是ROS 1 Noetic/catkin；本代码不能直接用ROS 2 Humble编译运行。

旧版`android_manager`不属于本定位组合，不应与`android_manager_4dof`同时运行。

软件包改名后必须让catkin重新生成软件包索引和目标，不能只复用旧`devel`：

```bash
cd /home/nvidia/libo/lidar-slam-v2
source /opt/ros/noetic/setup.bash
catkin_make --force-cmake --pkg util fast_lio fusion_4dof android_manager_4dof

cd /home/nvidia/libo/1ant
source /opt/ros/noetic/setup.bash
source /home/nvidia/libo/lidar-slam-v2/devel/setup.bash
catkin_make --force-cmake --pkg serial_reader
```

部署脚本、systemd/supervisor配置若仍写有`fusion_se2`、`android_manager_se2`或旧
launch名，也必须同步替换；本仓库外的启动配置无法由源码重命名自动更新。

**launch 文件**：
- `location_mode_4dof.launch` / `create_mode_4dof.launch`：定位/建图模式，各自
  `<include>` `laserMapping_only.launch` 拉起 `/laserMapping`，再起 `/fusion_4dof`。
  日常手动启动、或 android_manager_4dof 联合重启（§5.2/§5.3）都用这两个。
- `fusion_only.launch`：只起 `/fusion_4dof`，不碰 `/laserMapping`。供
  android_manager_4dof 的 `LioSlamCallback` 看门狗（诊断为仅 `/fusion_4dof` 卡死）
  使用。其`location_mode`参数直接表示fusion runtime：`false=CREATE_ORIGIN`、
  `true=TRACK_SAVED_ORIGIN`；manager会显式传入，不再拿`MODE_*`间接代替runtime，
  未知runtime也会拒绝重启而不会静默落到定位模式。建图原点已经提交后发生自动
  故障恢复，算法runtime可进入`TRACK_SAVED_ORIGIN`，但统一状态中的
  `workflow_mode`仍保持`WORKFLOW_MAPPING`，不代表用户的边界录制流程已经结束。
  不建议手动单独跑（除非确认`/laserMapping`已在跑，并明确知道应选择哪个runtime）。
- `laserMapping_only.launch`：只起 `/laserMapping`（FAST-LIO 本体），供 RECOVER
  （§5.2）和上面两个 launch 复用，也可单独手动 `roslaunch` 排障用。

**启动示例**：控制器/systemd/supervisor分别自启下述 manager 和建图或
定位 fusion launch，与手动执行效果相同。`android_manager_4dof`必须在线，
因为它是`/Mower/reboot`的执行者：
```bash
# manager（可由控制器单独自启）
roslaunch android_manager_4dof android_4dof.launch
# 二选一：建图或定位（可由控制器单独自启）
roslaunch fusion_4dof create_mode_4dof.launch
roslaunch fusion_4dof location_mode_4dof.launch
```

**android_manager_4dof 的话题参数**（`launch/android_4dof.launch`）：
`android_topic=/signal`、`gps_topic=/nanobot/gpsposition`、
`fusion_topic=/Mower/position`、`reboot_topic=/Mower/reboot`、
`stop_car_topic=/mower/manager_stop_car`、
`fusion_stop_car_topic=/mower/fusion_stop_car`、
`final_stop_car_topic=/mower/stop_car`、
`lio_static_status_topic=/Mower/lio_static_initialized`、
`map_path=.../Map/my_map.mp`。

**重启粒度对应关系**（务必保证 kill 的节点名与目标 launch 内 `<node name>` 一致，
否则 `restart_ros_nodes.sh` 会 kill 不到旧进程——这是本次从"并行隔离原型"整理为
"整体替换部署"时最容易出错的地方，已核对一致）：
| 触发源 | kill 的节点 | 拉起的 launch |
|---|---|---|
| android 切图/切模式/reboot 信号/GPS 低频看门狗（共 5 处） | `/fusion_4dof /laserMapping` | `location_mode_4dof.launch` 或 `create_mode_4dof.launch` |
| `LioSlamCallback`（LIO 健在但位置断流） | `/fusion_4dof` | `fusion_only.launch` |
| `fusion_4dof.cc` 内部 RECOVER（LIO 数据差 / GPS↔LIO 同步失败） | `/laserMapping` | `laserMapping_only.launch` |

---

## 11. RTK 种子 vs SE(2) 锁定 —— 对比功能

**动机**：旧版 ceres `fast_fusion` 能拿到的定位结果，本质就是本版 §4.2 判据 4
（**RTK 航向种子**：`θ = wrap(enu_yaw − lio_yaw)`，单帧计算、不依赖滑窗）的能力。
本版额外做了**滑窗 Umeyama 精化**（判据 1：拐弯后**锁定**，用多点拟合出更准的
θ,tx,ty）。为了验证"滑窗精化相对朴素单帧 RTK 种子到底有多少实际增益"，新增了一条
持续输出的对比通道——`GlobalOptimization::GetSeedOnlyTransform()`。

**与官方 `GetTransform()` 的区别**（务必分清，二者数值来源不同）：
| | `GetTransform()`（官方，生产用） | `GetSeedOnlyTransform()`（新增，仅供对比） |
|---|---|---|
| 数据来源 | 滑窗内全部配对点（Umeyama 拟合，锁定后冻结） | 最近一组通过门控的同时GPS/LIO配对 |
| 是否受"锁定/播种"状态机影响 | 是（§4.2 判据 1-5） | 否——每次调用都重算，与 `aligned_`/`locked_` 无关 |
| 平移 (tx,ty) 算法 | 窗口质心对齐 | `enu_position − R(θ)·lio_position`（单点反解） |
| 何时可用 | `IsAligned()` 为真后 | 收到过至少一帧 GPS + 一帧 LIO 且航向可信后（更早可用） |
| 是否写生产状态 | 是（`theta_/tx_/ty_/aligned_/locked_`） | 否，纯只读，零副作用 |

**如何对比**（三种手段，从直观到量化）：
1. **RViz 视觉对比（最直观）**：Fixed Frame 设为 `map`，同时添加
   `/Mower/viz/lio/path`（官方，播种/锁定后更精确）和 `/Mower/viz/rtk_seed/path`
   （纯种子，恒定单帧）两条 Path，再叠加 `/Mower/viz/gps/path`（RTK 原始轨迹）
   做参照。**预期现象**：车未拐弯（仍是"播种"档）时，两条线基本重合，因为此时
   `GetTransform()` 本身也在用判据 4 的种子公式；**车拐过弯、判据 1 命中"锁定"后**，
   两条线开始分叉——分叉幅度就是滑窗精化相对朴素单帧方案的实际增益；若长期几乎
   不分叉，说明本次场景下滑窗精化收益有限（例如场地本就平直、GPS 噪声本就很小）。
2. **数值对比（可量化）**：`rostopic echo /Mower/debug/lio` 与
   `rostopic echo /Mower/debug/rtk_seed`（或用 `rqt_plot` 同时画两者的
   `position_x`/`position_y`/`yaw`），直接读数比较；也可以自行写一个简单脚本订阅
   两个话题、按时间戳最近邻配对后算 Δx/Δy/Δyaw，取一段行驶里程的 RMS 作为"滑窗
   精化增益"的量化指标——本包目前只提供两条并列话题，不内置这个差值计算，需要的话
   按上面的话题接口自行实现即可。**离线看纯文本日志（无需 bag/实时话题）时**，
   `[4DOF-TUNE]` 这一行本身已经同时打印 `tx/ty`（窗口化，官方 `GetTransform`）与
   `seed_tx/seed_ty`（瞬时，`GetSeedOnlyTransform`）——两者逐行相减就是"窗口滞后
   量"的时间序列，是诊断 §12 提到的 LIO 尺度误差最直接的数据来源。
3. **`/Mower/position` 的 `position_state`（§8）本身也是弱信号**：`state=5`（仅
   播种未锁定）时官方输出就约等于 `rtk_seed`；`state=2`（已锁定）时官方输出已经
   走精化路径，此时再看 §5.3 提到的 `[4DOF] fallback in use but NOT locked yet`
   告警是否还在打印，可以快速判断当前是否仍处于"两者理论上应重合"的阶段。

**何时该担心（而不是当作正常噪声忽略）**：锁定（`state=2`）之后，若 `lio` 与
`rtk_seed` 两条轨迹**持续、系统性地**分叉超过量级（不是偶发抖动），且方向/幅度
不随时间收敛，参考 §9 已知限制与 §2 的航向约定核对——常见原因是杆臂标定误差、
GPS 多径、或 §2 提到的航向符号约定问题（§4.2b 的 `res_rms`/`outlier_ratio`
告警可以帮助排除"数据本身不自洽"这一类原因，但不覆盖航向符号问题）。

---

## 12. LIO 尺度误差现场验证方法

**背景**：本版 SE(2) 是纯刚体变换 `p_enu = R(θ)·p_lio + (tx,ty)`——只有旋转+平移
两个自由度，**没有尺度自由度**。如果 FAST-LIO 自身对"走了多远"的估计存在系统性
比例偏差（哪怕只有百分之一二），SE(2) 无论怎么调参都补不回来，只会表现为：车走得
越远、`/Mower/debug/lio`（本版官方输出）相对 `/Mower/debug/gps`（RTK参考，
不默认为真值）在
**前进方向**上越落后，且这个落后量在窗口刷新前会持续累积、刷新后又回落一部分——
而横向（垂直于前进方向）偏差通常小得多，因为纯尺度误差不怎么投影到横向。这跟"杆臂
标定错误"（表现为固定不变的偏移，不随行驶距离变化）或"航向符号约定错误"（§2，
表现为方向感全错）是三种不同的病因，排查方法也不同，不要混着调。

判断是否存在这个问题、以及问题出在哪一层，按下面顺序验证：

### 第一步：卷尺直线测距（判断"是不是尺度误差、误差多大"）
1. 找一段**平整、开阔、RTK 信号良好**的地面，用卷尺量出一段精确直线距离
   （建议 ≥10m，越长测量误差占比越小）。
2. 让车沿这条线**匀速直行**（不要转弯），开始/结束位置做好标记。
3. 对比 `/Mower/debug/lio` 在起止两个标记点之间报告的位移，与卷尺量的真实距离：
   `尺度误差% = (LIO报告距离 - 真实距离) / 真实距离 × 100%`。
4. 同一测试建议重复 2-3 次（正反方向各跑一次更好，能顺带排除"车头/车尾杆臂
   方向搞反"这类问题），看误差是否稳定重复——稳定的百分比误差指向真的尺度问题；
   如果每次误差忽大忽小甚至变号，更可能是随机噪声或个别帧异常，而不是系统性尺度。

### 第二步：切换 IMU-LiDAR 外参标定（判断问题是否出在 FAST-LIO 这一层）
`fast-lio/config/mid360_config.yaml` 里目前生效的 `extrinsic_R` 是单位阵（零旋转），
但文件里还留着一组被注释掉、带真实小角度旋转的标定值（见下）：
```yaml
# 当前生效：
extrinsic_T: [ -0.011, -0.02329, 0.04412 ]
extrinsic_R: [ 1, 0, 0,  0, 1, 0,  0, 0, 1 ]
# 被注释、未生效：
# extrinsic_T: [ -0.006769, -0.020180,  0.037570]
# extrinsic_R: [ 0.999996,  0.002177,  0.001933,
#               -0.002171,  0.999993, -0.003154,
#               -0.001940,  0.003150,  0.999993]
```
1. 备份当前 `mid360_config.yaml`。
2. 把注释掉的那组 `extrinsic_T`/`extrinsic_R` 换成生效值（当前生效的那组反过来注释掉）。
3. 重复第一步的直线测距测试，对比两组外参下的尺度误差百分比是否有明显变化。
4. 若换成"实测旋转"那组后误差明显变小 → 说明当前用的单位阵外参确实不够准，
   应该固定使用那组实测值；若两组差别不大 → 问题不在这一层，继续下一步。

### 第三步：重新做 LI-Init 外参标定（如果第二步两组都不准）
如果第一步确认有稳定的尺度误差、第二步两组现有外参都没能显著改善，说明现有的两组
标定数据本身可能都已经不准（比如硬件被拆装过、IMU/雷达相对位置变过），需要用
FAST-LIO 官方推荐的 LI-Init 标定流程重新采集一组外参，而不是继续在这两组老数据里
挑。标定完成后重复第一步验证。

**验证完成后**：把确认的尺度误差百分比、用的是哪一组外参告诉我，我再决定是从
FAST-LIO 标定层面还是从 fusion_4dof 的窗口策略（§13）层面继续处理——现有 SE(2)
架构下，尺度误差只能从源头（FAST-LIO 标定）根治，`se2_*` 参数最多只能缩小暴露量、
不能消除。

---

## 13. 牛耕式行距与 `se2_lock_cross_std` 的耦合

**现象**：割草机牛耕式作业，调转车头前后两条直线的间距（行距）经确认为 **0.4m**
（早先估计的 0.25m 不准确，已更正），而 `se2_lock_cross_std` 原值是 **0.35m**。

**推导**：`cross_std` 是当前滑窗内所有配对点在"垂直于行进方向"上的标准差。牛耕式
相邻两行近似反向平行、横向相距行距 `d`。若窗口内同时包含两行的点（数量占比
`w1`、`w2`，`w1+w2=1`），横向标准差的理论表达式是 `std = sqrt(w1·w2)·d`，
在 `w1=w2=0.5`（窗口对两行各占一半，最有利的情形）时取最大值 **`std_max = d/2`**。
代入 `d=0.4m` → **`std_max = 0.2m`**。

**用 07062.log 反推验证**（该日志录制时行距就是这 0.4m，无需重新采集；按航向
反转>120°识别出 46 段疑似转弯，剔除 6 段发生在运行前 931 秒、峰值<0.05m 的噪声
误判后，剩 40 段真实转弯）：
- **常规行距转弯**（29 段，峰值中位数 0.190m，**众数 0.135m（46 段里出现 13 次）**）
  ——中位数 0.190m、众数 0.135m 都**低于**上面推导的 0.2m 理论上限（分别为上限
  的 95%、67.5%），符合"真实转弯是连续弧线过渡、很难恰好做到窗口两行各占一半"
  的预期，推导自洽（用 0.25m 算的话中位数反而会超过理论上限，数字对不上，这次
  确认 0.4m 后才讲得通）。这一档**从未**摸到 0.35m 的门槛。
- **少数更大半径的转弯**（11 段，峰值 0.360-1.454m，中位数 0.498m）——大概率是
  地块边界/转角处半径更大的转向，不受 `d=0.4m` 这个简单两行模型约束，**这一档
  能摸到并超过 0.35m**。
- **`locked` 的真实历史（已修正我更早一轮的误判）**：全程只在 t+936.9s 发生**一次**
  `0→1` 跳变，此后一直保持 `locked=1` 到日志结束（占全程 64.2% 的时间），期间
  `cross_std` 后续再怎么回落也不会导致重新变回 `unlocked`（`locked_` 只在整体
  RESET 时才清零，见 §4.2/global_fusion.h 状态机说明）——所以"锁不上"不是永久性的，
  而是**要等到第一次够格的大转弯**（这次日志里等到了运行后 15.6 分钟），在那之前
  全程只能用较低精度的"播种"档（判据 3/4）。**这也解释了你最早反馈的"前进方向
  滞后"现象为什么会出现**：滞后的根源（§12，SE(2) 无尺度自由度）不需要"未锁定"
  也会发生，但未锁定的这 15.6 分钟里精度本就更低，两个问题会叠加。

**已应用的调整**：`config/fusion_4dof_params.yaml` 的 `se2_lock_cross_std` 已从
0.35 改为 **0.10**——低于常规转弯众数 0.135m 留出约 26% 安全边际，同时明显高于
直线段噪声地板（运行前 931 秒静默期 `cross_std` 在 0.003-0.04m，留有 2.5 倍以上
余量）。这样**常规行距转弯**（而不只是偶尔出现的大转弯）就能可靠触发/巩固锁定
判据，预期能把"首次锁定要等 15.6 分钟"的窗口大幅缩短，且此后每次正常掉头大概率
都会重新刷新一次 θ 的估计（而不是十几分钟才碰上一次），见 §11 相关说明。这个值
目前**还没有上车验证**，如果实测后发现锁定过于频繁抖动或者仍然锁不上，反馈实测
日志（含新的 `cross_std`/`locked` 序列）我再帮忙复核。

**若 0.10 之后仍不够用的备选杠杆**（备用，不建议在没有新证据前先动）：
1. **缩短 `se2_window_span_sec`**：当前 45s（实测受 `se2_max_window=400` 限制，
   真实窗口时间跨度更短，具体多少可以从 `window_n` 从 0 涨到 400 所用的时间反推）。
   窗口越短，转弯的点在窗口里占比越高、越接近 `d/2` 的理论上限（受益于"稀释效应
   变小"）；代价是整体估计噪声变大、更容易被单次 GPS 抖动带偏，与 README 一贯
   "精度优先"的取舍方向相反，需要你自己权衡是否接受。
2. **加大行距 `d`（治本，但可能受限于割草幅宽/覆盖率要求）**：`std_max = d/2`，
   行距越大理论上限越高、越容易锁定；但行距通常由割草刀盘宽度和覆盖率（留白/重叠）
   要求决定，能不能改、改多少不是 fusion_4dof 这层能替你决定的，需要结合机械/覆盖率
   约束权衡。

**`se2_lock_min_samples`(15) / `se2_lock_confirm_frames`(3) 不应为追求快速锁定而放宽**：
正常GPS频率下转弯窗口内的配对点数通常远超过15；三次确认现在还分别要求至少1s和
0.30m的新证据，因此最快也需要约2s及0.60m有效运动。这是刻意的独立性约束，不再把
三个相邻重叠窗口当成三次确认。若锁定困难，先核对 `cross_std`、同步、轨迹形状和
RTK创新拒绝日志，不应直接降低独立性要求。

---

## 14. 4DoF实测日志采集

锁定默认门限、RTK 1/2/3/4帧缺失回归、以及如何通过
`/Mower/localization_status`、`/Mower/localization_algorithm_status`、
`/Mower/position`和`/rosout`确认当前定位状态，见
[`docs/LOCK_OUTAGE_TEST_AND_STATUS_GUIDE.md`](docs/LOCK_OUTAGE_TEST_AND_STATUS_GUIDE.md)。

默认5Hz输出4类可按`stamp`直接配对的机器可解析记录：

- `[LIO4D-FAST]`：FAST-LIO原始W系与重力水平H系位姿，相对第一个有效输出的
  `dxyz/XY位移/3D位移`，以及重力、陀螺/加表偏置、速度、匹配点数和残差。
- `[4DOF-SYNC]`：同步时差、`lio_slam_level`原始IMU位姿、轴变换/杆臂补偿后
  H系base_link位姿、GPS经纬高/ENU、状态和INS/LIO姿态差。
- `[4DOF-INS]`：组导原始加速度、角速度、ENU速度、星数、置信度、对准状态和
  位置/航向/高程标准差。
- `[4DOF-STATE]`：`theta/tx/ty/tz`、锁定/竖直对齐状态、XY残差/异常率、
  z内点/标准差、全局LIO位姿、最终输出和实际姿态源。

建议同时保完整launch终端日志和bag：

```bash
roslaunch fusion_4dof location_mode_4dof.launch

rosbag record -O localization_4d.bag \
  /Mower/position /Mower/debug/gps /Mower/debug/lio /Mower/fusion \
  /Mower/lio_slam /Mower/lio_slam_level /Mower/lio_level_transform \
  /Mower/lio_static_initialized /Mower/lio_static_calibration \
  /nanobot/gpsposition /livox/imu /Mower/imu_odometry/ready \
  /Mower/imu_odometry/status /Mower/imu_odometry/fallback \
  /Mower/imu_odometry/test
```

不要用一个长期`tee`文件跨多次看门狗重启收集所有子进程输出。应复制ROS的
分节点日志、`rosout.log`以及脚本的分事务日志：

```bash
cp ~/.ros/log/latest/rosout.log localization_4d_rosout.log
cp -a ~/.ros/fusion_4dof_restarts localization_restart_transactions
```

交付时保留完整文件，不要只截告警行。若文件太大，可另外提取诊断行，但仍
建议保留原文件：

```bash
grep -E '\[LIO4D-FAST\]|\[4DOF-(SYNC|INS|STATE)\]' \
  localization_4d.log > localization_4d_diagnostic.log
```

一次完整实测建议包含：

1. 启动后保持完全静止至少5秒，记录静态初始化、4DoF建立时间。
2. 卷尺量出不少于10m直线；起点/终点各停3至5秒，正反向各走1次。
3. 完成至少2个牛耕式掉头，让`cross_std/locked`充分变化。
4. 在安全可控条件下测试GPS异常、LIO回退和GPS恢复，记录人工操作时刻。
5. 测一次仅重启`fusion_4dof`，再测一次FAST-LIO联合重启，用于验证代际恢复。

同时提供卷尺真值、直线起止时刻、地面是否有坡度、GPS遮挡/重启操作时刻和
当次使用的YAML/launch，才能区分LIO尺度、时间同步、GPS系统偏差、外参误差和
4DoF窗口滞后。

---

## 15. MID360 IMU受限积分里程计

### 15.1 能力边界

现场没有车轮反馈，本功能只使用MID360的三轴角速度和线加速度做捷联积分。
MID360 IMU同时也是FAST-LIO输入，因此它不是与LIO完全独立的第三定位源；
纯IMU位置又是加速度的二次积分，对零偏、姿态误差和振动非常敏感。

`/livox/imu` 的MID360线加速度原始单位是 `g`，原话题保持不变；
FAST-LIO和fusion分别在内部乘以 `imu_acc_scale` / `mid360_imu_acc_scale`
（MID360配置均为9.81）后按 `m/s²` 处理。角速度原本就是 `rad/s`，不做换算。
本功能只声称：

- 在RTK和LIO同时不可用，或主定位输出超时后，在已锁存停车的前提下提供
  有时间和距离上界的连续位姿；
- 不参与正常阶段的LIO质量判决，也不改变RTK/LIO选择；
- 不能覆盖MID360 IMU同时污染LIO和兜底积分的共因故障。

### 15.2 启动标定与两套积分器

FAST-LIO使用唯一的权威静止窗口：默认连续至少2秒、至少50帧，每帧角速度
范数不超过0.20rad/s、整窗平均不超过0.08rad/s，加速度范数在
8.0～11.5m/s²内。定位生命周期在此期间已经锁存停车，因此这些门限只负责排除
明显运动。默认`imu_init_max_acc_variance=0`，逐轴加速度方差仅作为诊断量和初始化
协方差，不再用一个跨设备不稳定的固定值阻塞启动；若针对特定IMU和安装完成了多车、
多温度及振动工况验证，可配置正值重新启用该硬门槛。确认后将原始IMU坐标系下的
均值/方差、样本数、持续时间、末帧时间和进程代际锁存发布到
`/Mower/lio_static_calibration`。

fusion只接受与`/Mower/lio_level_transform`代际完全相同、字段有限且方差非负的
统计消息；跨话题无论谁先到都先缓存，代际匹配后才应用。均值先通过
`lio_to_base_rotation`转换到`base_link`，第一个可信状态1/2/5到达后结合其
roll/pitch计算加速度零偏并取得全局位置/姿态锚点。fusion不再自行累计另一个
2秒窗口。

内部使用两套互不影响的实例：

- 生产兜底实例：正常状态下每次可信主定位都重锚，故障切换时从最新状态继续；
- 启动对比实例：仅当`imu_odom_test_from_startup=true`启用。接收FAST-LIO权威
  静止统计并取得首个可信主定位后只锚定一次，此后不再被正常定位校正，
  持续发布`/Mower/imu_odometry/test`供误差对比。

“从启动时刻积分”是指从启动权威静止窗口开始建立偏置基准；在首个可信全局位姿之前，
IMU本身无法给出可与地图坐标直接比较的全局位置，因此对比轨迹从该首个锚点开始发布。

### 15.3 受限回退合同

主定位超过`imu_odom_main_pose_timeout_sec`未刷新，并且RTK和LIO两个独立源
在收据侧都已不可用时，只有在“静止标定完成 + 存在最后一个可信生产位姿锚点 + MID360
IMU新鲜且积分连续”全部成立时才进入回退：

1. 立即发布并锁存`/mower/fusion_stop_car=true`；
2. `/Mower/position.position_state=6`，下游继续只把1/2/5当作作业有效；
3. 对MID360三轴角速度积分姿态，对去零偏比力旋转到ENU、扣除重力后进行
   速度和位置积分；
4. 默认最多10.0s且最多3.0m；任一数据过期、时间倒退、积分断裂、速度异常或超界，
   立即发布状态9并请求联合重启；
5. GPS或LIO任一源先恢复时立即停止state 6输出，等待生产状态1/2/5
   重新发布并重锚。

默认主定位超时为10.0秒。LIO-only重启会进入独立的受控恢复状态，在
“新generation + 静态初始化 + 4DoF + 首帧生产位姿”完成前始终停车，
但禁止进入state 6，也禁止manager的通用位置看门狗级联恢复。该恢复有
独立默认60秒硬上限；超时才证明恢复未收敛并请求full recovery。

`/Mower/imu_odometry/status`为`UInt8`：0=关闭或未就绪，1=生产积分器就绪，
2=受限回退中，3=里程计失效/超界且已请求联合重启。
`/Mower/imu_odometry/ready`为锁存启动握手：只有同代际权威统计已应用且首个
可信生产姿态已完成偏置/锚点初始化才为true；manager默认要求它为true才放车。
`/Mower/imu_odometry/fallback`只用于观察受限轨迹；状态6始终是停车态，
不能被下游当作继续作业的有效定位。
