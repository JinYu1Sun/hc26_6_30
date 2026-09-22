#include "fusion/pose_continuity_gate.h"

#include <cassert>
#include <cmath>

int main() {
  using fusion_4dof::LioMotionReference;
  fusion_4dof::PoseContinuityGate gate;
  LioMotionReference a{true, 10, 20, 1.0, 0.0, 0.0, 0.0};
  LioMotionReference b{true, 10, 20, 1.1, 0.1, 0.0, 0.0};
  LioMotionReference c{true, 10, 20, 1.2, 0.2, 0.0, 0.0};
  LioMotionReference d{true, 10, 20, 1.3, 0.3, 0.0, 0.0};
  LioMotionReference e{true, 10, 20, 1.4, 0.2, 0.0, 0.0};
  assert(gate.Accept(1.0, 0.0, 0.0, 0.0, 0.0, 1, &a));
  assert(gate.Accept(1.1, 0.1, 0.0, 0.0, 0.02, 1, &b));
  assert(!gate.Accept(1.2, 2.0, 0.0, 0.0, 0.02, 2, &c));
  // A rejected jump must not poison the trusted baseline. A second sample at
  // the jumped location remains rejected instead of being silently accepted.
  assert(!gate.Accept(1.3, 2.0, 0.0, 0.0, 0.02, 2, &d));
  assert(gate.Accept(1.4, 0.2, 0.0, 0.0, 0.03, 2, &e));
  // Processing may pause, but only header measurement time describes motion.
  assert(!gate.Accept(1.5, 0.9, 0.0, 0.0, 0.03, 1));
  assert(!gate.Accept(1.6, 0.9, 0.0, 0.0, 0.03, 1));
  assert(!gate.Accept(1.3, 0.2, 0.0, 0.0, 0.03, 1));
  // A 0.39 m true LIO increment over 0.68 s must not be mistaken for a
  // >0.25 m source correction; only the residual (~0.02 m) is capped.
  gate.Reset();
  LioMotionReference slow_a{true, 10, 20, 20.0, 0.0, 0.0, 0.0};
  LioMotionReference slow_b{true, 10, 20, 20.68, 0.39, 0.0, 0.0};
  assert(gate.Accept(20.0, 0.0, 0.0, 0.0, 0.0, 1, &slow_a));
  assert(gate.Accept(20.68, 0.37, 0.0, 0.0, 0.0, 2, &slow_b));
  gate.Reset();
  assert(gate.Accept(20.0, 0.0, 0.0, 0.0, 0.0, 1, &slow_a));
  assert(!gate.Accept(20.68, 0.70, 0.0, 0.0, 0.0, 2, &slow_b));
  LioMotionReference wrong_generation{true, 11, 1, 20.68, 0.39, 0.0, 0.0};
  assert(!gate.Accept(20.68, 0.39, 0.0, 0.0, 0.0, 2, &wrong_generation));
  gate.Reset();
  LioMotionReference rotated_a{true, 10, 20, 30.0, 0.0, 0.0, M_PI/2};
  LioMotionReference rotated_b{true, 10, 20, 30.5, 0.4, 0.0, M_PI/2};
  assert(gate.Accept(30.0, 1.0, 2.0, 0.0, 0.0, 1, &rotated_a));
  assert(gate.Accept(30.5, 1.0, 2.4, 0.0, 0.0, 2, &rotated_b));
  gate.Reset();
  LioMotionReference changed_transform_b{true, 10, 20, 40.5,
                                         0.4, 0.0, 1.0};
  LioMotionReference original_transform_a{true, 10, 20, 40.0,
                                          0.0, 0.0, 0.0};
  assert(gate.Accept(40.0, 0.0, 0.0, 0.0, 0.0, 1,
                     &original_transform_a));
  assert(gate.Accept(40.5, 0.4, 0.0, 0.0, 0.0, 2,
                     &changed_transform_b));
  // RTK-anchored coast and direct RTK both publish state=1. The return to
  // measured RTK is still a physical-source handover and needs the same
  // LIO-predicted correction limit.
  gate.Reset();
  LioMotionReference coast_a{true, 10, 20, 50.0, 0.0, 0.0, 0.0};
  LioMotionReference coast_b{true, 10, 20, 50.2, 0.1, 0.0, 0.0};
  LioMotionReference direct_c{true, 10, 20, 50.3, 0.2, 0.0, 0.0};
  assert(gate.Accept(50.0, 0.0, 0.0, 0.0, 0.0, 1, &coast_a));
  assert(gate.Accept(50.2, 0.1, 0.0, 0.0, 0.0, 1, &coast_b, true));
  assert(!gate.Accept(50.3, 0.50, 0.0, 0.0, 0.0, 1, &direct_c));
  assert(gate.Accept(50.3, 0.22, 0.0, 0.0, 0.0, 1, &direct_c));
  gate.Reset();
  assert(gate.Accept(10.0, 5.0, 6.0, 1.0, 0.0, 1));
  return 0;
}
