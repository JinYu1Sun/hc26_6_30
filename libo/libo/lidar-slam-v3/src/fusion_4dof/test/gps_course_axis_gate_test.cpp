#include "fusion/gps_course_axis_gate.h"
#include <cassert>
#include <cmath>

int main() {
  using fusion_4dof::GpsCourseAxisGate;
  using fusion_4dof::GpsCourseAxisResult;
  GpsCourseAxisGate reverse;
  GpsCourseAxisResult result = GpsCourseAxisResult::kInsufficientEvidence;
  for (int i = 0; i <= 10; ++i)
    result = reverse.Observe(1.0 + i * 0.05, -i * 0.04, 0.0,
                             0.0, true, 0.0, true);
  assert(result == GpsCourseAxisResult::kAlignedReverse);

  GpsCourseAxisGate turn;
  for (int i = 0; i <= 10; ++i)
    result = turn.Observe(2.0 + i * 0.05, 0.0, i * 0.04,
                          i * 0.04, true, 0.4, true);
  assert(result == GpsCourseAxisResult::kTurning);

  GpsCourseAxisGate nearly_stationary;
  for (int i = 0; i <= 12; ++i)
    result = nearly_stationary.Observe(2.0 + i * 0.05,
                                      0.0, i * 0.004, 0.0,
                                      true, 0.0, true);
  assert(result == GpsCourseAxisResult::kInsufficientEvidence);

  GpsCourseAxisGate lateral;
  int suspects = 0;
  int rejects = 0;
  for (int i = 0; i <= 15; ++i) {
    result = lateral.Observe(3.0 + i * 0.05, 0.0, i * 0.04,
                             0.0, true, 0.0, true);
    suspects += result == GpsCourseAxisResult::kSuspect;
    rejects += result == GpsCourseAxisResult::kRejected;
  }
  assert(suspects >= 2 && rejects > 0);
  // One lateral position step repeated by subsequent stationary fixes must
  // not be promoted into a three-frame hard conflict by overlapping windows.
  GpsCourseAxisGate one_step;
  for (int i = 0; i <= 15; ++i) {
    const double y = i < 6 ? 0.0 : 0.30;
    result = one_step.Observe(4.0 + i * 0.05, 0.0, y,
                              0.0, true, 0.0, true);
    assert(result != GpsCourseAxisResult::kRejected);
  }
  lateral.Reset();
  result = lateral.Observe(5.0, 0.0, 0.0, 0.0, false, 0.0, true);
  assert(result == GpsCourseAxisResult::kInsufficientEvidence);
  for (int i = 1; i <= 12; ++i)
    result = lateral.Observe(5.0 + i * 0.05, i * 0.04,
                             0.0, 0.0, true, 0.0, true);
  assert(result == GpsCourseAxisResult::kAlignedForward);
  assert(lateral.Observe(6.0, 0.0, 0.0, 0.0, true,
                         0.0, false) ==
         GpsCourseAxisResult::kInsufficientEvidence);
  return 0;
}
