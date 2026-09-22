#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>

namespace fusion_4dof {

// A synchronization failure is a property of one LIO callback, not of every
// stale GPS packet discarded while servicing that callback.  In particular,
// a 20 Hz GPS backlog consumed by the first 10 Hz LIO callback must contribute
// one failure at most.
inline int UpdateGpsLioSyncFailureCount(int current_count, bool matched,
                                        std::size_t stale_gps_count,
                                        bool waiting_for_future_gps,
                                        int matched_recovery_step = 2) {
  current_count = std::max(0, current_count);
  if (matched)
    return std::max(0, current_count - std::max(1, matched_recovery_step));
  if (stale_gps_count == 0 && !waiting_for_future_gps)
    return current_count;
  if (current_count == std::numeric_limits<int>::max())
    return current_count;
  return current_count + 1;
}

} // namespace fusion_4dof
