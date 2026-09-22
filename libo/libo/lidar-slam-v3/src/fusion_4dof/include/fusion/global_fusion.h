#ifndef GLOBAL_FUSION_H
#define GLOBAL_FUSION_H

// =============================================================================
// global_fusion.h  —— LIO -> map(ENU/高斯) 的轻量在线对齐估计器
//
//   设计变更（取代原 ceres 位姿图 GlobalOptimization）：
//     原实现是 VINS-Fusion globalOpt 的位姿图：每来一帧 GPS 就把至多上千个节点
//     的整图重建并 ceres 求解，唯一对外产物却只是“最新 LIO 位姿经 WGPS_T_WVIO
//     映射到 map 的 (x,y)”——本质是一个刚体对齐。重武器、轻目的。
//
//   本实现：水平面使用滑窗 SE(2)，竖直方向使用稳健高程偏移，组成
//   重力约束的4DoF对齐 (yaw, tx, ty, tz)：
//     - InputGPS 直接接收上层 GPS2Local 输出的 ENU 位置，与 LIO（已做杆臂
//       补偿、但不按fusion首帧重锚的 H系 base_link 位置）时间同步配对，
//       水平用闭式 Umeyama 估计 R(theta)、t，竖直使用稳健dz估计tz，
//       使 p_enu ≈ R(theta)·p_lio + t；估计器自身不再持有 GeographicLib/原点，
//       从而 GPS 输出帧、SE(2) 回退帧、调试帧严格同一 ENU（无 create 模式跳变）；
//     - GPS 失效/恢复确认时 InputGPS 仅缓存候选配对而不提交，(theta,t) 冻结；
//       候选连续达标后才原子重估，InputOdom 始终按已提交变换输出 LIO 回退位姿；
//     - 旋转的可观测性按点云 2D 展布判定：沿迹向足够→“已播种(aligned)”，
//       横迹向足够(经历拐弯)→“已锁定(locked)”；近直线段不更新已锁定的旋转，
//       避免直线退化把 theta 拉偏。
//
//   坐标系：当前实现已经使用纯ENU。该变更不兼容旧ceres旋转地图系，地图/边界
//   必须用本版重新记录；下游仍订阅原话题，但坐标语义必须按East/North理解。
//
//   线程：原版用后台线程 + 条件变量。实际部署中 InputOdom/InputGPS 均在
//   LioCallBack（单线程 spinner）内顺序调用，故本实现改为同步、无后台线程；
//   仍保留一把轻量互斥锁做防御（若上层改用多线程 spinner 亦安全）。
// =============================================================================

#include <array>
#include <cmath>
#include <deque>
#include <iostream>
#include <map>
#include <mutex>
#include <vector>

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>

class GlobalOptimization
{
public:
    GlobalOptimization();
    ~GlobalOptimization();

    // 对外接口 ----------------------------------------------------------------
    // InputGPS 直接接收【已换算到 ENU 的平面位置】(enu_x, enu_y) 与【ENU 航向】
    // enu_yaw（从 East 起、逆时针），与上层 GPS2Local 输出同一帧 —— 估计器不再
    // 自带 GeographicLib/原点，彻底消除“双 converter 不同原点”导致的 create 模式
    // 切换跳变，并保证 GPS 输出帧与 SE(2) 回退帧严格一致。
    // yaw_valid：本帧 RTK 航向是否可信（由上层按速度门控）。SE(2) 尚未基于轨迹
    // 估出旋转前，用 (enu_yaw - lio_yaw) 作冷启动种子直接定 LIO->ENU 旋转。
    // commit_alignment=false 时只缓存 GPS<->LIO 配对，不修改当前生产 SE(2)。
    // 用于 RTK 失效后的恢复确认期：候选样本达到上层稳定门限后，再由最后一帧
    // commit_alignment=true 原子重估并提交，避免单帧恢复 GPS 改写状态 2 的输出。
    // Returns true only when the sample was time matched and passed the
    // innovation gate against the last committed transform. Rejected samples
    // are not allowed to update the seed metadata or enter the fitting window.
    bool InputGPS(double t, double enu_x, double enu_y, double enu_z,
                  double enu_yaw, bool yaw_valid, bool z_valid,
                  bool commit_alignment = true);
    void InputOdom(double t, Eigen::Vector3d OdomP, Eigen::Quaterniond OdomQ);
    void GetGlobalOdom(Eigen::Vector3d &odomP, Eigen::Quaterniond &odomQ);

