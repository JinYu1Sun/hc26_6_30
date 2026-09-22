#include "LocalCartesian.hpp"
#include "fusion/alignment_state.h"
#include "fusion/fallback_policy.h"
#include "fusion/global_fusion.h"
#include "fusion/gps_outage_policy.h"
#include "fusion/gps_lio_sync_health.h"
#include "fusion/gps_motion_gate.h"
#include "fusion/gps_course_axis_gate.h"
#include "fusion/lio_time_alignment.h"
#include "fusion/lio_motion_gate.h"
#include "fusion/inertial_dead_reckoning.h"
#include "fusion/ins_attitude_conversion.h"
#include "fusion/map_bundle.h"
#include "fusion/pose_continuity_gate.h"
#include "fusion/rtk_quality_gate.h"
#include "util/GpsPosition.h"
#include "util/ImuStaticCalibration.h"
#include "util/LIOPose.h"
#include "util/LocalizationStatus.h"
#include "util/ManageLocalizationMap.h"
#include "util/Position.h"
#include <Eigen/Dense>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TransformStamped.h>
#include <limits.h>
#include <limits>
#include <mutex>
#include <nav_msgs/Odometry.h>
#include <nav_msgs/Path.h>
#include <queue>
#include <ros/ros.h>
#include <sensor_msgs/Imu.h>
#include <std_msgs/Bool.h>
#include <std_msgs/UInt8.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_datatypes.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <yaml-cpp/yaml.h>

constexpr auto DEG2RAD = M_PI / 180.0;

#define DEBUG

// =========== [辅助函数] ============================================
struct Vector {
  double x;
  double y;
};

bool isValidarray(const double *arr, const int size) {
  for (int i = 0; i < size; i++) {
    if (arr[i] == 0.0)
      return false;
  }
  return true;
}

// 计算两个点之间的欧氏距离
double computeDistance(const double *p1, const double *p2) {
  return std::sqrt((p1[0] - p2[0]) * (p1[0] - p2[0]) +
                   (p1[1] - p2[1]) * (p1[1] - p2[1]));
}

// ===================================================================

// =========== [RViz 可视化] =========================================
// 每一种位姿来源(纯GPS / 纯LIO / ceres融合 / 实际输出)对应一条 RvizTrack：
//   - odom_pub  : nav_msgs/Odometry，显示“当前位姿”（带朝向箭头），RViz 用
//                 Odometry 显示项即可看到位置 + 航向。
//   - path_pub  : nav_msgs/Path，累积历史轨迹，RViz 用 Path 显示项查看走过的线。
//   - child_frame_id : Odometry 的子坐标系名，便于区分。
// 所有轨迹统一发布在 viz_frame_id_(默认 "map") 下，RViz 把 Fixed Frame 设为该值
// 即可同时叠加对比各来源，无需额外 TF。
struct RvizTrack {
  ros::Publisher odom_pub;
  ros::Publisher path_pub;
  nav_msgs::Path path;
  std::string child_frame_id;
};
// ===================================================================

class FusionNode {
public:
  FusionNode() : nh_("~") {
    std::string node_name = ros::this_node::getName();
    std::string gps_topic, lio_slam_topic, fusion_topic, android_topic;
    std::string lio_static_status_topic, lio_generation_topic;
    ParameterInit();
    nh_.param<bool>(node_name + "/location_mode", location_mode_, false);
    nh_.param<std::string>(node_name + "/map_path", map_path_, "./my_map.mp");
    alignment_state_path_ = map_path_ + ".lio4d";
    nh_.param<std::string>(node_name + "/alignment_state_path",
                           alignment_state_path_, alignment_state_path_);
    // +++++++++++
    nh_.param<std::string>(
        node_name + "/last_pub_pos_path", last_pub_pos_path,
        "/home/nvidia/libo/lidar-slam-v2/src/fast-lio/Pos/last_pub_pos.txt");
    nh_.param<std::string>(node_name + "/gps_topic", gps_topic,
                           "/nanobot/gpsposition");
    nh_.param<std::string>(node_name + "/lio_slam_topic", lio_slam_topic,
                           "/Mower/lio_slam_level");
    nh_.param<std::string>(node_name + "/lio_static_status_topic",
                           lio_static_status_topic,
                           "/Mower/lio_static_initialized");
    nh_.param<std::string>(node_name + "/lio_generation_topic",
                           lio_generation_topic,
                           "/Mower/lio_level_transform");
    nh_.param<std::string>(node_name + "/fusion_topic", fusion_topic,
                           "/Mower/position");
    // +++++++++++
    nh_.param<std::string>(node_name + "/android_topic", android_topic,
                           "/signal");
    nh_.param<int>(node_name + "/map_index", map_index_, 0);
    nh_.param<int>(node_name + "/pub_fusion", pub_fusion, 0);
    nh_.param<int>(node_name + "/gps_stable_threshold", gps_stable_threshold_,
                   gps_stable_threshold_);
    nh_.param<int>(node_name + "/gps_recovery_floor", gps_recovery_floor_,
                   gps_recovery_floor_);
    nh_.param<int>(node_name + "/gps_initial_stable_count",
                   gps_initial_stable_count_, gps_initial_stable_count_);
    nh_.param<int>(node_name + "/gps_missing_grace_frames",
                   gps_missing_grace_frames_, gps_missing_grace_frames_);
    nh_.param<bool>(node_name + "/gps_grace_coast_output_enabled",
                    gps_grace_coast_output_enabled_,
                    gps_grace_coast_output_enabled_);
    nh_.param<int>(node_name + "/gps_transient_reacquire_frames",
                   gps_transient_reacquire_frames_,
                   gps_transient_reacquire_frames_);
    if (gps_stable_threshold_ < 1) {
      ROS_WARN("gps_stable_threshold must be >= 1; force to 50.");
      gps_stable_threshold_ = 50;
    }
    if (gps_recovery_floor_ < 0 ||
        gps_recovery_floor_ >= gps_stable_threshold_) {
      ROS_WARN("gps_recovery_floor must be in [0, threshold); force to 28.");
      gps_recovery_floor_ = std::min(28, gps_stable_threshold_ - 1);
    }
    gps_initial_stable_count_ =
        std::max(0, std::min(gps_initial_stable_count_, gps_stable_threshold_));
    if (gps_missing_grace_frames_ < 1 ||
        gps_transient_reacquire_frames_ < 1) {
      ROS_FATAL("GPS transient missing/reacquisition frame counts must be >=1.");
      ros::shutdown();
      return;
    }
    gps_stable_cnt_ = gps_initial_stable_count_;
    ROS_INFO("[GPS hysteresis] initial=%d recovery_floor=%d threshold=%d "
             "(recovery needs %d consecutive accepted samples)",
             gps_initial_stable_count_, gps_recovery_floor_,
             gps_stable_threshold_,
             gps_stable_threshold_ - gps_recovery_floor_);

    nh_.param<int>(node_name + "/rtk_required_ins_status",
                   rtk_quality_config_.required_ins_status, 3);
    nh_.param<int>(node_name + "/rtk_required_gps_flag",
                   rtk_quality_config_.required_gps_flag, 4);
    nh_.param<int>(node_name + "/rtk_required_ins_position_type",
                   rtk_quality_config_.required_ins_position_type, 56);
    nh_.param<int>(node_name + "/rtk_min_satellites",
                   rtk_quality_config_.min_satellites, 8);
    nh_.param<int>(node_name + "/rtk_max_differential_age_sec",
                   rtk_quality_config_.max_differential_age_sec, 10);
    nh_.param<double>(node_name + "/rtk_max_horizontal_std_m",
                      rtk_quality_config_.max_horizontal_std_m, 0.30);
    nh_.param<double>(node_name + "/rtk_max_height_std_m",
                      rtk_quality_config_.max_height_std_m, 0.50);
    nh_.param<double>(node_name + "/rtk_max_heading_std_deg",
                      rtk_quality_config_.max_heading_std_deg, 5.0);
    nh_.param<double>(node_name + "/gps_motion_min_dt_sec",
                      gps_motion_min_dt_sec_, gps_motion_min_dt_sec_);
    nh_.param<double>(node_name + "/gps_motion_max_dt_sec",
                      gps_motion_max_dt_sec_, gps_motion_max_dt_sec_);
    nh_.param<double>(node_name + "/gps_motion_max_speed_mps",
                      gps_motion_max_speed_mps_, gps_motion_max_speed_mps_);
    nh_.param<double>(node_name + "/gps_motion_margin_m",
                      gps_motion_margin_m_, gps_motion_margin_m_);
    nh_.param<double>(node_name + "/lio_motion_max_dt_sec",
                      lio_motion_max_dt_sec_, lio_motion_max_dt_sec_);
    nh_.param<double>(node_name + "/lio_motion_max_speed_mps",
                      lio_speed_threshold_, lio_speed_threshold_);
    nh_.param<double>(node_name + "/lio_motion_margin_m",
                      lio_motion_margin_m_, lio_motion_margin_m_);
    nh_.param<double>(node_name + "/lio_yaw_warning_min_speed_mps",
                      lio_yaw_warning_min_speed_mps_,
                      lio_yaw_warning_min_speed_mps_);
    nh_.param<double>(node_name + "/lio_yaw_warning_max_error_rad",
                      lio_yaw_warning_max_error_rad_,
                      lio_yaw_warning_max_error_rad_);
    nh_.param<double>(node_name + "/lio_time_alignment_max_pair_dt_sec",
                      lio_time_alignment_max_pair_dt_sec_,
                      lio_time_alignment_max_pair_dt_sec_);
    nh_.param<double>(node_name + "/lio_time_alignment_max_extrapolation_sec",
                      lio_time_alignment_max_extrapolation_sec_,
                      lio_time_alignment_max_extrapolation_sec_);
    nh_.param<double>(node_name + "/gps_coast_max_duration_sec",
                      gps_coast_max_duration_sec_,
                      gps_coast_max_duration_sec_);
    nh_.param<double>(node_name + "/gps_heading_suspect_coast_max_duration_sec",
                      gps_heading_suspect_coast_max_duration_sec_,
                      gps_heading_suspect_coast_max_duration_sec_);
    nh_.param<double>(node_name + "/output_max_measurement_age_sec",
                      output_max_measurement_age_sec_,
                      output_max_measurement_age_sec_);
    nh_.param<double>(node_name + "/output_max_future_stamp_sec",
                      output_max_future_stamp_sec_,
                      output_max_future_stamp_sec_);
    nh_.param<int>(node_name + "/gps_queue_max_size", gps_queue_max_size_,
                   gps_queue_max_size_);
    nh_.param<int>(node_name + "/lio_restart_after_bad_frames",
                   lio_restart_after_bad_frames_,
                   lio_restart_after_bad_frames_);
    nh_.param<double>(node_name + "/lio_bad_max_duration_sec",
                      lio_bad_max_duration_sec_, lio_bad_max_duration_sec_);
    nh_.param<int>(node_name + "/gps_lio_sync_restart_after_frames",
                   gps_lio_sync_restart_after_frames_,
                   gps_lio_sync_restart_after_frames_);
    nh_.param<double>(node_name + "/gps_lio_sync_bad_max_duration_sec",
                      gps_lio_sync_bad_max_duration_sec_,
                      gps_lio_sync_bad_max_duration_sec_);
    nh_.param<int>(node_name + "/lio_drift_hold_after_bad_frames",
                   lio_drift_hold_after_bad_frames_,
                   lio_drift_hold_after_bad_frames_);
    nh_.param<int>(node_name + "/lio_drift_hold_frames",
                   lio_drift_hold_frames_, lio_drift_hold_frames_);
    nh_.param<double>(node_name + "/lio_drift_hold_max_duration_sec",
                      lio_drift_hold_max_duration_sec_,
                      lio_drift_hold_max_duration_sec_);
    nh_.param<int>(node_name + "/lio_fallback_max_bad_frames",
                   lio_fallback_max_bad_frames_,
                   lio_fallback_max_bad_frames_);
    nh_.param<int>(node_name + "/stop_car_resume_frames",
                   stop_car_resume_frames_, stop_car_resume_frames_);
    if (rtk_quality_config_.required_gps_flag < 1 ||
        rtk_quality_config_.min_satellites < 1 ||
        rtk_quality_config_.max_differential_age_sec < 0 ||
        !std::isfinite(rtk_quality_config_.max_horizontal_std_m) ||
        rtk_quality_config_.max_horizontal_std_m <= 0.0 ||
        !std::isfinite(rtk_quality_config_.max_height_std_m) ||
        rtk_quality_config_.max_height_std_m <= 0.0 ||
        !std::isfinite(rtk_quality_config_.max_heading_std_deg) ||
        rtk_quality_config_.max_heading_std_deg <= 0.0) {
      ROS_FATAL("Invalid traceable RTK quality-gate parameters.");
      ros::shutdown();
      return;
    }
    if (!std::isfinite(gps_motion_min_dt_sec_) ||
        !std::isfinite(gps_motion_max_dt_sec_) ||
        gps_motion_min_dt_sec_ < 0.0 ||
        gps_motion_max_dt_sec_ <= gps_motion_min_dt_sec_) {
      ROS_FATAL("GPS motion dt limits must be finite and satisfy "
                "0 <= min < max (min=%.6f max=%.6f).",
                gps_motion_min_dt_sec_, gps_motion_max_dt_sec_);
      ros::shutdown();
      return;
    }
    const auto positive = [](double value) {
      return std::isfinite(value) && value > 0.0;
    };
    if (!positive(gps_motion_max_speed_mps_) ||
        !std::isfinite(gps_motion_margin_m_) || gps_motion_margin_m_ < 0.0 ||
        !positive(lio_motion_max_dt_sec_) ||
        !positive(lio_speed_threshold_) ||
        !std::isfinite(lio_motion_margin_m_) || lio_motion_margin_m_ < 0.0 ||
        !positive(lio_yaw_warning_min_speed_mps_) ||
        !positive(lio_yaw_warning_max_error_rad_) ||
        lio_yaw_warning_max_error_rad_ > M_PI ||
        !positive(lio_time_alignment_max_pair_dt_sec_) ||
        !std::isfinite(lio_time_alignment_max_extrapolation_sec_) ||
        lio_time_alignment_max_extrapolation_sec_ < 0.0 ||
        !positive(gps_coast_max_duration_sec_) ||
        !positive(gps_heading_suspect_coast_max_duration_sec_) ||
        gps_heading_suspect_coast_max_duration_sec_ <=
            gps_coast_max_duration_sec_ ||
        !positive(output_max_measurement_age_sec_) ||
        !std::isfinite(output_max_future_stamp_sec_) ||
        output_max_future_stamp_sec_ < 0.0 || gps_queue_max_size_ < 1 ||
        lio_restart_after_bad_frames_ < 1 ||
        !positive(lio_bad_max_duration_sec_) ||
        gps_lio_sync_restart_after_frames_ < 1 ||
        !positive(gps_lio_sync_bad_max_duration_sec_) ||
        lio_drift_hold_after_bad_frames_ < 1 ||
        lio_drift_hold_frames_ < 1 ||
        !positive(lio_drift_hold_max_duration_sec_) ||
        lio_fallback_max_bad_frames_ < 1 ||
        stop_car_resume_frames_ < 1) {
      ROS_FATAL("Invalid GPS/LIO motion, time-alignment, or coast parameters.");
      ros::shutdown();
      return;
    }
    fusion_4dof::GpsCourseAxisConfig axis_config;
    nh_.param<double>(node_name + "/gps_axis_min_window_sec",
                      axis_config.min_window_sec, axis_config.min_window_sec);
    nh_.param<double>(node_name + "/gps_axis_max_window_sec",
                      axis_config.max_window_sec, axis_config.max_window_sec);
    nh_.param<double>(node_name + "/gps_axis_min_displacement_m",
                      axis_config.min_displacement_m,
                      axis_config.min_displacement_m);
    nh_.param<double>(node_name + "/gps_axis_max_turn_rate_radps",
                      axis_config.max_turn_rate_radps,
                      axis_config.max_turn_rate_radps);
    double axis_max_error_deg = 40.0;
    nh_.param<double>(node_name + "/gps_axis_max_error_deg",
                      axis_max_error_deg, axis_max_error_deg);
    axis_config.max_axis_error_rad = axis_max_error_deg * DEG2RAD;
    nh_.param<int>(node_name + "/gps_axis_reject_after_windows",
                   axis_config.reject_after_windows,
                   axis_config.reject_after_windows);
    if (!std::isfinite(axis_config.min_window_sec) ||
        !std::isfinite(axis_config.max_window_sec) ||
        !std::isfinite(axis_config.min_displacement_m) ||
        !std::isfinite(axis_config.max_turn_rate_radps) ||
        !std::isfinite(axis_config.max_axis_error_rad) ||
        axis_config.min_window_sec <= 0.0 ||
        axis_config.max_window_sec <= axis_config.min_window_sec ||
        axis_config.min_displacement_m <= 0.0 ||
        axis_config.max_turn_rate_radps <= 0.0 ||
        axis_config.max_axis_error_rad <= 0.0 ||
        axis_config.max_axis_error_rad >= M_PI / 2.0 ||
        axis_config.reject_after_windows < 2) {
      ROS_FATAL("Invalid RTK body-axis course-gate parameters.");
      ros::shutdown();
      return;
    }
    gps_course_axis_gate_ = fusion_4dof::GpsCourseAxisGate(axis_config);
    gps_course_axis_turn_rate_radps_ = axis_config.max_turn_rate_radps;
    // Full nav_msgs/Path serialization runs in the LIO callback; production
    // therefore keeps visualization opt-in.
    nh_.param<bool>(node_name + "/enable_viz", enable_viz_, false);
    nh_.param<std::string>(node_name + "/viz_frame_id", viz_frame_id_, "map");
    nh_.param<int>(node_name + "/viz_path_max", viz_path_max_, 3000);
    nh_.param<bool>(node_name + "/diagnostic_log_enable",
                    diagnostic_log_enable_, true);
    nh_.param<double>(node_name + "/diagnostic_log_period_sec",
                      diagnostic_log_period_sec_, 1.0);
    if (!std::isfinite(diagnostic_log_period_sec_) ||
        diagnostic_log_period_sec_ <= 0.0) {
      ROS_WARN("diagnostic_log_period_sec must be > 0; force to 1.0s.");
      diagnostic_log_period_sec_ = 1.0;
    }

    // GpsPosition uses vehicle axes x=right, y=forward, z=up and 0.01 degree
    // fields. ROS base_link uses x=forward, y=left, z=up. Therefore, for the
    // documented small roll/pitch angles: ROS roll=INS roll and ROS
    // pitch=-INS pitch. Keep scale/sign parameters explicit because a vendor
    // firmware using a different Euler convention must not be silently used.
    nh_.param<std::string>(node_name + "/roll_pitch_source",
                           roll_pitch_source_, "ins");
    nh_.param<double>(node_name + "/ins_roll_scale_rad",
                      ins_roll_scale_rad_, 0.01 * DEG2RAD);
    nh_.param<double>(node_name + "/ins_pitch_scale_rad",
                      ins_pitch_scale_rad_, -0.01 * DEG2RAD);
    nh_.param<double>(node_name + "/ins_attitude_max_age_sec",
                      ins_attitude_max_age_sec_, 0.30);
    nh_.param<double>(node_name + "/ins_attitude_max_abs_deg",
                      ins_attitude_max_abs_rad_, 45.0);
    ins_attitude_max_abs_rad_ *= DEG2RAD;
    nh_.param<double>(node_name + "/ins_lio_max_disagreement_deg",
                      ins_lio_max_disagreement_rad_, 15.0);
    ins_lio_max_disagreement_rad_ *= DEG2RAD;
    nh_.param<bool>(node_name + "/ins_attitude_require_position_status",
                    ins_attitude_require_position_status_, false);
    if (roll_pitch_source_ != "ins" && roll_pitch_source_ != "lio") {
      ROS_WARN("Unknown roll_pitch_source='%s'; use 'ins'.",
               roll_pitch_source_.c_str());
      roll_pitch_source_ = "ins";
    }
    if (!std::isfinite(ins_roll_scale_rad_) ||
        !std::isfinite(ins_pitch_scale_rad_) ||
        !std::isfinite(ins_attitude_max_age_sec_) ||
        ins_attitude_max_age_sec_ <= 0.0 ||
        !std::isfinite(ins_attitude_max_abs_rad_) ||
        ins_attitude_max_abs_rad_ <= 0.0 ||
        !std::isfinite(ins_lio_max_disagreement_rad_) ||
        ins_lio_max_disagreement_rad_ <= 0.0) {
      ROS_FATAL("Invalid INS roll/pitch conversion or validity parameters.");
      ros::shutdown();
      return;
    }
    ROS_INFO("Output roll/pitch source=%s; INS conversion: roll*=%.9g, "
             "pitch*=%.9g rad/raw-unit, max_age=%.2fs, max_abs=%.1fdeg, "
             "INS/LIO disagreement<=%.1fdeg, LIO fallback enabled.",
             roll_pitch_source_.c_str(), ins_roll_scale_rad_,
             ins_pitch_scale_rad_, ins_attitude_max_age_sec_,
             ins_attitude_max_abs_rad_ / DEG2RAD,
             ins_lio_max_disagreement_rad_ / DEG2RAD);

    // ===== SE(2) 在线对齐 + LIO 帧代际检测 运行时参数（实地免重编译调参）=====
    // 缺省值与代码内置一致；在 launch 中覆盖即可现场调，无需重编译。
    // 缺省值以精度为主（更长窗口多平均、拐弯清晰才锁定；冷启动由 RTK 航向种子兜底）。
    int se2_max_window;
    double se2_window_span, se2_match_tol, se2_seed_along_std, se2_lock_cross_std;
    nh_.param<int>(node_name + "/se2_max_window", se2_max_window, 400);
    nh_.param<double>(node_name + "/se2_window_span_sec", se2_window_span, 45.0);
    nh_.param<double>(node_name + "/se2_match_tol_sec", se2_match_tol, 0.08);
    nh_.param<double>(node_name + "/se2_seed_along_std", se2_seed_along_std, 0.50);
    nh_.param<double>(node_name + "/se2_lock_cross_std", se2_lock_cross_std, 0.10);
    if (!std::isfinite(se2_match_tol) || se2_match_tol <= 0.0) {
      ROS_WARN("se2_match_tol_sec must be finite and > 0; force to 0.08s.");
      se2_match_tol = 0.08;
    }
    // 单一容差同时约束上层 GPS 队列选帧和估计器内 GPS->LIO 最近邻匹配，
    // 避免旧实现上层硬编码 80ms、估计器配置 50ms 的不一致。
    gps_lio_sync_tolerance_sec_ = se2_match_tol;
    globalEstimator_.SetParams(se2_max_window, se2_window_span, se2_match_tol,
                               se2_seed_along_std, se2_lock_cross_std);
    globalEstimator_.SetTimeAlignmentParams(
        lio_time_alignment_max_pair_dt_sec_,
        lio_time_alignment_max_extrapolation_sec_);
    // “锁定”稳健性 + 异常值鲁棒拟合参数（防小样本/单帧噪声误锁、防 GPS 野点污染整窗）。
    int se2_lock_min_samples, se2_lock_confirm_frames;
    double se2_outlier_residual_thresh;
    nh_.param<int>(node_name + "/se2_lock_min_samples", se2_lock_min_samples, 15);
    nh_.param<int>(node_name + "/se2_lock_confirm_frames", se2_lock_confirm_frames, 3);
    nh_.param<double>(node_name + "/se2_outlier_residual_thresh",
                      se2_outlier_residual_thresh, 0.5);
    if (se2_lock_min_samples < 3 || se2_lock_confirm_frames < 1 ||
        !std::isfinite(se2_outlier_residual_thresh) ||
        se2_outlier_residual_thresh <= 0.0) {
      ROS_FATAL("Invalid robust lock sample/confirmation/residual parameters.");
      ros::shutdown();
      return;
    }
    globalEstimator_.SetRobustParams(se2_lock_min_samples, se2_lock_confirm_frames,
                                     se2_outlier_residual_thresh);
    double lock_min_inlier_ratio, lock_min_along_std_m,
        lock_max_condition_number,
        lock_min_accumulated_turn_deg;
    nh_.param<double>(node_name + "/se2_lock_min_inlier_ratio",
                      lock_min_inlier_ratio, 0.70);
    nh_.param<double>(node_name + "/se2_lock_min_along_std_m",
                      lock_min_along_std_m, 0.50);
    nh_.param<double>(node_name + "/se2_lock_max_condition_number",
                      lock_max_condition_number, 50.0);
    nh_.param<double>(node_name + "/se2_lock_min_accumulated_turn_deg",
                      lock_min_accumulated_turn_deg, 30.0);
    if (!std::isfinite(lock_min_inlier_ratio) ||
        lock_min_inlier_ratio <= 0.5 || lock_min_inlier_ratio > 1.0 ||
        !std::isfinite(lock_min_along_std_m) ||
        lock_min_along_std_m <= 0.0 ||
        !std::isfinite(lock_max_condition_number) ||
        lock_max_condition_number < 1.0 ||
        !std::isfinite(lock_min_accumulated_turn_deg) ||
        lock_min_accumulated_turn_deg <= 0.0 ||
        lock_min_accumulated_turn_deg > 360.0) {
      ROS_FATAL("Invalid robust lock-geometry parameters.");
      ros::shutdown();
      return;
    }
    globalEstimator_.SetLockGeometryParams(
        lock_min_inlier_ratio, lock_min_along_std_m,
        lock_max_condition_number,
        lock_min_accumulated_turn_deg * DEG2RAD);
    double lock_confirmation_min_interval_sec,
        lock_confirmation_min_displacement_m;
    nh_.param<double>(node_name + "/se2_lock_confirmation_min_interval_sec",
                      lock_confirmation_min_interval_sec, 1.0);
    nh_.param<double>(node_name + "/se2_lock_confirmation_min_displacement_m",
                      lock_confirmation_min_displacement_m, 0.30);
    if (!std::isfinite(lock_confirmation_min_interval_sec) ||
        lock_confirmation_min_interval_sec <= 0.0 ||
        !std::isfinite(lock_confirmation_min_displacement_m) ||
        lock_confirmation_min_displacement_m <= 0.0) {
      ROS_FATAL("Invalid independent lock-confirmation parameters.");
      ros::shutdown();
      return;
    }
    globalEstimator_.SetLockIndependenceParams(
        lock_confirmation_min_interval_sec,
        lock_confirmation_min_displacement_m);
    double innovation_unlocked_xy, innovation_locked_xy,
        innovation_yaw_deg, innovation_z;
    nh_.param<double>(node_name + "/rtk_innovation_unlocked_xy_m",
                      innovation_unlocked_xy, 0.75);
    nh_.param<double>(node_name + "/rtk_innovation_locked_xy_m",
                      innovation_locked_xy, 0.40);
    nh_.param<double>(node_name + "/rtk_innovation_yaw_deg",
                      innovation_yaw_deg, 15.0);
    nh_.param<double>(node_name + "/rtk_innovation_z_m", innovation_z, 0.75);
    if (!std::isfinite(innovation_unlocked_xy) ||
        innovation_unlocked_xy <= 0.0 ||
        !std::isfinite(innovation_locked_xy) ||
        innovation_locked_xy <= 0.0 ||
        innovation_locked_xy > innovation_unlocked_xy ||
        !std::isfinite(innovation_yaw_deg) || innovation_yaw_deg <= 0.0 ||
        innovation_yaw_deg > 180.0 || !std::isfinite(innovation_z) ||
        innovation_z <= 0.0) {
      ROS_FATAL("Invalid RTK innovation parameters (locked XY must be no "
                "larger than unlocked XY).");
      ros::shutdown();
      return;
    }
    globalEstimator_.SetInnovationParams(
        innovation_unlocked_xy, innovation_locked_xy,
        innovation_yaw_deg * DEG2RAD, innovation_z);

    fusion_4dof::PoseContinuityConfig continuity_config;
    nh_.param<double>(node_name + "/output_max_speed_mps",
                      continuity_config.max_speed_mps, 3.0);
    nh_.param<double>(node_name + "/output_xy_margin_m",
                      continuity_config.xy_margin_m, 0.15);
    nh_.param<double>(node_name + "/output_max_xy_jump_m",
                      continuity_config.max_xy_jump_m, 0.75);
    nh_.param<double>(node_name + "/output_max_source_switch_xy_jump_m",
                      continuity_config.max_source_switch_xy_jump_m, 0.25);
    nh_.param<double>(node_name + "/output_max_yaw_rate_radps",
                      continuity_config.max_yaw_rate_radps, 2.0);
    nh_.param<double>(node_name + "/output_yaw_margin_deg",
                      output_yaw_margin_deg_, 11.5);
    continuity_config.yaw_margin_rad = output_yaw_margin_deg_ * DEG2RAD;
    nh_.param<double>(node_name + "/output_max_z_jump_m",
                      continuity_config.max_z_jump_m, 0.50);
    nh_.param<double>(node_name + "/output_max_dt_for_motion_sec",
                      continuity_config.max_dt_for_motion_sec, 1.0);
    if (!std::isfinite(continuity_config.max_speed_mps) ||
        continuity_config.max_speed_mps <= 0.0 ||
        !std::isfinite(continuity_config.xy_margin_m) ||
        continuity_config.xy_margin_m < 0.0 ||
        !std::isfinite(continuity_config.max_xy_jump_m) ||
        continuity_config.max_xy_jump_m <= 0.0 ||
        !std::isfinite(continuity_config.max_source_switch_xy_jump_m) ||
        continuity_config.max_source_switch_xy_jump_m <= 0.0 ||
        !std::isfinite(continuity_config.max_yaw_rate_radps) ||
        continuity_config.max_yaw_rate_radps <= 0.0 ||
        !std::isfinite(continuity_config.yaw_margin_rad) ||
        continuity_config.yaw_margin_rad < 0.0 ||
        !std::isfinite(continuity_config.max_z_jump_m) ||
        continuity_config.max_z_jump_m <= 0.0 ||
        !std::isfinite(continuity_config.max_dt_for_motion_sec) ||
        continuity_config.max_dt_for_motion_sec <= 0.0) {
      ROS_FATAL("Invalid final output-continuity parameters.");
      ros::shutdown();
      return;
    }
    output_continuity_gate_ =
        fusion_4dof::PoseContinuityGate(continuity_config);
    int vertical_min_samples;
    double vertical_max_std, vertical_outlier_threshold,
        vertical_max_abs_offset;
    nh_.param<int>(node_name + "/vertical_min_samples", vertical_min_samples,
                   10);
    nh_.param<double>(node_name + "/vertical_max_std", vertical_max_std, 0.30);
    nh_.param<double>(node_name + "/vertical_outlier_threshold",
                      vertical_outlier_threshold, 0.80);
    nh_.param<double>(node_name + "/vertical_max_abs_offset",
                      vertical_max_abs_offset, 1000.0);
    if (vertical_min_samples < 2 || !std::isfinite(vertical_max_std) ||
        vertical_max_std <= 0.0 ||
        !std::isfinite(vertical_outlier_threshold) ||
        vertical_outlier_threshold <= 0.0 ||
        !std::isfinite(vertical_max_abs_offset) ||
        vertical_max_abs_offset <= 0.0) {
      ROS_FATAL("Invalid 4DoF vertical-alignment parameters.");
      ros::shutdown();
      return;
    }
    globalEstimator_.SetVerticalParams(
        vertical_min_samples, vertical_max_std, vertical_outlier_threshold,
        vertical_max_abs_offset);
    nh_.param<bool>(node_name + "/vertical_require_ins_height_std",
                    vertical_require_ins_height_std_, false);
    nh_.param<double>(node_name + "/vertical_max_ins_height_std",
                      vertical_max_ins_height_std_, 0.50);
    if (!std::isfinite(vertical_max_ins_height_std_) ||
        vertical_max_ins_height_std_ <= 0.0) {
      ROS_FATAL("vertical_max_ins_height_std must be finite and > 0.");
      ros::shutdown();
      return;
    }
    // LIO 帧代际检测阈值（默认由 ParameterInit 设定，这里允许 launch 覆盖）。
    nh_.param<double>(node_name + "/lio_epoch_near_thresh", lio_epoch_near_thresh_,
                      lio_epoch_near_thresh_);
    nh_.param<double>(node_name + "/lio_epoch_far_thresh", lio_epoch_far_thresh_,
                      lio_epoch_far_thresh_);
    nh_.param<double>(node_name + "/lio_epoch_jump_thresh", lio_epoch_jump_thresh_,
                      lio_epoch_jump_thresh_);
    nh_.param<bool>(node_name + "/require_gravity_level_lio_frame",
                    require_gravity_level_lio_frame_, true);
    ROS_INFO("[4DoF] window=%d span=%.1fs match_tol=%.3fs seed_along=%.2fm "
             "lock_cross=%.2fm lock_min_samples=%d lock_confirm_frames=%d "
             "xy_outlier=%.2fm | z_min=%d z_std<=%.2fm z_outlier=%.2fm "
             "| epoch near=%.2f far=%.2f jump=%.2f",
             se2_max_window, se2_window_span, se2_match_tol, se2_seed_along_std,
             se2_lock_cross_std, se2_lock_min_samples, se2_lock_confirm_frames,
             se2_outlier_residual_thresh, vertical_min_samples,
             vertical_max_std, vertical_outlier_threshold,
             lio_epoch_near_thresh_, lio_epoch_far_thresh_,
             lio_epoch_jump_thresh_);

    LoadLioExtrinsic(node_name);
    fusion_4dof::InertialDeadReckoningConfig imu_config;
    nh_.param<bool>(node_name + "/imu_odom_enable", imu_odom_enable_, true);
    nh_.param<bool>(node_name + "/imu_odom_test_from_startup",
                    imu_odom_test_from_startup_, false);
    nh_.param<double>(node_name + "/mid360_imu_acc_scale",
                      mid360_imu_acc_scale_, 1.0);
    nh_.param<double>(node_name + "/imu_odom_gravity_mps2",
                      imu_config.gravity_mps2, 9.80665);
    nh_.param<double>(node_name + "/imu_odom_max_age_sec",
                      imu_config.max_imu_age_sec, 0.10);
    nh_.param<double>(node_name + "/imu_odom_max_integration_gap_sec",
                      imu_config.max_integration_gap_sec, 0.10);
    nh_.param<double>(node_name + "/imu_odom_max_gyro_radps",
                      imu_config.max_gyro_radps, 3.0);
    nh_.param<double>(node_name + "/imu_odom_max_accel_mps2",
                      imu_config.max_accel_mps2, 30.0);
    nh_.param<double>(node_name + "/imu_odom_max_velocity_mps",
                      imu_config.max_velocity_mps, 2.0);
    nh_.param<double>(node_name + "/imu_odom_fallback_max_duration_sec",
                      imu_config.fallback_max_duration_sec, 10.0);
    nh_.param<double>(node_name + "/imu_odom_fallback_max_distance_m",
                      imu_config.fallback_max_distance_m, 3.0);
    nh_.param<double>(node_name + "/imu_odom_stationary_gyro_threshold_radps",
                      imu_config.stationary_gyro_threshold_radps, 0.03);
    nh_.param<double>(node_name + "/imu_odom_stationary_accel_threshold_mps2",
                      imu_config.stationary_accel_threshold_mps2, 0.20);
    nh_.param<double>(node_name + "/imu_odom_stationary_confirmation_sec",
                      imu_config.stationary_confirmation_sec, 0.50);
    nh_.param<double>(node_name + "/imu_odom_main_pose_timeout_sec",
                      imu_odom_main_pose_timeout_sec_, 10.0);
    nh_.param<double>(node_name + "/lio_recovery_grace_timeout_sec",
                      lio_recovery_grace_timeout_sec_, 60.0);
    if (!std::isfinite(mid360_imu_acc_scale_) ||
        mid360_imu_acc_scale_ <= 0.0 ||
        !std::isfinite(imu_config.gravity_mps2) ||
        imu_config.gravity_mps2 <= 0.0 ||
        !std::isfinite(imu_config.max_imu_age_sec) ||
        imu_config.max_imu_age_sec <= 0.0 ||
        !std::isfinite(imu_config.max_integration_gap_sec) ||
        imu_config.max_integration_gap_sec <= 0.0 ||
        !std::isfinite(imu_config.max_gyro_radps) ||
        imu_config.max_gyro_radps <= 0.0 ||
        !std::isfinite(imu_config.max_accel_mps2) ||
        imu_config.max_accel_mps2 <= 0.0 ||
        !std::isfinite(imu_config.max_velocity_mps) ||
        imu_config.max_velocity_mps <= 0.0 ||
        !std::isfinite(imu_config.fallback_max_duration_sec) ||
        imu_config.fallback_max_duration_sec <= 0.0 ||
        !std::isfinite(imu_config.fallback_max_distance_m) ||
        imu_config.fallback_max_distance_m <= 0.0 ||
        !std::isfinite(imu_config.stationary_gyro_threshold_radps) ||
        imu_config.stationary_gyro_threshold_radps <= 0.0 ||
        !std::isfinite(imu_config.stationary_accel_threshold_mps2) ||
        imu_config.stationary_accel_threshold_mps2 <= 0.0 ||
        !std::isfinite(imu_config.stationary_confirmation_sec) ||
        imu_config.stationary_confirmation_sec <= 0.0 ||
        imu_config.stationary_gyro_threshold_radps >=
            imu_config.max_gyro_radps ||
        imu_config.stationary_accel_threshold_mps2 >=
            imu_config.max_accel_mps2 ||
        imu_config.stationary_confirmation_sec >
            imu_config.fallback_max_duration_sec ||
        !std::isfinite(imu_odom_main_pose_timeout_sec_) ||
        imu_odom_main_pose_timeout_sec_ <= 0.0 ||
        !std::isfinite(lio_recovery_grace_timeout_sec_) ||
        lio_recovery_grace_timeout_sec_ <= imu_odom_main_pose_timeout_sec_ ||
        imu_config.max_integration_gap_sec >
            imu_odom_main_pose_timeout_sec_) {
      ROS_FATAL("Invalid MID360 inertial-odometry safety parameters.");
      ros::shutdown();
      return;
    }
    imu_odom_max_velocity_mps_ = imu_config.max_velocity_mps;
    imu_odom_fallback_max_duration_for_log_ =
        imu_config.fallback_max_duration_sec;
    imu_odom_fallback_max_distance_for_log_ =
        imu_config.fallback_max_distance_m;
    imu_fallback_ = fusion_4dof::InertialDeadReckoning(imu_config);
    imu_test_ = fusion_4dof::InertialDeadReckoning(imu_config);
    if (location_mode_ && !ReadMap()) {
      // 定位模式的全部 GPS/融合输出都依赖建图时保存的 ENU 原点。
      // 原点缺失或损坏时继续启动会让 GPS2Local 使用零值/残值建立地图系，
      // 产生形式上可发布、实际却不属于所选地图的危险位置。因此直接停止本节点，
      // 交由上层停车/故障恢复逻辑处理，绝不带病进入 system_init_。
      algorithm_fault_code_ = util::LocalizationStatus::FAULT_MAP_INVALID;
      algorithm_reason_ = "location mode is waiting for a valid map bundle";
      ROS_ERROR("Location mode map is invalid: %s. Fusion stays alive in a "
                "fail-closed WAIT_MAP state so manage_localization_map can "
                "recover it without restarting FAST-LIO.", map_path_.c_str());
    }
    // +++++++++++
    subGps_ =
        nh_.subscribe(gps_topic, 5, &FusionNode::GPSCallBack, this); // 0822
    subLioGeneration_ =
        nh_.subscribe(lio_generation_topic, 1,
                      &FusionNode::LioGenerationCallBack, this);
    // Subscribe before the pose stream. The status is latched by FAST-LIO and
    // defaults false locally, so no normal-mode pose can escape during startup.
    subLioStatic_ = nh_.subscribe(lio_static_status_topic, 1,
                                  &FusionNode::LioStaticStatusCallBack, this);
    subLio_ = nh_.subscribe(lio_slam_topic, 1, &FusionNode::LioCallBack, this);
    std::string mid360_imu_topic, lio_static_calibration_topic;
    nh_.param<std::string>(node_name + "/mid360_imu_topic", mid360_imu_topic,
                           "/livox/imu");
    mid360_imu_topic_ = nh_.resolveName(mid360_imu_topic);
    nh_.param<std::string>(node_name + "/lio_static_calibration_topic",
                           lio_static_calibration_topic,
                           "/Mower/lio_static_calibration");
    subMid360Imu_ = nh_.subscribe(mid360_imu_topic, 100,
                                  &FusionNode::Mid360ImuCallBack, this);
    subLioStaticCalibration_ = nh_.subscribe(
        lio_static_calibration_topic, 1,
        &FusionNode::LioStaticCalibrationCallBack, this);

    pubPosition_ = nh_.advertise<util::Position>(fusion_topic, 1);
    std::string algorithm_status_topic;
    nh_.param<std::string>(node_name + "/algorithm_status_topic",
                           algorithm_status_topic,
                           "/Mower/localization_algorithm_status");
    pub_algorithm_status_ =
        nh_.advertise<util::LocalizationStatus>(algorithm_status_topic, 1,
                                                true);
    std::string map_service_name;
    nh_.param<std::string>(node_name + "/map_service", map_service_name,
                           "/Mower/manage_localization_map");
    map_service_ = nh_.advertiseService(
        map_service_name, &FusionNode::ManageLocalizationMap, this);
    std::string reboot_topic;
    nh_.param<std::string>(node_name + "/reboot_topic", reboot_topic, "/Mower/reboot");
    // Latched so a short manager reconnect cannot lose a one-shot FAULT
    // request while this process is waiting to be restarted.
    pub_fusion_Reboot_ = nh_.advertise<std_msgs::Bool>(reboot_topic, 1, true);
    pub_gps_Reboot_ = nh_.advertise<std_msgs::Bool>("/nanobot/reboot", 1);
    nh_.param<std::string>(node_name + "/stop_car_topic", stop_car_topic_,
                           "/mower/fusion_stop_car");
    // A recovery stop is safety state, not an edge event. Late subscribers
    // must receive it until this node (or the manager) explicitly releases it.
    pub_stop_car_ = nh_.advertise<std_msgs::Bool>(stop_car_topic_, 1, true);
    pub_imu_odom_status_ =
        nh_.advertise<std_msgs::UInt8>("/Mower/imu_odometry/status", 1, true);
    pub_imu_odom_ready_ =
        nh_.advertise<std_msgs::Bool>("/Mower/imu_odometry/ready", 1, true);
    pub_imu_fallback_odom_ = nh_.advertise<nav_msgs::Odometry>(
        "/Mower/imu_odometry/fallback", 10);
    pub_imu_test_odom_ =
        nh_.advertise<nav_msgs::Odometry>("/Mower/imu_odometry/test", 10);
    // Neutral ownership is safe only for a fault-free cold start. A map/config
    // fault may already have been detected before publishers are advertised;
    // the dedicated fusion request must agree with the algorithm status from
    // its very first latched sample.
    stop_car_active_ =
        algorithm_fault_code_ != util::LocalizationStatus::FAULT_NONE;
    PublishStopCar(stop_car_active_);
    PublishInertialOdometryReady(false);
    imu_odom_timer_ = nh_.createTimer(
        ros::Duration(0.05), &FusionNode::InertialOdometryTimer, this);
    algorithm_status_timer_ = nh_.createTimer(
        ros::Duration(0.20), &FusionNode::AlgorithmStatusTimer, this);
    PublishAlgorithmStatus();
    ROS_INFO("[IMU-ODOM] enabled=%d, startup comparison=%d, main-output "
             "timeout=%.2fs, fallback bounds=%.1fs/%.1fm. The comparison "
             "trajectory anchors once after startup static calibration.",
             imu_odom_enable_ ? 1 : 0,
             imu_odom_test_from_startup_ ? 1 : 0,
             imu_odom_main_pose_timeout_sec_,
             imu_odom_fallback_max_duration_for_log_,
             imu_odom_fallback_max_distance_for_log_);
#ifdef DEBUG
    pubDebugGPS_ = nh_.advertise<util::Position>("/Mower/debug/gps", 1);
    pubDebugLIO_ = nh_.advertise<util::Position>("/Mower/debug/lio", 1);
    pubDebugFusion_ = nh_.advertise<util::Position>("/Mower/fusion", 1);
    // 纯 RTK 位置+航向种子变换（globalEstimator_.GetSeedOnlyTransform，与滑窗/锁定
    // 状态无关，每帧重算）映射后的 LIO 位姿，供与 /Mower/debug/lio（官方 GetTransform，
    // 播种/锁定后更精确）持续对比，验证 SE(2) 滑窗估计相对朴素单帧方案的实际增益。
    pubDebugRtkSeed_ = nh_.advertise<util::Position>("/Mower/debug/rtk_seed", 1);
#endif
    SetupVizTracks();
  }

