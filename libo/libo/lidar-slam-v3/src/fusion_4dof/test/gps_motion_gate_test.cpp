#include "fusion/gps_motion_gate.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

int main() {
  using fusion_4dof::EvaluateGpsMotion;
  using fusion_4dof::GpsMotionResult;
  using fusion_4dof::GpsMotionResultName;
  using fusion_4dof::GpsMotionState;

  GpsMotionState state;
  double speed = 0.0, motion_yaw = 0.0;
  assert(EvaluateGpsMotion(1.0, 0.0, 0.0, 0.0, true, &state, &speed,
                           &motion_yaw) ==
         GpsMotionResult::kBaselineEstablished);
  assert(EvaluateGpsMotion(1.1, 0.1, 0.0, 0.0, true, &state, &speed,
                           &motion_yaw) == GpsMotionResult::kAccepted);

  // A 0.3 s gap is baseline-only, not proof of a bad RTK position.
  assert(EvaluateGpsMotion(1.4, 0.3, 0.0, 0.0, true, &state, &speed,
                           &motion_yaw) ==
         GpsMotionResult::kTimestampDiscontinuity);
  assert(state.stamp == 1.4 && state.x == 0.3);
  assert(EvaluateGpsMotion(1.5, 0.4, 0.0, 0.0, true, &state, &speed,
                           &motion_yaw) == GpsMotionResult::kAccepted);

  // A hard motion rejection does not advance the trusted RTK baseline.
  assert(EvaluateGpsMotion(1.6, 1.0, 0.0, 0.0, true, &state, &speed,
                           &motion_yaw) ==
         GpsMotionResult::kExcessiveSpeed);
  assert(std::fabs(state.stamp - 1.5) < 1e-12);
  assert(EvaluateGpsMotion(1.7, 0.6, 0.0, M_PI, true, &state, &speed,
                           &motion_yaw) == GpsMotionResult::kAccepted);
  assert(EvaluateGpsMotion(1.8, 0.6, 0.2, 0.0, true, &state, &speed,
                           &motion_yaw) ==
         GpsMotionResult::kHeadingMismatch);

  // Out-of-order timestamps must not advance the trusted motion baseline.
  GpsMotionState reverse_state;
  double stamp_delta = 0.0;
  assert(EvaluateGpsMotion(2.0, 0.0, 0.0, 0.0, false, &reverse_state, &speed,
                           &motion_yaw, 0.08, 0.12, 2.0, 0.5,
                           40.0 * M_PI / 180.0, &stamp_delta) ==
         GpsMotionResult::kBaselineEstablished);
  assert(std::isnan(stamp_delta));
  assert(EvaluateGpsMotion(2.0, 0.0, 0.0, 0.0, false, &reverse_state, &speed,
                           &motion_yaw, 0.08, 0.12, 2.0, 0.5,
                           40.0 * M_PI / 180.0, &stamp_delta) ==
         GpsMotionResult::kTimestampDiscontinuity);
  assert(stamp_delta == 0.0 && reverse_state.stamp == 2.0);
  assert(EvaluateGpsMotion(1.9, 0.1, 0.0, 0.0, false, &reverse_state, &speed,
                           &motion_yaw, 0.08, 0.12, 2.0, 0.5,
                           40.0 * M_PI / 180.0, &stamp_delta) ==
         GpsMotionResult::kInvalid);
  assert(std::fabs(stamp_delta + 0.1) < 1e-12);
  assert(reverse_state.stamp == 2.0);
  assert(EvaluateGpsMotion(2.067, 0.04, 0.0, 0.0, false, &reverse_state,
                           &speed, &motion_yaw, 0.01, 0.25) ==
         GpsMotionResult::kAccepted);
  // 15 Hz and a slightly early 20 Hz packet are ordinary, not cadence faults.
  assert(EvaluateGpsMotion(2.133667, 0.08, 0.0, 0.0, false, &reverse_state,
                           &speed, &motion_yaw, 0.01, 0.25) ==
         GpsMotionResult::kAccepted);
  assert(EvaluateGpsMotion(2.182, 0.10, 0.0, 0.0, false, &reverse_state,
                           &speed, &motion_yaw, 0.01, 0.25) ==
         GpsMotionResult::kAccepted);
  // Body heading opposite the travel course is legal reverse motion. With
  // no verified gear/command input, motion yaw must not be a hard conflict.
  GpsMotionState backing_state;
  assert(EvaluateGpsMotion(3.0, 0.0, 0.0, 0.0, false, &backing_state,
                           &speed, &motion_yaw, 0.01, 0.25) ==
         GpsMotionResult::kBaselineEstablished);
  assert(EvaluateGpsMotion(3.1, -0.07, 0.0, 0.0, false, &backing_state,
                           &speed, &motion_yaw, 0.01, 0.25) ==
         GpsMotionResult::kAccepted);
  // Current deployment treats sustained ~2 Hz selection as abnormal. The
  // 0.45 s upper bound still covers occasional 6-7 Hz and a ~0.4 s recovery
  // sample after three 10 Hz selection holes.
  GpsMotionState slow_lio_selection;
  assert(EvaluateGpsMotion(5.0, 0.0, 0.0, 0.0, false,
                           &slow_lio_selection, &speed, &motion_yaw,
                           0.01, 0.45, 2.0, 0.0, 0.0, nullptr, 0.08) ==
         GpsMotionResult::kBaselineEstablished);
  assert(EvaluateGpsMotion(5.40, 0.28, 0.0, 0.0, false,
                           &slow_lio_selection, &speed, &motion_yaw,
                           0.01, 0.45, 2.0, 0.0, 0.0, nullptr, 0.08) ==
         GpsMotionResult::kAccepted);
  assert(EvaluateGpsMotion(5.90, 0.63, 0.0, 0.0, false,
                           &slow_lio_selection, &speed, &motion_yaw,
                           0.01, 0.45, 2.0, 0.0, 0.0, nullptr, 0.08) ==
         GpsMotionResult::kTimestampDiscontinuity);
  assert(std::string(GpsMotionResultName(GpsMotionResult::kAccepted)) ==
         "accepted");

  std::cout << "gps_motion_gate_test: PASS" << std::endl;
  return 0;
}