    // 对齐健康标志（供 fusion 状态机使用）---------------------------------------
    bool IsAligned() const; // SE(2) 已播种，回退位姿可用（沿迹向展布达标）
    bool IsLocked() const;  // 旋转已被 2D 展布良好观测（经历过拐弯）
    bool Is4DAligned() const; // 水平SE(2)与tz均已提交
    bool IsVerticalAligned() const;

    // 当前 LIO->ENU 的 SE(2)：p_enu = R(theta)·p_lio + (tx,ty)。返回是否已对齐。
    // 供上层把“调试 LIO 位姿”用与生产回退完全相同的变换映射到 ENU，保持一致。
    bool GetTransform(double &theta, double &tx, double &ty) const;
    bool GetTransform4D(double &theta, double &tx, double &ty,
                        double &tz) const;

    // 纯 RTK 位置+航向种子变换（不经滑窗/锁定，每帧用最近一次 GPS/LIO 单点重算）：
    // theta = wrap(enu_yaw - lio_yaw)，(tx,ty) 由该 theta 反解单点平移。与 SE(2)
    // 滑窗/锁定状态完全独立、纯只读、无副作用——供上层持续对比“仅用 RTK 位置+
    // 航向做 LIO->ENU”与官方 GetTransform()（播种/锁定后更精确）之间的差异，
    // 用于验证 SE(2) 滑窗估计相对朴素单帧方案的实际增益。返回是否已有足够数据
    // （至少一帧 GPS 与一帧 LIO 且航向可信）计算该变换。
    bool GetSeedOnlyTransform(double &theta, double &tx, double &ty) const;
    bool GetSeedOnlyTransform4D(double &theta, double &tx, double &ty,
                                double &tz) const;

    // 运行时参数注入（由上层从 launch 读入后调用，便于实地免重编译调参）。
    // max_window：配对窗口最大长度(帧)；window_span_sec：窗口最大时间跨度(s)；
    // match_tol_sec：GPS↔LIO 配对时间容差(s)；seed_along_std/lock_cross_std：
    // 判“已播种/已锁定”所需的沿/横迹向标准差(m)。非法值（<=0）将被忽略。
    void SetParams(int max_window, double window_span_sec, double match_tol_sec,
                   double seed_along_std, double lock_cross_std);
    // Bounded LIO interpolation pair and forward/backward extrapolation.
    void SetTimeAlignmentParams(double max_pair_dt_sec,
                                double max_extrapolation_sec);

    // “锁定”稳健性参数（防止小样本/单帧异常点误锁）与异常值鲁棒拟合参数：
    // lock_min_samples：判“锁定”前至少需要的鲁棒内点数；
    // lock_confirm_frames：需要的时间/位移独立验证组数（非相邻回调计数）；
    // outlier_residual_thresh：鲁棒重拟合中判“异常点”的残差阈值(m)。非法值（<=0/<2）忽略。
    void SetRobustParams(int lock_min_samples, int lock_confirm_frames,
                         double outlier_residual_thresh);
    void SetLockGeometryParams(double min_inlier_ratio,
                               double min_along_std_m,
                               double max_condition_number,
                               double min_accumulated_turn_rad);
    void SetLockIndependenceParams(double min_interval_sec,
                                   double min_displacement_m);

    // Pre-window RTK innovation limits. Once a committed transform exists,
    // compare the synchronized RTK observation with T_committed(LIO) before
    // it can contaminate the window. Locked/recovery operation deliberately
    // uses the tighter horizontal bound.
    void SetInnovationParams(double unlocked_xy_m, double locked_xy_m,
                             double yaw_rad, double z_m);

    // 竖直对齐：tz_i=GPS_ENU_z-LIO_H_z。仅当有效内点数、内点占比
    // 和标准差均达标时提交；否则冻结上次tz，不让GPS高程野值改写输出。
    void SetVerticalParams(int min_samples, double max_std,
                           double outlier_threshold,
                           double max_abs_offset);

    // 仅用于fusion_only同一LIO代际恢复。代际/地图校验在上层完成。
    bool RestoreTransform4D(double theta, double tx, double ty, double tz,
                            bool horizontal_locked);

    // LIO 帧代际变化（/laserMapping 重启）后的外科手术式局部复位。
    // 清空 LIO↔ENU 配对窗口与对齐状态（估计器已无 GPS 原点可保留）。
    void ResetLioCoupled();

    // 地图原点/身份切换专用复位。在 ResetLioCoupled 的基础上，
    // 还必须清除旧 ENU 下的最后 GPS 位置/航向/高程种子，防止
    // GetSeedOnlyTransform 在新 GPS 到来前混用新旧坐标系。
    void ResetForMapSwitch();

