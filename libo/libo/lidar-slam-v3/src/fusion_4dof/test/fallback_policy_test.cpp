#include "fusion/fallback_policy.h"

#include <cassert>
#include <iostream>

int main() {
  using fusion_4dof::ControlledLioRecoveryTimedOut;
  using fusion_4dof::ShouldEnterRestrictedImuFallback;

  assert(ShouldEnterRestrictedImuFallback(true, false, false, false));
  assert(!ShouldEnterRestrictedImuFallback(false, false, false, false));
  assert(!ShouldEnterRestrictedImuFallback(true, true, false, false));
  assert(!ShouldEnterRestrictedImuFallback(true, false, true, false));
  assert(!ShouldEnterRestrictedImuFallback(true, false, false, true));
  assert(!ControlledLioRecoveryTimedOut(true, 60.0, 60.0));
  assert(ControlledLioRecoveryTimedOut(true, 60.001, 60.0));
  assert(!ControlledLioRecoveryTimedOut(false, 100.0, 60.0));
  std::cout << "fallback_policy_test: PASS" << std::endl;
  return 0;
}