  ~FusionNode() { SaveAlignmentState(true); }

private:
  static bool IsProductionPositionState(uint32_t state) {
    return state == 1 || state == 2 || state == 5;
  }

  bool PublishProductionPosition(
      const util::Position &pose,
      const fusion_4dof::LioMotionReference *lio_reference = nullptr,
      bool rtk_coast = false) {
    const double measurement_time = pose.header.stamp.toSec();
    const double measurement_age =
        (ros::Time::now() - pose.header.stamp).toSec();
    if (pose.header.stamp.isZero() || !std::isfinite(measurement_age) ||
        measurement_age < -output_max_future_stamp_sec_ ||
        measurement_age > output_max_measurement_age_sec_) {
      ROS_ERROR_THROTTLE(1.0,
                         "Reject stale/future production pose (age=%.3fs).",
                         measurement_age);
      // Discard backlogged data and brake immediately. Recovery is owned by
      // fresh accepted production poses, not by LIO health or callback rate.
      output_timestamp_stop_active_ = true;
      output_timestamp_resume_count_ = 0;
      PublishStopCar(true);
      return false;
    }
    if (!output_continuity_gate_.Accept(
            measurement_time, pose.position_x, pose.position_y,
            pose.position_z,
            pose.yaw, static_cast<int>(pose.position_state),
            lio_reference, rtk_coast)) {
      RequestFullRestart(
          "candidate output violates trusted pose/source continuity gate");
      ROS_ERROR_THROTTLE(
          1.0,
          "Reject /Mower/position without advancing trusted baseline "
          "(state=%u, xyz_yaw=%.3f,%.3f,%.3f,%.3f); keep vehicle stopped.",
          pose.position_state, pose.position_x, pose.position_y,
          pose.position_z, pose.yaw);
      return false;
    }
    fusion_failed_cnt_ = 0;
    if (map_switch_pending_ && IsProductionPositionState(pose.position_state)) {
      active_map_name_ = pending_map_name_;
      active_map_uuid_ = pending_map_uuid_;
      active_origin_checksum_ = pending_origin_checksum_;
      pending_map_name_.clear();
      pending_map_uuid_.clear();
      pending_origin_checksum_.clear();
      map_switch_pending_ = false;
      ROS_INFO("Map identity committed with first valid pose: %s (%s).",
               active_map_name_.c_str(), active_map_uuid_.c_str());
    }
    last_position_state_ = pose.position_state;
    const double publish_steady_sec = ros::SteadyTime::now().toSec();
    last_position_publish_steady_sec_ = publish_steady_sec;
    pubPosition_.publish(pose);
    if (output_timestamp_stop_active_ &&
        ++output_timestamp_resume_count_ >= 5) {
      output_timestamp_stop_active_ = false;
      output_timestamp_resume_count_ = 0;
      if (!stop_car_active_ && !imu_odom_stop_active_ && !reboot &&
          algorithm_fault_code_ == util::LocalizationStatus::FAULT_NONE)
        PublishStopCar(false);
    }
    if (stop_car_active_ && !map_switch_pending_ && !reboot &&
        algorithm_fault_code_ == util::LocalizationStatus::FAULT_NONE &&
        IsProductionPositionState(pose.position_state) &&
        globalEstimator_.Is4DAligned()) {
      // Count final, fully gated production poses—not raw LIO callbacks. A
      // time gap breaks consecutiveness even if no rejected callback reached
      // this function.
      if (stop_car_last_trusted_output_steady_sec_ <= 0.0 ||
          publish_steady_sec - stop_car_last_trusted_output_steady_sec_ > 0.5)
        stop_car_resume_cnt_ = 1;
      else
        ++stop_car_resume_cnt_;
      stop_car_last_trusted_output_steady_sec_ = publish_steady_sec;
      if (stop_car_resume_cnt_ >= stop_car_resume_frames_) {
        stop_car_active_ = false;
        stop_car_resume_cnt_ = 0;
        stop_car_last_trusted_output_steady_sec_ = 0.0;
        if (!output_timestamp_stop_active_ && !imu_odom_stop_active_)
          PublishStopCar(false);
        ROS_WARN("Consecutive trusted production poses verified -> release "
                 "fusion stop request.");
      }
    }
    if (controlled_lio_recovery_ && lio_recovery_new_generation_received_ &&
        IsProductionPositionState(pose.position_state) &&
        globalEstimator_.Is4DAligned()) {
      controlled_lio_recovery_ = false;
      lio_recovery_new_generation_received_ = false;
      lio_recovery_start_steady_sec_ = 0.0;
      algorithm_reason_ = "LIO-only recovery completed on new generation";
      ROS_WARN("Controlled LIO recovery completed after new-generation 4DoF "
               "alignment and a trusted production pose.");
    }
    return true;
  }

  uint8_t PositionSourceFromState(uint32_t state) const {
    switch (state) {
    case 1:
      return util::LocalizationStatus::SOURCE_RTK;
    case 2:
      return util::LocalizationStatus::SOURCE_LIO_LOCKED;
    case 5:
      return util::LocalizationStatus::SOURCE_LIO_SEEDED;
    case 6:
      return util::LocalizationStatus::SOURCE_IMU_RESTRICTED;
    default:
      return util::LocalizationStatus::SOURCE_NONE;
    }
  }

  void PublishAlgorithmStatus() {
    util::LocalizationStatus status;
    status.header.stamp = ros::Time::now();
    status.header.frame_id = "map";
    status.reporter = ros::this_node::getName();
    status.requested_mode = location_mode_
                                ? util::LocalizationStatus::MODE_LOCALIZATION
                                : util::LocalizationStatus::MODE_MAPPING;
    status.active_mode = status.requested_mode;
    // fusion only owns its process runtime. The manager owns the user-visible
    // workflow because a mapping workflow may intentionally recover by loading
    // its already committed origin in location-style runtime.
    status.workflow_mode = util::LocalizationStatus::WORKFLOW_UNKNOWN;
    status.requested_runtime_mode =
        location_mode_
            ? util::LocalizationStatus::RUNTIME_TRACK_SAVED_ORIGIN
            : util::LocalizationStatus::RUNTIME_CREATE_ORIGIN;
    status.active_runtime_mode = status.requested_runtime_mode;
    status.position_state = last_position_state_;
    status.position_source = PositionSourceFromState(last_position_state_);
    const double position_age = last_position_publish_steady_sec_ > 0.0
                                    ? ros::SteadyTime::now().toSec() -
                                          last_position_publish_steady_sec_
                                    : std::numeric_limits<double>::infinity();
    status.position_valid = !output_timestamp_stop_active_ &&
                            IsProductionPositionState(last_position_state_) &&
                            position_age >= 0.0 && position_age <= 1.0;

    status.ins_status_received = ins_status_received_;
    status.ins_status = latest_ins_status_;
    status.ins_vehicle_align = latest_ins_vehicle_align_;
    status.ins_heading_flag = latest_ins_heading_flag_;
    status.ins_position_status = latest_ins_position_status_;
    status.ins_solution_good =
        ins_status_received_ && latest_ins_status_ == 3;
    // Do not expose the 1ant publisher's motion-derived positionStatus as INS
    // initialization.  A stationary receiver can have a valid INS solution
    // while that diagnostic remains zero indefinitely.
    status.ins_initialized =
        ins_status_received_ && latest_ins_status_ == 3;

    status.fast_lio_initialized =
        lio_static_initialized_ && lio_generation_valid_ &&
        lio_level_transform_ready_;
    status.fusion_generation_valid = fusion_generation_valid_;
    status.fusion_generation_sec = fusion_generation_sec_;
    status.fusion_generation_nsec = fusion_generation_nsec_;
    status.lio_generation_valid = lio_generation_valid_;
    status.lio_generation_sec = lio_generation_sec_;
    status.lio_generation_nsec = lio_generation_nsec_;
    status.map_origin_ready = map_origin_ready_;
    status.map_origin_committed = map_origin_ready_;
    status.alignment_ready = globalEstimator_.Is4DAligned();
    status.alignment_locked = globalEstimator_.IsLocked();
    status.imu_odometry_ready = imu_odom_ready_;
    const double steady_now = ros::SteadyTime::now().toSec();
    status.gps_good = latest_gps_good_ && last_gps_good_steady_sec_ > 0.0 &&
                      steady_now - last_gps_good_steady_sec_ <= 1.0;
    status.lio_good = latest_lio_good_ && last_lio_good_steady_sec_ > 0.0 &&
                      steady_now - last_lio_good_steady_sec_ <= 0.5;
    status.active_map = active_map_name_;
    status.pending_map = pending_map_name_;
    status.active_map_uuid = active_map_uuid_;
    status.pending_map_uuid = pending_map_uuid_;
    status.map_schema_version = fusion_4dof::kMapMetadataSchemaVersion;
    status.coordinate_frame = fusion_4dof::kMapCoordinateFrame;
    status.origin_checksum = map_switch_pending_ ? pending_origin_checksum_
                                                  : active_origin_checksum_;
    // In mapping mode a missing map is allowed during startup, but it is not
    // yet a valid output coordinate frame. Keep permission and readiness
    // separate: map_valid becomes true only after the origin is committed.
    status.map_valid = map_origin_ready_;
    status.algorithm_status_valid = true;
    const bool fault_active =
        reboot ||
        algorithm_fault_code_ != util::LocalizationStatus::FAULT_NONE;
    // FAULT is terminal from this process generation's perspective, not a
    // concurrent map/recovery transition. The lifecycle manager may start a
    // new, separately identified recovery after consuming restart_scope.
    status.transitioning =
        !fault_active &&
        (map_switch_pending_ || controlled_lio_recovery_ ||
         stop_car_active_ || output_timestamp_stop_active_);
    status.restart_scope = requested_restart_scope_;
    status.stop_required = stop_car_active_ || output_timestamp_stop_active_ ||
                           imu_odom_stop_active_ ||
                           controlled_lio_recovery_ || reboot ||
                           algorithm_fault_code_ !=
                               util::LocalizationStatus::FAULT_NONE;
    status.fault_code = algorithm_fault_code_;

    // Fault must dominate map-switch/recovery presentation. Otherwise a
    // simultaneous failure was exposed as "switching map" with a non-zero
    // fault code, an impossible tuple for operators and downstream logic.
    if (fault_active) {
      status.phase = util::LocalizationStatus::PHASE_FAULT;
      status.reason = algorithm_reason_.empty()
                          ? "localization algorithm requested recovery"
                          : algorithm_reason_;
    } else if (map_switch_pending_) {
      status.phase = util::LocalizationStatus::PHASE_SWITCHING_MAP;
      status.reason = algorithm_reason_.empty()
                          ? "new map origin committed; rebuilding 4DoF alignment"
                          : algorithm_reason_;
    } else if (requested_restart_scope_ ==
                   util::LocalizationStatus::RESTART_LIO ||
               controlled_lio_recovery_) {
      status.phase = util::LocalizationStatus::PHASE_RECOVERING_LIO;
      status.reason = algorithm_reason_.empty()
                          ? "controlled LIO restart/re-alignment in progress"
                          : algorithm_reason_;
    } else if (!status.fast_lio_initialized) {
      status.phase = util::LocalizationStatus::PHASE_WAIT_FAST_LIO;
      status.reason = "waiting for FAST-LIO static initialization";
    } else if (location_mode_ && !map_origin_ready_) {
      status.phase = util::LocalizationStatus::PHASE_WAIT_MAP;
      status.reason = "waiting for a valid map origin";
    } else if (!system_init_) {
      status.phase = status.ins_initialized
                         ? util::LocalizationStatus::PHASE_WAIT_GPS
                         : util::LocalizationStatus::PHASE_WAIT_INS;
      status.reason = status.ins_initialized
                          ? "INS solution ready; waiting for stable RTK position quality"
                          : "waiting for valid receiver INS solution (INS_Status != 3)";
    } else if (!status.alignment_ready) {
      status.phase = util::LocalizationStatus::PHASE_WAIT_ALIGNMENT;
      status.reason = "waiting for complete LIO-to-ENU 4DoF alignment";
    } else if (output_timestamp_stop_active_) {
      status.phase = util::LocalizationStatus::PHASE_WAIT_TRUSTED_OUTPUT;
      status.reason = "waiting for consecutive fresh output timestamps";
    } else if (stop_car_active_) {
      status.phase = util::LocalizationStatus::PHASE_WAIT_TRUSTED_OUTPUT;
      status.reason = "waiting for consecutive trusted localization output";
    } else if (last_position_state_ == 1) {
      status.phase = util::LocalizationStatus::PHASE_READY_RTK;
      status.reason = status.position_valid ? "RTK localization ready"
                                            : "RTK output is stale";
    } else if (last_position_state_ == 2) {
      status.phase = util::LocalizationStatus::PHASE_READY_LIO_LOCKED;
      status.reason = status.position_valid ? "locked LIO fallback ready"
                                            : "locked LIO output is stale";
    } else if (last_position_state_ == 5) {
      status.phase = util::LocalizationStatus::PHASE_READY_LIO_SEEDED;
      status.reason = status.position_valid ? "seeded LIO fallback ready"
                                            : "seeded LIO output is stale";
    } else if (last_position_state_ == 6) {
      status.phase = util::LocalizationStatus::PHASE_READY_IMU_RESTRICTED;
      status.reason = imu_fallback_.stationary_hold_active()
                          ? "restricted IMU stationary hold (ZUPT/ZARU); "
                            "braking only"
                          : "restricted IMU dead reckoning; braking only";
    } else {
      status.phase = util::LocalizationStatus::PHASE_WAIT_ALIGNMENT;
      status.reason = "aligned but no current production position";
    }
    // Enforce the public-state invariant after all phase/stop decisions. The
    // last published pose may still be younger than one second when a map
    // switch, recovery or fault begins; it is historical at that point, not a
    // currently usable production output.
    status.position_valid =
        status.position_valid && !status.stop_required &&
        !status.transitioning &&
        status.restart_scope == util::LocalizationStatus::RESTART_NONE &&
        status.fault_code == util::LocalizationStatus::FAULT_NONE;
    pub_algorithm_status_.publish(status);
  }

  void AlgorithmStatusTimer(const ros::TimerEvent &) {
    PublishAlgorithmStatus();
  }

  void PublishInertialOdometryReady(bool ready) {
    if (imu_odom_ready_ == ready && imu_odom_ready_published_)
      return;
    imu_odom_ready_ = ready;
    imu_odom_ready_published_ = true;
    std_msgs::Bool msg;
    msg.data = ready;
    pub_imu_odom_ready_.publish(msg);
  }

  void PublishInertialOdometryStatus(uint8_t status) {
    // 0=disabled/unready, 1=ready, 2=restricted fallback active,
    // 3=stale/expired and joint restart requested.
    std_msgs::UInt8 msg;
    msg.data = status;
    pub_imu_odom_status_.publish(msg);
  }

  bool StaticCalibrationGenerationMatches(
      const util::ImuStaticCalibration &msg) const {
    return lio_generation_valid_ && !msg.lio_generation.isZero() &&
           msg.lio_generation.sec == lio_generation_sec_ &&
           msg.lio_generation.nsec == lio_generation_nsec_;
  }

  bool ValidateStaticCalibrationMessage(
      const util::ImuStaticCalibration &msg) const {
    if (!msg.valid || msg.header.stamp.isZero() || msg.imu_topic.empty() ||
        msg.lio_generation.isZero() || !std::isfinite(msg.duration_sec) ||
        msg.duration_sec <= 0.0 || msg.sample_count < 2 ||
        !std::isfinite(msg.max_angular_velocity_norm_radps) ||
        msg.max_angular_velocity_norm_radps < 0.0)
      return false;
    for (int i = 0; i < 3; ++i) {
      if (!std::isfinite(msg.mean_acceleration_mps2[i]) ||
          !std::isfinite(msg.mean_angular_velocity_radps[i]) ||
          !std::isfinite(msg.acceleration_variance[i]) ||
          !std::isfinite(msg.angular_velocity_variance[i]) ||
          msg.acceleration_variance[i] < 0.0 ||
          msg.angular_velocity_variance[i] < 0.0)
        return false;
    }
    return true;
  }

