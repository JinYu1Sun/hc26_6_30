#pragma once

#include <algorithm>
#include <cmath>

namespace fusion_4dof {

inline double WrapAngle(double angle) {
  return std::atan2(std::sin(angle), std::cos(angle));
}

struct PoseContinuityConfig {
  double max_speed_mps = 3.0;
  double xy_margin_m = 0.15;
  double max_xy_jump_m = 0.75;
  double max_source_switch_xy_jump_m = 0.25;
  double max_yaw_rate_radps = 2.0;
  double yaw_margin_rad = 0.20;
  double max_z_jump_m = 0.50;
  double max_dt_for_motion_sec = 1.0;
};

struct LioMotionReference {
  bool valid = false;
  unsigned int generation_sec = 0;
  unsigned int generation_nsec = 0;
  double stamp = 0.0;
  double raw_x = 0.0;
  double raw_y = 0.0;
  double map_yaw_from_raw = 0.0;
};

class PoseContinuityGate {
public:
  explicit PoseContinuityGate(const PoseContinuityConfig &config = {})
      : config_(config) {}

  void Reset() {
    valid_ = false;
    last_lio_reference_ = {};
    last_rtk_coast_ = false;
  }

  bool Accept(double measurement_time_sec, double x, double y, double z,
              double yaw, int source,
              const LioMotionReference *lio_reference = nullptr,
              bool rtk_coast = false) {
    if (!std::isfinite(measurement_time_sec) || !std::isfinite(x) ||
        !std::isfinite(y) || !std::isfinite(z) || !std::isfinite(yaw))
      return false;
    if (!valid_) {
      Commit(measurement_time_sec, x, y, z, yaw, source, lio_reference,
             rtk_coast);
      return true;
    }

    const double dt = measurement_time_sec - time_sec_;
    if (!std::isfinite(dt) || dt <= 0.0)
      return false;
    const double bounded_dt = std::min(dt, config_.max_dt_for_motion_sec);
    const double allowed_xy = std::min(
        config_.max_xy_jump_m,
        config_.xy_margin_m + config_.max_speed_mps * bounded_dt);
    const double allowed_yaw =
        config_.yaw_margin_rad + config_.max_yaw_rate_radps * bounded_dt;
    const double xy_jump = std::hypot(x - x_, y - y_);
    const double yaw_jump = std::fabs(WrapAngle(yaw - yaw_));
    // At a source handover subtract motion measured in the same LIO generation
    // and mapped using the *previous* output's transform. The remaining
    // innovation is the coordinate correction, not the vehicle's travel.
    bool source_correction_ok = true;
    const bool rtk_lio_handover = source != source_ &&
        (source == 1 || source == 2 || source == 5) &&
        (source_ == 1 || source_ == 2 || source_ == 5);
    const bool coast_to_direct_rtk =
        source == 1 && source_ == 1 && last_rtk_coast_ && !rtk_coast;
    if (rtk_lio_handover || coast_to_direct_rtk) {
      const bool comparable = ReferenceMatchesStamp(lio_reference,
                                                    measurement_time_sec) &&
          last_lio_reference_.valid &&
          lio_reference->generation_sec == last_lio_reference_.generation_sec &&
          lio_reference->generation_nsec == last_lio_reference_.generation_nsec;
      if (!comparable) return false;
      const double raw_dx = lio_reference->raw_x - last_lio_reference_.raw_x;
      const double raw_dy = lio_reference->raw_y - last_lio_reference_.raw_y;
      const double c = std::cos(last_lio_reference_.map_yaw_from_raw);
      const double s = std::sin(last_lio_reference_.map_yaw_from_raw);
      const double motion_x = c * raw_dx - s * raw_dy;
      const double motion_y = s * raw_dx + c * raw_dy;
      const double correction =
          std::hypot(x - (x_ + motion_x), y - (y_ + motion_y));
      source_correction_ok =
          std::isfinite(correction) &&
          std::hypot(motion_x, motion_y) <= allowed_xy &&
          correction <= config_.max_source_switch_xy_jump_m;
    }
    if (xy_jump > allowed_xy ||
        !source_correction_ok ||
        yaw_jump > allowed_yaw ||
        std::fabs(z - z_) > config_.max_z_jump_m)
      return false; // Rejection never advances the trusted baseline.

    Commit(measurement_time_sec, x, y, z, yaw, source, lio_reference,
           rtk_coast);
    return true;
  }

  int last_source() const { return source_; }
  bool valid() const { return valid_; }

private:
  static bool ReferenceMatchesStamp(const LioMotionReference *reference,
                                    double stamp) {
    return reference && reference->valid &&
           std::isfinite(reference->stamp) &&
           std::isfinite(reference->raw_x) &&
           std::isfinite(reference->raw_y) &&
           std::isfinite(reference->map_yaw_from_raw) &&
           std::fabs(reference->stamp - stamp) <= 0.001;
  }
  void Commit(double time_sec, double x, double y, double z, double yaw,
              int source, const LioMotionReference *lio_reference,
              bool rtk_coast) {
    time_sec_ = time_sec;
    x_ = x;
    y_ = y;
    z_ = z;
    yaw_ = WrapAngle(yaw);
    source_ = source;
    last_rtk_coast_ = source == 1 && rtk_coast;
    last_lio_reference_ = ReferenceMatchesStamp(lio_reference, time_sec)
                              ? *lio_reference : LioMotionReference{};
    valid_ = true;
  }

  PoseContinuityConfig config_;
  bool valid_ = false;
  double time_sec_ = 0.0;
  double x_ = 0.0;
  double y_ = 0.0;
  double z_ = 0.0;
  double yaw_ = 0.0;
  int source_ = 0;
  LioMotionReference last_lio_reference_;
  bool last_rtk_coast_ = false;
};

} // namespace fusion_4dof
