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
//   本实现：内部换成“滑窗 2D 刚体(SE(2)) 对齐”：
//     - InputGPS 直接接收上层 GPS2Local 输出的 ENU 平面位置，与 LIO（已做杆臂
//       补偿+锚定的 base_link 位置）时间同步配对，闭式 Umeyama 估计 R(theta)、t，
//       使 p_enu ≈ R(theta)·p_lio + t；估计器自身不再持有 GeographicLib/原点，
//       从而 GPS 输出帧、SE(2) 回退帧、调试帧严格同一 ENU（无 create 模式跳变）；
//     - GPS 失效时不调用 InputGPS，(theta,t) 自动冻结，InputOdom 仍按冻结变换
//       输出 LIO 回退位姿（无跳变切换）；
//     - 旋转的可观测性按点云 2D 展布判定：沿迹向足够→“已播种(aligned)”，
//       横迹向足够(经历拐弯)→“已锁定(locked)”；近直线段不更新已锁定的旋转，
//       避免直线退化把 theta 拉偏。
//
//   坐标系：纯 ENU。把位置帧改为纯 ENU 是“破坏既有地图”的协同改动，需
//   同时改边界存储与下游，故不在本次内联。
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
    void InputGPS(double t, double enu_x, double enu_y, double enu_yaw, bool yaw_valid);
    void InputOdom(double t, Eigen::Vector3d OdomP, Eigen::Quaterniond OdomQ);
    void GetGlobalOdom(Eigen::Vector3d &odomP, Eigen::Quaterniond &odomQ);

    // 对齐健康标志（供 fusion 状态机使用）---------------------------------------
    bool IsAligned() const; // SE(2) 已播种，回退位姿可用（沿迹向展布达标）
    bool IsLocked() const;  // 旋转已被 2D 展布良好观测（经历过拐弯）

    // 当前 LIO->ENU 的 SE(2)：p_enu = R(theta)·p_lio + (tx,ty)。返回是否已对齐。
    // 供上层把“调试 LIO 位姿”用与生产回退完全相同的变换映射到 ENU，保持一致。
    bool GetTransform(double &theta, double &tx, double &ty) const;

    // 运行时参数注入（由上层从 launch 读入后调用，便于实地免重编译调参）。
    // max_window：配对窗口最大长度(帧)；window_span_sec：窗口最大时间跨度(s)；
    // match_tol_sec：GPS↔LIO 配对时间容差(s)；seed_along_std/lock_cross_std：
    // 判“已播种/已锁定”所需的沿/横迹向标准差(m)。非法值（<=0）将被忽略。
    void SetParams(int max_window, double window_span_sec, double match_tol_sec,
                   double seed_along_std, double lock_cross_std);

    // LIO 帧代际变化（/laserMapping 重启）后的外科手术式局部复位。
    // 清空 LIO↔ENU 配对窗口与对齐状态（估计器已无 GPS 原点可保留）。
    void ResetLioCoupled();

    // 置位后于下次 InputOdom/InputGPS 内整体清空。
    bool clear_data;

private:
    void UpdateAlignment_();       // 滑窗 Umeyama 估计 (theta_, tx_, ty_)
    void RemapLastGlobal_();        // 用当前 SE(2) 重新映射最近一帧 LIO -> 输出
    void ApplyClearIfRequested_();  // 处理 clear_data（整体清空）

    // 单条时间同步的配对：同一时刻的 (LIO 平面位置, map 平面位置)
    struct MatchPair
    {
        double t;
        double lx, ly; // LIO 平面位置
        double mx, my; // map 平面位置
    };
    std::deque<MatchPair> window_;

    // 最近的 LIO 位姿缓存（按时间，用于与 GPS 做最近邻时间配对）
    std::map<double, std::array<double, 3>> lioPoseMap_;
    Eigen::Vector3d lastLioP_;
    Eigen::Quaterniond lastLioQ_;
    bool hasLastLio_;

    // RTK 航向冷启动种子：SE(2) 未锁定且沿迹基线不足时，用节点启动后的 RTK 航向
    // 直接给出 LIO->ENU 旋转（短时短距精度足够，避免冷启动空窗 FAULT）。
    double last_enu_yaw_;  // 最近一帧 GPS 的 ENU 航向
    double last_lio_yaw_;  // 最近一帧 LIO 的平面航向
    bool last_yaw_valid_;  // 最近一帧 RTK 航向是否可信（上层按速度门控）

    // 当前 LIO->map 的 SE(2)：p_map = R(theta_)·p_lio + (tx_, ty_)
    double theta_;
    double tx_, ty_;
    bool aligned_;
    bool locked_;

    // 输出缓存（lastLioP_ 经当前 SE(2) 映射）
    Eigen::Vector3d lastGlobalP_;
    Eigen::Quaterniond lastGlobalQ_;

    mutable std::mutex mtx_;

    // 参数（构造函数内给定默认值）---------------------------------------------
    size_t max_window_;        // 配对窗口最大长度
    double max_window_span_;   // 配对窗口最大时间跨度(s)
    double match_time_tol_;    // GPS↔LIO 配对的时间容差(s)
    double seed_along_std_;    // 判“已播种”所需的沿迹向标准差(m)
    double lock_cross_std_;    // 判“已锁定”所需的横迹向标准差(m)
    size_t lio_cache_max_;     // lioPoseMap_ 最大容量
};

#endif // GLOBAL_FUSION_H