  bool TryApplyLioStaticCalibration() {
    if (!have_pending_imu_static_calibration_ ||
        !StaticCalibrationGenerationMatches(
            pending_imu_static_calibration_))
      return false;
    if (!pending_imu_static_calibration_.valid)
      return false;
    if (!ValidateStaticCalibrationMessage(
            pending_imu_static_calibration_)) {
      ROS_ERROR_THROTTLE(
          1.0, "Reject malformed FAST-LIO static calibration statistics.");
      PublishInertialOdometryStatus(3);
      RequestFullRestart(
          "Malformed same-generation FAST-LIO static calibration.");
      return false;
    }
    if (imu_calibration_applied_ &&
        applied_imu_calibration_generation_sec_ == lio_generation_sec_ &&
        applied_imu_calibration_generation_nsec_ == lio_generation_nsec_)
      return true;

    if (pending_imu_static_calibration_.imu_topic != mid360_imu_topic_) {
      ROS_ERROR("Reject FAST-LIO static calibration: source topic '%s' does "
                "not match fusion MID360 topic '%s'.",
                pending_imu_static_calibration_.imu_topic.c_str(),
                mid360_imu_topic_.c_str());
      PublishInertialOdometryStatus(3);
      RequestFullRestart("FAST-LIO/fusion IMU topic mismatch.");
      return false;
    }

    if (!mid360_imu_frame_id_.empty() &&
        !pending_imu_static_calibration_.header.frame_id.empty() &&
        mid360_imu_frame_id_ !=
            pending_imu_static_calibration_.header.frame_id) {
      ROS_ERROR("Reject FAST-LIO static calibration: raw IMU frame '%s' "
                "does not match fusion /livox/imu frame '%s'.",
                pending_imu_static_calibration_.header.frame_id.c_str(),
                mid360_imu_frame_id_.c_str());
      PublishInertialOdometryStatus(3);
      RequestFullRestart("FAST-LIO/fusion IMU frame mismatch.");
      return false;
    }

    const Eigen::Vector3d mean_accel_imu(
        pending_imu_static_calibration_.mean_acceleration_mps2[0],
        pending_imu_static_calibration_.mean_acceleration_mps2[1],
        pending_imu_static_calibration_.mean_acceleration_mps2[2]);
    const Eigen::Vector3d mean_gyro_imu(
        pending_imu_static_calibration_.mean_angular_velocity_radps[0],
        pending_imu_static_calibration_.mean_angular_velocity_radps[1],
        pending_imu_static_calibration_.mean_angular_velocity_radps[2]);
    const Eigen::Quaterniond imu_to_base = lio_to_base_rotation_.inverse();
    const Eigen::Vector3d mean_accel_base = imu_to_base * mean_accel_imu;
    const Eigen::Vector3d mean_gyro_base = imu_to_base * mean_gyro_imu;
    if (!imu_fallback_.SetStaticCalibration(mean_accel_base,
                                             mean_gyro_base)) {
      ROS_ERROR("Reject FAST-LIO static calibration after IMU-to-base "
                "conversion: mean values are nonphysical.");
      PublishInertialOdometryStatus(3);
      RequestFullRestart(
          "Nonphysical FAST-LIO static calibration statistics.");
      return false;
    }
    // The startup-comparison trajectory consumes exactly the first accepted
    // FAST-LIO window and must never be reset by a later laserMapping
    // generation, otherwise it would cease to be a free-running comparison.
    if (imu_odom_test_from_startup_ &&
        !imu_test_.has_static_calibration() &&
        !imu_test_.SetStaticCalibration(mean_accel_base, mean_gyro_base)) {
      ROS_ERROR("Unable to initialize startup-comparison IMU calibration.");
      PublishInertialOdometryStatus(3);
      RequestFullRestart(
          "Startup-comparison IMU calibration initialization failed.");
      return false;
    }
    imu_calibration_applied_ = true;
    applied_imu_calibration_generation_sec_ = lio_generation_sec_;
    applied_imu_calibration_generation_nsec_ = lio_generation_nsec_;
    imu_calibration_frame_id_ =
        pending_imu_static_calibration_.header.frame_id;
    ROS_INFO("[IMU-ODOM] accepted authoritative FAST-LIO static window: "
             "generation=%u.%09u duration=%.3fs samples=%u frame='%s' "
             "mean_acc=[%.6f %.6f %.6f] mean_gyro=[%.7f %.7f %.7f].",
             lio_generation_sec_, lio_generation_nsec_,
             pending_imu_static_calibration_.duration_sec,
             pending_imu_static_calibration_.sample_count,
             imu_calibration_frame_id_.c_str(), mean_accel_base.x(),
             mean_accel_base.y(), mean_accel_base.z(), mean_gyro_base.x(),
             mean_gyro_base.y(), mean_gyro_base.z());
    return true;
  }

  void LioStaticCalibrationCallBack(
      const util::ImuStaticCalibrationConstPtr &msg) {
    if (msg == nullptr)
      return;
    if (msg->lio_generation.isZero()) {
      ROS_ERROR_THROTTLE(1.0,
                         "Reject static-calibration marker without LIO generation.");
      return;
    }
    if (lio_generation_valid_) {
      const ros::Time current_generation(lio_generation_sec_,
                                         lio_generation_nsec_);
      if (msg->lio_generation < current_generation) {
        ROS_WARN_THROTTLE(
            1.0, "Ignore stale FAST-LIO static calibration generation.");
        return;
      }
      if (!msg->valid && msg->lio_generation == current_generation &&
          imu_calibration_applied_) {
        ROS_WARN_THROTTLE(
            1.0, "Ignore delayed static-calibration invalid marker after "
                 "the same generation was accepted.");
        return;
      }
    }
    if (!ObserveLioProcessGeneration(msg->lio_generation)) {
      ROS_WARN_THROTTLE(1.0,
                        "Ignore stale FAST-LIO static calibration generation.");
      return;
    }
    pending_imu_static_calibration_ = *msg;
    have_pending_imu_static_calibration_ = true;
    TryApplyLioStaticCalibration();
  }

  void Mid360ImuCallBack(const sensor_msgs::ImuConstPtr &msg) {
    if (!imu_odom_enable_ || msg == nullptr)
      return;
    if (mid360_imu_frame_id_.empty())
      mid360_imu_frame_id_ = msg->header.frame_id;
    if (!imu_calibration_frame_id_.empty() &&
        !msg->header.frame_id.empty() &&
        msg->header.frame_id != imu_calibration_frame_id_) {
      imu_fallback_.InvalidateStaticCalibration();
      imu_calibration_applied_ = false;
      PublishInertialOdometryReady(false);
      PublishInertialOdometryStatus(3);
      ROS_ERROR_THROTTLE(
          1.0, "[IMU-ODOM] MID360 frame changed from '%s' to '%s'; "
               "invalidate calibration.",
          imu_calibration_frame_id_.c_str(), msg->header.frame_id.c_str());
      RequestFullRestart("MID360 IMU frame changed after static calibration.");
      return;
    }
    const Eigen::Vector3d accel_imu(
        msg->linear_acceleration.x * mid360_imu_acc_scale_,
        msg->linear_acceleration.y * mid360_imu_acc_scale_,
        msg->linear_acceleration.z * mid360_imu_acc_scale_);
    const Eigen::Vector3d omega_imu(msg->angular_velocity.x,
                                    msg->angular_velocity.y,
                                    msg->angular_velocity.z);
    // lio_to_base_rotation_ maps base-frame vectors into the leveled IMU axes;
    // both acceleration and angular velocity therefore use its inverse.
    const Eigen::Quaterniond imu_to_base = lio_to_base_rotation_.inverse();
    const Eigen::Vector3d accel_base = imu_to_base * accel_imu;
    const Eigen::Vector3d omega_base = imu_to_base * omega_imu;
    const double stamp = msg->header.stamp.isZero()
                             ? 0.0
                             : msg->header.stamp.toSec();
    const bool fallback_ok =
        imu_fallback_.InputImu(stamp, accel_base, omega_base);
    const bool test_ok =
        !imu_odom_test_from_startup_ ||
        imu_test_.InputImu(stamp, accel_base, omega_base);
    if (!fallback_ok || !test_ok) {
      ROS_WARN_THROTTLE(
          1.0, "[IMU-ODOM] rejected invalid, reversed, or gapped MID360 IMU sample.");
    }
  }

  bool ActivateRestrictedFallback(const char *reason) {
    if (!imu_odom_enable_) {
      PublishInertialOdometryStatus(3);
      return false;
    }
    if (!imu_fallback_.fallback_active() &&
        !imu_fallback_.StartFallback(ros::Time::now().toSec())) {
      PublishInertialOdometryStatus(3);
      ROS_ERROR("[IMU-ODOM] fallback unavailable (%s): MID360 IMU is stale, "
                "discontinuous, not statically calibrated, or has no trusted "
                "global pose anchor.",
                reason != nullptr ? reason : "unknown");
      return false;
    }
    PublishStopCar(true); // state 6 is braking-only and never authorizes work
    imu_odom_stop_active_ = true;
    PublishInertialOdometryStatus(2);
    ROS_ERROR_THROTTLE(1.0,
                       "[IMU-ODOM] restricted fallback active: %s; stop is "
                       "latched, max %.2fs/%.2fm is enforced internally.",
                       reason != nullptr ? reason : "main pose unavailable",
                       imu_odom_fallback_max_duration_for_log_,
                       imu_odom_fallback_max_distance_for_log_);
    return true;
  }

  bool PublishInertialOdometry(
      const fusion_4dof::InertialDeadReckoning &integrator,
      ros::Publisher *publisher, const ros::Time &stamp,
      const std::string &child_frame_id) {
    if (publisher == nullptr)
      return false;
    Eigen::Vector3d position, velocity;
    Eigen::Quaterniond q;
    if (!integrator.GetState(&position, &q, &velocity))
      return false;
    nav_msgs::Odometry odom;
    odom.header.stamp = stamp;
    odom.header.frame_id = "map";
    odom.child_frame_id = child_frame_id;
    odom.pose.pose.position.x = position.x();
    odom.pose.pose.position.y = position.y();
    odom.pose.pose.position.z = position.z();
    odom.pose.pose.orientation.x = q.x();
    odom.pose.pose.orientation.y = q.y();
    odom.pose.pose.orientation.z = q.z();
    odom.pose.pose.orientation.w = q.w();
    const Eigen::Vector3d velocity_base = q.inverse() * velocity;
    odom.twist.twist.linear.x = velocity_base.x();
    odom.twist.twist.linear.y = velocity_base.y();
    odom.twist.twist.linear.z = velocity_base.z();
    publisher->publish(odom);
    return true;
  }

  void PublishRestrictedInertialPose(const ros::Time &stamp,
                                     uint32_t position_state) {
    Eigen::Vector3d position, velocity;
    Eigen::Quaterniond q;
    if (!imu_fallback_.GetState(&position, &q, &velocity))
      return;
    tf::Quaternion tf_q(q.x(), q.y(), q.z(), q.w());
    double roll = 0.0, pitch = 0.0, yaw = 0.0;
    tf::Matrix3x3(tf_q).getRPY(roll, pitch, yaw);
    util::Position pose;
    pose.header.stamp = stamp;
    pose.header.frame_id = "base_link";
    pose.position_x = position.x();
    pose.position_y = position.y();
    pose.position_z = position.z();
    pose.roll = roll;
    pose.pitch = pitch;
    pose.yaw = yaw;
    pose.position_state = position_state;
    if (!PublishProductionPosition(pose))
      return;
    PublishInertialOdometry(imu_fallback_, &pub_imu_fallback_odom_, stamp,
                            "imu_fallback");
  }

  void InertialOdometryTimer(const ros::TimerEvent &) {
    if (!imu_odom_enable_ || reboot)
      return;
    const ros::Time now = ros::Time::now();
    const double now_sec = now.toSec();
    const double steady_now = ros::SteadyTime::now().toSec();
    if (imu_odom_test_from_startup_ && imu_test_.initialized() &&
        imu_test_.SourceStatus(now_sec) ==
            fusion_4dof::InertialDeadReckoning::Status::kUsable) {
      PublishInertialOdometry(imu_test_, &pub_imu_test_odom_, now,
                              "imu_startup_test");
    }
    if (fusion_4dof::ControlledLioRecoveryTimedOut(
            controlled_lio_recovery_,
            steady_now - lio_recovery_start_steady_sec_,
            lio_recovery_grace_timeout_sec_)) {
      controlled_lio_recovery_ = false;
      PublishInertialOdometryStatus(3);
      RequestFullRestart("Controlled LIO recovery exceeded its explicit "
                         "generation/re-alignment grace timeout.");
      return;
    }
    if (controlled_lio_recovery_) {
      // /laserMapping is deliberately absent, then the new generation must
      // finish static initialization and 4DoF alignment. This expected gap is
      // neither total source loss nor permission to emit state 6.
      PublishInertialOdometryStatus(0);
      return;
    }

    const bool gps_usable =
        latest_raw_gps_usable_ && last_raw_gps_usable_steady_sec_ > 0.0 &&
        steady_now - last_raw_gps_usable_steady_sec_ <= 1.0;
    const bool lio_usable =
        latest_lio_good_ && last_lio_good_steady_sec_ > 0.0 &&
        steady_now - last_lio_good_steady_sec_ <= 1.0;
    const bool main_pose_timed_out =
        last_main_pose_rx_sec_ > 0.0 &&
        now_sec - last_main_pose_rx_sec_ > imu_odom_main_pose_timeout_sec_;

    if (imu_fallback_.fallback_active() && (gps_usable || lio_usable)) {
      imu_fallback_.CancelFallback();
      // State 6 is no longer the current source. Clearing it also hands an
      // abnormally prolonged "source returned but no production pose" gap
      // back to the manager's generic watchdog instead of suppressing it.
      last_position_state_ = 0;
      PublishInertialOdometryStatus(1);
      ROS_WARN("[IMU-ODOM] primary GPS/LIO source returned; stop restricted "
               "state-6 output and wait for a trusted production pose.");
      return;
    }
    if (!imu_fallback_.fallback_active() &&
        fusion_4dof::ShouldEnterRestrictedImuFallback(
            main_pose_timed_out, gps_usable, lio_usable,
            controlled_lio_recovery_)) {
      if (!ActivateRestrictedFallback("main /Mower/position output timeout")) {
        PublishInertialOdometryStatus(3);
        RequestFullRestart("GPS and LIO were both unavailable for the full "
                           "timeout and restricted MID360 inertial odometry "
                           "is unavailable.");
        return;
      }
    }
    if (!imu_fallback_.fallback_active()) {
      const auto source = imu_fallback_.SourceStatus(now_sec);
      PublishInertialOdometryStatus(
          source == fusion_4dof::InertialDeadReckoning::Status::kUsable ? 1
                                                                        : 0);
      return;
    }

    const auto status = imu_fallback_.SourceStatus(now_sec);
    if (status == fusion_4dof::InertialDeadReckoning::Status::kUsable) {
      PublishRestrictedInertialPose(now, 6);
      PublishInertialOdometryStatus(2);
      return;
    }

    // Never extrapolate through a missing sample and never exceed either
    // bound. Publish one explicitly invalid pose before requesting recovery.
    PublishRestrictedInertialPose(now, 9);
    PublishInertialOdometryStatus(3);
    RequestFullRestart(status ==
                               fusion_4dof::InertialDeadReckoning::Status::kExpired
                           ? "Restricted MID360 inertial-odometry time/distance limit reached."
                           : "Restricted MID360 inertial-odometry input became stale.");
  }

