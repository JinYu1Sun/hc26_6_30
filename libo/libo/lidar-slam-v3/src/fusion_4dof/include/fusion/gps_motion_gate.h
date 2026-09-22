#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

namespace fusion_4dof {

enum class GpsMotionResult {
  kAccepted,
  kBaselineEstablished,
  kTimestampDiscontinuity,
  kInvalid,
  kExcessiveSpeed,
  kHeadingSuspect,
  kHeadingMismatch
};

inline const char *GpsMotionResultName(GpsMotionResult result) {
  switch (result) {
    case GpsMotionResult::kAccepted:
      return "accepted";
    case GpsMotionResult::kBaselineEstablished:
      return "baseline_established";
    case GpsMotionResult::kTimestampDiscontinuity:
      return "timestamp_discontinuity";
    case GpsMotionResult::kInvalid:
      return "invalid";
    case GpsMotionResult::kExcessiveSpeed:
      return "excessive_speed";
    case GpsMotionResult::kHeadingSuspect:
      return "heading_suspect";
    case GpsMotionResult::kHeadingMismatch:
      return "heading_mismatch";
  }
  return "unknown";
}

inline bool IsGpsMotionBaselineOnly(GpsMotionResult result) {
  // A single direction-suspect window is likewise not a publishable RTK
  // sample, but is not yet sufficient evidence for a hard source conflict.
  return result == GpsMotionResult::kBaselineEstablished ||
         result == GpsMotionResult::kTimestampDiscontinuity ||
         result == GpsMotionResult::kHeadingSuspect;
}

struct GpsMotionState {
  bool valid = false;
  double stamp = 0.0;
  double x = 0.0;
  double y = 0.0;
};

// Checks motion internal to the RTK stream. Cross-source position consistency
// remains the responsibility of the synchronized 4DoF innovation gate.
inline GpsMotionResult EvaluateGpsMotion(
    double stamp, double x, double y, double yaw_rad, bool check_heading,
    GpsMotionState *state, double *speed_mps, double *motion_yaw_rad,
    double min_dt_sec = 0.05, double max_dt_sec = 0.25,
    double max_speed_mps = 2.0, double heading_check_min_speed_mps = 0.5,
    double max_heading_difference_rad = 40.0 * M_PI / 180.0,
    double *stamp_delta_sec = nullptr, double motion_margin_m = 0.08) {
  if (stamp_delta_sec != nullptr)
    *stamp_delta_sec = std::numeric_limits<double>::quiet_NaN();
  if (state == nullptr || speed_mps == nullptr || motion_yaw_rad == nullptr ||
      !std::isfinite(stamp) || stamp <= 0.0 || !std::isfinite(x) ||
      !std::isfinite(y) || !std::isfinite(yaw_rad) ||
      !std::isfinite(max_speed_mps) || max_speed_mps <= 0.0 ||
      !std::isfinite(motion_margin_m) || motion_margin_m < 0.0)
    return GpsMotionResult::kInvalid;

  *speed_mps = 0.0;
  *motion_yaw_rad = 0.0;
  if (!state->valid) {
    state->valid = true;
    state->stamp = stamp;
    state->x = x;
    state->y = y;
    return GpsMotionResult::kBaselineEstablished;
  }

  const double raw_dt = stamp - state->stamp;
  if (stamp_delta_sec != nullptr)
    *stamp_delta_sec = raw_dt;
  // Duplicate time is not a new motion observation, but it also does not
  // contradict the last trusted RTK pose. Only time reversal is invalid.
  if (raw_dt == 0.0)
    return GpsMotionResult::kTimestampDiscontinuity;
  if (raw_dt < 0.0)
    return GpsMotionResult::kInvalid;
  const double dt = raw_dt;
  if (!(dt >= min_dt_sec && dt <= max_dt_sec)) {
    state->stamp = stamp;
    state->x = x;
    state->y = y;
    return GpsMotionResult::kTimestampDiscontinuity;
  }

  const double dx = x - state->x;
  const double dy = y - state->y;
  *speed_mps = std::hypot(dx, dy) / dt;
  // At 10-50 ms cadence, centimetre-level GNSS noise is a large apparent
  // velocity. Test a displacement envelope instead of a bare quotient.
  if (!std::isfinite(*speed_mps) ||
      std::hypot(dx, dy) > max_speed_mps * dt + motion_margin_m)
    return GpsMotionResult::kExcessiveSpeed;

  if (std::hypot(dx, dy) > 1e-6)
    *motion_yaw_rad = std::atan2(dy, dx);
  if (check_heading && *speed_mps > heading_check_min_speed_mps) {
    const double yaw_difference = std::fabs(std::atan2(
        std::sin(*motion_yaw_rad - yaw_rad),
        std::cos(*motion_yaw_rad - yaw_rad)));
    // Even legacy direct users of this helper must treat 180-degree travel
    // as legal reverse, not as heading drift.
    if (std::min(yaw_difference, M_PI - yaw_difference) >
        max_heading_difference_rad)
      return GpsMotionResult::kHeadingMismatch;
  }

  state->stamp = stamp;
  state->x = x;
  state->y = y;
  return GpsMotionResult::kAccepted;
}

}  // namespace fusion_4dof
