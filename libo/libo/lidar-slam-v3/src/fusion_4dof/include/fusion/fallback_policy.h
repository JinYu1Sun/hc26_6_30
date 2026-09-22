#pragma once

namespace fusion_4dof {

// Restricted inertial output is a last-resort braking aid.  A missing fused
// pose alone is insufficient: both independent absolute/relative sources must
// also be unavailable, the outage must have reached its timeout, and a
// controlled LIO restart/re-alignment must not be in progress.
inline bool ShouldEnterRestrictedImuFallback(bool main_pose_timed_out,
                                             bool gps_usable,
                                             bool lio_usable,
                                             bool controlled_lio_recovery) {
  return main_pose_timed_out && !gps_usable && !lio_usable &&
         !controlled_lio_recovery;
}

inline bool ControlledLioRecoveryTimedOut(bool controlled_lio_recovery,
                                          double elapsed_sec,
                                          double timeout_sec) {
  return controlled_lio_recovery && elapsed_sec >= 0.0 && timeout_sec > 0.0 &&
         elapsed_sec > timeout_sec;
}

}  // namespace fusion_4dof
