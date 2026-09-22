#include "fusion/lio_time_alignment.h"
#include "fusion/pose_continuity_gate.h"
#include <cassert>
#include <cmath>

int main() {
  using fusion_4dof::TimedLioRawPose;
  using fusion_4dof::LioPoseAtMeasurementTime;
  const TimedLioRawPose a{true, 1, 2, 10.0, 0.0, 0.0, 0.0, 3.0};
  const TimedLioRawPose b{true, 1, 2, 10.1, 0.1, 0.2, 0.3, -3.0};
  TimedLioRawPose out;
  assert(LioPoseAtMeasurementTime(a, b, 10.05, &out));
  assert(std::fabs(out.x - 0.05) < 1e-9);
  assert(std::fabs(out.y - 0.10) < 1e-9);
  assert(std::fabs(std::fabs(out.yaw) - M_PI) < 1e-9);
  assert(LioPoseAtMeasurementTime(a, b, 10.14, &out));
  assert(std::fabs(out.x - 0.14) < 1e-9);
  assert(!LioPoseAtMeasurementTime(a, b, 10.16, &out));
  TimedLioRawPose other = b;
  other.generation_nsec = 3;
  assert(!LioPoseAtMeasurementTime(a, other, 10.05, &out));
  other = b;
  other.stamp = 10.4;
  other.x = 0.4;
  assert(LioPoseAtMeasurementTime(a, other, 10.35, &out, 0.50, 0.05));
  assert(!LioPoseAtMeasurementTime(a, other, 10.35, &out, 0.25, 0.05));
  other = b;
  other.stamp = 10.3;
  assert(!LioPoseAtMeasurementTime(a, other, 10.05, &out));
  // The RTK anchor, its LIO reference, coast, and subsequent handover must
  // refer to the same physical instants rather than mixed GPS/LIO stamps.
  fusion_4dof::PoseContinuityGate continuity;
  fusion_4dof::LioMotionReference anchor_ref{
      true, 1, 2, 10.05, 0.05, 0.10, 0.0};
  fusion_4dof::LioMotionReference coast_ref{
      true, 1, 2, 10.20, 0.20, 0.40, 0.0};
  fusion_4dof::LioMotionReference fallback_ref{
      true, 1, 2, 10.30, 0.30, 0.60, 0.0};
  assert(continuity.Accept(10.05, 0.05, 0.10, 0.0, 0.0, 1,
                           &anchor_ref));
  assert(continuity.Accept(10.20, 0.20, 0.40, 0.0, 0.0, 1,
                           &coast_ref));
  assert(continuity.Accept(10.30, 0.30, 0.60, 0.0, 0.0, 2,
                           &fallback_ref));
  return 0;
}