    // 清空 GPS<->LIO 配对窗口和候选 GPS 元数据，但保留当前已提交的 SE(2)
    // 及最近 LIO。用于进入/中断 RTK 恢复确认时丢弃不连续候选样本。
    void ClearGpsWindowKeepTransform();

    // 请求整体清空（等价于原 clear_data=true）：加锁写入，于下次 InputOdom/InputGPS
    // 内实际执行清空。取代原公开裸 bool 成员，避免未来若改多线程 spinner 时的数据竞争。
    void RequestClear();

    // 拟合质量自检（供上层记录/告警；多数异常同时作为本次提交否决门）：
    // GetFitResidualRms：最近一次成功拟合的（鲁棒剔除后）内点残差 RMS(m)，
    //   持续偏大提示 GPS 多径/时间同步/杆臂标定等存在系统性问题；
    // GetOutlierRatio：最近一次拟合中被判为异常值剔除的点占比 [0,1]。
    // 单纯阈值不改变 aligned_/locked_；但内点不占多数时 UpdateAlignment_ 会拒绝提交。
    double GetFitResidualRms() const;
    double GetOutlierRatio() const;
    size_t GetHorizontalInlierCount() const;
    double GetLockConditionNumber() const;
    // 历史名称为 AccumulatedTurn；实际返回解缠航向的覆盖范围，
    // 避免将每帧小抖动累加成虚假转弯。
    double GetAccumulatedTurnRad() const;
    double GetVerticalStd() const;
    double GetVerticalOutlierRatio() const;
    size_t GetVerticalInlierCount() const;

    // 调参诊断量（供上层周期性打印，便于实地测试后离线复盘参数是否合适）：
    // GetAlongStd/GetCrossStd：最近一次重估用的沿/横迹向标准差(m)，与
    //   se2_seed_along_std/se2_lock_cross_std 对照即可判断阈值是否合适；
    // GetWindowSize：当前配对窗口内的点数，与 se2_lock_min_samples 对照。
    double GetAlongStd() const;
    double GetCrossStd() const;
    size_t GetWindowSize() const;

private:
    void ResetLioCoupledUnlocked_();
    void ClearLastGpsSeedUnlocked_();
    void UpdateAlignment_();       // 滑窗 Umeyama 估计 (theta_, tx_, ty_)
    void UpdateVerticalAlignment_(); // 稳健估计 tz_
    void RemapLastGlobal_();        // 用当前 SE(2) 重新映射最近一帧 LIO -> 输出
    void ApplyClearIfRequested_();  // 处理 clear_data_（整体清空）

    // 单条时间同步的配对：同一时刻的 (LIO 平面位置, map 平面位置)
    struct MatchPair
    {
        double t;
        double lx, ly, lz, lyaw; // LIO H系base_link位置和航向
        double mx, my, mz; // map/ENU系位置
        bool z_valid;
    };
    std::deque<MatchPair> window_;

    // 对一组配对点做一次闭式 2D Umeyama 拟合：质心、沿/横迹标准差、旋转角。
    // 私有静态方法（不依赖实例状态），供 UpdateAlignment_ 对“全窗口”与“剔除
    // 异常值后的内点”各调用一次，逻辑不重复。
    static void FitUmeyama2D_(const std::deque<MatchPair> &pts, double &cx_l, double &cy_l,
                              double &cx_m, double &cy_m, double &along_std,
                              double &cross_std, double &theta_out);

    // 最近的 LIO 位姿缓存（按时间，用于与 GPS 做最近邻时间配对）
    // x, y, z, yaw. Keeping yaw at the synchronized timestamp avoids comparing
    // a delayed RTK heading with the newest, rather than matched, LIO attitude.
    std::map<double, std::array<double, 4>> lioPoseMap_;
    Eigen::Vector3d lastLioP_;
    Eigen::Quaterniond lastLioQ_;
    bool hasLastLio_;

    // The LIO pose actually paired with the last accepted RTK sample.  Seed
    // calculations must use this pose, not the newest callback pose, otherwise
    // normal GPS transport latency is converted into a false yaw/translation.
    Eigen::Vector3d lastMatchedLioP_;
    bool hasLastMatchedLio_;

    // RTK 航向冷启动种子：SE(2) 未锁定且沿迹基线不足时，用节点启动后的 RTK 航向
    // 直接给出 LIO->ENU 旋转（短时短距精度足够，避免冷启动空窗 FAULT）。
    double last_enu_yaw_;  // 最近一帧 GPS 的 ENU 航向
    double last_lio_yaw_;  // 与最近一帧已接受RTK同时匹配的LIO航向
    bool last_yaw_valid_;  // 最近一帧 RTK 航向是否通过上层真实质量/派生航向门控