  void AnchorInertialOdometry(const ros::Time &stamp, double x, double y,
                              double z, double roll, double pitch,
                              double yaw) {
    if (!imu_odom_enable_)
      return;
    (void)stamp; // production measurement time is retained on its own output;
                 // the inertial anchor is a receipt-time handover boundary.
    const double receipt_time = ros::Time::now().toSec();
    last_main_pose_rx_sec_ = receipt_time;
    const Eigen::Vector3d position(x, y, z);
    Eigen::Vector3d velocity = Eigen::Vector3d::Zero();
    if (have_previous_main_pose_) {
      const double dt = receipt_time - previous_main_pose_rx_sec_;
      if (dt >= 0.05 && dt <= 0.50) {
        Eigen::Vector3d candidate =
            (position - previous_main_position_) / dt;
        // Ground mower: do not seed unobservable/noisy vertical velocity from
        // differentiated GNSS/LIO height. IMU acceleration may still evolve z
        // during fallback, and the 3-D speed/distance bounds remain enforced.
        candidate.z() = 0.0;
        if (candidate.allFinite() &&
            candidate.norm() <= imu_odom_max_velocity_mps_)
          velocity = candidate;
      }
    }
    previous_main_position_ = position;
    previous_main_pose_rx_sec_ = receipt_time;
    have_previous_main_pose_ = true;
    const Eigen::Quaterniond q =
        Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
        Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
        Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());
    // Production state 6 is braking-only.  Do not feed it a velocity obtained
    // by differentiating two noisy global positions.  The free-running test
    // trajectory still receives that estimate so dynamic accuracy can be
    // evaluated without weakening the safety path.
    const bool production_ready = imu_fallback_.SetMainPose(
        receipt_time, position, q, Eigen::Vector3d::Zero(), true);
    PublishInertialOdometryReady(production_ready &&
                                 imu_fallback_.bias_ready());
    if (imu_odom_test_from_startup_)
      imu_test_.SetMainPose(receipt_time, position, q, velocity, false);
    if (imu_odom_stop_active_) {
      imu_odom_stop_active_ = false;
      if (!stop_car_active_ && !output_timestamp_stop_active_)
        PublishStopCar(false);
      ROS_WARN("[IMU-ODOM] trusted production pose recovered; restricted "
               "fallback exited.");
    }
  }

  struct DiagnosticFrame {
    DiagnosticFrame() {
      const double nan = std::numeric_limits<double>::quiet_NaN();
      lio_level_imu_p.setConstant(nan);
      lio_level_q = Eigen::Quaterniond(nan, nan, nan, nan);
      lio_h_base_p.setConstant(nan);
      lio_h_base_rpy.setConstant(nan);
      gps_enu.fill(nan);
      lio_map.fill(nan);
      output.fill(nan);
      latitude = longitude = height = gps_stamp = gps_lio_dt = nan;
      gps_height_std = gps_heading_std = nan;
      gps_lat_std = gps_lon_std = nan;
      ins_raw_roll = ins_raw_pitch = nan;
      ins_converted_roll = ins_converted_pitch = nan;
      azimuth_raw = nan;
      ins_acc.setConstant(nan);
      ins_gyro.setConstant(nan);
      ins_velocity.setConstant(nan);
      lio_speed = nan;
      gps_speed = gps_motion_yaw_deg = nan;
      gps_motion_dt = gps_motion_abs_dt = nan;
    }

    ros::Time stamp;
    uint32_t seq = 0;
    Eigen::Vector3d lio_level_imu_p;
    Eigen::Quaterniond lio_level_q;
    Eigen::Vector3d lio_h_base_p;
    Eigen::Vector3d lio_h_base_rpy;
    bool lio_state = false;
    bool lio_speed_good = false;
    bool lio_input = false;
    double lio_speed;
    double gps_speed;
    double gps_motion_yaw_deg;
    double gps_motion_dt;
    double gps_motion_abs_dt;
    fusion_4dof::GpsMotionResult gps_motion_result =
        fusion_4dof::GpsMotionResult::kInvalid;

    bool gps_synced = false;
    bool gps_good = false;
    bool gps_sample_valid = false;
    bool gps_yaw_valid = false;
    bool gps_z_valid = false;
    double gps_stamp;
    double gps_lio_dt;
    double latitude;
    double longitude;
    double height;
    int gps_flag = 0;
    int position_status = 0;
    int ins_status = 0;
    int gps_heading_flag = 0;
    int gps_position_flag = 0;
    int gps_num_sv = 0;
    int gps_age = 0;
    int gps_confidence = 0;
    int vehicle_align = 0;
    double gps_height_std;
    double gps_heading_std;
    double gps_lat_std;
    double gps_lon_std;
    double azimuth_raw;
    double ins_raw_roll;
    double ins_raw_pitch;
    double ins_converted_roll;
    double ins_converted_pitch;
    Eigen::Vector3d ins_acc;
    Eigen::Vector3d ins_gyro;
    // east,north,up in the source message units (cm/s).
    Eigen::Vector3d ins_velocity;
    std::array<double, 4> gps_enu;
    std::array<double, 4> lio_map;

    bool output_valid = false;
    double position_state = -1.0;
    // x,y,z,roll,pitch,yaw
    std::array<double, 6> output;
  };

  void MaybeLog4DoFDiagnostic(const DiagnosticFrame &d,
                              const char *phase) {
    if (!diagnostic_log_enable_)
      return;
    const double wall_now = ros::WallTime::now().toSec();
    if (last_diagnostic_log_wall_time_ > 0.0 &&
        wall_now - last_diagnostic_log_wall_time_ <
            diagnostic_log_period_sec_)
      return;
    last_diagnostic_log_wall_time_ = wall_now;

    double theta = 0.0, tx = 0.0, ty = 0.0, tz = 0.0;
    const bool aligned4d =
        globalEstimator_.GetTransform4D(theta, tx, ty, tz);
    if (!aligned4d)
      globalEstimator_.GetTransform(theta, tx, ty);

    Eigen::Vector3d global_p = Eigen::Vector3d::Constant(
        std::numeric_limits<double>::quiet_NaN());
    Eigen::Quaterniond global_q(
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN());
    double global_roll = std::numeric_limits<double>::quiet_NaN();
    double global_pitch = std::numeric_limits<double>::quiet_NaN();
    double global_yaw = std::numeric_limits<double>::quiet_NaN();
    if (d.lio_input && aligned4d) {
      globalEstimator_.GetGlobalOdom(global_p, global_q);
      tf::Quaternion q(global_q.x(), global_q.y(), global_q.z(), global_q.w());
      tf::Matrix3x3(q).getRPY(global_roll, global_pitch, global_yaw);
    }

    const double ins_age = latest_ins_attitude_valid_
                               ? std::fabs((d.stamp -
                                            latest_ins_attitude_stamp_)
                                               .toSec())
                               : std::numeric_limits<double>::quiet_NaN();
    double ins_lio_roll_diff = std::numeric_limits<double>::quiet_NaN();
    double ins_lio_pitch_diff = std::numeric_limits<double>::quiet_NaN();
    if (latest_ins_attitude_valid_ &&
        d.lio_h_base_rpy.allFinite()) {
      ins_lio_roll_diff = std::fabs(std::atan2(
          std::sin(latest_ins_roll_ - d.lio_h_base_rpy.x()),
          std::cos(latest_ins_roll_ - d.lio_h_base_rpy.x())));
      ins_lio_pitch_diff = std::fabs(std::atan2(
          std::sin(latest_ins_pitch_ - d.lio_h_base_rpy.y()),
          std::cos(latest_ins_pitch_ - d.lio_h_base_rpy.y())));
    }
    const bool ins_selected =
        roll_pitch_source_ == "ins" && latest_ins_attitude_valid_ &&
        std::isfinite(ins_age) && ins_age <= ins_attitude_max_age_sec_ &&
        (!d.lio_input ||
         std::max(ins_lio_roll_diff, ins_lio_pitch_diff) <=
             ins_lio_max_disagreement_rad_);
    const char *attitude_source =
        ins_selected ? "ins" : (d.lio_input ? "lio" : "none");

    ROS_INFO(
        "[4DOF-SYNC] phase=%s stamp=%.9f seq=%u gen=%u.%09u static=%d "
        "system=%d | lio_state=%d speed_good=%d speed=%.4f drift=%d "
        "level_imu_xyz=(%.6f,%.6f,%.6f) level_q_wxyz=(%.9f,%.9f,%.9f,%.9f) "
        "h_base_xyz=(%.6f,%.6f,%.6f) h_base_rpy=(%.7f,%.7f,%.7f) | "
        "gps_sync=%d gps_dt=%.6f good=%d sample_valid=%d flag=%d "
        "gps_speed=%.5f gps_motion_yaw_deg=%.5f motion_result=%s "
        "motion_dt=%.6f motion_abs_dt=%.6f motion_dt_window=(%.6f,%.6f) "
        "stable=%d "
        "gps_failed_cnt=%d gps_failed_flag=%d missing=%d transient=%d "
        "reacq=%d sync_failed=%d "
        "pos_status=%d ins_status=%d heading_flag=%d yaw_valid=%d z_valid=%d "
        "geo=(%.10f,%.10f,%.4f) enu_xyzyaw=(%.6f,%.6f,%.6f,%.7f) "
        "hstd=%.5f heading_std=%.5f ins_raw_rp=(%.5f,%.5f) "
        "ins_conv_rp=(%.7f,%.7f) ins_age=%.4f ins_lio_diff=(%.7f,%.7f)",
        phase, d.stamp.toSec(), d.seq, lio_generation_sec_,
        lio_generation_nsec_, lio_static_initialized_ ? 1 : 0,
        system_init_ ? 1 : 0, d.lio_state ? 1 : 0,
        d.lio_speed_good ? 1 : 0, d.lio_speed,
        lio_drift_detected_ ? 1 : 0, d.lio_level_imu_p.x(),
        d.lio_level_imu_p.y(), d.lio_level_imu_p.z(), d.lio_level_q.w(),
        d.lio_level_q.x(), d.lio_level_q.y(), d.lio_level_q.z(),
        d.lio_h_base_p.x(), d.lio_h_base_p.y(), d.lio_h_base_p.z(),
        d.lio_h_base_rpy.x(), d.lio_h_base_rpy.y(),
        d.lio_h_base_rpy.z(), d.gps_synced ? 1 : 0, d.gps_lio_dt,
        d.gps_good ? 1 : 0, d.gps_sample_valid ? 1 : 0, d.gps_flag,
        d.gps_speed, d.gps_motion_yaw_deg,
        fusion_4dof::GpsMotionResultName(d.gps_motion_result),
        d.gps_motion_dt, d.gps_motion_abs_dt, gps_motion_min_dt_sec_,
        gps_motion_max_dt_sec_, gps_stable_cnt_, gps_failed_cnt_,
        gps_failed_flag_ ? 1 : 0, gps_missing_cnt_,
        gps_transient_fallback_ ? 1 : 0, gps_transient_reacquire_cnt_,
        sync_failed_cnt_, d.position_status, d.ins_status,
        d.gps_heading_flag,
        d.gps_yaw_valid ? 1 : 0, d.gps_z_valid ? 1 : 0, d.latitude,
        d.longitude, d.height, d.gps_enu[0], d.gps_enu[1], d.gps_enu[2],
        d.gps_enu[3], d.gps_height_std, d.gps_heading_std,
        d.ins_raw_roll, d.ins_raw_pitch, d.ins_converted_roll,
        d.ins_converted_pitch, ins_age, ins_lio_roll_diff,
        ins_lio_pitch_diff);

    ROS_INFO(
        "[4DOF-INS] stamp=%.9f gps_sync=%d azimuth_raw=%.5f "
        "acc_xyz=(%.6f,%.6f,%.6f) gyro_xyz=(%.6f,%.6f,%.6f) "
        "vel_enu_cmps=(%.4f,%.4f,%.4f) confidence=%d pos_flag=%d "
        "heading_flag=%d num_sv=%d gps_age=%d ins_status=%d vehicle_align=%d "
        "std_lat_lon_h_heading=(%.6f,%.6f,%.6f,%.6f)",
        d.stamp.toSec(), d.gps_synced ? 1 : 0, d.azimuth_raw,
        d.ins_acc.x(), d.ins_acc.y(), d.ins_acc.z(), d.ins_gyro.x(),
        d.ins_gyro.y(), d.ins_gyro.z(), d.ins_velocity.x(),
        d.ins_velocity.y(), d.ins_velocity.z(), d.gps_confidence,
        d.gps_position_flag, d.gps_heading_flag, d.gps_num_sv, d.gps_age,
        d.ins_status, d.vehicle_align, d.gps_lat_std, d.gps_lon_std,
        d.gps_height_std, d.gps_heading_std);

    ROS_INFO(
        "[4DOF-STATE] stamp=%.9f phase=%s attitude_source=%s | "
        "xy_aligned=%d z_aligned=%d aligned4d=%d locked=%d "
        "theta=%.9f t=(%.6f,%.6f,%.6f) along=%.5f cross=%.5f n=%zu "
        "xy_rms=%.5f xy_outliers=%.5f z_std=%.5f z_inliers=%zu "
        "z_outliers=%.5f | lio_map_xyzyaw=(%.6f,%.6f,%.6f,%.7f) "
        "est_global_xyz=(%.6f,%.6f,%.6f) est_global_rpy=(%.7f,%.7f,%.7f) "
        "output_valid=%d state=%.0f output_xyzrpy=(%.6f,%.6f,%.6f,%.7f,%.7f,%.7f)",
        d.stamp.toSec(), phase, attitude_source,
        globalEstimator_.IsAligned() ? 1 : 0,
        globalEstimator_.IsVerticalAligned() ? 1 : 0, aligned4d ? 1 : 0,
        globalEstimator_.IsLocked() ? 1 : 0, theta, tx, ty, tz,
        globalEstimator_.GetAlongStd(), globalEstimator_.GetCrossStd(),
        globalEstimator_.GetWindowSize(),
        globalEstimator_.GetFitResidualRms(),
        globalEstimator_.GetOutlierRatio(), globalEstimator_.GetVerticalStd(),
        globalEstimator_.GetVerticalInlierCount(),
        globalEstimator_.GetVerticalOutlierRatio(), d.lio_map[0],
        d.lio_map[1], d.lio_map[2], d.lio_map[3], global_p.x(), global_p.y(),
        global_p.z(), global_roll, global_pitch, global_yaw,
        d.output_valid ? 1 : 0, d.position_state, d.output[0], d.output[1],
        d.output[2], d.output[3], d.output[4], d.output[5]);
  }

  void LoadLioExtrinsic(const std::string &node_name) {
    // 这里的外参含义是 /Mower/lio_slam_level 的IMU坐标系 -> base_link，
    // 不是 mid360_config.yaml 中的 LiDAR-IMU 外参。
    // 平移参数按 base_link/车体轴填写，代码内部再换算到 lio 轴。
    std::vector<double> lio_to_base_trans_baseframe{0.0, 0.0, 0.0};
    std::vector<double> lio_to_base_rpy_deg{0.0, 0.0, 0.0};

    bool has_baseframe_trans =
        nh_.getParam(node_name + "/lio_to_base_trans_baseframe",
                     lio_to_base_trans_baseframe);
    if (!has_baseframe_trans) {
      nh_.param<std::vector<double>>(node_name + "/lio_to_base_trans",
                                     lio_to_base_trans_baseframe,
                                     lio_to_base_trans_baseframe);
    }
    nh_.param<std::vector<double>>(node_name + "/lio_to_base_rpy_deg",
                                   lio_to_base_rpy_deg, lio_to_base_rpy_deg);

    if (lio_to_base_trans_baseframe.size() != 3) {
      ROS_WARN("Invalid param %s/lio_to_base_trans_baseframe, expected 3 "
               "elements. Using [0, 0, 0].",
               node_name.c_str());
      lio_to_base_trans_baseframe = {0.0, 0.0, 0.0};
    }

    if (lio_to_base_rpy_deg.size() != 3) {
      ROS_WARN("Invalid param %s/lio_to_base_rpy_deg, expected 3 elements. "
               "Using [0, 0, 0].",
               node_name.c_str());
      lio_to_base_rpy_deg = {0.0, 0.0, 0.0};
    }

    // lio_to_base_translation_baseframe_:
    //   向量“从 /Mower/lio_slam_level 的IMU原点 指向 base_link 原点”，
    //   但分量是按 base_link 轴表达的。
    lio_to_base_translation_baseframe_ = Eigen::Vector3d(
        lio_to_base_trans_baseframe[0], lio_to_base_trans_baseframe[1],
        lio_to_base_trans_baseframe[2]);

    tf::Quaternion tf_extrinsic_q;
    tf_extrinsic_q.setRPY(lio_to_base_rpy_deg[0] * DEG2RAD,
                          lio_to_base_rpy_deg[1] * DEG2RAD,
                          lio_to_base_rpy_deg[2] * DEG2RAD);
    // lio_to_base_rotation_:
    //   用于把 base_link 轴表达的向量转换到 lio 对齐后的轴表达；
    //   同时在位姿合成时满足 q_w_base = q_w_lio * q_lio_base。
    lio_to_base_rotation_ =
        Eigen::Quaterniond(tf_extrinsic_q.w(), tf_extrinsic_q.x(),
                           tf_extrinsic_q.y(), tf_extrinsic_q.z())
            .normalized();

    // 将“按 base_link 轴表达的杆臂”换算成“按 lio 轴表达的杆臂”。
    lio_to_base_translation_ =
        lio_to_base_rotation_ * lio_to_base_translation_baseframe_;

    ROS_INFO(
        "Loaded lio_to_base extrinsic(base frame): t = [%.4f, %.4f, %.4f], "
        "rpy_deg = [%.2f, %.2f, %.2f]",
        lio_to_base_translation_baseframe_.x(),
        lio_to_base_translation_baseframe_.y(),
        lio_to_base_translation_baseframe_.z(), lio_to_base_rpy_deg[0],
        lio_to_base_rpy_deg[1], lio_to_base_rpy_deg[2]);
    ROS_INFO(
        "Converted lio_to_base translation(in lio frame): [%.4f, %.4f, %.4f]",
        lio_to_base_translation_.x(), lio_to_base_translation_.y(),
        lio_to_base_translation_.z());
  }

  // 建立 5 条可视化轨迹：纯GPS / 纯LIO(官方GetTransform) / ceres融合 /
  // 实际输出位姿 / RTK种子(GetSeedOnlyTransform，与滑窗/锁定无关，见 README §11)。
  // 话题统一前缀 /Mower/viz/<name>/{odom,path}，便于在 RViz 中按需添加。
  void SetupVizTracks() {
    if (!enable_viz_)
      return;
    SetupOneVizTrack(viz_gps_, "gps", "gps");
    SetupOneVizTrack(viz_lio_, "lio", "lio");
    SetupOneVizTrack(viz_fusion_, "fusion", "fusion");
    SetupOneVizTrack(viz_output_, "output", "base_link");
    SetupOneVizTrack(viz_rtk_seed_, "rtk_seed", "rtk_seed");
    ROS_INFO("[viz] RViz visualization enabled. Fixed Frame = '%s'. Topics: "
             "/Mower/viz/{gps,lio,fusion,output,rtk_seed}/{odom,path}",
             viz_frame_id_.c_str());
  }

  void SetupOneVizTrack(RvizTrack &track, const std::string &name,
                        const std::string &child_frame) {
    const std::string base = "/Mower/viz/" + name;
    track.odom_pub = nh_.advertise<nav_msgs::Odometry>(base + "/odom", 10);
    track.path_pub = nh_.advertise<nav_msgs::Path>(base + "/path", 1);
    track.path.header.frame_id = viz_frame_id_;
    track.child_frame_id = child_frame;
  }

  // 发布一帧位姿到对应轨迹：同时刷新 Odometry(当前位姿) 与 Path(累积轨迹)。
  // x,y,z 单位米，roll/pitch/yaw 单位弧度，均在 viz_frame_id_ 坐标系下。
  void PublishVizPose(RvizTrack &track, const ros::Time &stamp, double x,
                      double y, double z, double roll, double pitch,
                      double yaw) {
    if (!enable_viz_)
      return;

    geometry_msgs::Quaternion q =
        tf::createQuaternionMsgFromRollPitchYaw(roll, pitch, yaw);

    nav_msgs::Odometry odom;
    odom.header.stamp = stamp;
    odom.header.frame_id = viz_frame_id_;
    odom.child_frame_id = track.child_frame_id;
    odom.pose.pose.position.x = x;
    odom.pose.pose.position.y = y;
    odom.pose.pose.position.z = z;
    odom.pose.pose.orientation = q;
    track.odom_pub.publish(odom);

    geometry_msgs::PoseStamped ps;
    ps.header = odom.header;
    ps.pose = odom.pose.pose;
    track.path.header.stamp = stamp;
    track.path.poses.push_back(ps);
    // 限制轨迹长度，避免长时间运行内存无界增长。
    if (viz_path_max_ > 0 &&
        static_cast<int>(track.path.poses.size()) > viz_path_max_) {
      track.path.poses.erase(track.path.poses.begin(),
                             track.path.poses.begin() +
                                 (track.path.poses.size() - viz_path_max_));
    }
    track.path_pub.publish(track.path);
  }

  // 把“实际输出位姿”广播为 viz_frame_id_ -> base_link 的 TF，
  // 方便在 RViz 中以 base_link 关联车体模型/点云。
  void BroadcastBaseLinkTF(const ros::Time &stamp, double x, double y, double z,
                           double roll, double pitch, double yaw) {
    if (!enable_viz_)
      return;
    tf::Transform transform;
    transform.setOrigin(tf::Vector3(x, y, z));
    tf::Quaternion q;
    q.setRPY(roll, pitch, yaw);
    transform.setRotation(q);
    viz_tf_broadcaster_.sendTransform(
        tf::StampedTransform(transform, stamp, viz_frame_id_, "base_link"));
  }

  bool ConvertInsRollPitch(const util::GpsPosition &msg, double *roll,
                           double *pitch) const {
    return fusion_4dof::ConvertInsRollPitch(
        msg.roll, msg.pitch, msg.positionStatus, ins_roll_scale_rad_,
        ins_pitch_scale_rad_, ins_attitude_max_abs_rad_,
        ins_attitude_require_position_status_, roll, pitch);
  }

  void UpdateInsAttitude(const util::GpsPosition &msg) {
    latest_ins_attitude_valid_ =
        ConvertInsRollPitch(msg, &latest_ins_roll_, &latest_ins_pitch_);
    latest_ins_attitude_stamp_ = msg.header.stamp;
    if (!latest_ins_attitude_valid_) {
      ROS_WARN_THROTTLE(2.0,
                        "INS roll/pitch invalid; output will fall back to "
                        "gravity-leveled LIO attitude when available.");
    }
  }

  bool SelectOutputRollPitch(const ros::Time &stamp, bool lio_available,
                             double lio_roll, double lio_pitch, double *roll,
                             double *pitch) const {
    if (roll == nullptr || pitch == nullptr)
      return false;
    *roll = 0.0;
    *pitch = 0.0;
    if (lio_available && std::isfinite(lio_roll) &&
        std::isfinite(lio_pitch)) {
      *roll = lio_roll;
      *pitch = lio_pitch;
    }
    if (roll_pitch_source_ != "ins")
      return lio_available;
    if (!latest_ins_attitude_valid_) {
      if (!lio_available)
        ROS_WARN_THROTTLE(1.0, "Neither INS nor LIO roll/pitch is valid; "
                               "suppress pose publication.");
      return lio_available;
    }
    const double age = std::fabs((stamp - latest_ins_attitude_stamp_).toSec());
    if (std::isfinite(age) && age <= ins_attitude_max_age_sec_) {
      if (lio_available && lio_static_initialized_ &&
          std::isfinite(lio_roll) && std::isfinite(lio_pitch)) {
        const double roll_diff = std::fabs(std::atan2(
            std::sin(latest_ins_roll_ - lio_roll),
            std::cos(latest_ins_roll_ - lio_roll)));
        const double pitch_diff = std::fabs(std::atan2(
            std::sin(latest_ins_pitch_ - lio_pitch),
            std::cos(latest_ins_pitch_ - lio_pitch)));
        if (std::max(roll_diff, pitch_diff) >
            ins_lio_max_disagreement_rad_) {
          ROS_WARN_THROTTLE(
              2.0,
              "Reject INS roll/pitch for output: disagreement with "
              "gravity-leveled LIO is roll=%.2fdeg pitch=%.2fdeg.",
              roll_diff / DEG2RAD, pitch_diff / DEG2RAD);
          return true;
        }
      }
      *roll = latest_ins_roll_;
      *pitch = latest_ins_pitch_;
      return true;
    } else {
      ROS_WARN_THROTTLE(2.0,
                        "INS roll/pitch is stale (age=%.3fs); using LIO "
                        "roll/pitch for this output.",
                        age);
    }
    if (!lio_available)
      ROS_WARN_THROTTLE(1.0, "INS roll/pitch is stale and no valid LIO "
                             "attitude is available; suppress pose publication.");
    return lio_available;
  }

  bool CanonicalizeOutputRPY(double candidate_roll, double candidate_pitch,
                             double candidate_yaw, double *roll,
                             double *pitch, double *yaw) const {
    if (roll == nullptr || pitch == nullptr || yaw == nullptr ||
        !std::isfinite(candidate_roll) || !std::isfinite(candidate_pitch) ||
        !std::isfinite(candidate_yaw))
      return false;
    tf::Quaternion q;
    q.setRPY(candidate_roll, candidate_pitch, candidate_yaw);
    if (!std::isfinite(q.length2()) || q.length2() < 1e-12)
      return false;
    q.normalize();
    tf::Matrix3x3(q).getRPY(*roll, *pitch, *yaw);
    *yaw = std::atan2(std::sin(*yaw), std::cos(*yaw));
    return std::isfinite(*roll) && std::isfinite(*pitch) &&
           std::isfinite(*yaw);
  }

  fusion_4dof::RtkQualityObservation
  RtkQualityOf(const util::GpsPosition &msg) const {
    fusion_4dof::RtkQualityObservation quality;
    quality.gps_flag = msg.gps_flag;
    quality.ins_status = msg.INS_Status;
    quality.ins_position_type = msg.INS_GpsFlag_Pos;
    quality.satellites = msg.INS_NumSV;
    quality.differential_age_sec = msg.INS_Gps_Age;
    quality.position_status = msg.positionStatus;
    quality.latitude_std_m = msg.INS_Std_Lat;
    quality.longitude_std_m = msg.INS_Std_Lon;
    quality.height_std_m = msg.INS_Std_LocatHeight;
    quality.heading_std_deg = msg.INS_Std_Heading;
    return quality;
  }

  void GPSCallBack(const util::GpsPositionConstPtr &msg) {
    if (msg == nullptr)
      return;
    const double stamp = msg->header.stamp.toSec();
    const double measurement_age =
        (ros::Time::now() - msg->header.stamp).toSec();
    if (msg->header.stamp.isZero() || !std::isfinite(measurement_age) ||
        measurement_age < -output_max_future_stamp_sec_ ||
        measurement_age > output_max_measurement_age_sec_) {
      ROS_WARN_THROTTLE(1.0,
                        "Reject stale/future GPS before mutating fusion state "
                        "(age=%.3fs).",
                        measurement_age);
      return;
    }
    if (last_gps_callback_stamp_ > 0.0 &&
        stamp < last_gps_callback_stamp_) {
      ROS_WARN_THROTTLE(1.0,
                        "Reject out-of-order GPS timestamp (current=%.9f "
                        "last=%.9f).",
                        stamp, last_gps_callback_stamp_);
      return;
    }
    last_gps_callback_stamp_ = stamp;
    ins_status_received_ = true;
    latest_ins_status_ = msg->INS_Status;
    latest_ins_vehicle_align_ = msg->INS_VehicleAlign;
    latest_ins_heading_flag_ = msg->INS_GpsFlag_Heading;
    latest_ins_position_status_ = msg->positionStatus;
    // This receipt-side source gate is intentionally independent of the
    // LIO-driven GPS synchronizer. During an expected LIO-only restart no LIO
    // callback runs, but a healthy RTK stream still means the entire
    // localization system is not unavailable.
    const bool raw_gps_usable =
        fusion_4dof::IsRtkPositionQualityGood(RtkQualityOf(*msg),
                                              rtk_quality_config_) &&
        std::isfinite(msg->latitude) && std::isfinite(msg->longitude) &&
        std::isfinite(msg->height) && msg->latitude >= -90.0 &&
        msg->latitude <= 90.0 && msg->longitude >= -180.0 &&
        msg->longitude <= 180.0;
    latest_raw_gps_usable_ = raw_gps_usable;
    if (raw_gps_usable)
      last_raw_gps_usable_steady_sec_ = ros::SteadyTime::now().toSec();
    // Do not let a position/INS-quality rejected packet refresh the attitude
    // cache merely because its numeric roll/pitch fields look finite.
    if (raw_gps_usable) {
      UpdateInsAttitude(*msg);
    } else {
      latest_ins_attitude_valid_ = false;
      latest_ins_attitude_stamp_ = msg->header.stamp;
    }
    if (location_mode_ && !map_origin_ready_) {
      ROS_WARN_THROTTLE(1.0, "Drop GPS processing until a valid map bundle is activated.");
      return;
    }
    {
      std::unique_lock<std::mutex> lock(gps_mutex_);
      gps_queue_.push(msg);
      while (static_cast<int>(gps_queue_.size()) > gps_queue_max_size_) {
        gps_queue_.pop();
        ROS_WARN_THROTTLE(1.0,
                          "GPS synchronization queue reached its bound; "
                          "drop oldest measurement.");
      }
    }

    if (pub_fusion == 2) {
      if (!raw_gps_usable || !fusion_4dof::IsRtkHeadingQualityGood(
                                 RtkQualityOf(*msg), rtk_quality_config_)) {
        ROS_WARN_THROTTLE(
            1.0, "Pure-GPS output suppressed: traceable RTK position/heading "
                 "quality is not valid.");
        return;
      }
      double location_gps[4]; // [0]x [1]y [2]高程 [3]航向
      memset(location_gps, 0, sizeof(double) * 4);
      double latitude = msg->latitude;
      double longitude = msg->longitude;
      double altitude = msg->height;
      // +++++++++++
      double gauss_yaw = msg->azimuth - 9000;
      if (gauss_yaw < 0) {
        gauss_yaw += 36000;
      }
      gauss_yaw *= 0.01;

      GPS2Local(latitude, longitude, altitude, gauss_yaw, location_gps);

      util::Position gps_position;
      gps_position.header.stamp = msg->header.stamp;
      gps_position.header.frame_id = "base_link";
      gps_position.position_x = location_gps[0];
      gps_position.position_y = location_gps[1];
      gps_position.position_z = location_gps[2];
      double output_roll = 0.0, output_pitch = 0.0;
      if (!SelectOutputRollPitch(msg->header.stamp, false, 0.0, 0.0,
                                 &output_roll, &output_pitch))
        return;
      double output_yaw = location_gps[3];
      if (!CanonicalizeOutputRPY(output_roll, output_pitch, output_yaw,
                                 &output_roll, &output_pitch, &output_yaw))
        return;
      gps_position.roll = output_roll;
      gps_position.pitch = output_pitch;
      gps_position.yaw = output_yaw;
      gps_position.position_state = 3;
      if (!PublishProductionPosition(gps_position))
        return;
      // [viz] 纯 GPS 模式：画 GPS 轨迹与输出轨迹 + TF。
      PublishVizPose(viz_gps_, msg->header.stamp, location_gps[0],
                     location_gps[1], location_gps[2], output_roll,
                     output_pitch,
                     output_yaw);
      PublishVizPose(viz_output_, msg->header.stamp, location_gps[0],
                     location_gps[1], location_gps[2], output_roll,
                     output_pitch,
                     output_yaw);
      BroadcastBaseLinkTF(msg->header.stamp, location_gps[0], location_gps[1],
                          location_gps[2], output_roll, output_pitch,
                          output_yaw);
    }
  }

  bool SaveAlignmentState(bool force = false) {
    if (!map_origin_ready_ || !lio_generation_valid_ ||
        !globalEstimator_.Is4DAligned() || map_switch_pending_ ||
        active_map_uuid_.empty() || active_origin_checksum_.empty())
      return false;
    const double now = ros::WallTime::now().toSec();
    if (!force && last_alignment_save_wall_time_ > 0.0 &&
        now - last_alignment_save_wall_time_ < 1.0)
      return true;

    double theta = 0.0, tx = 0.0, ty = 0.0, tz = 0.0;
    if (!globalEstimator_.GetTransform4D(theta, tx, ty, tz))
      return false;
    fusion_4dof::AlignmentState state;
    state.generation_sec = lio_generation_sec_;
    state.generation_nsec = lio_generation_nsec_;
    state.map_index = static_cast<int32_t>(map_index_);
    state.origin_latitude = origin_latitude_;
    state.origin_longitude = origin_longitude_;
    state.origin_height = origin_height_;
    state.origin_yaw = origin_gauss_yaw_;
    state.theta = theta;
    state.tx = tx;
    state.ty = ty;
    state.tz = tz;
    state.horizontal_locked = globalEstimator_.IsLocked();
    state.map_uuid = active_map_uuid_;
    state.coordinate_frame = fusion_4dof::kMapCoordinateFrame;
    state.origin_checksum = active_origin_checksum_;
    const fusion_4dof::AlignmentStateBytes data =
        fusion_4dof::EncodeAlignmentState(state);
    fusion_4dof::AlignmentState encoded_check;
    if (!fusion_4dof::DecodeAlignmentState(data, &encoded_check) ||
        !fusion_4dof::SameAlignmentIdentity(state, encoded_check)) {
      ROS_ERROR_THROTTLE(2.0,
                         "Refuse to persist non-canonical 4DoF map identity.");
      return false;
    }

    const std::string tmp_path = alignment_state_path_ + ".tmp";
    std::ofstream out(tmp_path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
      ROS_ERROR_THROTTLE(2.0, "Cannot open 4DoF state temp file: %s",
                         tmp_path.c_str());
      return false;
    }
    out.write(reinterpret_cast<const char *>(data.data()), data.size());
    out.flush();
    const bool write_ok = out.good();
    out.close();
    if (!write_ok || out.fail()) {
      ROS_ERROR_THROTTLE(2.0, "Failed to write complete 4DoF state: %s",
                         tmp_path.c_str());
      std::remove(tmp_path.c_str());
      return false;
    }
    if (std::rename(tmp_path.c_str(), alignment_state_path_.c_str()) != 0) {
      ROS_ERROR_THROTTLE(2.0,
                         "Failed to atomically replace 4DoF state %s.",
                         alignment_state_path_.c_str());
      std::remove(tmp_path.c_str());
      return false;
    }
    last_alignment_save_wall_time_ = now;
    return true;
  }

  bool RestoreAlignmentStateForCurrentGeneration() {
    if (!map_origin_ready_ || !lio_generation_valid_)
      return false;
    std::ifstream in(alignment_state_path_, std::ios::binary);
    if (!in.is_open())
      return false;
    fusion_4dof::AlignmentStateBytes data{};
    in.read(reinterpret_cast<char *>(data.data()), data.size());
    const std::streamsize count = in.gcount();
    char trailing = 0;
    const bool has_trailing = static_cast<bool>(in.read(&trailing, 1));
    in.close();
    if (count != static_cast<std::streamsize>(data.size()) || has_trailing) {
      ROS_WARN("Reject malformed 4DoF state %s (size/trailing mismatch).",
               alignment_state_path_.c_str());
      return false;
    }
    fusion_4dof::AlignmentState saved;
    if (!fusion_4dof::DecodeAlignmentState(data, &saved)) {
      ROS_WARN("Reject 4DoF state %s: magic/version/checksum invalid.",
               alignment_state_path_.c_str());
      return false;
    }
    if (saved.generation_sec != lio_generation_sec_ ||
        saved.generation_nsec != lio_generation_nsec_) {
      ROS_INFO("Ignore persisted 4DoF state from another LIO generation.");
      return false;
    }
    fusion_4dof::AlignmentState expected;
    expected.generation_sec = lio_generation_sec_;
    expected.generation_nsec = lio_generation_nsec_;
    expected.map_index = static_cast<int32_t>(map_index_);
    expected.origin_latitude = origin_latitude_;
    expected.origin_longitude = origin_longitude_;
    expected.origin_height = origin_height_;
    expected.origin_yaw = origin_gauss_yaw_;
    expected.map_uuid = active_map_uuid_;
    expected.coordinate_frame = fusion_4dof::kMapCoordinateFrame;
    expected.origin_checksum = active_origin_checksum_;
    const bool same_map = !expected.map_uuid.empty() &&
                          !expected.origin_checksum.empty() &&
                          fusion_4dof::SameAlignmentIdentity(saved, expected);
    if (!same_map) {
      ROS_WARN("Reject persisted 4DoF state: LIO generation or map "
               "UUID/frame/checksum/origin identity differs.");
      return false;
    }
    if (!globalEstimator_.RestoreTransform4D(
            saved.theta, saved.tx, saved.ty, saved.tz,
            saved.horizontal_locked)) {
      ROS_WARN("Reject persisted 4DoF state: transform values invalid.");
      return false;
    }
    // Same FAST-LIO generation + identical persisted map origin makes the
    // restored transform sufficient for continuous LIO fallback even if GPS
    // is temporarily unavailable during a fusion_only restart.
    system_init_ = true;
    heading_flag_ = 1;
    ROS_INFO("Restored same-generation 4DoF: theta=%.6f tx=%.3f ty=%.3f "
             "tz=%.3f locked=%d.",
             saved.theta, saved.tx, saved.ty, saved.tz,
             saved.horizontal_locked ? 1 : 0);
    return true;
  }

  // Observe FAST-LIO process identity independently of static readiness.  The
  // initial invalid ImuStaticCalibration marker is published at process start,
  // whereas the gravity-level transform is intentionally unavailable until a
  // stationary window succeeds.  Keeping those facts separate lets the
  // manager prove that a restart changed process generation without imposing
  // a sensor/operator-dependent deadline on static calibration.
  bool ObserveLioProcessGeneration(const ros::Time &generation) {
    if (generation.isZero())
      return false;
    const ros::Time current(lio_generation_sec_, lio_generation_nsec_);
    if (lio_generation_valid_ && generation < current)
      return false;
    const bool first = !lio_generation_valid_;
    const bool changed = lio_generation_valid_ && generation != current;
    if (!first && !changed)
      return true;

    {
      std::lock_guard<std::mutex> lock(gps_mutex_);
      std::queue<util::GpsPositionConstPtr> empty;
      gps_queue_.swap(empty);
      sync_failed_cnt_ = 0;
      gps_lio_sync_bad_first_steady_sec_ = 0.0;
    }
    imu_fallback_.InvalidateStaticCalibration();
    imu_calibration_applied_ = false;
    PublishInertialOdometryReady(false);
    applied_imu_calibration_generation_sec_ = 0;
    applied_imu_calibration_generation_nsec_ = 0;
    imu_calibration_frame_id_.clear();
    lio_generation_sec_ = generation.sec;
    lio_generation_nsec_ = generation.nsec;
    lio_generation_valid_ = true;
    lio_level_transform_ready_ = false;
    lio_static_initialized_ = false;

    if (changed) {
      globalEstimator_.ResetLioCoupled();
      gps_grace_anchor_valid_ = false;
      gps_grace_anchor_stamp_ = 0.0;
      previous_lio_raw_pose_.valid = false;
      has_prev_raw_lio_ = false;
      has_last_lio_seq_ = false;
      lio_seen_far_ = false;
      last_lio_stamp_ = 0.0;
      lio_drift_detected_ = false;
      lio_drift_count_ = 0;
      lio_failed_cnt_ = 0;
      lio_bad_first_steady_sec_ = 0.0;
      lio_drift_hold_end_stamp_ = 0.0;
      last_accepted_lio_input_stamp_ = 0.0;
      // Recovery proof is generation-local. A few fresh frames accumulated
      // before the restart must not release a stop in the new LIO generation.
      output_timestamp_stop_active_ = true;
      output_timestamp_resume_count_ = 0;
      PublishStopCar(true);
      stop_car_active_ = true;
      stop_car_resume_cnt_ = 0;
      stop_car_last_trusted_output_steady_sec_ = 0.0;
      if (controlled_lio_recovery_)
        lio_recovery_new_generation_received_ = true;
      ROS_WARN("New FAST-LIO process generation observed; invalidate previous "
               "4DoF alignment and wait for static calibration.");
    }
    return true;
  }

  void LioGenerationCallBack(
      const geometry_msgs::TransformStampedConstPtr &msg) {
    if (msg == nullptr) {
      ROS_ERROR_THROTTLE(1.0, "Reject null LIO generation transform.");
      return;
    }
    const auto &q = msg->transform.rotation;
    const double q_norm =
        std::sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    if (msg->header.stamp.isZero() || msg->header.frame_id != "lio_level" ||
        msg->child_frame_id != "lio_odom_raw" || !std::isfinite(q_norm) ||
        q_norm < 1e-6) {
      ROS_ERROR_THROTTLE(1.0, "Reject invalid LIO generation transform.");
      return;
    }
    const uint32_t sec = msg->header.stamp.sec;
    const uint32_t nsec = msg->header.stamp.nsec;
    const ros::Time incoming_generation(sec, nsec);
    const ros::Time current_generation(lio_generation_sec_,
                                       lio_generation_nsec_);
    // Usually the process-start calibration marker establishes identity first.
    // Still treat a transform carrying a different generation as the first
    // ready transform even if that marker was lost or delivered later.
    const bool first_level_transform =
        !lio_level_transform_ready_ || !lio_generation_valid_ ||
        incoming_generation != current_generation;
    if (!ObserveLioProcessGeneration(incoming_generation)) {
      ROS_ERROR_THROTTLE(1.0, "Reject stale LIO generation transform.");
      return;
    }
    lio_level_transform_ready_ = true;
    // FAST-LIO publishes this transform only after its quality-gated static
    // initialization and fixed gravity leveling have succeeded.  Therefore it
    // is also the authoritative, generation-tagged readiness event.  This
    // avoids a cross-topic startup race between the latched Bool and transform.
    lio_static_initialized_ = true;
    if (first_level_transform &&
        requested_restart_scope_ == util::LocalizationStatus::RESTART_LIO) {
      requested_restart_scope_ = util::LocalizationStatus::RESTART_NONE;
      algorithm_fault_code_ = util::LocalizationStatus::FAULT_NONE;
      algorithm_reason_ = "new FAST-LIO generation statically ready; realigning";
    }
    // The transform and calibration are separate latched topics. Either may
    // arrive first; apply only after their generation tokens agree.
    TryApplyLioStaticCalibration();
    // A duplicate latched message from the same generation must not replace a
    // newer in-memory solution with an older persisted snapshot.
    if (first_level_transform)
      RestoreAlignmentStateForCurrentGeneration();
  }

  void LioStaticStatusCallBack(const std_msgs::BoolConstPtr &msg) {
    if (msg == nullptr)
      return;
    if (!msg->data && !lio_level_transform_ready_) {
      lio_static_initialized_ = false;
      globalEstimator_.ResetLioCoupled();
      ROS_WARN_THROTTLE(1.0,
                        "FAST-LIO static initialization is not confirmed; "
                        "normal fusion pose output remains inhibited.");
    } else if (msg->data) {
      // Do not make the Bool alone sufficient: the generation-tagged level
      // transform must also have arrived before LIO callbacks are accepted.
      lio_static_initialized_ =
          lio_generation_valid_ && lio_level_transform_ready_;
      ROS_INFO("FAST-LIO static initialization confirmed; leveled LIO input "
               "may now become valid.");
    } else {
      // The status is monotonic within one FAST-LIO process.  A delayed latched
      // false from the Bool connection must not invalidate an already received
      // generation transform.  On a real FAST-LIO restart no leveled poses are
      // published before the new transform, whose changed token performs reset.
      ROS_WARN_THROTTLE(1.0,
                        "Ignore delayed static=false after a valid LIO "
                        "generation transform.");
    }
  }

  void LioCallBack(const util::LIOPoseConstPtr &msg) {
    // Is_rate_ok();

    if (pub_fusion != 2 &&
        (!lio_static_initialized_ || !lio_generation_valid_ ||
         !lio_level_transform_ready_)) {
      ROS_WARN_THROTTLE(1.0,
                        "Drop LIO/GPS fusion callback until FAST-LIO static "
                        "initialization and generation identity are ready.");
      return;
    }
    if (location_mode_ && !map_origin_ready_) {
      ROS_WARN_THROTTLE(1.0, "Keep fusion fail-closed until a valid map bundle is activated.");
      return;
    }
    if (!Is_LIO_USABLE(msg) || pub_fusion == 2) {
      return;
    }

    Eigen::Vector3d lio_t(msg->position_x, msg->position_y, msg->position_z);
    Eigen::Quaterniond lio_q;

    lio_q.w() = msg->q_w;
    lio_q.x() = msg->q_x;
    lio_q.y() = msg->q_y;
    lio_q.z() = msg->q_z;
    bool lio_state_ = msg->lio_state;
    DiagnosticFrame diagnostic;
    diagnostic.stamp = msg->header.stamp;
    diagnostic.seq = msg->header.seq;
    diagnostic.lio_level_imu_p = lio_t;
    diagnostic.lio_level_q = lio_q;
    diagnostic.lio_state = lio_state_;

    // ===== [RESET 状态] LIO 帧代际变化检测（/laserMapping 重启）=====
    // 三重判据，任一命中即判定 LIO 帧已重置，对 LIO↔map 对齐做外科手术式局部
    // 复位（清滑窗/4DoF、保留 GPS 原点、重置 LIO 速度/漂移基准）：
    //   (1) 自触发标志 lio_restart_pending_：本节点(/fusion_4dof)自己发起的LIO重启，
    //       重启前置位，重启后首帧 LIO 到来即确定性命中（最可靠）；
    //   (2) header.seq 回退：/laserMapping 重启后是新 publisher，seq 从 0 重新
    //       计数，故 cur_seq < last_seq 即代际变化（无需改消息/laserMapping）；
    //   (3) 位姿瞬时大跳变且回到原点附近：外部重启或 seq 不可用时的兜底，
    //       要求非物理单帧跳变以避免牛耕式正常驶回原点被误判。
    // 判定用原始 lio_t（LIOPrecess 之前）。
    {
      uint32_t cur_seq = msg->header.seq;
      double lio_norm = lio_t.norm();
      double lio_jump =
          has_prev_raw_lio_ ? (lio_t - prev_raw_lio_t_).norm() : 0.0;

      bool by_flag = lio_restart_pending_;
      bool by_seq = has_last_lio_seq_ && cur_seq < last_lio_seq_;
      bool by_jump = lio_seen_far_ && lio_norm < lio_epoch_near_thresh_ &&
                     lio_jump > lio_epoch_jump_thresh_;

      if (by_flag || by_seq || by_jump) {
        ROS_WARN("Detected LIO frame reset (flag=%d seq=%d jump=%d, jump=%.2fm): "
                 "surgical reset of LIO->map alignment.",
                 by_flag, by_seq, by_jump, lio_jump);
        globalEstimator_.ResetLioCoupled();
        gps_grace_anchor_valid_ = false;
        gps_grace_anchor_stamp_ = 0.0;
        previous_lio_raw_pose_.valid = false;
        lio_restart_pending_ = false;
        lio_seen_far_ = false;
        last_lio_stamp_ = 0.0;   // 重置速度检测基准，避免本帧误判漂移
        lio_drift_detected_ = false;
        lio_drift_count_ = 0;
        lio_failed_cnt_ = 0;
        lio_bad_first_steady_sec_ = 0.0;
        lio_drift_hold_end_stamp_ = 0.0;
        sync_failed_cnt_ = 0;
        gps_lio_sync_bad_first_steady_sec_ = 0.0;
      }

      if (lio_norm > lio_epoch_far_thresh_)
        lio_seen_far_ = true;
      prev_raw_lio_t_ = lio_t; // 记录“原始” LIO 位置（LIOPrecess 修改前）
      has_prev_raw_lio_ = true;
      last_lio_seq_ = cur_seq;
      has_last_lio_seq_ = true;
    }

    double xyy_lio[4]; // [0]x [1]y [2]高程 [3]航向（官方 GetTransform 映射）
    memset(xyy_lio, 0, sizeof(double) * 4);
    double xyy_seed[4]; // 同上，但用 GetSeedOnlyTransform（纯 RTK 种子，见 README §11）
    memset(xyy_seed, 0, sizeof(double) * 4);
    lio_raw_x_ = 0;
    lio_raw_y_ = 0;
    lio_raw_yaw_ = 0;
    bool seed_transform_available = false;
    const bool lio_enu_transform_available =
        LIOPrecess(lio_t, lio_q, xyy_lio, xyy_seed,
                   &seed_transform_available);
    const fusion_4dof::TimedLioRawPose preceding_lio_raw_pose =
        previous_lio_raw_pose_;
    fusion_4dof::TimedLioRawPose current_lio_raw_pose;
    // Provisional until the LIO quality/speed/drift gates below accept it.
    // Early-return paths must never leave an unhealthy sample in history.
    current_lio_raw_pose.valid = false;
    current_lio_raw_pose.generation_sec = lio_generation_sec_;
    current_lio_raw_pose.generation_nsec = lio_generation_nsec_;
    current_lio_raw_pose.stamp = lio_stamp_;
    current_lio_raw_pose.x = lio_t.x();
    current_lio_raw_pose.y = lio_t.y();
    current_lio_raw_pose.z = lio_t.z();
    current_lio_raw_pose.yaw = lio_raw_yaw_;
    previous_lio_raw_pose_ = current_lio_raw_pose;
    diagnostic.lio_h_base_p = lio_t;
    diagnostic.lio_h_base_rpy =
        Eigen::Vector3d(lio_raw_roll_, lio_raw_pitch_, lio_raw_yaw_);
    std::copy(xyy_lio, xyy_lio + 4, diagnostic.lio_map.begin());

    // Publish LIO position.
    if (!reboot && pub_fusion == 3 && lio_state_) {
      util::Position lio_position;
      lio_position.header.stamp = msg->header.stamp;
      lio_position.header.frame_id = "base_link";
      lio_position.position_x = xyy_lio[0];
      lio_position.position_y = xyy_lio[1];
      lio_position.position_z = xyy_lio[2];
      double output_roll = 0.0, output_pitch = 0.0;
      if (!SelectOutputRollPitch(msg->header.stamp, true, lio_raw_roll_,
                                 lio_raw_pitch_, &output_roll,
                                 &output_pitch))
        return;
      double output_yaw = xyy_lio[3];
      if (!CanonicalizeOutputRPY(output_roll, output_pitch, output_yaw,
                                 &output_roll, &output_pitch, &output_yaw))
        return;
      lio_position.roll = output_roll;
      lio_position.pitch = output_pitch;
      lio_position.yaw = output_yaw;
      lio_position.position_state = 4;
      diagnostic.output_valid = true;
      diagnostic.position_state = 4;
      diagnostic.output = {xyy_lio[0], xyy_lio[1], xyy_lio[2], output_roll,
                           output_pitch, output_yaw};
      diagnostic.lio_input = true;
      diagnostic.lio_speed_good = true;
      MaybeLog4DoFDiagnostic(diagnostic, "pure_lio");
      if (!PublishProductionPosition(lio_position))
        return;
      // [viz] 纯 LIO 模式：同时画 LIO 轨迹与输出轨迹 + TF。
      PublishVizPose(viz_lio_, msg->header.stamp, xyy_lio[0], xyy_lio[1],
                     xyy_lio[2], output_roll, output_pitch, output_yaw);
      PublishVizPose(viz_output_, msg->header.stamp, xyy_lio[0], xyy_lio[1],
                     xyy_lio[2], output_roll, output_pitch, output_yaw);
      BroadcastBaseLinkTF(msg->header.stamp, xyy_lio[0], xyy_lio[1], xyy_lio[2],
                          output_roll, output_pitch, output_yaw);
      publish_count++;
      return;
    }

    // LIO速度判断
    double lio_speed = 0.0;
    double lio_pos_yaw = 0.0;
    // lio_speed_flag_：当前帧的LIO状态，lio_drift_detected_：当前帧或漂移期设置的LIO的状态
    bool lio_speed_flag_ =
        lio_state_ ? Is_LIO_GOOD(lio_t, lio_stamp_, &lio_speed,
                                 lio_raw_yaw_, &lio_pos_yaw)
                   : false;
    latest_lio_good_ = lio_state_ && lio_speed_flag_;
    if (latest_lio_good_)
      last_lio_good_steady_sec_ = ros::SteadyTime::now().toSec();
    diagnostic.lio_speed = lio_speed;
    diagnostic.lio_speed_good = lio_speed_flag_;

    if (lio_drift_count_ > 0 &&
        lio_stamp_ >= lio_drift_hold_end_stamp_)
      lio_drift_count_ = 0;
    lio_drift_detected_ = lio_drift_count_ > 0 ? true : lio_drift_detected_;
    lio_drift_count_ =
        lio_drift_count_ > 0 ? --lio_drift_count_ : lio_drift_count_;

    // LIO数据输入
    bool inputLIO_flag_ = false;
    if (lio_state_ && lio_speed_flag_) {
      if (system_init_ && !lio_drift_detected_) {
        globalEstimator_.InputOdom(lio_stamp_, lio_t, lio_q);
        inputLIO_flag_ = true;
        diagnostic.lio_input = true;
      }
      lio_failed_cnt_ = 0;
      lio_bad_first_steady_sec_ = 0.0;
    } else {
      lio_failed_cnt_++;
      const double bad_now = ros::SteadyTime::now().toSec();
      if (lio_bad_first_steady_sec_ <= 0.0)
        lio_bad_first_steady_sec_ = bad_now;
      if (lio_failed_cnt_ >= lio_restart_after_bad_frames_ ||
          bad_now - lio_bad_first_steady_sec_ >= lio_bad_max_duration_sec_)
      {
        if (controlled_lio_recovery_) {
          lio_failed_cnt_ = 0;
          lio_bad_first_steady_sec_ = 0.0;
          algorithm_reason_ = "new LIO generation remains unhealthy during "
                              "the controlled recovery grace window";
          return;
        }
        // [死角判定] RTK 也处于失效状态(gps_failed_flag_)：仅重启 LIO 后无
        // RTK 可重锚 LIO->map，因此进入 FAULT：立即停车并请求 manager 联合重启。
        if (gps_failed_flag_) {
          // Stop after the configured frame/time limit, while the independent
          // timer verifies total GPS+LIO loss before emergency-source action.
          PublishStopCar(true);
          stop_car_active_ = true;
          algorithm_reason_ =
              "LIO bad and RTK unavailable; waiting for total-source-loss timeout";
          return;
        }
        // [RECOVER] RTK尚可用：只重启/laserMapping；/fusion_4dof保持运行，
        // 并在新LIO代际到达后局部清空、重新完成LIO->ENU 4DoF对齐。
        lio_restart_pending_ = true; // 自触发：重启后首帧新 LIO 即确定性复位
        PublishStopCar(true);        // 重启 /laserMapping 前请求停车，避免移动中重启 LIO
        stop_car_active_ = true;
        stop_car_resume_cnt_ = 0;
        stop_car_last_trusted_output_steady_sec_ = 0.0;
        ROS_WARN("LIO data are not good, restart the node /lasermapping !");
        RequestLioRestart("LIO quality remained invalid beyond the configured frame/time limit.");
        globalEstimator_.RequestClear();
        // 重置lio相关状态！！！
        lio_drift_detected_ = false;
        lio_drift_count_ = 0;
        lio_failed_cnt_ = 0;
        lio_bad_first_steady_sec_ = 0.0;
        sync_failed_cnt_ = 0;
        fusion_failed_cnt_ = 0;
        return;
      }

      // if (lio_failed_cnt_ >= 20) //
      // 可能导致当RTK数据较差时只能重启节点但由于无法得到RTK定位而无法得到定位结果，仅重启/lasermapping？？？
      // {
      //     lio_bad_flag_ = true;
      // }

      // if (location_mode_ && lio_failed_cnt_ >= 20)
      // {
      //     reboot = true;
      //     ROS_WARN("LIO raw data are not good, restart the node!");
      //     return;
      // }
    }
    current_lio_raw_pose.valid = inputLIO_flag_;
    previous_lio_raw_pose_.valid = inputLIO_flag_;

    if (lio_failed_cnt_ >= lio_drift_hold_after_bad_frames_) {
      lio_drift_count_ = lio_drift_hold_frames_;
      lio_drift_hold_end_stamp_ =
          lio_stamp_ + lio_drift_hold_max_duration_sec_;
    }
    //------------------------------LIO数据处理----------------------------------//

    //------------------------------GPS数据处理----------------------------------//
    double latitude = 0., longitude = 0., altitude = 0., gps_flag = 0.,
           gauss_yaw = 0., gps_stamp = 0.;
    double gps_height_std = std::numeric_limits<double>::quiet_NaN();
    bool gps_yaw_valid = false;
    ros::Time gps_msg_stamp(0);
    util::GpsPositionConstPtr gps_msg;

    // 从当前已到达的 GPS 队列中选与本帧 LIO 时间最近的一条，而不是旧实现中
    // “窗口内第一条”。GPS 约 20Hz、LIO 约 10Hz 时，取第一条会稳定落后
    // 30~80ms。候选帧在本次选择后全部消费，窗口上界之外的未来帧留给下一帧
    // LIO；这样既不复用 GPS 观测，也不会让队列按 GPS-LIO 频差持续增长。
    bool waiting_for_future_gps = false;
    std::size_t stale_gps_count = 0;
    double best_abs_dt = std::numeric_limits<double>::infinity();
    {
      std::lock_guard<std::mutex> lock(gps_mutex_);
      while (!gps_queue_.empty()) {
        const util::GpsPositionConstPtr candidate = gps_queue_.front();
        const double candidate_stamp = candidate->header.stamp.toSec();
        const double candidate_dt = candidate_stamp - lio_stamp_;

        if (candidate_dt < -gps_lio_sync_tolerance_sec_) {
          // 已经不可能与当前或后续 LIO 成为容差内最近邻，直接丢弃。
          ++stale_gps_count;
          gps_queue_.pop();
          continue;
        }
        if (candidate_dt > gps_lio_sync_tolerance_sec_) {
          // 队列按时间有序；后续帧只会更晚，保留到下一次 LIO 回调。
          waiting_for_future_gps = true;
          break;
        }

        const double abs_dt = std::fabs(candidate_dt);
        if (abs_dt < best_abs_dt) {
          best_abs_dt = abs_dt;
          gps_msg = candidate;
        }
        gps_queue_.pop();
      }
    }

    if (gps_msg != nullptr) {
      gps_msg_stamp = gps_msg->header.stamp;
      gps_stamp = gps_msg->header.stamp.toSec();
      diagnostic.gps_synced = true;
      diagnostic.gps_stamp = gps_stamp;
      diagnostic.gps_lio_dt = gps_stamp - lio_stamp_;
      latitude = gps_msg->latitude;
      longitude = gps_msg->longitude;
      altitude = gps_msg->height;
      gps_flag = gps_msg->gps_flag;
      gps_height_std = gps_msg->INS_Std_LocatHeight;
      const fusion_4dof::RtkQualityObservation rtk_quality =
          RtkQualityOf(*gps_msg);
      gps_yaw_valid = fusion_4dof::IsRtkHeadingQualityGood(
          rtk_quality, rtk_quality_config_);
      heading_flag_ = gps_yaw_valid ? 1 : 0;
      diagnostic.position_status = gps_msg->positionStatus;
      diagnostic.ins_status = gps_msg->INS_Status;
      diagnostic.gps_heading_flag = gps_msg->INS_GpsFlag_Heading;
      diagnostic.gps_position_flag = gps_msg->INS_GpsFlag_Pos;
      diagnostic.gps_num_sv = gps_msg->INS_NumSV;
      diagnostic.gps_age = gps_msg->INS_Gps_Age;
      diagnostic.gps_confidence = gps_msg->gps_confidence;
      diagnostic.vehicle_align = gps_msg->INS_VehicleAlign;
      diagnostic.gps_lat_std = gps_msg->INS_Std_Lat;
      diagnostic.gps_lon_std = gps_msg->INS_Std_Lon;
      diagnostic.gps_heading_std = gps_msg->INS_Std_Heading;
      diagnostic.azimuth_raw = gps_msg->azimuth;
      diagnostic.ins_raw_roll = gps_msg->roll;
      diagnostic.ins_raw_pitch = gps_msg->pitch;
      diagnostic.ins_acc =
          Eigen::Vector3d(gps_msg->acc_x, gps_msg->acc_y, gps_msg->acc_z);
      diagnostic.ins_gyro =
          Eigen::Vector3d(gps_msg->rot_x, gps_msg->rot_y, gps_msg->rot_z);
      diagnostic.ins_velocity =
          Eigen::Vector3d(gps_msg->eastVelocity, gps_msg->northVelocity,
                          gps_msg->upVelocity);
      double converted_roll = 0.0, converted_pitch = 0.0;
      if (ConvertInsRollPitch(*gps_msg, &converted_roll, &converted_pitch)) {
        diagnostic.ins_converted_roll = converted_roll;
        diagnostic.ins_converted_pitch = converted_pitch;
      }
      gauss_yaw = gps_msg->azimuth - 9000;
      if (gauss_yaw < 0) {
        gauss_yaw += 36000;
      }
      gauss_yaw *= 0.01;
    }

    // Count at most once for this LIO callback.  stale_gps_count is diagnostic
    // evidence only; its magnitude must not turn queue backlog into a false
    // "50 consecutive LIO frames" recovery request.
    sync_failed_cnt_ = fusion_4dof::UpdateGpsLioSyncFailureCount(
        sync_failed_cnt_, gps_msg != nullptr, stale_gps_count,
        waiting_for_future_gps);

    const double sync_now = ros::SteadyTime::now().toSec();
    if (sync_failed_cnt_ == 0)
      gps_lio_sync_bad_first_steady_sec_ = 0.0;
    else if (gps_lio_sync_bad_first_steady_sec_ <= 0.0)
      gps_lio_sync_bad_first_steady_sec_ = sync_now;
    if (sync_failed_cnt_ >= gps_lio_sync_restart_after_frames_ ||
        (gps_lio_sync_bad_first_steady_sec_ > 0.0 &&
         sync_now - gps_lio_sync_bad_first_steady_sec_ >=
             gps_lio_sync_bad_max_duration_sec_)) {
      if (controlled_lio_recovery_) {
        sync_failed_cnt_ = 0;
        gps_lio_sync_bad_first_steady_sec_ = 0.0;
        algorithm_reason_ = "new LIO generation is not yet GPS-synchronized "
                            "during the controlled recovery grace window";
        return;
      }
      // [死角判定] RTK 也失效：不做无效的 LIO-only 恢复，进入 FAULT
      // 并请求 manager 联合重启。
      if (gps_failed_flag_) {
        PublishStopCar(true);
        stop_car_active_ = true;
        algorithm_reason_ = "GPS/LIO sync and RTK unavailable; waiting for "
                            "total-source-loss timeout";
        return;
      }
      // reboot = true;
      // ROS_WARN("---sync failed, restart the node!");

      // std_msgs::Bool gps_reboot_msg;
      // gps_reboot_msg.data = true;
      // pub_gps_Reboot_.publish(gps_reboot_msg);
      // return;

      lio_restart_pending_ = true; // 自触发：重启后首帧新 LIO 即确定性复位
      PublishStopCar(true);        // 重启 /laserMapping 前请求停车，避免移动中重启 LIO
      stop_car_active_ = true;
      stop_car_resume_cnt_ = 0;
      stop_car_last_trusted_output_steady_sec_ = 0.0;
      ROS_WARN("---sync failed, restart the node /lasermapping !");
      RequestLioRestart("GPS/LIO synchronization failed beyond the configured frame/time limit.");
      globalEstimator_.RequestClear();
      // 重置lio相关状态！！！
      lio_drift_detected_ = false;
      lio_drift_count_ = 0;
      lio_failed_cnt_ = 0;
      sync_failed_cnt_ = 0;
      gps_lio_sync_bad_first_steady_sec_ = 0.0;
      fusion_failed_cnt_ = 0;
      return;
    }

    // GPS状态检测和速度判断
    bool gps_good_flag = false;
    double gps_speed = 0.0;
    double gps_pos_yaw = 0.0;
    double location_gps[4]; // [0]x [1]y [2]高程 [3]航向
    memset(location_gps, 0, sizeof(double) * 4);
    // gps_stamp = gps_flag > 0 ? lio_stamp_ : 0;
    // gps_good_flag：当前帧或漂移期设置的GPS的状态，不能代表当前帧的GPS状态
    const bool receiver_quality_good =
        gps_msg != nullptr && fusion_4dof::IsRtkPositionQualityGood(
                                  RtkQualityOf(*gps_msg), rtk_quality_config_);
    fusion_4dof::GpsMotionResult gps_motion_result =
        fusion_4dof::GpsMotionResult::kInvalid;
    const double lio_pair_dt = current_lio_raw_pose.stamp -
                               preceding_lio_raw_pose.stamp;
    const bool lio_turn_rate_trusted =
        preceding_lio_raw_pose.valid && current_lio_raw_pose.valid &&
        preceding_lio_raw_pose.generation_sec ==
            current_lio_raw_pose.generation_sec &&
        preceding_lio_raw_pose.generation_nsec ==
            current_lio_raw_pose.generation_nsec &&
        lio_pair_dt > 0.0 &&
        lio_pair_dt <= lio_time_alignment_max_pair_dt_sec_;
    const double lio_turn_rate_radps = lio_turn_rate_trusted
        ? std::fabs(fusion_4dof::WrapAngle(
              current_lio_raw_pose.yaw - preceding_lio_raw_pose.yaw)) /
              lio_pair_dt
        : std::numeric_limits<double>::quiet_NaN();
    // LIO runs even when no RTK sample is selected. A turn in such a gap
    // must still break the GPS straight-course window; otherwise the next
    // low-rate sample can compare positions from opposite sides of a turn.
    if (lio_turn_rate_trusted &&
        lio_turn_rate_radps >= gps_course_axis_turn_rate_radps_)
      gps_course_axis_gate_.Reset();
    if (receiver_quality_good) {
      gps_motion_result =
          CheckGpsMotion(latitude, longitude, altitude, gps_flag, gauss_yaw,
                         gps_yaw_valid, lio_turn_rate_radps,
                         lio_turn_rate_trusted,
                         gps_stamp, location_gps, &gps_speed,
                         &gps_pos_yaw, &diagnostic.gps_motion_dt);
    } else {
      gps_course_axis_gate_.Reset();
    }
    diagnostic.gps_motion_result = gps_motion_result;
    diagnostic.gps_motion_abs_dt = std::isfinite(diagnostic.gps_motion_dt)
                                       ? std::fabs(diagnostic.gps_motion_dt)
                                       : diagnostic.gps_motion_dt;
    gps_good_flag =
        gps_motion_result == fusion_4dof::GpsMotionResult::kAccepted;
    diagnostic.gps_speed = gps_speed;
    diagnostic.gps_motion_yaw_deg = gps_pos_yaw;

    bool gps_sample_valid =
        gps_good_flag && gps_stamp > 0.0 && std::isfinite(latitude) &&
        std::isfinite(longitude) && std::isfinite(altitude) &&
        gps_flag == rtk_quality_config_.required_gps_flag;
    bool vertical_sample_valid = false;
    bool gps_estimator_accepted = false;
    bool gps_alignment_committed_this_frame = false;
    if (gps_sample_valid && system_init_) {
      vertical_sample_valid =
          std::isfinite(location_gps[2]) && std::isfinite(altitude) &&
          (!vertical_require_ins_height_std_ ||
           (std::isfinite(gps_height_std) && gps_height_std >= 0.0 &&
            gps_height_std <= vertical_max_ins_height_std_));
      fusion_4dof::GpsOutageState pre_observation_state;
      pre_observation_state.stable_count = gps_stable_cnt_;
      pre_observation_state.transient_reacquire_count =
          gps_transient_reacquire_cnt_;
      pre_observation_state.failed = gps_failed_flag_;
      pre_observation_state.transient_fallback = gps_transient_fallback_;
      const bool commit_if_accepted = fusion_4dof::ShouldCommitAcceptedGps(
          pre_observation_state, gps_stable_threshold_,
          gps_transient_reacquire_frames_);
      gps_estimator_accepted = globalEstimator_.InputGPS(
          gps_stamp, location_gps[0], location_gps[1], location_gps[2],
          location_gps[3], gps_yaw_valid, vertical_sample_valid,
          commit_if_accepted);
      gps_sample_valid = gps_estimator_accepted;
      gps_alignment_committed_this_frame =
          gps_estimator_accepted && commit_if_accepted;
      if (!gps_estimator_accepted) {
        ROS_WARN_THROTTLE(
            1.0,
            "Reject synchronized RTK before candidate window: inconsistent "
            "with the last committed 4DoF/LIO trajectory.");
      }
    }
    latest_gps_good_ = gps_sample_valid;
    if (latest_gps_good_)
      last_gps_good_steady_sec_ = ros::SteadyTime::now().toSec();
    diagnostic.gps_good = gps_good_flag;
    diagnostic.gps_sample_valid = gps_sample_valid;
    diagnostic.gps_yaw_valid = gps_yaw_valid;
    diagnostic.latitude = latitude;
    diagnostic.longitude = longitude;
    diagnostic.height = altitude;
    diagnostic.gps_flag = static_cast<int>(gps_flag);
    diagnostic.gps_height_std = gps_height_std;
    std::copy(location_gps, location_gps + 4, diagnostic.gps_enu.begin());

    // Never let one unusable RTK observation switch source. Missing transport,
    // cadence/baseline gaps, receiver-quality failures, implausible motion,
    // and innovation rejection all quarantine the current RTK sample and use
    // the same bounded RTK-anchor/LIO coast. The fourth consecutive unusable
    // observation (or the measurement-time cap) confirms full failure.
    const bool gps_observation_missing = gps_msg == nullptr;
    fusion_4dof::GpsOutageState outage_state;
    outage_state.stable_count = gps_stable_cnt_;
    outage_state.failed_count = gps_failed_cnt_;
    outage_state.missing_count = gps_missing_cnt_;
    outage_state.transient_reacquire_count = gps_transient_reacquire_cnt_;
    outage_state.failed = gps_failed_flag_;
    outage_state.transient_fallback = gps_transient_fallback_;
    const bool gps_motion_baseline_only =
        receiver_quality_good &&
        fusion_4dof::IsGpsMotionBaselineOnly(gps_motion_result);
    const bool gps_heading_suspect_observation =
        receiver_quality_good &&
        gps_motion_result == fusion_4dof::GpsMotionResult::kHeadingSuspect;
    const bool gps_heading_conflict_observation =
        receiver_quality_good &&
        gps_motion_result == fusion_4dof::GpsMotionResult::kHeadingMismatch;
    const fusion_4dof::GpsObservation observation =
        fusion_4dof::ClassifyGpsObservation(
            !gps_observation_missing, receiver_quality_good,
            gps_motion_baseline_only, gps_sample_valid,
            gps_heading_suspect_observation);
    fusion_4dof::GpsOutageResult outage_result =
        fusion_4dof::AdvanceGpsOutageState(
            observation, system_init_, gps_stable_threshold_,
            gps_recovery_floor_, gps_missing_grace_frames_,
            gps_transient_reacquire_frames_, &outage_state);
    const bool gps_coast_observation =
        observation == fusion_4dof::GpsObservation::kMissing ||
        observation == fusion_4dof::GpsObservation::kBaselineOnly ||
        observation == fusion_4dof::GpsObservation::kHeadingSuspect ||
        observation == fusion_4dof::GpsObservation::kRejected;
    // Frame count alone is not a real-time safety bound if LIO slows down.
    // The transport-only coast has a short time cap. Provisional heading
    // evidence has its own, longer but still finite cap so it cannot spend
    // the three-frame missing-data budget before three independent checks.
    const bool gps_heading_evidence_observation =
        gps_heading_suspect_observation || gps_heading_conflict_observation;
    const double coast_time_limit = gps_heading_evidence_observation
        ? gps_heading_suspect_coast_max_duration_sec_
        : gps_coast_max_duration_sec_;
    if (gps_grace_anchor_valid_ &&
        (!lio_generation_valid_ ||
         gps_grace_anchor_generation_sec_ != lio_generation_sec_ ||
         gps_grace_anchor_generation_nsec_ != lio_generation_nsec_))
      gps_grace_anchor_valid_ = false;
    if (system_init_ && gps_coast_observation &&
        !gps_grace_anchor_valid_ && !outage_state.failed &&
        !outage_state.transient_fallback &&
        output_continuity_gate_.valid() &&
        output_continuity_gate_.last_source() == 1) {
      // An already-published RTK mode cannot be safely extended without a
      // same-generation anchor. Fall back explicitly instead of silently
      // returning no position for an unbounded number of callbacks.
      outage_state.failed = true;
      outage_state.stable_count = gps_recovery_floor_;
      outage_state.transient_reacquire_count = 0;
      outage_result.clear_candidate_window = true;
      outage_result.entered_full_failure = true;
      ROS_WARN("RTK coast anchor unavailable; enter explicit LIO fallback.");
    }
    if (system_init_ && gps_coast_observation &&
        gps_grace_anchor_valid_ && !outage_state.failed &&
        !outage_state.transient_fallback &&
        fusion_4dof::GpsCoastTimeExpired(
            lio_stamp_, gps_grace_anchor_stamp_, coast_time_limit)) {
      outage_state.failed = true;
      outage_state.stable_count = gps_recovery_floor_;
      outage_state.transient_reacquire_count = 0;
      outage_result.clear_candidate_window = true;
      outage_result.entered_full_failure = true;
      ROS_WARN("RTK coast exceeded %.2f s measurement-time limit (%s).",
               coast_time_limit,
               gps_heading_evidence_observation ? "heading evidence"
                                                : "RTK unavailable");
    }
    gps_stable_cnt_ = outage_state.stable_count;
    gps_failed_cnt_ = outage_state.failed_count;
    gps_missing_cnt_ = outage_state.missing_count;
    gps_transient_reacquire_cnt_ = outage_state.transient_reacquire_count;
    gps_failed_flag_ = outage_state.failed;
    gps_transient_fallback_ = outage_state.transient_fallback;
    if (outage_result.clear_candidate_window)
      globalEstimator_.ClearGpsWindowKeepTransform();

    if (outage_result.entered_full_failure) {
      if (gps_coast_observation) {
        ROS_WARN("RTK accepted sample unavailable beyond %d-frame/time grace: switch to "
                 "frozen 4DoF/LIO; full recovery requires %d consecutive "
                 "accepted samples.",
                 gps_missing_grace_frames_,
                 gps_stable_threshold_ - gps_recovery_floor_);
      }
    } else if (gps_observation_missing && !gps_failed_flag_) {
      ROS_WARN_THROTTLE(
          1.0,
          "RTK synchronized sample missing (%d/%d grace): keep RTK mode "
          "with last-RTK-anchored LIO propagation; do not switch source.",
          gps_missing_cnt_, gps_missing_grace_frames_);
    } else if (gps_heading_suspect_observation && !gps_failed_flag_) {
      ROS_WARN_THROTTLE(1.0, "RTK heading suspect: coast from last trusted "
                             "RTK anchor, awaiting independent evidence; "
                             "do not consume transport-missing frame grace.");
    } else if (gps_motion_baseline_only && !gps_failed_flag_) {
      ROS_WARN_THROTTLE(
          1.0,
          "RTK motion baseline only: result=%s signed_dt=%.6f abs_dt=%.6f "
          "required=(%.6f, %.6f); preserve GPS stability count.",
          fusion_4dof::GpsMotionResultName(gps_motion_result),
          diagnostic.gps_motion_dt, diagnostic.gps_motion_abs_dt,
          gps_motion_min_dt_sec_, gps_motion_max_dt_sec_);
    } else if (observation == fusion_4dof::GpsObservation::kRejected &&
               !gps_failed_flag_) {
      ROS_WARN_THROTTLE(
          1.0,
          "RTK sample rejected and quarantined (%d/%d grace): keep RTK mode "
          "with last-trusted-RTK/LIO propagation; never publish rejected RTK.",
          gps_missing_cnt_, gps_missing_grace_frames_);
    }
    if (outage_result.transient_recovered) {
      ROS_INFO("RTK transient gap recovered after %d consecutive accepted "
               "samples.",
               gps_transient_reacquire_frames_);
    }
    //------------------------------GPS数据处理----------------------------------//

    // 长时间无RTK信号，加载上次保存的定位数据作为当前位置
    // if (location_mode_ && no_rtk_cnt_ == 40) //
    // {
    //     double last_pub_pos[3] = {0.0, 0.0, 0.0};
    //     if (LoadFusionPositionFromFile(last_pub_pos))
    //     {
    //         ROS_INFO("load last published position: %f, %f, %f",
    //         last_pub_pos[0], last_pub_pos[1], last_pub_pos[2]); no_rtk_cnt_ =
    //         0; geoConverter_.Reverse(last_pub_pos[0], last_pub_pos[1], 0.0,
    //         latitude, longitude, altitude); gps_flag = 4; gauss_yaw =
    //         last_pub_pos[2]; gps_good_flag = true;
    //     }
    //     else
    //     {
    //         ROS_WARN("Failed to load last published position, using default
    //         values.");
    //     }
    // }

    // 初始化及GPS数据输入
    bool inputGPS_flag_ = false;

    if (gps_sample_valid && !system_init_ &&
        gps_stable_cnt_ >= gps_stable_threshold_) {
      if (!location_mode_) {
        // Commit the map origin only at the threshold-crossing stable sample.
        // The provisional converter used by GPS quality checks is reset to
        // this exact sample below, so this run and later map reuse are equal.
        if (!SaveMap(latitude, longitude, altitude, gauss_yaw)) {
          // 建图原点没有可靠落盘时，不能宣告系统初始化完成。
          ROS_ERROR_THROTTLE(
              1.0,
              "Failed to persist create-mode map origin to %s; keep system "
              "uninitialized and retry on the next accepted GPS sample.",
              map_path_.c_str());
          MaybeLog4DoFDiagnostic(diagnostic, "map_save_failed");
          return;
        }
        origin_latitude_ = latitude;
        origin_longitude_ = longitude;
        origin_height_ = altitude;
        origin_gauss_yaw_ = gauss_yaw;
        map_origin_ready_ = true;
        // Publish the durable-commit boundary immediately so the live manager
        // can choose TRACK_SAVED_ORIGIN for a subsequent recovery.
        PublishAlgorithmStatus();
      }
      system_init_ = true;
      // Rebuild from the committed origin. In create mode this replaces the
      // provisional quality-check origin with the exact just-saved origin; in
      // location mode it deterministically reloads the saved map origin.
      gps_init_ = false;
      GPS2Local(latitude, longitude, altitude, gauss_yaw, location_gps);
      // Create mode commits this very RTK fix as the durable ENU origin.
      // Motion history accumulated in the provisional first-fix frame is
      // now in a different coordinate system; rebasing it prevents a false
      // high-speed/heading rejection on the next RTK sample.
      last_gps_stamp_ = gps_stamp;
      last_gps_[0] = location_gps[0];
      last_gps_[1] = location_gps[1];
      last_gps_[2] = location_gps[2];
      gps_course_axis_gate_.Reset();
      std::copy(location_gps, location_gps + 4, diagnostic.gps_enu.begin());
      gps_failed_cnt_ = 0;
      gps_stable_cnt_ = gps_stable_threshold_;
      gps_failed_flag_ = false;
      no_rtk_cnt_ = 0;
    } else {
      if (!system_init_) {
        no_rtk_cnt_++;
        gps_drift_count_ = 0; //???
        lio_drift_count_ = 0;
        lio_drift_detected_ = false;
        ROS_WARN("GPS data is not good, please check the GPS module!");
        MaybeLog4DoFDiagnostic(diagnostic, "wait_system_init");
        return;
      }
    }

    if (gps_sample_valid && system_init_) {
      std::copy(location_gps, location_gps + 4, diagnostic.gps_enu.begin());
      // 传入 GPS2Local 已算好的 ENU 平面位置（location_gps[0]=East,[1]=North）与
      // ENU 航向 location_gps[3]，与 GPS 对外输出同一帧，保证 SE(2) 回退帧一致。
      // 单天线发布器的positionStatus依赖车辆运动，不再作为4DoF启动硬门控。
      // 种子航向仍须通过完整RTK/INS位置质量和DRPVA航向标准差门控；否则水平
      // 位置样本只能在运动基线充足后完成对齐。
      // （冷启动时 SE(2) 尚未由轨迹估出旋转，估计器即用此 RTK 航向作种子定 LIO->ENU）
      // 失效恢复期只积累候选 GPS<->LIO 配对，严禁修改已提交 SE(2)。达到稳定
      // 门限的同一帧才一次性用完整候选窗重估并提交；随后下方状态机也在同一
      // 回调切到 position_state=1，保证“坐标来源变化”和“状态变化”原子一致。
      const bool commit_alignment =
          (!gps_failed_flag_ && !gps_transient_fallback_) ||
          (gps_failed_flag_ &&
           gps_stable_cnt_ >= gps_stable_threshold_);
      if (!gps_estimator_accepted) {
        vertical_sample_valid =
            std::isfinite(location_gps[2]) && std::isfinite(altitude) &&
            (!vertical_require_ins_height_std_ ||
             (std::isfinite(gps_height_std) && gps_height_std >= 0.0 &&
              gps_height_std <= vertical_max_ins_height_std_));
        gps_estimator_accepted = globalEstimator_.InputGPS(
            gps_stamp, location_gps[0], location_gps[1], location_gps[2],
            location_gps[3], gps_yaw_valid, vertical_sample_valid,
            commit_alignment);
        gps_alignment_committed_this_frame =
            gps_estimator_accepted && commit_alignment;
      }
      diagnostic.gps_z_valid = vertical_sample_valid;
      if (!vertical_sample_valid) {
        ROS_WARN_THROTTLE(
            2.0,
            "Reject GPS height for tz update: INS height std=%.3f (required=%d "
            "max=%.3f). Horizontal alignment may still update.",
            gps_height_std, vertical_require_ins_height_std_ ? 1 : 0,
            vertical_max_ins_height_std_);
      }
      if (commit_alignment && globalEstimator_.Is4DAligned() &&
          !active_map_uuid_.empty() && !map_switch_pending_ &&
          !SaveAlignmentState()) {
        ROS_WARN_THROTTLE(
            5.0,
            "4DoF is valid but its same-generation restart state was not "
            "persisted; fusion_only restart may need GPS to re-align.");
      }
      inputGPS_flag_ = gps_estimator_accepted;
      if (gps_failed_flag_ && !commit_alignment) {
        ROS_INFO_THROTTLE(
            1.0,
            "GPS recovery candidate %d/%d: committed 4DoF remains frozen.",
            gps_stable_cnt_ - gps_recovery_floor_,
            gps_stable_threshold_ - gps_recovery_floor_);
      }

      // [自检] SE(2) 拟合质量：残差 RMS/异常值占比持续偏大，提示 GPS 多径、
      // 时间同步或杆臂标定可能存在系统性问题（节流告警，避免刷屏）。
      double se2_res_rms = globalEstimator_.GetFitResidualRms();
      double se2_outlier_ratio = globalEstimator_.GetOutlierRatio();
      if (se2_res_rms > 0.3 || se2_outlier_ratio > 0.3) {
        ROS_WARN_THROTTLE(5.0,
                          "[4DOF] fit quality degraded: residual_rms=%.2fm "
                          "outlier_ratio=%.2f (check GPS multipath / time sync "
                          "/ lever-arm calibration).",
                          se2_res_rms, se2_outlier_ratio);
      }
    }

    // 提取融合定位结果
    double fusion_pos_[4]; // x,y,z,yaw in map/ENU
    memset(fusion_pos_, 0, sizeof(double) * 4);
    if (inputLIO_flag_) {
      GetPosition(fusion_pos_); // [0..3] = 4DoF映射后的x,y,z,yaw
      std::copy(fusion_pos_, fusion_pos_ + 4, diagnostic.lio_map.begin());
      // GPS 可用时优先用 GPS 的 ENU 航向；不可用时保留4DoF航向，
      // 不再用 xyy_lio[3]（其走旧 xy_offset_ 路径，非 ENU 自洽，仅供调试）。
      if (gps_alignment_committed_this_frame && gps_yaw_valid)
        fusion_pos_[3] = location_gps[3];
    }

    // x/y已对齐但tz尚未稳定时，不能将局部z冒充map z。正常
    // 融合模式必须等待4DoF整体有效，期间manager持续停车。
    if (!globalEstimator_.Is4DAligned()) {
      ROS_WARN_THROTTLE(
          1.0,
          "Waiting for complete 4DoF alignment (xy/yaw=%d, z=%d, "
          "z_inliers=%zu, z_std=%.3fm); no valid map pose published.",
          globalEstimator_.IsAligned() ? 1 : 0,
          globalEstimator_.IsVerticalAligned() ? 1 : 0,
          globalEstimator_.GetVerticalInlierCount(),
          globalEstimator_.GetVerticalStd());
      MaybeLog4DoFDiagnostic(diagnostic, "wait_4d_alignment");
      return;
    }

    double map_theta = 0.0, map_tx = 0.0, map_ty = 0.0, map_tz = 0.0;
    const bool have_map_transform = globalEstimator_.GetTransform4D(
        map_theta, map_tx, map_ty, map_tz);
    fusion_4dof::TimedLioRawPose lio_raw_at_gps;
    const bool lio_gps_time_aligned = inputLIO_flag_ &&
        have_map_transform && gps_msg != nullptr &&
        fusion_4dof::LioPoseAtMeasurementTime(
            preceding_lio_raw_pose, current_lio_raw_pose, gps_stamp,
            &lio_raw_at_gps, lio_time_alignment_max_pair_dt_sec_,
            lio_time_alignment_max_extrapolation_sec_);
    double lio_map_at_gps[4] = {};
    if (lio_gps_time_aligned) {
      const double c = std::cos(map_theta), s = std::sin(map_theta);
      lio_map_at_gps[0] = c * lio_raw_at_gps.x - s * lio_raw_at_gps.y + map_tx;
      lio_map_at_gps[1] = s * lio_raw_at_gps.x + c * lio_raw_at_gps.y + map_ty;
      lio_map_at_gps[2] = lio_raw_at_gps.z + map_tz;
      lio_map_at_gps[3] = fusion_4dof::WrapAngle(
          map_theta + lio_raw_at_gps.yaw);
    }

    double pos_state = -1;
    // gps恢复稳定可用时置gps为正常可用，pub赋值时使用gps
    gps_failed_flag_ =
        gps_stable_cnt_ >= gps_stable_threshold_ ? false : gps_failed_flag_;

    double xyy_p[4]; // x,y,z,yaw
    memset(xyy_p, 0, sizeof(double) * 4);
    // RTK may drive production only after the configured recovery evidence.
    // During a bounded transport-only hole, retain state=1 and propagate the
    // last trusted RTK pose by LIO increments. This is an RTK-anchored coast,
    // not a frozen RTK sample and not an unannounced raw-LIO substitution.
    // A received-but-rejected observation is never published. During the
    // bounded grace it uses the same trusted RTK-anchor/LIO propagation as a
    // transport hole; only consecutive/time expiry changes source.
    if (!gps_failed_flag_ && !gps_transient_fallback_ &&
        gps_stable_cnt_ >= gps_stable_threshold_) {
      if (inputGPS_flag_)
      {
        if (inputLIO_flag_ && !lio_gps_time_aligned) {
          gps_grace_anchor_valid_ = false;
          ROS_WARN_THROTTLE(1.0, "Suppress RTK output: no same-generation "
                                  "LIO pose at the RTK measurement time.");
          return;
        }
        xyy_p[0] = location_gps[0];
        xyy_p[1] = location_gps[1];
        // 即使GPS正常，z也用LIO相对高程+tz的连续结果，不逐帧跟随
        // GNSS高程噪声。GPS z只用于稳健更新tz。
        xyy_p[2] = inputLIO_flag_ ? lio_map_at_gps[2] : location_gps[2];
        // A position-quality RTK sample may legitimately arrive before the
        // 1ant multi-observation heading gate is ready. Never publish its raw
        // heading as valid in that case; retain the synchronized 4DoF/LIO yaw.
        if (!inputLIO_flag_ && !gps_yaw_valid) {
          ROS_WARN_THROTTLE(1.0, "RTK heading is not independently validated "
                                 "and no LIO yaw is available; suppress output.");
          return;
        }
        xyy_p[3] = gps_yaw_valid ? location_gps[3] : lio_map_at_gps[3];
        pos_state = 1;
        no_pos_cnt_ = 0;
      } else if (gps_coast_observation && inputLIO_flag_ &&
                 gps_grace_anchor_valid_ &&
                 fusion_4dof::ShouldPublishGpsGraceCoast(
                     gps_grace_coast_output_enabled_, observation,
                     outage_state, gps_missing_grace_frames_)) {
        xyy_p[0] = gps_grace_anchor_output_[0] +
                   (fusion_pos_[0] - gps_grace_anchor_lio_[0]);
        xyy_p[1] = gps_grace_anchor_output_[1] +
                   (fusion_pos_[1] - gps_grace_anchor_lio_[1]);
        xyy_p[2] = fusion_pos_[2];
        xyy_p[3] = fusion_4dof::WrapAngle(
            gps_grace_anchor_output_[3] + fusion_4dof::WrapAngle(
                fusion_pos_[3] - gps_grace_anchor_lio_[3]));
        pos_state = 1;
        no_pos_cnt_ = 0;
      } else if (gps_coast_observation &&
                 !gps_grace_coast_output_enabled_ &&
                 (observation == fusion_4dof::GpsObservation::kHeadingSuspect ||
                  fusion_4dof::IsGpsUnavailableWithinGrace(
                      outage_state, gps_missing_grace_frames_))) {
        // The operator explicitly chose a gap in the output stream over an
        // RTK-anchored LIO propagation. Keep counting evidence; do not freeze
        // the last pose and do not relabel the unpublished result as LIO.
        ROS_WARN_THROTTLE(
            1.0,
            "RTK is unavailable within grace and GPS coast output is "
            "disabled: suppress this localization frame (%d/%d).",
            gps_missing_cnt_, gps_missing_grace_frames_);
        MaybeLog4DoFDiagnostic(diagnostic, "gps_grace_output_disabled");
        return;
      } else {
        ROS_WARN_THROTTLE(
            1.0,
            "RTK grace cannot coast safely (missing=%d anchor=%d LIO=%d); "
            "suppress this pose instead of freezing or mislabelling it.",
            gps_missing_cnt_, gps_grace_anchor_valid_ ? 1 : 0,
            inputLIO_flag_ ? 1 : 0);
        MaybeLog4DoFDiagnostic(diagnostic, "gps_grace_no_sync");
        return;
      }
    } else if (lio_failed_cnt_ < lio_fallback_max_bad_frames_ &&
               globalEstimator_.Is4DAligned()) {
      // [N_LIO] 仅当 LIO->map 4DoF完整有效才允许回退到融合位姿；
      // 否则落入下面的 else→停车报障(FAULT)，不输出未对齐的 LIO 回退。
      ROS_WARN_THROTTLE(1.0, "RTK is not stable; use frozen 4DoF/LIO.");
      if (inputLIO_flag_) {
        memcpy(xyy_p, fusion_pos_, sizeof(double) * 4);
        no_pos_cnt_ = 0;
        // 融合回退分两档质量，编码进 position_state（下游按需分别处理）：
        //   2 = 已锁定（经历过拐弯的全 Umeyama 拟合，精度最高，语义与旧版本一致）；
        //   5 = 仅播种未锁定（沿迹平均方向或 RTK 航向种子），精度明显更低——
        //       短时短距通常无碍，但若长直线段上恰好长时间丢 RTK，误差会随距离
        //       累积（原理见 README §4.2/§4.2b）。
        // ⚠ 新增的 5 是本次改动引入的新状态值：所有订阅 /Mower/position 并按
        // position_state 判"是否有可用定位"的下游，都需要把 5 也当作有效位置
        // （只是精度较低），否则会把这段时间误判为"无定位"。已知消费者：
        // android_manager_4dof以及当前crawler_control/task_node.cpp均已把5
        // 纳入有效位置；旧版/备份消费者仍不得与本定位组合混用。
        bool se2_locked = globalEstimator_.IsLocked();
        pos_state = se2_locked ? 2 : 5;
        if (!se2_locked) {
          ROS_WARN_THROTTLE(5.0,
                            "[4DOF] fallback in use but NOT locked yet "
                            "(position_state=5, seed/along-baseline quality "
                            "only) -- accuracy may degrade over long straight "
                            "runs without RTK.");
        }
      } else {
        MaybeLog4DoFDiagnostic(diagnostic, "fallback_no_lio");
        return;
      }
    } else {
      // A single bad fusion frame cannot enter emergency IMU mode. The 20 Hz
      // timer owns this transition and requires ten seconds of simultaneous
      // GPS+LIO unavailability.
      PublishStopCar(true);
      stop_car_active_ = true;
      algorithm_reason_ =
          "GPS and LIO currently unusable; restricted fallback still gated";
      MaybeLog4DoFDiagnostic(diagnostic, "fault_no_source");
      return;
    }

    // 直接 RTK 位姿使用 GPS 测量时间；RTK 锚定的 LIO 延拓虽仍标 state=1，
    // 其物理时刻却是当前 LIO 时间，不能误用收到但未通过门控的 GPS stamp。
    // LIO 回退同样使用当前 LIO 时间。
    const ros::Time output_stamp =
        (pos_state == 1.0 && inputGPS_flag_ && !gps_msg_stamp.isZero())
            ? gps_msg_stamp : msg->header.stamp;
    fusion_4dof::LioMotionReference output_lio_reference;
    if (inputLIO_flag_ && have_map_transform && current_lio_raw_pose.valid) {
      const fusion_4dof::TimedLioRawPose &raw_at_output =
          pos_state == 1.0 && inputGPS_flag_ ? lio_raw_at_gps
                                             : current_lio_raw_pose;
      output_lio_reference.valid = raw_at_output.valid;
      output_lio_reference.generation_sec = raw_at_output.generation_sec;
      output_lio_reference.generation_nsec = raw_at_output.generation_nsec;
      output_lio_reference.stamp = raw_at_output.stamp;
      output_lio_reference.raw_x = raw_at_output.x;
      output_lio_reference.raw_y = raw_at_output.y;
      output_lio_reference.map_yaw_from_raw = map_theta;
    }
    double output_roll = 0.0, output_pitch = 0.0;
    if (!SelectOutputRollPitch(output_stamp, inputLIO_flag_,
                               lio_global_roll_, lio_global_pitch_,
                               &output_roll, &output_pitch)) {
      MaybeLog4DoFDiagnostic(diagnostic, "attitude_invalid");
      return;
    }
    double output_yaw = xyy_p[3];
    if (!CanonicalizeOutputRPY(output_roll, output_pitch, output_yaw,
                               &output_roll, &output_pitch, &output_yaw)) {
      MaybeLog4DoFDiagnostic(diagnostic, "attitude_nonfinite");
      return;
    }
    diagnostic.output_valid = true;
    diagnostic.position_state = pos_state;
    diagnostic.output = {xyy_p[0], xyy_p[1], xyy_p[2], output_roll,
                         output_pitch, output_yaw};

    // [调参诊断] 周期性(节流到 1Hz，避免刷屏)打印完整 SE(2) 状态，供实地测试
    // 后把节点日志整份交回来做离线复盘：能否/何时播种、何时锁定、along/cross_std
    // 相对 se2_seed_along_std/se2_lock_cross_std/se2_lock_min_samples 的余量、
    // 拟合残差/异常值占比是否偏高等，都能从这一行序列里复原。tag 固定为
    // [4DOF-TUNE] 便于事后 grep。
    {
      double se2_theta = 0.0, se2_tx = 0.0, se2_ty = 0.0, se2_tz = 0.0;
      bool se2_aligned = globalEstimator_.GetTransform(se2_theta, se2_tx, se2_ty);
      bool four_d_aligned = globalEstimator_.GetTransform4D(
          se2_theta, se2_tx, se2_ty, se2_tz);
      bool se2_locked = globalEstimator_.IsLocked();
      // 瞬时 RTK 种子变换（GetSeedOnlyTransform，与滑窗/锁定无关，见 README §11）
      // 一并打进同一行日志：seed_tx/seed_ty 只用"最新一帧"反解，不受窗口质心
      // 平均影响；把它跟窗口化的 tx/ty 对照，能直接从纯文本日志（无需回放 bag）
      // 算出"窗口平移"与"瞬时平移"的逐秒差值，量化尺度误差/窗口滞后的大小。
      double seed_theta = 0.0, seed_tx = 0.0, seed_ty = 0.0;
      bool seed_ok = globalEstimator_.GetSeedOnlyTransform(seed_theta, seed_tx, seed_ty);
      ROS_INFO_THROTTLE(
          1.0,
          "[4DOF-TUNE] t=%.2f | gps_good=%d gps_failed_cnt=%d gps_failed_flag=%d "
          "gps_stable_cnt=%d gps_missing_cnt=%d transient=%d reacq=%d | "
          "lio_state=%d lio_failed_cnt=%d lio_drift=%d | "
          "xy_aligned=%d aligned4d=%d locked=%d theta=%.3frad(%.1fdeg) "
          "tx=%.2f ty=%.2f tz=%.2f | "
          "seed_ok=%d seed_theta=%.3frad(%.1fdeg) seed_tx=%.2f seed_ty=%.2f | "
          "along_std=%.3fm cross_std=%.3fm window_n=%zu xy_inliers=%zu "
          "condition_number=%.2f turn=%.1fdeg | res_rms=%.3fm "
          "outlier_ratio=%.2f | z_aligned=%d tz_std=%.3fm z_inliers=%zu "
          "z_outliers=%.2f | pos_state=%.0f xyz_yaw=(%.2f,%.2f,%.2f,%.3f)",
          lio_stamp_, gps_good_flag ? 1 : 0, gps_failed_cnt_,
          gps_failed_flag_ ? 1 : 0, gps_stable_cnt_, gps_missing_cnt_,
          gps_transient_fallback_ ? 1 : 0, gps_transient_reacquire_cnt_,
          lio_state_ ? 1 : 0, lio_failed_cnt_,
          lio_drift_detected_ ? 1 : 0, se2_aligned ? 1 : 0,
          four_d_aligned ? 1 : 0, se2_locked ? 1 : 0, se2_theta,
          se2_theta * 180.0 / M_PI, se2_tx, se2_ty, se2_tz,
          seed_ok ? 1 : 0, seed_theta, seed_theta * 180.0 / M_PI, seed_tx, seed_ty,
          globalEstimator_.GetAlongStd(), globalEstimator_.GetCrossStd(),
          globalEstimator_.GetWindowSize(),
          globalEstimator_.GetHorizontalInlierCount(),
          globalEstimator_.GetLockConditionNumber(),
          globalEstimator_.GetAccumulatedTurnRad() / DEG2RAD,
          globalEstimator_.GetFitResidualRms(),
          globalEstimator_.GetOutlierRatio(),
          globalEstimator_.IsVerticalAligned() ? 1 : 0,
          globalEstimator_.GetVerticalStd(),
          globalEstimator_.GetVerticalInlierCount(),
          globalEstimator_.GetVerticalOutlierRatio(), pos_state, xyy_p[0],
          xyy_p[1], xyy_p[2], xyy_p[3]);
    }

    // Publish position.
    if (!reboot && pub_fusion == 1) {
      util::Position fusion_position;
      fusion_position.header.stamp = output_stamp;
      fusion_position.header.frame_id = "base_link";
      fusion_position.position_x = xyy_p[0];
      fusion_position.position_y = xyy_p[1];
      fusion_position.position_z = xyy_p[2];
      fusion_position.roll = output_roll;
      fusion_position.pitch = output_pitch;
      fusion_position.yaw = output_yaw;
      fusion_position.position_state = pos_state;
      if (!PublishProductionPosition(fusion_position,
                                     &output_lio_reference,
                                     pos_state == 1.0 && !inputGPS_flag_)) {
        diagnostic.output_valid = false;
        MaybeLog4DoFDiagnostic(diagnostic, "output_continuity_reject");
        return;
      }
      const double fusion_time = ros::SteadyTime::now().toSec();
      last_fusion_time_ = fusion_time;
      memcpy(last_fusion_pos_, xyy_p, sizeof(double) * 4);
      // Commit the coast anchor only after the direct RTK pose passed the
      // final production continuity gate. Coasting frames must never move the
      // anchor, otherwise LIO error would be recursively re-anchored as RTK.
      if (pos_state == 1 && inputGPS_flag_) {
        gps_grace_anchor_valid_ = inputLIO_flag_ && lio_gps_time_aligned;
        if (gps_grace_anchor_valid_) {
          std::copy(lio_map_at_gps, lio_map_at_gps + 4,
                    gps_grace_anchor_lio_);
          std::copy(xyy_p, xyy_p + 4, gps_grace_anchor_output_);
          gps_grace_anchor_stamp_ = gps_stamp;
          gps_grace_anchor_generation_sec_ = lio_generation_sec_;
          gps_grace_anchor_generation_nsec_ = lio_generation_nsec_;
        }
      }
      MaybeLog4DoFDiagnostic(
          diagnostic,
          pos_state != 1
              ? "lio_fallback"
              : (gps_heading_suspect_observation
                     ? "gps_heading_suspect_coast"
                     : (gps_coast_observation ? "gps_grace_coast"
                                              : "normal_gps")));
      if (fusion_position.position_state == 1 ||
          fusion_position.position_state == 2 ||
          fusion_position.position_state == 5) {
        AnchorInertialOdometry(output_stamp, xyy_p[0], xyy_p[1], xyy_p[2],
                               output_roll, output_pitch, output_yaw);
      }
      // 保存最后一个 fusion_position 数据到文件
      // SaveFusionPositionToFile(xyy_p);
      // [viz] 实际对外发布的定位结果（pos_state: 1=GPS, 2=融合回退已锁定,
      // 5=融合回退仅播种未锁定，见 util/msg/Position.msg 顶部注释）+ map->base_link TF。
      PublishVizPose(viz_output_, output_stamp, xyy_p[0], xyy_p[1],
                     xyy_p[2], output_roll, output_pitch, output_yaw);
      BroadcastBaseLinkTF(output_stamp, xyy_p[0], xyy_p[1], xyy_p[2],
                          output_roll, output_pitch, output_yaw);

      publish_count++;
    }

#ifdef DEBUG

    double xyy_g[4]; // [0]x [1]y [2]高程 [3]航向
    memset(xyy_g, 0, sizeof(double) * 4);
    if (gps_good_flag) {
      GPS2Local(latitude, longitude, altitude, gauss_yaw, xyy_g);
    }
    // /Mower/debug/gps represents a usable, time-matched RTK observation, not
    // a heartbeat. Include the estimator innovation gate (when initialized),
    // and never publish a zero pose with LIO roll/pitch for an absent/rejected
    // RTK sample.
    if (gps_sample_valid) {
      util::Position gps2xyz_msg;
      gps2xyz_msg.header.frame_id = "map";
      gps2xyz_msg.header.stamp = gps_msg_stamp;
      gps2xyz_msg.position_x = xyy_g[0];
      gps2xyz_msg.position_y = xyy_g[1];
      gps2xyz_msg.position_z = xyy_g[2]; // 高程(相对原点)
      gps2xyz_msg.position_state = 1;
      double debug_gps_roll = 0.0, debug_gps_pitch = 0.0;
      SelectOutputRollPitch(gps_msg_stamp, inputLIO_flag_, lio_global_roll_,
                            lio_global_pitch_, &debug_gps_roll,
                            &debug_gps_pitch);
      gps2xyz_msg.roll = debug_gps_roll;
      gps2xyz_msg.pitch = debug_gps_pitch;
      gps2xyz_msg.yaw = xyy_g[3];
      pubDebugGPS_.publish(gps2xyz_msg);
      PublishVizPose(viz_gps_, gps_msg_stamp, xyy_g[0], xyy_g[1], xyy_g[2], 0.0,
                     0.0, xyy_g[3]);
    }

    // /debug/lio and the RViz map track explicitly claim the global "map"
    // frame.  Do not publish the local-H identity fallback under that frame
    // name while SE(2) is still unavailable; doing so silently mixes two
    // coordinate systems and makes trajectory comparisons invalid.
    if (lio_state_ && lio_enu_transform_available) {
      util::Position LIO_msg;
      LIO_msg.header.frame_id = "map";
      LIO_msg.header.stamp = msg->header.stamp;
      LIO_msg.position_x = xyy_lio[0];
      LIO_msg.position_y = xyy_lio[1];
      LIO_msg.position_z = xyy_lio[2]; // 4DoF对齐后的ENU高程
      LIO_msg.position_state = lio_drift_detected_;
      LIO_msg.roll = lio_global_roll_;
      LIO_msg.pitch = lio_global_pitch_;
      LIO_msg.yaw = xyy_lio[3];
      pubDebugLIO_.publish(LIO_msg);
      // [viz] 纯 LIO 位姿（含 roll/pitch，能看出雷达下倾的姿态）。
      PublishVizPose(viz_lio_, msg->header.stamp, xyy_lio[0], xyy_lio[1],
                     xyy_lio[2], lio_global_roll_, lio_global_pitch_,
                     xyy_lio[3]);

      if (seed_transform_available) {
        // 对比用：纯 RTK 位置+航向种子变换（与滑窗/锁定无关，每帧重算，见
        // README §11）。只有确有种子变换时才允许标为 map。
        util::Position rtk_seed_msg;
        rtk_seed_msg.header.frame_id = "map";
        rtk_seed_msg.header.stamp = msg->header.stamp;
        rtk_seed_msg.position_x = xyy_seed[0];
        rtk_seed_msg.position_y = xyy_seed[1];
        rtk_seed_msg.position_z = xyy_seed[2];
        rtk_seed_msg.yaw = xyy_seed[3];
        pubDebugRtkSeed_.publish(rtk_seed_msg);
        PublishVizPose(viz_rtk_seed_, msg->header.stamp, xyy_seed[0],
                       xyy_seed[1], xyy_seed[2], 0.0, 0.0, xyy_seed[3]);
      }

      util::Position fusion_msg;
      fusion_msg.header.frame_id = "map";
      fusion_msg.header.stamp = msg->header.stamp;
      fusion_msg.position_x = fusion_pos_[0];
      fusion_msg.position_y = fusion_pos_[1];
      fusion_msg.position_z = fusion_pos_[2];
      fusion_msg.yaw = fusion_pos_[3];
      // 恢复候选帧虽然有 GPS 输入，但尚未获准影响生产融合结果，不能标成 1。
      fusion_msg.position_state = gps_alignment_committed_this_frame ? 1 : 0;
      pubDebugFusion_.publish(fusion_msg);
      // [viz] ceres 融合位姿：仅在本帧确有融合输出时绘制。
      if (inputLIO_flag_) {
        PublishVizPose(viz_fusion_, msg->header.stamp, fusion_pos_[0],
                       fusion_pos_[1], fusion_pos_[2], lio_global_roll_,
                       lio_global_pitch_, fusion_pos_[3]);
      }
    }

#endif
  }

  bool Is_LIO_GOOD(const Eigen::Vector3d &lio_t, const double lio_stamp_,
                   double *lio_speed, double lio_yaw, double *lio_pos_yaw) {
    const double previous_stamp = last_lio_stamp_;
    Eigen::Vector3d delta = Eigen::Vector3d::Zero();
    const auto motion_result =
        fusion_4dof::EvaluateLioMotionAndAdvanceBaseline(
            lio_stamp_, lio_t, lio_motion_max_dt_sec_, lio_speed_threshold_,
            lio_motion_margin_m_, &last_lio_stamp_, &last_lio_t_, lio_speed,
            &delta);
    // Low callback rates are legitimate, but an invalid/too-old pair is not
    // evidence of a healthy trajectory. Never bypass the speed check by
    // returning true for an out-of-range measurement interval.
    if (motion_result != fusion_4dof::LioMotionResult::kAccepted) {
      ROS_WARN_THROTTLE(1.0, "Reject LIO motion pair: dt=%.3fs (max=%.3fs).",
                        previous_stamp > 0.0 ? lio_stamp_ - previous_stamp : 0.0,
                        lio_motion_max_dt_sec_);
      lio_drift_detected_ = true;
      return false;
    } else {
      // +++++++++++调整参数
      if (location_mode_ &&
          *lio_speed > lio_yaw_warning_min_speed_mps_) {
        // 计算前后两帧的位置航向角
        double delta_x = delta.x();
        double delta_y = delta.y();
        *lio_pos_yaw = atan2(delta_y, delta_x); // 计算航向角（弧度）

        // 将计算的航向角转换到 [-π, π] 范围
        *lio_pos_yaw = ConvertYaw(*lio_pos_yaw);
        while (*lio_pos_yaw >= 2 * M_PI) {
          *lio_pos_yaw -= 2 * M_PI;
        }

        // 对比计算的航向角与 LIO 消息中的航向角
        double yaw_diff = fabs(*lio_pos_yaw - lio_yaw);
        if (yaw_diff > M_PI) {
          yaw_diff = 2 * M_PI - yaw_diff;
        }

        // 如果航向角差值过大，标记为漂移
        // +++++++++++调整参数
        if (yaw_diff > lio_yaw_warning_max_error_rad_) {
          // lio_drift_detected_ = true;
          // lio_drift_count_ = yaw_diff > 0.87 ? 30 : 10;
          ROS_WARN("LIO drift detected due to yaw difference: *lio_pos_yaw = "
                   "%.2f, lio_yaw = %.2f, yaw_diff = %.2f",
                   *lio_pos_yaw, lio_yaw, yaw_diff);
          // return false;
        }
      }
      lio_drift_detected_ = false;
      return true;
    }
  }

