#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <cmath>
#include <cstddef>

namespace fusion_4dof {

struct InertialDeadReckoningConfig {
  double gravity_mps2 = 9.80665;
  double max_imu_age_sec = 0.10;
  double max_integration_gap_sec = 0.10;
  double max_gyro_radps = 3.0;
  double max_accel_mps2 = 30.0;
  double max_velocity_mps = 2.0;
  double fallback_max_duration_sec = 10.0;
  double fallback_max_distance_m = 3.0;
  // State 6 is a braking-only mode.  Once the bias-corrected IMU has remained
  // quiet for this long, apply zero-velocity/zero-angular-rate constraints so
  // residual bias cannot be double-integrated into fictitious travel.
  double stationary_gyro_threshold_radps = 0.03;
  double stationary_accel_threshold_mps2 = 0.20;
  double stationary_confirmation_sec = 0.50;
};

class InertialDeadReckoning {
public:
  enum class Status { kUnready, kUsable, kStale, kExpired };

  explicit InertialDeadReckoning(
      const InertialDeadReckoningConfig &config = {})
      : config_(config) {}

  bool InputImu(double stamp, const Eigen::Vector3d &accel_base,
                const Eigen::Vector3d &gyro_base) {
    if (!Finite(stamp) || stamp <= 0.0 || !accel_base.allFinite() ||
        !gyro_base.allFinite() ||
        accel_base.norm() > config_.max_accel_mps2 ||
        gyro_base.norm() > config_.max_gyro_radps) {
      BreakIntegration();
      return false;
    }

    latest_accel_base_ = accel_base;
    latest_gyro_base_ = gyro_base;
    latest_imu_stamp_ = stamp;
    have_imu_ = true;

    if (!initialized_ || !bias_ready_) {
      integration_continuous_ = false;
      return true;
    }

    if (last_integration_stamp_ <= 0.0) {
      PrimeIntegration(stamp, accel_base, gyro_base);
      return true;
    }
    const double dt = stamp - last_integration_stamp_;
    if (dt <= 0.0 || dt > config_.max_integration_gap_sec) {
      BreakIntegration();
      PrimeIntegration(stamp, accel_base, gyro_base);
      return false;
    }

    const Eigen::Vector3d omega_now = gyro_base - gyro_bias_base_;
    const Eigen::Vector3d omega_mid = 0.5 * (last_omega_base_ + omega_now);
    const Eigen::Quaterniond dq_half = DeltaQuaternion(omega_mid, 0.5 * dt);
    const Eigen::Quaterniond q_mid = (q_enu_base_ * dq_half).normalized();
    const Eigen::Vector3d specific_now = accel_base - accel_bias_base_;
    const Eigen::Vector3d specific_mid =
        0.5 * (last_specific_force_base_ + specific_now);
    const Eigen::Vector3d accel_enu =
        q_mid * specific_mid +
        Eigen::Vector3d(0.0, 0.0, -config_.gravity_mps2);
    if (!accel_enu.allFinite()) {
      BreakIntegration();
      return false;
    }

    if (fallback_active_) {
      // Hysteresis prevents one noisy sample from releasing an established
      // stationary hold.  This constraint is intentionally restricted to the
      // emergency/braking trajectory: constant-speed motion is not observable
      // from an IMU alone and must never be suppressed in the diagnostic
      // free-running integrator.
      const double release_scale = stationary_hold_active_ ? 2.0 : 1.0;
      const bool quiet =
          omega_now.norm() <=
              release_scale * config_.stationary_gyro_threshold_radps &&
          stationary_reference_valid_ &&
          (specific_now - stationary_specific_force_reference_base_).norm() <=
              release_scale * config_.stationary_accel_threshold_mps2;
      if (quiet) {
        if (stationary_candidate_duration_sec_ <= 0.0) {
          stationary_candidate_position_ = position_enu_;
          stationary_candidate_attitude_ = q_enu_base_;
        }
        stationary_candidate_duration_sec_ += dt;
        if (stationary_candidate_duration_sec_ >=
            config_.stationary_confirmation_sec)
          stationary_hold_active_ = true;
        // Freeze from the first quiet sample, rather than publishing half a
        // second of fictitious motion and then jumping back when confirmation
        // completes. The confirmed flag controls hysteresis/diagnostics; the
        // candidate freeze is safe because state 6 is already braking-only.
        position_enu_ = stationary_candidate_position_;
        velocity_enu_.setZero();
        q_enu_base_ = stationary_candidate_attitude_;
        last_specific_force_base_ = specific_now;
        last_omega_base_ = omega_now;
        last_integration_stamp_ = stamp;
        integration_continuous_ = true;
        return true;
      } else {
        ResetStationaryConstraint();
      }
    }

    const Eigen::Vector3d delta_position =
        velocity_enu_ * dt + 0.5 * accel_enu * dt * dt;
    position_enu_ += delta_position;
    velocity_enu_ += accel_enu * dt;
    q_enu_base_ =
        (q_enu_base_ * DeltaQuaternion(omega_mid, dt)).normalized();
    integrated_distance_m_ += delta_position.norm();
    if (fallback_active_)
      fallback_distance_m_ += delta_position.norm();

    last_specific_force_base_ = specific_now;
    last_omega_base_ = omega_now;
    last_integration_stamp_ = stamp;
    integration_continuous_ = true;

    if (!position_enu_.allFinite() || !velocity_enu_.allFinite() ||
        !q_enu_base_.coeffs().allFinite() ||
        velocity_enu_.norm() > config_.max_velocity_mps) {
      BreakIntegration();
      return false;
    }
    return true;
  }

