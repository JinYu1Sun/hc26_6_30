#include "fusion/lio_motion_gate.h"

#include <cassert>
#include <cmath>

int main() {
  using fusion_4dof::EvaluateLioMotion;
  using fusion_4dof::LioMotionResult;
  double speed = -1.0;
  assert(EvaluateLioMotion(0.10, 0.08, 0.50, 2.0, 0.08, &speed) ==
         LioMotionResult::kAccepted);
  assert(EvaluateLioMotion(0.20, 0.16, 0.50, 2.0, 0.08, &speed) ==
         LioMotionResult::kAccepted); // 5 Hz
  assert(EvaluateLioMotion(0.40, 0.32, 0.50, 2.0, 0.08, &speed) ==
         LioMotionResult::kAccepted); // 2.5 Hz
  assert(EvaluateLioMotion(0.04, 0.30, 0.50, 2.0, 0.08, &speed) ==
         LioMotionResult::kExcessiveMotion);
  assert(EvaluateLioMotion(0.20, 0.60, 0.50, 2.0, 0.08, &speed) ==
         LioMotionResult::kExcessiveMotion);
  assert(EvaluateLioMotion(0.55, 0.01, 0.50, 2.0, 0.08, &speed) ==
         LioMotionResult::kExcessiveGap);
  assert(EvaluateLioMotion(0.0, 0.0, 0.50, 2.0, 0.08, &speed) ==
         LioMotionResult::kInvalidInterval);

  // Regression for map switching: fusion readiness may be false while LIO is
  // continuous. Even if the first post-switch pair exceeds max_dt, that finite
  // sample must advance the health baseline so the next adjacent pair recovers.
  double last_stamp = 10.0;
  Eigen::Vector3d last_position(0.0, 0.0, 0.0);
  Eigen::Vector3d delta;
  assert(fusion_4dof::EvaluateLioMotionAndAdvanceBaseline(
             10.61, Eigen::Vector3d(0.01, 0.0, 0.0), 0.60, 2.0, 0.08,
             &last_stamp, &last_position, &speed, &delta) ==
         LioMotionResult::kExcessiveGap);
  assert(std::fabs(last_stamp - 10.61) < 1e-12);
  assert(fusion_4dof::EvaluateLioMotionAndAdvanceBaseline(
             10.71, Eigen::Vector3d(0.02, 0.0, 0.0), 0.60, 2.0, 0.08,
             &last_stamp, &last_position, &speed, &delta) ==
         LioMotionResult::kAccepted);
  assert(std::fabs(last_stamp - 10.71) < 1e-12);
  assert(std::fabs(speed - 0.1) < 1e-12);
  return 0;
}