private:
  void ParameterInit() {
    // +++++++++++调整参数
    fail_cnt_ = 0;
    origin_gauss_yaw_ = 0.;
    map_index_ = 0;
    origin_latitude_ = 0.;
    origin_longitude_ = 0.;
    origin_height_ = 0.;
    map_origin_ready_ = false;
    last_position_state_ = 0;
    last_position_publish_steady_sec_ = 0.0;
    ins_status_received_ = false;
    latest_ins_status_ = 0;
    latest_ins_vehicle_align_ = 0;
    latest_ins_heading_flag_ = 0;
    latest_ins_position_status_ = 0;
    latest_gps_good_ = false;
    latest_lio_good_ = false;
    latest_raw_gps_usable_ = false;
    last_gps_good_steady_sec_ = 0.0;
    last_lio_good_steady_sec_ = 0.0;
    last_raw_gps_usable_steady_sec_ = 0.0;
    requested_restart_scope_ = util::LocalizationStatus::RESTART_NONE;
    algorithm_fault_code_ = util::LocalizationStatus::FAULT_NONE;
    algorithm_reason_.clear();
    active_map_name_.clear();
    active_map_uuid_.clear();
    active_origin_checksum_.clear();
    pending_map_name_.clear();
    pending_map_uuid_.clear();
    pending_origin_checksum_.clear();
    map_switch_pending_ = false;
    alignment_state_path_.clear();
    lio_generation_valid_ = false;
    lio_level_transform_ready_ = false;
    lio_generation_sec_ = 0;
    lio_generation_nsec_ = 0;
    const ros::WallTime fusion_generation = ros::WallTime::now();
    fusion_generation_valid_ = !fusion_generation.isZero();
    fusion_generation_sec_ = fusion_generation.sec;
    fusion_generation_nsec_ = fusion_generation.nsec;
    last_alignment_save_wall_time_ = 0.0;
    vertical_require_ins_height_std_ = false;
    vertical_max_ins_height_std_ = 0.50;
    diagnostic_log_enable_ = true;
    diagnostic_log_period_sec_ = 1.0;
    last_diagnostic_log_wall_time_ = 0.0;
    gps_lio_sync_tolerance_sec_ = 0.08;
    gps_motion_min_dt_sec_ = 0.01;
    gps_motion_max_dt_sec_ = 0.45;
    gps_motion_max_speed_mps_ = 2.0;
    gps_motion_margin_m_ = 0.08;
    gps_grace_coast_output_enabled_ = true;
    lio_motion_max_dt_sec_ = 0.60;
    lio_motion_margin_m_ = 0.08;
    lio_yaw_warning_min_speed_mps_ = 0.50;
    lio_yaw_warning_max_error_rad_ = 0.70;
    lio_time_alignment_max_pair_dt_sec_ = 0.60;
    lio_time_alignment_max_extrapolation_sec_ = 0.05;
    gps_coast_max_duration_sec_ = 0.75;
    gps_heading_suspect_coast_max_duration_sec_ = 1.50;
    output_max_measurement_age_sec_ = 0.50;
    output_max_future_stamp_sec_ = 0.05;
    gps_queue_max_size_ = 200;
    last_gps_callback_stamp_ = 0.0;
    last_accepted_lio_input_stamp_ = 0.0;
    lio_restart_after_bad_frames_ = 20;
    lio_bad_max_duration_sec_ = 2.0;
    lio_bad_first_steady_sec_ = 0.0;
    gps_lio_sync_restart_after_frames_ = 50;
    gps_lio_sync_bad_max_duration_sec_ = 5.0;
    gps_lio_sync_bad_first_steady_sec_ = 0.0;
    lio_drift_hold_after_bad_frames_ = 3;
    lio_drift_hold_frames_ = 30;
    lio_drift_hold_max_duration_sec_ = 3.0;
    lio_drift_hold_end_stamp_ = 0.0;
    lio_fallback_max_bad_frames_ = 5;
    gps_init_ = false;
    system_init_ = false;
    location_mode_ = false;
    memset(xy_offset_, 0, sizeof(double) * 3);
    memset(last_gps_, 0, sizeof(double) * 3);
    last_gps_stamp_ = 0.0;
    gps_course_axis_gate_.Reset();
    gps_drift_count_ = 0;
    last_lio_t_ = Eigen::Vector3d(0, 0, 0);
    last_lio_stamp_ = 0.0;
    pub_fusion = 0;
    lio_frame_align_rotation_ = Eigen::Quaterniond(
        Eigen::AngleAxisd(M_PI / 2.0, Eigen::Vector3d::UnitZ()));
    lio_to_base_translation_baseframe_ = Eigen::Vector3d::Zero();
    lio_to_base_translation_ = Eigen::Vector3d::Zero();
    lio_to_base_rotation_ = Eigen::Quaterniond::Identity();
    lio_seen_far_ = false;
    lio_epoch_near_thresh_ = 0.5;
    lio_epoch_far_thresh_ = 3.0;
    lio_epoch_jump_thresh_ = 2.0; // 单帧 >2m 位移在割草机速度下非物理 → 视为重启跳变
    prev_raw_lio_t_ = Eigen::Vector3d(0, 0, 0);
    has_prev_raw_lio_ = false;
    last_lio_seq_ = 0;
    has_last_lio_seq_ = false;
    lio_restart_pending_ = false;
    controlled_lio_recovery_ = false;
    lio_recovery_new_generation_received_ = false;
    lio_recovery_start_steady_sec_ = 0.0;
    lio_recovery_grace_timeout_sec_ = 60.0;
    stop_car_active_ = false;
    output_timestamp_stop_active_ = false;
    output_timestamp_resume_count_ = 0;
    imu_odom_enable_ = true;
    imu_odom_test_from_startup_ = false;
    imu_odom_stop_active_ = false;
    imu_odom_ready_ = false;
    imu_odom_ready_published_ = false;
    have_pending_imu_static_calibration_ = false;
    imu_calibration_applied_ = false;
    applied_imu_calibration_generation_sec_ = 0;
    applied_imu_calibration_generation_nsec_ = 0;
    mid360_imu_frame_id_.clear();
    imu_calibration_frame_id_.clear();
    imu_odom_main_pose_timeout_sec_ = 10.0;
    last_main_pose_rx_sec_ = 0.0;
    previous_main_pose_rx_sec_ = 0.0;
    previous_main_position_.setZero();
    have_previous_main_pose_ = false;
    imu_odom_max_velocity_mps_ = 2.0;
    imu_odom_fallback_max_duration_for_log_ = 10.0;
    imu_odom_fallback_max_distance_for_log_ = 3.0;
    stop_car_resume_cnt_ = 0;
    stop_car_last_trusted_output_steady_sec_ = 0.0;
    stop_car_resume_frames_ = 5;
    lio_speed_threshold_ = 2;
    lio_stamp_ = 0;
    lio_drift_count_ = 0;
    lio_drift_detected_ = false;
    lio_raw_x_ = 0.0;
    lio_raw_y_ = 0.0;
    lio_raw_yaw_ = 0.0;
    lio_raw_roll_ = 0.0;
    lio_raw_pitch_ = 0.0;
    lio_global_roll_ = 0.0;
    lio_global_pitch_ = 0.0;
    roll_pitch_source_ = "ins";
    ins_roll_scale_rad_ = 0.01 * DEG2RAD;
    ins_pitch_scale_rad_ = -0.01 * DEG2RAD;
    ins_attitude_max_age_sec_ = 0.30;
    ins_attitude_max_abs_rad_ = 45.0 * DEG2RAD;
    ins_lio_max_disagreement_rad_ = 15.0 * DEG2RAD;
    ins_attitude_require_position_status_ = true;
    latest_ins_attitude_valid_ = false;
    latest_ins_attitude_stamp_ = ros::Time(0);
    latest_ins_roll_ = 0.0;
    latest_ins_pitch_ = 0.0;
    lio_static_initialized_ = false;
    no_pos_cnt_ = 0;
    no_rtk_cnt_ = 0;
    reboot = false;
    gps_failed_cnt_ = 0;
    lio_failed_cnt_ = 0;
    last_fusion_time_ = 0.0;
    memset(last_fusion_pos_, 0, sizeof(double) * 4);
    fusion_failed_cnt_ = 0;

    pub_overtime_cnt_ = 0;

    gps_initial_stable_count_ = 0;
    gps_stable_cnt_ = gps_initial_stable_count_;
    gps_recovery_floor_ = 28;   // 失效后保留计数，默认仅需 22 个连续好样本恢复
    gps_stable_threshold_ = 50; // GPS稳定阈值
    gps_missing_grace_frames_ = 3;
    gps_transient_reacquire_frames_ = 3;
    gps_missing_cnt_ = 0;
    gps_transient_reacquire_cnt_ = 0;
    gps_transient_fallback_ = false;
    gps_failed_flag_ = false;
    gps_grace_anchor_valid_ = false;
    gps_grace_anchor_stamp_ = 0.0;
    gps_grace_anchor_generation_sec_ = 0;
    gps_grace_anchor_generation_nsec_ = 0;
    previous_lio_raw_pose_.valid = false;
    std::memset(gps_grace_anchor_lio_, 0, sizeof(gps_grace_anchor_lio_));
    std::memset(gps_grace_anchor_output_, 0,
                sizeof(gps_grace_anchor_output_));

    publish_window_start = 0;
    publish_count = 0;

    publish_error_cnt_ = 0;

    heading_flag_ = 0;

    sync_failed_cnt_ = 0;

    lio_bad_flag_ = false;

    reboot_pub_flag_ = false;
    require_gravity_level_lio_frame_ = true;
  }
  // Convert the traceable RTK position to ENU, then classify its temporal
  // motion evidence without conflating baseline establishment with failure.
  fusion_4dof::GpsMotionResult
  CheckGpsMotion(double latitude, double longitude, double altitude,
                 double gps_flag, double gauss_yaw, bool gps_yaw_valid,
                 double independent_turn_rate_radps,
                 bool turn_rate_trusted,
                 double gps_stamp,
                 double *location_gps, double *gps_speed,
                 double *gps_pos_yaw, double *gps_stamp_delta_sec) {
    // 检查 GPS 数据是否有效
    if (gps_stamp > 0.0 && std::isfinite(latitude) &&
        std::isfinite(longitude) && std::isfinite(altitude) &&
        std::isfinite(gauss_yaw) && latitude >= -90.0 && latitude <= 90.0 &&
        longitude >= -180.0 && longitude <= 180.0 &&
        gps_flag == rtk_quality_config_.required_gps_flag) {
      GPS2Local(latitude, longitude, altitude, gauss_yaw, location_gps);
      fusion_4dof::GpsMotionState motion_state;
      motion_state.valid = last_gps_stamp_ > 0.0;
      motion_state.stamp = last_gps_stamp_;
      motion_state.x = last_gps_[0];
      motion_state.y = last_gps_[1];
      double motion_yaw_rad = 0.0;
      fusion_4dof::GpsMotionResult result =
          fusion_4dof::EvaluateGpsMotion(
              gps_stamp, location_gps[0], location_gps[1], location_gps[3],
              false, &motion_state, gps_speed, &motion_yaw_rad,
              gps_motion_min_dt_sec_, gps_motion_max_dt_sec_,
              gps_motion_max_speed_mps_, 0.0,
              0.0, gps_stamp_delta_sec, gps_motion_margin_m_);
      if (result == fusion_4dof::GpsMotionResult::kAccepted) {
        const auto axis_result = gps_course_axis_gate_.Observe(
            gps_stamp, location_gps[0], location_gps[1], location_gps[3],
            gps_yaw_valid, independent_turn_rate_radps,
            turn_rate_trusted);
        if (axis_result == fusion_4dof::GpsCourseAxisResult::kSuspect)
          result = fusion_4dof::GpsMotionResult::kHeadingSuspect;
        else if (axis_result == fusion_4dof::GpsCourseAxisResult::kRejected)
          result = fusion_4dof::GpsMotionResult::kHeadingMismatch;
        else if (axis_result ==
                 fusion_4dof::GpsCourseAxisResult::kAlignedReverse)
          ROS_INFO_THROTTLE(5.0, "RTK displacement is consistent with "
                                  "straight reverse motion; not drift.");
      } else {
        gps_course_axis_gate_.Reset();
      }
      last_gps_stamp_ = motion_state.valid ? motion_state.stamp : 0.0;
      last_gps_[0] = motion_state.x;
      last_gps_[1] = motion_state.y;
      last_gps_[2] = location_gps[2];
      *gps_pos_yaw = motion_yaw_rad * 180.0 / M_PI;
      if (result == fusion_4dof::GpsMotionResult::kExcessiveSpeed)
        ROS_WARN("GPS drift detected due to high speed: %.2f m/s", *gps_speed);
      else if (result == fusion_4dof::GpsMotionResult::kHeadingMismatch)
        ROS_WARN("GPS drift detected due to motion/heading disagreement.");
      return result;
    }

    // GPS 数据无效
    gps_course_axis_gate_.Reset();
    ROS_WARN("GPS data is not valid.");
    return fusion_4dof::GpsMotionResult::kInvalid;
  }

  bool Is_LIO_USABLE(const util::LIOPoseConstPtr &msg) {
    // Compatibility guard for any legacy path that still only sets `reboot`.
    // Primary FAULT paths call RequestFullRestart() immediately and therefore
    // no longer depend on another LIO message arriving.
    if (reboot) {
      return false;
    }

    // 检查 LIO 消息是否为空
    if (!msg) {
      ROS_ERROR("Received null LIO message.");
      return false;
    }

    if (require_gravity_level_lio_frame_ &&
        msg->header.frame_id != "lio_level") {
      ROS_ERROR_THROTTLE(
          1.0,
          "Reject LIO frame '%s': fusion_4dof requires gravity-leveled "
          "LIO input with frame_id='lio_level'.",
          msg->header.frame_id.c_str());
      return false;
    }

    const bool finite_pose =
        std::isfinite(msg->position_x) && std::isfinite(msg->position_y) &&
        std::isfinite(msg->position_z) && std::isfinite(msg->q_w) &&
        std::isfinite(msg->q_x) && std::isfinite(msg->q_y) &&
        std::isfinite(msg->q_z);
    const double quaternion_norm =
        std::sqrt(msg->q_w * msg->q_w + msg->q_x * msg->q_x +
                  msg->q_y * msg->q_y + msg->q_z * msg->q_z);
    if (!finite_pose || !std::isfinite(quaternion_norm) ||
        quaternion_norm < 1e-6) {
      ROS_ERROR_THROTTLE(1.0, "Reject invalid/non-finite leveled LIO pose.");
      return false;
    }

    // 检查 LIO 时间戳是否有效
    lio_stamp_ = msg->header.stamp.toSec();
    if (lio_stamp_ <= 0) {
      ROS_WARN("Invalid LIO timestamp: %f", lio_stamp_);
      return false;
    }

    const double measurement_age =
        (ros::Time::now() - msg->header.stamp).toSec();
    if (!std::isfinite(measurement_age) ||
        measurement_age < -output_max_future_stamp_sec_ ||
        measurement_age > output_max_measurement_age_sec_) {
      ROS_ERROR_THROTTLE(
          1.0,
          "Reject stale/future LIO before mutating estimator state "
          "(age=%.3fs).",
          measurement_age);
      output_timestamp_stop_active_ = true;
      output_timestamp_resume_count_ = 0;
      PublishStopCar(true);
      return false;
    }
    if (last_accepted_lio_input_stamp_ > 0.0 &&
        lio_stamp_ < last_accepted_lio_input_stamp_) {
      ROS_ERROR_THROTTLE(1.0,
                         "Reject regressed LIO timestamp before mutating "
                         "estimator state (current=%.9f last=%.9f).",
                         lio_stamp_, last_accepted_lio_input_stamp_);
      output_timestamp_stop_active_ = true;
      output_timestamp_resume_count_ = 0;
      PublishStopCar(true);
      return false;
    }
    last_accepted_lio_input_stamp_ = lio_stamp_;

    return true; // 检查通过
  }

  std::string NamedMapPath(const std::string &name) const {
    const std::size_t slash = map_path_.find_last_of("/\\");
    const std::string directory =
        slash == std::string::npos ? std::string() : map_path_.substr(0, slash + 1);
    return directory + name + ".mp";
  }

  fusion_4dof::MapBundle CurrentMapBundle(const std::string &logical_name,
                                          const std::string &uuid,
                                          uint64_t created_unix_ns,
                                          std::string *error) const {
    fusion_4dof::MapBundle bundle;
    bundle.origin.map_index = static_cast<int32_t>(map_index_);
    bundle.origin.latitude = origin_latitude_;
    bundle.origin.longitude = origin_longitude_;
    bundle.origin.height = origin_height_;
    bundle.origin.gauss_yaw = origin_gauss_yaw_;
    if (!fusion_4dof::EncodeMapOrigin(bundle.origin, &bundle.origin_bytes,
                                      error))
      return fusion_4dof::MapBundle();
    bundle.metadata.map_uuid = uuid;
    bundle.metadata.logical_name = logical_name;
    bundle.metadata.origin_checksum =
        fusion_4dof::OriginChecksum(bundle.origin_bytes);
    bundle.metadata.created_unix_ns = created_unix_ns;
    return bundle;
  }

  uint64_t CurrentUnixNanoseconds() const {
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
  }

  void FillMapServiceResponse(util::ManageLocalizationMap::Response &response,
                              bool success, bool alignment_preserved,
                              const std::string &reason) const {
    response.success = success;
    response.alignment_preserved = alignment_preserved;
    response.switch_pending = map_switch_pending_;
    response.active_map = active_map_name_;
    response.active_map_uuid = active_map_uuid_;
    response.pending_map = pending_map_name_;
    response.pending_map_uuid = pending_map_uuid_;
    response.map_schema_version = fusion_4dof::kMapMetadataSchemaVersion;
    response.coordinate_frame = fusion_4dof::kMapCoordinateFrame;
    response.origin_checksum = map_switch_pending_ ? pending_origin_checksum_
                                                    : active_origin_checksum_;
    response.reason = reason;
  }

  void ResetMapCoupledState(const fusion_4dof::MapBundle &target) {
    globalEstimator_.ResetForMapSwitch();
    imu_fallback_.ResetGlobalAnchor();
    imu_test_.ResetGlobalAnchor();
    PublishInertialOdometryReady(false);
    last_main_pose_rx_sec_ = 0.0;
    previous_main_pose_rx_sec_ = 0.0;
    have_previous_main_pose_ = false;
    map_index_ = target.origin.map_index;
    origin_latitude_ = target.origin.latitude;
    origin_longitude_ = target.origin.longitude;
    origin_height_ = target.origin.height;
    origin_gauss_yaw_ = target.origin.gauss_yaw;
    map_origin_ready_ = true;
    location_mode_ = true;
    gps_init_ = false;
    system_init_ = false;
    heading_flag_ = 0;
    std::memset(xy_offset_, 0, sizeof(xy_offset_));
    last_gps_stamp_ = 0.0;
    std::memset(last_gps_, 0, sizeof(last_gps_));
    gps_course_axis_gate_.Reset();
    gps_drift_count_ = 0;
    gps_failed_cnt_ = 0;
    gps_failed_flag_ = false;
    gps_stable_cnt_ = 0;
    gps_missing_cnt_ = 0;
    gps_transient_reacquire_cnt_ = 0;
    gps_transient_fallback_ = false;
    gps_grace_anchor_valid_ = false;
    gps_grace_anchor_stamp_ = 0.0;
    gps_grace_anchor_generation_sec_ = 0;
    gps_grace_anchor_generation_nsec_ = 0;
    previous_lio_raw_pose_.valid = false;
    std::memset(gps_grace_anchor_lio_, 0, sizeof(gps_grace_anchor_lio_));
    std::memset(gps_grace_anchor_output_, 0,
                sizeof(gps_grace_anchor_output_));
    latest_gps_good_ = false;
    last_gps_good_steady_sec_ = 0.0;
    last_position_state_ = 0;
    last_position_publish_steady_sec_ = 0.0;
    last_fusion_time_ = 0.0;
    std::memset(last_fusion_pos_, 0, sizeof(last_fusion_pos_));
    output_continuity_gate_.Reset();
    fusion_failed_cnt_ = 0;
    no_pos_cnt_ = 0;
    no_rtk_cnt_ = 0;
    sync_failed_cnt_ = 0;
    gps_lio_sync_bad_first_steady_sec_ = 0.0;
    lio_bad_first_steady_sec_ = 0.0;
    lio_drift_hold_end_stamp_ = 0.0;
    publish_error_cnt_ = 0;
    pub_overtime_cnt_ = 0;
    fail_cnt_ = 0;
    last_alignment_save_wall_time_ = 0.0;
    publish_count = 0;
    publish_window_start = 0.0;
    viz_gps_.path.poses.clear();
    viz_lio_.path.poses.clear();
    viz_fusion_.path.poses.clear();
    viz_output_.path.poses.clear();
    viz_rtk_seed_.path.poses.clear();
    {
      std::lock_guard<std::mutex> lock(gps_mutex_);
      std::queue<util::GpsPositionConstPtr> empty;
      gps_queue_.swap(empty);
    }
    // Deliberately preserve last_lio_stamp_/last_lio_t_, lio_failed_cnt_,
    // lio_drift_* and the generation/static-calibration identity: they
    // describe the continuously running FAST-LIO sensor stream, not the old
    // map frame. LioCallBack continues advancing that health baseline while
    // system_init_ is false, so preserving it cannot create a stale-dt loop.
  }

  bool ManageLocalizationMap(
      util::ManageLocalizationMap::Request &request,
      util::ManageLocalizationMap::Response &response) {
    if (!fusion_4dof::IsSafeLogicalMapName(request.map_name)) {
      FillMapServiceResponse(response, false, false,
                             "invalid logical map name");
      return true;
    }
    if (request.command !=
            util::ManageLocalizationMap::Request::SAVE_CURRENT_AND_ACTIVATE &&
        request.command !=
            util::ManageLocalizationMap::Request::ACTIVATE_EXISTING) {
      FillMapServiceResponse(response, false, false,
                             "unsupported map transaction command");
      return true;
    }
    if (reboot || requested_restart_scope_ !=
                      util::LocalizationStatus::RESTART_NONE) {
      FillMapServiceResponse(response, false, false,
                             "localization recovery is active; map transaction rejected");
      return true;
    }

    const std::string named_path = NamedMapPath(request.map_name);
    fusion_4dof::MapBundle target;
    std::string error;
    if (request.command ==
        util::ManageLocalizationMap::Request::SAVE_CURRENT_AND_ACTIVATE) {
      if (!map_origin_ready_ || !system_init_ ||
          !lio_static_initialized_ || !lio_generation_valid_ ||
          !globalEstimator_.Is4DAligned()) {
        FillMapServiceResponse(response, false, false,
                               "cannot save/activate before map origin, "
                               "FAST-LIO generation, system init and 4DoF "
                               "alignment are all ready");
        return true;
      }
      const std::string uuid = fusion_4dof::GenerateUuidV4();
      target = CurrentMapBundle(request.map_name, uuid,
                                CurrentUnixNanoseconds(), &error);
      if (!error.empty() ||
          !fusion_4dof::WriteMapBundleAtomically(
              named_path, target, request.overwrite, &error)) {
        FillMapServiceResponse(response, false, false,
                               "save map transaction failed: " + error);
        return true;
      }
    } else {
      bool metadata_missing = false;
      if (!fusion_4dof::LoadMapBundle(named_path, request.map_name, true,
                                      &target, &metadata_missing, &error)) {
        FillMapServiceResponse(response, false, false,
                               "load map transaction failed: " + error);
        return true;
      }
      if (metadata_missing) {
        target.metadata.map_uuid = fusion_4dof::GenerateUuidV4();
        target.metadata.logical_name = request.map_name;
        target.metadata.origin_checksum =
            fusion_4dof::OriginChecksum(target.origin_bytes);
        target.metadata.created_unix_ns = CurrentUnixNanoseconds();
        if (!fusion_4dof::WriteMapBundleAtomically(named_path, target, true,
                                                   &error)) {
          FillMapServiceResponse(response, false, false,
                                 "legacy map metadata migration failed: " +
                                     error);
          return true;
        }
      }
    }

    error.clear();
    const fusion_4dof::MapBundle current = CurrentMapBundle(
        request.map_name, target.metadata.map_uuid,
        target.metadata.created_unix_ns, &error);
    const bool same_coordinate_frame =
        map_origin_ready_ && error.empty() &&
        current.origin_bytes == target.origin_bytes;

    // The canonical runtime slot is committed before any in-memory origin is
    // changed. A failed disk transaction therefore cannot create a runtime
    // pose in a map that would disappear after process restart.
    if (!fusion_4dof::WriteMapBundleAtomically(map_path_, target, true,
                                               &error)) {
      algorithm_fault_code_ = util::LocalizationStatus::FAULT_MAP_INVALID;
      algorithm_reason_ = "canonical map transaction failed: " + error;
      PublishStopCar(true);
      stop_car_active_ = true;
      FillMapServiceResponse(response, false, false, algorithm_reason_);
      PublishAlgorithmStatus();
      return true;
    }

    location_mode_ = true;
    requested_restart_scope_ = util::LocalizationStatus::RESTART_NONE;
    reboot = false;
    algorithm_fault_code_ = util::LocalizationStatus::FAULT_NONE;
    if (same_coordinate_frame) {
      active_map_name_ = target.metadata.logical_name;
      active_map_uuid_ = target.metadata.map_uuid;
      active_origin_checksum_ = target.metadata.origin_checksum;
      pending_map_name_.clear();
      pending_map_uuid_.clear();
      pending_origin_checksum_.clear();
      map_switch_pending_ = false;
      if (stop_car_active_ &&
          requested_restart_scope_ == util::LocalizationStatus::RESTART_NONE) {
        if (!output_timestamp_stop_active_ && !imu_odom_stop_active_)
          PublishStopCar(false);
        stop_car_active_ = false;
        stop_car_resume_cnt_ = 0;
        stop_car_last_trusted_output_steady_sec_ = 0.0;
      }
      algorithm_reason_ = "map activated in the current coordinate frame; "
                          "4DoF alignment preserved";
      if (globalEstimator_.Is4DAligned() && !SaveAlignmentState(true)) {
        ROS_WARN("Map activation succeeded, but UUID-bound 4DoF cache could "
                 "not be persisted; a later fusion restart may re-align.");
      }
      FillMapServiceResponse(response, true, true, algorithm_reason_);
    } else {
      ResetMapCoupledState(target);
      pending_map_name_ = target.metadata.logical_name;
      pending_map_uuid_ = target.metadata.map_uuid;
      pending_origin_checksum_ = target.metadata.origin_checksum;
      map_switch_pending_ = true;
      algorithm_reason_ = "map origin switched without restarting FAST-LIO; "
                          "waiting for fresh GPS/LIO 4DoF alignment";
      PublishStopCar(true);
      stop_car_active_ = true;
      stop_car_resume_cnt_ = 0;
      stop_car_last_trusted_output_steady_sec_ = 0.0;
      FillMapServiceResponse(response, true, false, algorithm_reason_);
    }
    PublishAlgorithmStatus();
    return true;
  }

  bool SaveMap(double latitude, double longitude, double height,
               double gauss_yaw) {
    static_assert(sizeof(int) == 4 && sizeof(double) == 8,
                  "The persisted map-origin format requires 4-byte int and "
                  "8-byte double.");
    const bool valid_origin =
        std::isfinite(latitude) && std::isfinite(longitude) &&
        std::isfinite(height) && std::isfinite(gauss_yaw) &&
        latitude >= -90.0 && latitude <= 90.0 && longitude >= -180.0 &&
        longitude <= 180.0 && gauss_yaw >= 0.0 && gauss_yaw < 360.0;
    if (!valid_origin) {
      ROS_ERROR("Error(Save map): Refuse invalid map origin: "
                "lat=%.10f lon=%.10f h=%.3f yaw=%.3f.",
                latitude, longitude, height, gauss_yaw);
      return false;
    }

    // The canonical origin and its identity/checksum are one transaction.
    // Writing only the legacy 36-byte .mp would leave an existing
    // .meta.yaml bound to the previous origin and make the next location-mode
    // recovery fail closed with a checksum mismatch.
    fusion_4dof::MapBundle bundle;
    bundle.origin.map_index = static_cast<int32_t>(map_index_);
    bundle.origin.latitude = latitude;
    bundle.origin.longitude = longitude;
    bundle.origin.height = height;
    bundle.origin.gauss_yaw = gauss_yaw;
    std::string error;
    if (!fusion_4dof::EncodeMapOrigin(bundle.origin, &bundle.origin_bytes,
                                      &error)) {
      ROS_ERROR("Error(Save map): Cannot encode map origin: %s.",
                error.c_str());
      return false;
    }
    bundle.metadata.map_uuid = fusion_4dof::GenerateUuidV4();
    // This is the durable canonical slot of an unfinished mapping workflow;
    // SAVE_CURRENT_AND_ACTIVATE later gives the user map its final logical name
    // and a separate UUID. Session provenance remains an explicit TODO.
    bundle.metadata.logical_name = "unsaved_mapping";
    bundle.metadata.origin_checksum =
        fusion_4dof::OriginChecksum(bundle.origin_bytes);
    bundle.metadata.created_unix_ns = CurrentUnixNanoseconds();
    if (!fusion_4dof::WriteMapBundleAtomically(map_path_, bundle, true,
                                               &error)) {
      ROS_ERROR("Error(Save map): Failed to commit map bundle %s: %s.",
                map_path_.c_str(), error.c_str());
      return false;
    }
    active_map_name_ = bundle.metadata.logical_name;
    active_map_uuid_ = bundle.metadata.map_uuid;
    active_origin_checksum_ = bundle.metadata.origin_checksum;
    return true;
  }

  bool ReadMap() {
    fusion_4dof::MapBundle bundle;
    bool metadata_missing = false;
    std::string error;
    if (!fusion_4dof::LoadMapBundle(map_path_, std::string(), true, &bundle,
                                    &metadata_missing, &error)) {
      ROS_ERROR("Error(Read map bundle): %s", error.c_str());
      return false;
    }
    map_index_ = bundle.origin.map_index;
    origin_latitude_ = bundle.origin.latitude;
    origin_longitude_ = bundle.origin.longitude;
    origin_height_ = bundle.origin.height;
    origin_gauss_yaw_ = bundle.origin.gauss_yaw;
    active_origin_checksum_ =
        fusion_4dof::OriginChecksum(bundle.origin_bytes);
    if (!metadata_missing) {
      active_map_name_ = bundle.metadata.logical_name;
      active_map_uuid_ = bundle.metadata.map_uuid;
      active_origin_checksum_ = bundle.metadata.origin_checksum;
    } else {
      ROS_WARN("Legacy canonical map has no persistent logical identity; "
               "save_map/use_map will migrate it to schema v1 metadata.");
    }
    map_origin_ready_ = true;
    return true;
  }

  void GPS2Local(double latitude, double longitude, double height,
                 double gauss_yaw, double *xyy) {
    if (!gps_init_) {
      if (!location_mode_) {
        geoConverter_.Reset(latitude, longitude, height);
        origin_gauss_yaw_ = gauss_yaw;
      } else {
        geoConverter_.Reset(origin_latitude_, origin_longitude_,
                            origin_height_);
        double u_xyz[3];
        geoConverter_.Forward(latitude, longitude, height, u_xyz[0], u_xyz[1],
                              u_xyz[2]);
        if (system_init_) {
          xy_offset_[0] = u_xyz[0] * cos(origin_gauss_yaw_ * DEG2RAD) +
                          u_xyz[1] * sin(origin_gauss_yaw_ * DEG2RAD);
          xy_offset_[1] = -u_xyz[0] * sin(origin_gauss_yaw_ * DEG2RAD) +
                          u_xyz[1] * cos(origin_gauss_yaw_ * DEG2RAD);
          xy_offset_[2] = gauss_yaw - origin_gauss_yaw_;
        }
      }
      gps_init_ = true;
    }
    double xyz[3];
    geoConverter_.Forward(latitude, longitude, height, xyz[0], xyz[1], xyz[2]);
    // 纯 ENU：位置直接取 East/North，不再绕 origin_gauss_yaw_ 旋转 ——
    // 从源头消除 RTK 航向杠杆 θ·d（origin_gauss_yaw_ 仅保留用于存储/调试）。
    xyy[0] = xyz[0]; // East
    xyy[1] = xyz[1]; // North
    // ENU 航向：由 GpsPosition 的 gauss_yaw 换算到“从 East 起、逆时针”的 ENU yaw。
    // 换算式由旧代码自身一致性导出：旧版 Is_GPS_GOOD 中 published_yaw == atan2(Δy,Δx)
    // 恒成立，代入旧 published_yaw 与旧旋转关系可解得 yaw_enu = gauss_yaw*DEG2RAD + π/2，
    // 不含 origin_gauss_yaw_（故航向亦无单点航向杠杆）。范围 (-π,π]（ConvertYaw）。
    // 注意：gauss_yaw = azimuth - 90°，故 yaw_enu = azimuth（弧度）。这只有在 INS 的
    // azimuth 本就是“正东=0、逆时针”(GpsPosition.msg 所述)时才是正确的绝对 ENU 航向。
    // 旧版航向是“相对起点”(gauss_yaw - origin_gauss_yaw)，对 azimuth 约定的任何常量
    // 偏差免疫(会抵消)；本版是绝对 ENU，不再抵消 —— 故务必实地核对约定：车头朝正东时
    // 发布 yaw≈0、朝正北时≈+π/2。若实测为“正北=0、顺时针”，应改为 (π/2 - azimuth)。
    double yaw = ConvertYaw(gauss_yaw * DEG2RAD + M_PI / 2);
    // xyy[2] 存相对原点高程(ENU up)，航向在 xyy[3]。位置数组须为 4 维。
    xyy[2] = xyz[2];
    xyy[3] = yaw;
  }

  double inline ConvertYaw(double yaw) {
    if (yaw > M_PI)
      yaw -= M_PI * 2;
    if (yaw < -M_PI)
      yaw += M_PI * 2;
    return yaw;
  }

  bool LIOPrecess(Eigen::Vector3d &lio_t, Eigen::Quaterniond &lio_q,
                  double *xyy_lio, double *xyy_seed,
                  bool *seed_transform_available) {
    // 第一步：复现旧代码中的平面轴变换
    //   old: lio_x = -msg->position_y; lio_y = msg->position_x;
    // 这里统一为 3D 固定旋转，位置和姿态同时处理。
    lio_t = lio_frame_align_rotation_ * lio_t;
    lio_q.normalize();
    lio_q = (lio_frame_align_rotation_ * lio_q).normalized();

    // 第二步：标准 6DoF 杆臂补偿
    //   p_w_base = p_w_lio + R_w_lio * t_lio_base
    //   q_w_base = q_w_lio * q_lio_base
    lio_t += lio_q * lio_to_base_translation_;
    lio_q = (lio_q * lio_to_base_rotation_).normalized();

    // 不再以fusion进程的“第一帧”重新定义xyz原点。FAST-LIO每代H系的
    // 任意原点（包括杆臂带来的固定平移）应完整由4DoF的tx/ty/tz吸收。
    // 否则fusion_only重启会把同一H系再次归零，z和全局平移都会跳变。

    double rol_t = 0, pitch_t = 0, yaw_t = 0;
    tf::Quaternion q_t(lio_q.x(), lio_q.y(), lio_q.z(), lio_q.w());
    tf::Matrix3x3(q_t).getRPY(rol_t, pitch_t, yaw_t);
    yaw_t = ConvertYaw(yaw_t);
    lio_raw_x_ = lio_t.x();
    lio_raw_y_ = lio_t.y();
    lio_raw_yaw_ = yaw_t;
    lio_raw_roll_ = rol_t;
    lio_raw_pitch_ = pitch_t;

    // 调试 LIO 位姿用与生产回退【完全相同】的4DoF(LIO H->ENU) 映射：
    // 估计器输入的就是这里杆臂补偿后、未重锚的base_link，故同源一致，
    // viz/pub_fusion==3 调试与对外发布严格同帧（不再用旧 xy_offset_ 旋转帧）。
    double se2_theta = 0.0, se2_tx = 0.0, se2_ty = 0.0, se2_tz = 0.0;
    const bool enu_transform_available =
        globalEstimator_.GetTransform4D(se2_theta, se2_tx, se2_ty, se2_tz);
    if (enu_transform_available) {
      double cse = cos(se2_theta), sse = sin(se2_theta);
      xyy_lio[0] = cse * lio_t.x() - sse * lio_t.y() + se2_tx;
      xyy_lio[1] = sse * lio_t.x() + cse * lio_t.y() + se2_ty;
      xyy_lio[2] = lio_t.z() + se2_tz;
      xyy_lio[3] = ConvertYaw(se2_theta + yaw_t);
    } else {
      // 尚未完成4DoF：数值仅供内部诊断，上层不得将其标为map或发布为有效位置。
      xyy_lio[0] = lio_t.x();
      xyy_lio[1] = lio_t.y();
      xyy_lio[2] = lio_t.z();
      xyy_lio[3] = yaw_t;
    }

    // 对比用：纯 RTK 位置+航向种子变换（与滑窗/锁定状态无关，每帧重算，见
    // README §11）映射同一个 lio_t，与上面官方 GetTransform 的结果并排发布，
    // 用于评估 SE(2) 滑窗估计相对朴素单帧方案的实际增益。计算方式、恒等回退
    // 分支均与上面 xyy_lio 对称，只是变换来源换成 GetSeedOnlyTransform。
    double seed_theta = 0.0, seed_tx = 0.0, seed_ty = 0.0, seed_tz = 0.0;
    const bool have_seed_transform =
        globalEstimator_.GetSeedOnlyTransform4D(seed_theta, seed_tx, seed_ty,
                                                seed_tz);
    if (seed_transform_available != nullptr)
      *seed_transform_available = have_seed_transform;
    if (have_seed_transform) {
      double css = cos(seed_theta), sss = sin(seed_theta);
      xyy_seed[0] = css * lio_t.x() - sss * lio_t.y() + seed_tx;
      xyy_seed[1] = sss * lio_t.x() + css * lio_t.y() + seed_ty;
      xyy_seed[2] = lio_t.z() + seed_tz;
      xyy_seed[3] = ConvertYaw(seed_theta + yaw_t);
    } else {
      xyy_seed[0] = lio_t.x();
      xyy_seed[1] = lio_t.y();
      xyy_seed[2] = lio_t.z();
      xyy_seed[3] = yaw_t;
    }
    return enu_transform_available;
  }

  void GetPosition(double *xyy_gps) {
    Eigen::Vector3d global_t;
    Eigen::Quaterniond global_q;
    globalEstimator_.GetGlobalOdom(global_t, global_q);
    double roll, pitch, yaw;
    tf::Quaternion tf_q(global_q.x(), global_q.y(), global_q.z(), global_q.w());
    tf::Matrix3x3(tf_q).getRPY(roll, pitch, yaw);
    // global_q = Rz(theta_H_to_ENU) * q_H_base。从这个【最终旋转矩阵】
    // 提取的RPY是与最终map->base_link四元数严格一致的LIO姿态。
    // 不再叠加 xy_offset_[2]（否则与 theta_ 重复计旋转）。
    yaw = ConvertYaw(yaw);
    lio_global_roll_ = roll;
    lio_global_pitch_ = pitch;
    xyy_gps[0] = global_t.x();
    xyy_gps[1] = global_t.y();
    xyy_gps[2] = global_t.z();
    xyy_gps[3] = yaw;
  }

  void Is_rate_ok() {
    publish_window_start = (publish_window_start == 0)
                               ? ros::Time::now().toSec()
                               : publish_window_start;
    double current_time = ros::Time::now().toSec();
    double window_duration = current_time - publish_window_start;
    // printf("current_time = %14.4f\n", current_time);
    // printf("publish_window_start = %14.4f\n", publish_window_start);
    // std::cout << "Duration: " << window_duration << std::endl;

    if (system_init_ && heading_flag_ &&
        window_duration >= 0.95) // 检查 1 秒内的发布频率
    {
      double publish_frequency = publish_count / window_duration;
      if (publish_frequency < 8) {
        publish_error_cnt_++;
        reboot = publish_error_cnt_ >= 20 ? true : reboot;
        if (reboot) {
          ROS_WARN(
              "Publish frequency is too low: %.2f Hz, restarting the node!",
              publish_frequency);
        }
      } else {
        publish_error_cnt_ = 0;
      }
      publish_window_start = current_time;
      publish_count = 0; // 重置计数器
    }
  }

  void SaveFusionPositionToFile(const double *fusion_pos) {
    std::ofstream file(last_pub_pos_path, std::ios::out);
    if (!file.is_open()) {
      ROS_ERROR("Failed to open file for saving fusion position: %s",
                last_pub_pos_path.c_str());
      return;
    }
    file << fusion_pos[0] << " " << fusion_pos[1] << " " << fusion_pos[2]
         << std::endl; // x, y, yaw
    file.close();
  }
  bool LoadFusionPositionFromFile(double *fusion_pos) {
    std::ifstream file(last_pub_pos_path, std::ios::in);
    if (!file.is_open()) {
      // ROS_WARN("No saved fusion position file found: %s",
      // last_pub_pos_path.c_str());
      return false;
    }
    file >> fusion_pos[0] >> fusion_pos[1] >> fusion_pos[2]; // x, y, yaw
    file.close();
    ROS_INFO("Loaded fusion position from file: %s", last_pub_pos_path.c_str());
    return true;
  }