  // Consume the single authoritative static window confirmed by FAST-LIO.
  // The caller is responsible for matching its LIO-generation token before
  // transforming these raw-IMU means into base_link. This replaces the old
  // independent detector and deliberately resets the production integrator so
  // the next trusted main pose finalizes the new acceleration bias.
  bool SetStaticCalibration(const Eigen::Vector3d &mean_accel_base,
                            const Eigen::Vector3d &mean_gyro_base) {
    if (!mean_accel_base.allFinite() || !mean_gyro_base.allFinite() ||
        mean_accel_base.norm() <= 1e-6 ||
        mean_accel_base.norm() > config_.max_accel_mps2 ||
        mean_gyro_base.norm() > config_.max_gyro_radps)
      return false;
    calibration_mean_accel_base_ = mean_accel_base;
    calibration_mean_gyro_base_ = mean_gyro_base;
    calibration_candidate_ready_ = true;
    bias_ready_ = false;
    accel_bias_base_.setZero();
    gyro_bias_base_.setZero();
    initialized_ = false;
    integration_continuous_ = false;
    position_enu_.setZero();
    velocity_enu_.setZero();
    q_enu_base_.setIdentity();
    fallback_active_ = false;
    fallback_broken_ = false;
    last_integration_stamp_ = 0.0;
    fallback_start_stamp_ = 0.0;
    fallback_distance_m_ = 0.0;
    integrated_distance_m_ = 0.0;
    stationary_reference_valid_ = false;
    ResetStationaryConstraint();
    return true;
  }

  void InvalidateStaticCalibration() {
    calibration_mean_accel_base_.setZero();
    calibration_mean_gyro_base_.setZero();
    calibration_candidate_ready_ = false;
    bias_ready_ = false;
    accel_bias_base_.setZero();
    gyro_bias_base_.setZero();
    initialized_ = false;
    integration_continuous_ = false;
    position_enu_.setZero();
    velocity_enu_.setZero();
    q_enu_base_.setIdentity();
    fallback_active_ = false;
    fallback_broken_ = false;
    last_integration_stamp_ = 0.0;
    fallback_start_stamp_ = 0.0;
    fallback_distance_m_ = 0.0;
    integrated_distance_m_ = 0.0;
    stationary_reference_valid_ = false;
    ResetStationaryConstraint();
  }

  // A map-origin switch changes only the global ENU anchor. Preserve the
  // base-frame IMU calibration, but make every global pose unavailable until
  // a trusted production pose in the new map re-anchors the integrator.
  void ResetGlobalAnchor() {
    initialized_ = false;
    integration_continuous_ = false;
    position_enu_.setZero();
    velocity_enu_.setZero();
    q_enu_base_.setIdentity();
    fallback_active_ = false;
    fallback_broken_ = false;
    last_integration_stamp_ = 0.0;
    fallback_start_stamp_ = 0.0;
    fallback_distance_m_ = 0.0;
    integrated_distance_m_ = 0.0;
    stationary_reference_valid_ = false;
    ResetStationaryConstraint();
  }

