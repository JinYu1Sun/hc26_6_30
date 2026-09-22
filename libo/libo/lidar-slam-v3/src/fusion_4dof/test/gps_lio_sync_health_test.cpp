#include "fusion/gps_lio_sync_health.h"

#include <cassert>
#include <iostream>

int main() {
  using fusion_4dof::UpdateGpsLioSyncFailureCount;

  // One callback discarding an arbitrarily large backlog is one failure.
  assert(UpdateGpsLioSyncFailureCount(0, false, 70, false) == 1);
  assert(UpdateGpsLioSyncFailureCount(12, false, 1000, true) == 13);

  // A future-only mismatch is also one failure per LIO callback.
  assert(UpdateGpsLioSyncFailureCount(4, false, 0, true) == 5);

  // An empty queue contains no evidence of a timestamp mismatch.
  assert(UpdateGpsLioSyncFailureCount(4, false, 0, false) == 4);

  // A valid pair recovers the health counter even if stale packets preceded it.
  assert(UpdateGpsLioSyncFailureCount(4, true, 25, false) == 2);
  assert(UpdateGpsLioSyncFailureCount(1, true, 0, false) == 0);

  std::cout << "gps_lio_sync_health_test: PASS" << std::endl;
  return 0;
}
