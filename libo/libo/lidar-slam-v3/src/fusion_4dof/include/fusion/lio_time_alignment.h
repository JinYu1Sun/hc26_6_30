#pragma once

#include <cmath>

namespace fusion_4dof {

struct TimedLioRawPose {
  bool valid = false;
  unsigned int generation_sec = 0;
  unsigned int generation_nsec = 0;
  double stamp = 0.0;
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  double yaw = 0.0;
};

inline bool LioPoseAtMeasurementTime(const TimedLioRawPose &before,
                                     const TimedLioRawPose &after,
                                     double target_stamp,
                                     TimedLioRawPose *out,
                                     double max_pair_dt_sec = 0.25,
                                     double max_extrapolation_sec = 0.05) {
  if (!out || !before.valid || !after.valid ||
      before.generation_sec != after.generation_sec ||
      before.generation_nsec != after.generation_nsec ||
      !std::isfinite(target_stamp) || !std::isfinite(before.stamp) ||
      !std::isfinite(after.stamp) || !std::isfinite(before.x) ||
      !std::isfinite(before.y) || !std::isfinite(before.z) ||
      !std::isfinite(before.yaw) || !std::isfinite(after.x) ||
      !std::isfinite(after.y) || !std::isfinite(after.z) ||
      !std::isfinite(after.yaw))
    return false;
  const double dt = after.stamp - before.stamp;
  if (!(dt > 0.0 && dt <= max_pair_dt_sec) ||
      target_stamp < before.stamp - max_extrapolation_sec ||
      target_stamp > after.stamp + max_extrapolation_sec)
    return false;
  const double ratio = (target_stamp - before.stamp) / dt;
  const double yaw_delta = std::atan2(std::sin(after.yaw - before.yaw),
                                     std::cos(after.yaw - before.yaw));
  *out = after;
  out->stamp = target_stamp;
  out->x = before.x + ratio * (after.x - before.x);
  out->y = before.y + ratio * (after.y - before.y);
  out->z = before.z + ratio * (after.z - before.z);
  out->yaw = std::atan2(std::sin(before.yaw + ratio * yaw_delta),
                       std::cos(before.yaw + ratio * yaw_delta));
  return std::isfinite(out->x) && std::isfinite(out->y) &&
         std::isfinite(out->z) && std::isfinite(out->yaw);
}

}  // namespace fusion_4dof