    // 供 GetSeedOnlyTransform 用的最近一帧已通过时间与创新门控的 GPS 平面位置，
    // 与 last_enu_yaw_、lastMatchedLioP_ 属于同一个配对。
    double last_enu_x_;   // 最近一帧 GPS 的 ENU x
    double last_enu_y_;   // 最近一帧 GPS 的 ENU y
    double last_enu_z_;
    bool last_z_valid_;
    bool has_last_gps_;   // 是否已收到过至少一帧 GPS（与 last_yaw_valid_ 语义不同：
                          // 后者只表示"航向"是否可信，前者表示"位置"是否已有值）

    // 当前 LIO->map 的 SE(2)：p_map = R(theta_)·p_lio + (tx_, ty_)
    double theta_;
    double tx_, ty_;
    double tz_;
    bool aligned_;
    bool locked_;
    bool vertical_aligned_;

    // “锁定”确认计数：cross_std（鲁棒剔除后）达标且时间/位移相互独立的
    // 观测组数，达 lock_confirm_frames_ 才提交锁定。
    int lock_confirm_cnt_;
    double last_lock_confirmation_t_;
    double last_lock_confirmation_lx_;
    double last_lock_confirmation_ly_;
    bool pending_lock_transform_valid_;
    double pending_lock_theta_;
    double pending_lock_tx_;
    double pending_lock_ty_;

    // 拟合质量自检输出（由 UpdateAlignment_ 每次成功配对后刷新）
    double last_fit_residual_rms_; // 鲁棒重拟合内点残差 RMS(m)；未评估时为 0
    double last_outlier_ratio_;    // 本次拟合中异常值占比 [0,1]；未评估时为 0
    double last_vertical_std_;
    double last_vertical_outlier_ratio_;
    size_t last_vertical_inlier_count_;

    // 调参诊断量（由 UpdateAlignment_ 每次重估后刷新，不参与门控逻辑）
    double last_along_std_; // 最近一次沿迹向标准差(m)
    double last_cross_std_; // 最近一次横迹向标准差(m)
    size_t last_window_n_;  // 最近一次重估时的窗口点数
    size_t last_horizontal_inlier_count_;
    double last_lock_condition_number_;
    double last_accumulated_turn_rad_;

    // 输出缓存（lastLioP_ 经当前 SE(2) 映射）
    Eigen::Vector3d lastGlobalP_;
    Eigen::Quaterniond lastGlobalQ_;

    mutable std::mutex mtx_;

    // 置位后于下次 InputOdom/InputGPS 内整体清空；仅内部访问，外部经 RequestClear()
    // 加锁写入（原为公开裸 bool，未来若改多线程 spinner 将成为唯一未受 mtx_ 保护的
    // 成员，故收敛为私有 + 加锁 setter）。
    bool clear_data_;

    // 参数（构造函数内给定默认值）---------------------------------------------
    size_t max_window_;        // 配对窗口最大长度
    double max_window_span_;   // 配对窗口最大时间跨度(s)
    double match_time_tol_;    // GPS↔LIO 配对的时间容差(s)
    double max_lio_pair_dt_sec_;
    double max_lio_extrapolation_sec_;
    double seed_along_std_;    // 判“已播种”所需的沿迹向标准差(m)
    double lock_cross_std_;    // 判“已锁定”所需的横迹向标准差(m)
    size_t lio_cache_max_;     // lioPoseMap_ 最大容量

    // 锁定稳健性 + 鲁棒拟合参数（构造函数内给定默认值，可经 SetRobustParams 覆盖）
    size_t lock_min_samples_;        // 判“锁定”前窗口至少需要的配对点数
    int lock_confirm_frames_;        // 所需独立达标观测组数
    double outlier_residual_thresh_; // 鲁棒重拟合中判“异常点”的残差阈值(m)
    double innovation_unlocked_xy_m_;
    double innovation_locked_xy_m_;
    double innovation_yaw_rad_;
    double innovation_z_m_;
    double lock_confirmation_min_interval_sec_;
    double lock_confirmation_min_displacement_m_;
    double lock_min_inlier_ratio_;
    double lock_min_along_std_m_;
    double lock_max_condition_number_;
    double lock_min_accumulated_turn_rad_;
    size_t vertical_min_samples_;
    double vertical_max_std_;
    double vertical_outlier_threshold_;
    double vertical_max_abs_offset_;
};

#endif // GLOBAL_FUSION_H