private:
  ros::NodeHandle nh_;
  ros::Publisher pubPosition_, pub_fusion_Reboot_, pub_gps_Reboot_;
  ros::Publisher pub_algorithm_status_;
  ros::Publisher pub_imu_odom_status_, pub_imu_odom_ready_,
      pub_imu_fallback_odom_,
      pub_imu_test_odom_;
  ros::Subscriber subGps_, subLio_, subLioStatic_, subLioGeneration_,
      subAndroid_, subMid360Imu_, subLioStaticCalibration_;
  ros::Timer imu_odom_timer_;
  ros::Timer algorithm_status_timer_;
  ros::ServiceServer map_service_;
  fusion_4dof::InertialDeadReckoning imu_fallback_, imu_test_;
  bool imu_odom_enable_;
  bool imu_odom_test_from_startup_;
  double mid360_imu_acc_scale_;
  bool imu_odom_stop_active_;
  bool imu_odom_ready_;
  bool imu_odom_ready_published_;
  util::ImuStaticCalibration pending_imu_static_calibration_;
  bool have_pending_imu_static_calibration_;
  bool imu_calibration_applied_;
  uint32_t applied_imu_calibration_generation_sec_;
  uint32_t applied_imu_calibration_generation_nsec_;
  std::string mid360_imu_frame_id_;
  std::string mid360_imu_topic_;
  std::string imu_calibration_frame_id_;
  double imu_odom_main_pose_timeout_sec_;
  double lio_recovery_grace_timeout_sec_;
  double last_main_pose_rx_sec_;
  double previous_main_pose_rx_sec_;
  Eigen::Vector3d previous_main_position_;
  bool have_previous_main_pose_;
  double imu_odom_max_velocity_mps_;
  double imu_odom_fallback_max_duration_for_log_;
  double imu_odom_fallback_max_distance_for_log_;
  bool location_mode_;
  bool gps_init_;
  bool system_init_;
  int fail_cnt_;
  int map_index_;

  double origin_gauss_yaw_, origin_latitude_, origin_longitude_, origin_height_;
  std::string map_path_;
  std::string alignment_state_path_;
  std::string active_map_name_, active_map_uuid_, active_origin_checksum_;
  std::string pending_map_name_, pending_map_uuid_, pending_origin_checksum_;
  bool map_switch_pending_;
  bool map_origin_ready_;
  bool fusion_generation_valid_;
  uint32_t fusion_generation_sec_;
  uint32_t fusion_generation_nsec_;
  uint32_t last_position_state_;
  double last_position_publish_steady_sec_;
  bool ins_status_received_;
  uint8_t latest_ins_status_;
  uint8_t latest_ins_vehicle_align_;
  uint8_t latest_ins_heading_flag_;
  int32_t latest_ins_position_status_;
  bool latest_gps_good_;
  bool latest_lio_good_;
  bool latest_raw_gps_usable_;
  double last_gps_good_steady_sec_;
  double last_lio_good_steady_sec_;
  double last_raw_gps_usable_steady_sec_;
  uint8_t requested_restart_scope_;
  uint16_t algorithm_fault_code_;
  std::string algorithm_reason_;
  bool lio_generation_valid_;
  bool lio_level_transform_ready_;
  uint32_t lio_generation_sec_;
  uint32_t lio_generation_nsec_;
  double last_alignment_save_wall_time_;
  bool vertical_require_ins_height_std_;
  double vertical_max_ins_height_std_;
  bool diagnostic_log_enable_;
  double diagnostic_log_period_sec_;
  double last_diagnostic_log_wall_time_;
  double gps_lio_sync_tolerance_sec_;
  double gps_motion_min_dt_sec_;
  double gps_motion_max_dt_sec_;
  double gps_motion_max_speed_mps_;
  double gps_motion_margin_m_;
  bool gps_grace_coast_output_enabled_;
  double lio_motion_max_dt_sec_;
  double lio_motion_margin_m_;
  double lio_yaw_warning_min_speed_mps_;
  double lio_yaw_warning_max_error_rad_;
  double lio_time_alignment_max_pair_dt_sec_;
  double lio_time_alignment_max_extrapolation_sec_;
  double gps_coast_max_duration_sec_;
  double gps_heading_suspect_coast_max_duration_sec_;
  double output_max_measurement_age_sec_;
  double output_max_future_stamp_sec_;
  int gps_queue_max_size_;
  double last_gps_callback_stamp_;
  double last_accepted_lio_input_stamp_;
  int lio_restart_after_bad_frames_;
  double lio_bad_max_duration_sec_;
  double lio_bad_first_steady_sec_;
  int gps_lio_sync_restart_after_frames_;
  double gps_lio_sync_bad_max_duration_sec_;
  double gps_lio_sync_bad_first_steady_sec_;
  int lio_drift_hold_after_bad_frames_;
  int lio_drift_hold_frames_;
  double lio_drift_hold_max_duration_sec_;
  double lio_drift_hold_end_stamp_;
  int lio_fallback_max_bad_frames_;
  fusion_4dof::RtkQualityConfig rtk_quality_config_;
  fusion_4dof::PoseContinuityGate output_continuity_gate_;
  double output_yaw_margin_deg_;
  std::string last_pub_pos_path;
  std::queue<util::GpsPositionConstPtr> gps_queue_;
  std::mutex gps_mutex_;

  double xy_offset_[3];
  double last_gps_[3];

  GlobalOptimization globalEstimator_;
  GeographicLib::LocalCartesian geoConverter_;

  double last_gps_stamp_;
  fusion_4dof::GpsCourseAxisGate gps_course_axis_gate_;
  double gps_course_axis_turn_rate_radps_ = 0.25;
  int gps_drift_count_;

  double last_lio_stamp_;
  Eigen::Vector3d last_lio_t_;

  int pub_fusion;

  Eigen::Quaterniond lio_frame_align_rotation_;
  Eigen::Vector3d lio_to_base_translation_baseframe_;
  Eigen::Vector3d lio_to_base_translation_;
  Eigen::Quaterniond lio_to_base_rotation_;

  // LIO 帧代际变化检测（/laserMapping 重启回原点）
  bool lio_seen_far_;            // LIO 是否已离开原点
  double lio_epoch_near_thresh_; // 判“回到原点附近”的距离阈值(m)
  double lio_epoch_far_thresh_;  // 判“已远离原点”的距离阈值(m)
  double lio_epoch_jump_thresh_; // 判“瞬时大跳变”的单帧位移阈值(m)
  Eigen::Vector3d prev_raw_lio_t_; // 上一帧原始 LIO 位置（LIOPrecess 修改前）
  bool has_prev_raw_lio_;          // prev_raw_lio_t_ 是否已记录
  uint32_t last_lio_seq_;          // 上一帧 LIO header.seq（代际检测）
  bool has_last_lio_seq_;          // last_lio_seq_ 是否已记录
  bool lio_restart_pending_;       // 本节点已发起 LIO 重启、待首帧新 LIO 确认
  bool controlled_lio_recovery_;   // 有界的LIO-only断流/新代重对齐宽限期
  bool lio_recovery_new_generation_received_;
  double lio_recovery_start_steady_sec_;

  // 重启/laserMapping期间的独立停车请求。android_manager_4dof 将它与
  // manager自身请求做 OR 后唯一发布 /mower/stop_car，避免多个 Bool
  // 发布者的 false 互相覆盖。
  ros::Publisher pub_stop_car_;
  std::string stop_car_topic_;     // 默认 /mower/fusion_stop_car
  bool stop_car_active_;           // 正处于“为重启 LIO 而停车”状态
  bool output_timestamp_stop_active_; // 消息积压/未来时间戳导致的独立停车
  int output_timestamp_resume_count_;
  int stop_car_resume_cnt_;        // 重启后 LIO 连续健康帧计数
  int stop_car_resume_frames_;     // 达此连续健康帧数才解除停车
  double stop_car_last_trusted_output_steady_sec_;

  void PublishStopCar(bool stop) {
    std_msgs::Bool m;
    m.data = stop;
    pub_stop_car_.publish(m);
  }

  void RequestFullRestart(const char *reason) {
    reboot = true;
    requested_restart_scope_ = util::LocalizationStatus::RESTART_ALL;
    algorithm_fault_code_ = util::LocalizationStatus::FAULT_ALGORITHM;
    algorithm_reason_ = reason != nullptr ? reason : "Localization FAULT.";
    PublishStopCar(true);
    PublishInertialOdometryReady(false);
    if (reboot_pub_flag_)
      return;
    std_msgs::Bool fusion_reboot_msg;
    fusion_reboot_msg.data = true;
    pub_fusion_Reboot_.publish(fusion_reboot_msg);
    reboot_pub_flag_ = true;
    ROS_ERROR("%s /Mower/reboot-equivalent topic published; waiting for "
              "android_manager_4dof to restart /fusion_4dof and "
              "/laserMapping.",
              reason != nullptr ? reason : "Localization FAULT.");
  }

  void RequestLioRestart(const char *reason) {
    if (requested_restart_scope_ == util::LocalizationStatus::RESTART_LIO ||
        controlled_lio_recovery_) {
      ROS_WARN_THROTTLE(1.0, "Ignore duplicate LIO restart request while the "
                             "controlled recovery window owns this outage.");
      return;
    }
    requested_restart_scope_ = util::LocalizationStatus::RESTART_LIO;
    algorithm_fault_code_ = util::LocalizationStatus::FAULT_NONE;
    algorithm_reason_ = reason != nullptr ? reason : "LIO recovery requested.";
    lio_restart_pending_ = true;
    controlled_lio_recovery_ = true;
    lio_recovery_new_generation_received_ = false;
    lio_recovery_start_steady_sec_ = ros::SteadyTime::now().toSec();
    PublishStopCar(true);
    stop_car_active_ = true;
    stop_car_resume_cnt_ = 0;
    stop_car_last_trusted_output_steady_sec_ = 0.0;
    PublishAlgorithmStatus();
    ROS_WARN("%s manager will restart /laserMapping only.",
             algorithm_reason_.c_str());
  }

  double lio_speed_threshold_;
  double lio_stamp_;
  int lio_drift_count_;
  bool lio_drift_detected_;

  double lio_raw_x_;
  double lio_raw_y_;
  double lio_raw_yaw_;
  double lio_raw_roll_;
  double lio_raw_pitch_;
  double lio_global_roll_;
  double lio_global_pitch_;

  // Real-time externally published base_link roll/pitch. The INS conversion
  // follows util/GpsPosition.msg's documented right/forward axes; a fresh and
  // valid INS sample is preferred, otherwise the gravity-leveled LIO attitude
  // is used for the current LIO-driven output.
  std::string roll_pitch_source_;
  double ins_roll_scale_rad_;
  double ins_pitch_scale_rad_;
  double ins_attitude_max_age_sec_;
  double ins_attitude_max_abs_rad_;
  double ins_lio_max_disagreement_rad_;
  bool ins_attitude_require_position_status_;
  bool latest_ins_attitude_valid_;
  ros::Time latest_ins_attitude_stamp_;
  double latest_ins_roll_;
  double latest_ins_pitch_;
  bool lio_static_initialized_;

  int no_pos_cnt_;
  bool reboot;

  int gps_failed_cnt_; // gps连续异常计数
  int lio_failed_cnt_;

  double last_fusion_time_;
  double last_fusion_pos_[4]; // x,y,z,yaw

  int fusion_failed_cnt_;

  int pub_overtime_cnt_;

  int gps_stable_cnt_;
  int gps_initial_stable_count_;
  int gps_recovery_floor_;
  int gps_stable_threshold_;
  int gps_missing_grace_frames_;
  int gps_transient_reacquire_frames_;
  int gps_missing_cnt_;
  int gps_transient_reacquire_cnt_;
  bool gps_transient_fallback_;
  bool gps_failed_flag_;
  bool gps_grace_anchor_valid_;
  double gps_grace_anchor_stamp_;
  unsigned int gps_grace_anchor_generation_sec_;
  unsigned int gps_grace_anchor_generation_nsec_;
  double gps_grace_anchor_lio_[4];
  double gps_grace_anchor_output_[4];
  fusion_4dof::TimedLioRawPose previous_lio_raw_pose_;

  double publish_window_start;
  int publish_count;

  int no_rtk_cnt_;
  int publish_error_cnt_;

  int heading_flag_;

  int sync_failed_cnt_;

  bool lio_bad_flag_;

  bool reboot_pub_flag_;
  bool require_gravity_level_lio_frame_;

#ifdef DEBUG
  ros::Publisher pubDebugGPS_, pubDebugLIO_, pubDebugFusion_, pubDebugRtkSeed_;
#endif

  // ===== RViz 可视化（纯增量，不影响融合算法逻辑）=====
  bool enable_viz_;             // 是否发布可视化话题
  std::string viz_frame_id_;    // 所有可视化的统一坐标系（默认 "map"）
  int viz_path_max_;            // 单条 Path 最大点数，限制内存
  RvizTrack viz_gps_;           // 纯 GPS 位姿
  RvizTrack viz_lio_;           // 纯 LIO 位姿（官方 GetTransform，播种/锁定后更精确）
  RvizTrack viz_fusion_;        // ceres 融合位姿
  RvizTrack viz_output_;        // 实际对外发布的定位结果
  RvizTrack viz_rtk_seed_;      // 纯 RTK 位置+航向种子变换（GetSeedOnlyTransform，见 README §11）
  tf::TransformBroadcaster viz_tf_broadcaster_;
};

int main(int argc, char **argv) {
  // launch 会覆盖该默认名；直接运行可执行文件时也保持与部署节点名一致。
  ros::init(argc, argv, "fusion_4dof");
  FusionNode node;
  ros::spin();
  return 0;
}
