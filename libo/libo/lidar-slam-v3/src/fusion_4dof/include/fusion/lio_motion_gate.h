#pragma once

#include <Eigen/Core>
#include <cmath>

namespace fusion_4dof {

enum class LioMotionResult {
  kAccepted,
  kInvalidInterval,
  kExcessiveGap,
  kExcessiveMotion
};

inline LioMotionResult EvaluateLioMotion(
    double dt, double distance_m, double max_pair_dt_sec,
    double max_speed_mps, double margin_m, double *speed_mps) {
  if (speed_mps) *speed_mps = 0.0;
  if (!speed_mps || !std::isfinite(dt) || dt <= 0.0 ||
      !std::isfinite(distance_m) || distance_m < 0.0 ||
      !std::isfinite(max_pair_dt_sec) || max_pair_dt_sec <= 0.0 ||
      !std::isfinite(max_speed_mps) || max_speed_mps <= 0.0 ||
      !std::isfinite(margin_m) || margin_m < 0.0)
    return LioMotionResult::kInvalidInterval;
  if (dt > max_pair_dt_sec)
    return LioMotionResult::kExcessiveGap;
  *speed_mps = distance_m / dt;
  if (!std::isfinite(*speed_mps) ||
      distance_m > max_speed_mps * dt + margin_m)
    return LioMotionResult::kExcessiveMotion;
  return LioMotionResult::kAccepted;
}

// Evaluate a stream pair and advance the baseline independently of fusion/map
// readiness.  A rejected but finite sample becomes the next baseline so a
// single long callback gap can recover on the following adjacent sample
// instead of growing into a permanent stale-dt loop.
inline LioMotionResult EvaluateLioMotionAndAdvanceBaseline(
    double stamp, const Eigen::Vector3d &position,
    double max_pair_dt_sec, double max_speed_mps, double margin_m,
    double *last_stamp, Eigen::Vector3d *last_position,
    double *speed_mps, Eigen::Vector3d *delta_position = nullptr) {
  if (speed_mps) *speed_mps = 0.0;
  if (delta_position) delta_position->setZero();
  const bool position_finite = std::isfinite(position.x()) &&
                               std::isfinite(position.y()) &&
                               std::isfinite(position.z());
  if (!last_stamp || !last_position || !speed_mps ||
      !std::isfinite(stamp) || stamp <= 0.0 || !position_finite)
    return LioMotionResult::kInvalidInterval;

  const bool baseline_finite = std::isfinite(last_position->x()) &&
                               std::isfinite(last_position->y()) &&
                               std::isfinite(last_position->z());
  if (*last_stamp <= 0.0 || !std::isfinite(*last_stamp) ||
      !baseline_finite) {
    *last_stamp = stamp;
    *last_position = position;
    return LioMotionResult::kAccepted;
  }

  const double dt = stamp - *last_stamp;
  const Eigen::Vector3d delta = position - *last_position;
  // Advance before returning any pair-quality result. This state follows the
  // live LIO stream and must not be frozen by system_init_ or a map switch.
  *last_stamp = stamp;
  *last_position = position;
  if (delta_position) *delta_position = delta;
  return EvaluateLioMotion(dt, delta.norm(), max_pair_dt_sec, max_speed_mps,
                           margin_m, speed_mps);
}

}  // namespace fusion_4dof