  // A trusted production pose supplies the otherwise unobservable global
  // origin/yaw and converts the completed startup-static calibration into
  // base-frame IMU biases. In test mode allow_reanchor=false after the first
  // anchor, so the trajectory remains free-running for error comparison.
  bool SetMainPose(double stamp, const Eigen::Vector3d &position_enu,
                   const Eigen::Quaterniond &q_enu_base,
                   const Eigen::Vector3d &velocity_enu,
                   bool allow_reanchor) {
    if (!Finite(stamp) || stamp <= 0.0 || !position_enu.allFinite() ||
        !velocity_enu.allFinite() || !q_enu_base.coeffs().allFinite() ||
        q_enu_base.norm() < 1e-6)
      return false;
    const Eigen::Quaterniond q = q_enu_base.normalized();
    FinalizeBiasIfReady(q);
    if (!bias_ready_)
      return false;
    if (initialized_ && !allow_reanchor)
      return true;

    position_enu_ = position_enu;
    velocity_enu_ = velocity_enu;
    q_enu_base_ = q;
    trusted_anchor_position_ = position_enu;
    trusted_anchor_attitude_ = q;
    stationary_specific_force_reference_base_ =
        q.inverse() * Eigen::Vector3d(0.0, 0.0, config_.gravity_mps2);
    stationary_reference_valid_ = true;
    initialized_ = true;
    integration_continuous_ = false;
    fallback_active_ = false;
    fallback_broken_ = false;
    fallback_distance_m_ = 0.0;
    last_integration_stamp_ = 0.0;
    ResetStationaryConstraint();
    if (have_imu_)
      PrimeIntegration(latest_imu_stamp_, latest_accel_base_,
                       latest_gyro_base_);
    return true;
  }

  bool StartFallback(double now) {
    if (SourceStatus(now) != Status::kUsable)
      return false;
    fallback_active_ = true;
    fallback_broken_ = false;
    fallback_start_stamp_ = now;
    fallback_distance_m_ = 0.0;
    // The last trusted pose is position-authoritative, but its finite-
    // difference velocity is not: centimetre-level localization jitter can
    // otherwise become a persistent false velocity in state 6.
    velocity_enu_.setZero();
    ResetStationaryConstraint();
    const Eigen::Vector3d latest_omega = latest_gyro_base_ - gyro_bias_base_;
    const Eigen::Vector3d latest_specific =
        latest_accel_base_ - accel_bias_base_;
    if (stationary_reference_valid_ &&
        latest_omega.norm() <= config_.stationary_gyro_threshold_radps &&
        (latest_specific - stationary_specific_force_reference_base_).norm() <=
            config_.stationary_accel_threshold_mps2) {
      // Discard drift accumulated while waiting for both primary-source
      // timeout gates. Quiet raw IMU evidence proves that this interval was
      // stationary relative to the last trusted pose; integrated attitude is
      // deliberately not used in this test because gyro bias is the drift
      // source being guarded against.
      position_enu_ = trusted_anchor_position_;
      q_enu_base_ = trusted_anchor_attitude_;
      ResetStationaryConstraint();
    }
    return true;
  }

  // Stop publishing the emergency trajectory when either primary source has
  // returned. Keep the last trusted/integrated state so the next production
  // pose can re-anchor normally; this function never makes IMU output valid.
  void CancelFallback() {
    fallback_active_ = false;
    fallback_broken_ = false;
    fallback_start_stamp_ = 0.0;
    fallback_distance_m_ = 0.0;
    ResetStationaryConstraint();
  }

  Status SourceStatus(double now) const {
    if (!initialized_ || !bias_ready_ || !have_imu_)
      return Status::kUnready;
    if (!Finite(now) || now < latest_imu_stamp_ ||
        now - latest_imu_stamp_ > config_.max_imu_age_sec ||
        !integration_continuous_ || fallback_broken_)
      return Status::kStale;
    if (fallback_active_ &&
        (now - fallback_start_stamp_ > config_.fallback_max_duration_sec ||
         fallback_distance_m_ > config_.fallback_max_distance_m))
      return Status::kExpired;
    return Status::kUsable;
  }

  bool GetState(Eigen::Vector3d *position_enu,
                Eigen::Quaterniond *q_enu_base,
                Eigen::Vector3d *velocity_enu) const {
    if (!initialized_ || position_enu == nullptr || q_enu_base == nullptr ||
        velocity_enu == nullptr)
      return false;
    *position_enu = position_enu_;
    *q_enu_base = q_enu_base_;
    *velocity_enu = velocity_enu_;
    return true;
  }

  bool fallback_active() const { return fallback_active_; }
  bool bias_ready() const { return bias_ready_; }
  bool initialized() const { return initialized_; }
  bool stationary_hold_active() const { return stationary_hold_active_; }
  bool has_static_calibration() const {
    return calibration_candidate_ready_ || bias_ready_;
  }
  double fallback_distance_m() const { return fallback_distance_m_; }
  double integrated_distance_m() const { return integrated_distance_m_; }

private:
  static bool Finite(double value) { return std::isfinite(value); }

  static Eigen::Quaterniond DeltaQuaternion(const Eigen::Vector3d &omega,
                                             double dt) {
    const Eigen::Vector3d rotation = omega * dt;
    const double angle = rotation.norm();
    if (angle < 1e-12)
      return Eigen::Quaterniond::Identity();
    return Eigen::Quaterniond(Eigen::AngleAxisd(angle, rotation / angle));
  }

  void FinalizeBiasIfReady(const Eigen::Quaterniond &q_enu_base) {
    if (bias_ready_ || !calibration_candidate_ready_)
      return;
    const Eigen::Vector3d expected_specific_force_base =
        q_enu_base.inverse() *
        Eigen::Vector3d(0.0, 0.0, config_.gravity_mps2);
    accel_bias_base_ =
        calibration_mean_accel_base_ - expected_specific_force_base;
    gyro_bias_base_ = calibration_mean_gyro_base_;
    bias_ready_ = accel_bias_base_.allFinite() && gyro_bias_base_.allFinite();
  }

  void PrimeIntegration(double stamp, const Eigen::Vector3d &accel,
                        const Eigen::Vector3d &gyro) {
    last_integration_stamp_ = stamp;
    last_specific_force_base_ = accel - accel_bias_base_;
    last_omega_base_ = gyro - gyro_bias_base_;
    integration_continuous_ = false;
  }

  void BreakIntegration() {
    integration_continuous_ = false;
    if (fallback_active_)
      fallback_broken_ = true;
  }

  void ResetStationaryConstraint() {
    stationary_candidate_duration_sec_ = 0.0;
    stationary_hold_active_ = false;
    stationary_candidate_position_ = position_enu_;
    stationary_candidate_attitude_ = q_enu_base_;
  }

  InertialDeadReckoningConfig config_;
  bool have_imu_ = false;
  double latest_imu_stamp_ = 0.0;
  Eigen::Vector3d latest_accel_base_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d latest_gyro_base_ = Eigen::Vector3d::Zero();

  Eigen::Vector3d calibration_mean_accel_base_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d calibration_mean_gyro_base_ = Eigen::Vector3d::Zero();
  bool calibration_candidate_ready_ = false;
  bool bias_ready_ = false;
  Eigen::Vector3d accel_bias_base_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d gyro_bias_base_ = Eigen::Vector3d::Zero();

  bool initialized_ = false;
  bool integration_continuous_ = false;
  double last_integration_stamp_ = 0.0;
  Eigen::Vector3d position_enu_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d velocity_enu_ = Eigen::Vector3d::Zero();
  Eigen::Quaterniond q_enu_base_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d last_specific_force_base_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d last_omega_base_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d trusted_anchor_position_ = Eigen::Vector3d::Zero();
  Eigen::Quaterniond trusted_anchor_attitude_ =
      Eigen::Quaterniond::Identity();
  Eigen::Vector3d stationary_specific_force_reference_base_ =
      Eigen::Vector3d::Zero();
  bool stationary_reference_valid_ = false;
  double integrated_distance_m_ = 0.0;

  bool fallback_active_ = false;
  bool fallback_broken_ = false;
  double fallback_start_stamp_ = 0.0;
  double fallback_distance_m_ = 0.0;
  double stationary_candidate_duration_sec_ = 0.0;
  bool stationary_hold_active_ = false;
  Eigen::Vector3d stationary_candidate_position_ = Eigen::Vector3d::Zero();
  Eigen::Quaterniond stationary_candidate_attitude_ =
      Eigen::Quaterniond::Identity();
};

} // namespace fusion_4dof
