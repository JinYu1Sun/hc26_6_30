#pragma once

#include <cmath>
#include <cstdint>

namespace android_manager_4dof {

// ROS Bool topics from multiple publishers have no OR semantics. The final
// controller-facing stop must be computed explicitly and may be released only
// after every independent localization owner has released its request.
inline bool AggregateLocalizationStop(bool manager_stop, bool fusion_stop) {
  return manager_stop || fusion_stop;
}

// Strong type used by restart transitions.  Keep this separate from the
// legacy MODE_* values: workflow/mode and fusion process runtime are different
// state dimensions and must never be interchangeable integers.
enum class RecoveryRuntime {
  kUnknown,
  kCreateOrigin,
  kTrackSavedOrigin,
};

// Runtime selection is intentionally independent of the workflow value. A
// mapping workflow remains mapping across either result; only the fusion
// process startup behavior changes.
inline RecoveryRuntime SelectAutomaticRecoveryRuntime(
    bool mapping_workflow, bool map_origin_committed) {
  return mapping_workflow && !map_origin_committed
             ? RecoveryRuntime::kCreateOrigin
             : RecoveryRuntime::kTrackSavedOrigin;
}

// A low production-position rate does not by itself prove that FAST-LIO is
// unhealthy.  Preserve a fresh, quality-good, statically initialized LIO and
// recover fusion only; use the disruptive full restart when any one of those
// independent LIO health gates is missing.
inline bool LowFrequencyNeedsFullRestart(bool lio_stream_fresh,
                                         bool lio_quality_good,
                                         bool lio_static_ready) {
  return !(lio_stream_fresh && lio_quality_good && lio_static_ready);
}

// Receipt time alone cannot establish sensor health: a live publisher may be
// replaying an old queue forever. Validate source time before a callback is
// allowed to refresh watchdog state.
inline bool MeasurementStampFresh(double stamp_sec, double now_sec,
                                  double max_age_sec,
                                  double max_future_sec) {
  if (!std::isfinite(stamp_sec) || !std::isfinite(now_sec) ||
      !std::isfinite(max_age_sec) || !std::isfinite(max_future_sec) ||
      stamp_sec <= 0.0 || max_age_sec <= 0.0 || max_future_sec < 0.0)
    return false;
  const double age = now_sec - stamp_sec;
  return std::isfinite(age) && age >= -max_future_sec &&
         age <= max_age_sec;
}

// If replacing fusion did not restore output, repeating that same partial
// recovery cannot clear state owned by FAST-LIO. Escalate even when stale LIO
// messages are still arriving at a high callback rate.
inline bool PositionTimeoutNeedsFullRestart(
    bool lio_stream_fresh, bool verified_fusion_only_recovery_wait) {
  return !lio_stream_fresh || verified_fusion_only_recovery_wait;
}

// This is intentionally independent of restart history. A manager that starts
// while fusion is already stuck has no prior generation-verification context,
// but must still bound a WAIT_TRUSTED_OUTPUT state once all genuine
// sensor/operator-dependent prerequisites are ready.
inline bool FreshOutputWatchdogEligible(bool fast_lio_initialized,
                                        bool map_valid,
                                        bool alignment_ready,
                                        bool position_valid,
                                        bool waiting_for_trusted_output) {
  return fast_lio_initialized && map_valid && alignment_ready &&
         !position_valid && waiting_for_trusted_output;
}

// Some bounded outages are explicitly owned by fusion: LIO-only recovery waits
// for a new generation/static initialization/4DoF, while state 6 enforces its
// own time/distance/IMU-continuity limits. Neither may simultaneously feed the
// manager's generic position watchdog and create a competing restart.
inline bool PositionWatchdogAllowed(bool transitioning,
                                    bool position_recovery_wait,
                                    bool restart_in_progress,
                                    bool algorithm_owns_outage) {
  return (!transitioning || position_recovery_wait) && !restart_in_progress &&
         !algorithm_owns_outage;
}

struct GenerationToken {
  bool valid = false;
  uint32_t sec = 0;
  uint32_t nsec = 0;
};

inline bool GenerationChanged(const GenerationToken &before,
                              const GenerationToken &after) {
  return before.valid && after.valid &&
         (before.sec != after.sec || before.nsec != after.nsec);
}

inline bool GenerationAdvanced(const GenerationToken &before,
                               const GenerationToken &after) {
  return before.valid && after.valid &&
         (after.sec > before.sec ||
          (after.sec == before.sec && after.nsec > before.nsec));
}

inline bool GenerationAtOrAfter(const GenerationToken &value,
                                const GenerationToken &floor) {
  return value.valid && floor.valid &&
         (value.sec > floor.sec ||
          (value.sec == floor.sec && value.nsec >= floor.nsec));
}

// A restart normally proves identity by advancing a known baseline. A mode
// command can, however, arrive before the manager has received its first
// algorithm heartbeat. In that case accept only a process token created at or
// after the wall-clock restart request; accepting merely "any valid token"
// would let a delayed latched status from the old process pass.
inline bool RestartGenerationObserved(bool expected,
                                      const GenerationToken &before,
                                      const GenerationToken &after,
                                      const GenerationToken &request_floor) {
  if (!expected) return true;
  return before.valid ? GenerationAdvanced(before, after)
                      : GenerationAtOrAfter(after, request_floor);
}

inline bool ExpectedRestartGenerationsObserved(
    bool fusion_restart_expected, bool lio_restart_expected,
    const GenerationToken &fusion_before,
    const GenerationToken &fusion_after,
    const GenerationToken &lio_before,
    const GenerationToken &lio_after,
    const GenerationToken &request_floor) {
  if (!fusion_restart_expected && !lio_restart_expected) return false;
  return RestartGenerationObserved(fusion_restart_expected, fusion_before,
                                   fusion_after, request_floor) &&
         RestartGenerationObserved(lio_restart_expected, lio_before,
                                   lio_after, request_floor);
}

inline bool GenerationRegressed(const GenerationToken &previous,
                                const GenerationToken &incoming) {
  if (!previous.valid) return false;
  if (!incoming.valid) return true;
  return GenerationAdvanced(incoming, previous);
}

inline bool UsablePositionInvariant(bool candidate_valid, bool stop_required,
                                    bool transitioning, uint16_t fault_code,
                                    uint8_t restart_scope) {
  return candidate_valid && !stop_required && !transitioning &&
         fault_code == 0 && restart_scope == 0;
}

inline bool RequiredStopInvariant(bool stop_required, bool transitioning,
                                  uint16_t fault_code,
                                  uint8_t restart_scope) {
  return !(transitioning || fault_code != 0 || restart_scope != 0) ||
         stop_required;
}

inline bool ExpectedRestartGenerationsChanged(
    bool fusion_restart_expected, bool lio_restart_expected,
    const GenerationToken &fusion_before,
    const GenerationToken &fusion_after,
    const GenerationToken &lio_before,
    const GenerationToken &lio_after) {
  if (!fusion_restart_expected && !lio_restart_expected)
    return false;
  return (!fusion_restart_expected ||
          GenerationAdvanced(fusion_before, fusion_after)) &&
         (!lio_restart_expected || GenerationAdvanced(lio_before, lio_after));
}

// Restart has two independent phases. Only failure to prove that the expected
// processes changed identity is deadline-bound. Once identity is proven,
// sensor/operator-dependent static/alignment readiness may wait indefinitely
// with stop latched. Once those gates are ready, the manager separately bounds
// a no-fresh-output wait through its position watchdog.
inline bool RestartGenerationVerificationTimedOut(
    bool verification_required, bool new_generation_seen,
    double deadline_sec, double now_sec) {
  return verification_required && !new_generation_seen &&
         deadline_sec > 0.0 && now_sec > deadline_sec;
}

inline bool PostRestartReadinessHeartbeatStale(
    bool transitioning, bool restart_in_progress, bool map_worker_running,
    bool verification_required, bool new_generation_seen,
    bool algorithm_status_stale) {
  return transitioning && !restart_in_progress && !map_worker_running &&
         verification_required && new_generation_seen &&
         algorithm_status_stale;
}

// A full recovery queued during LIO-only recovery is based on an earlier
// health snapshot.  It is safe to discard only with positive proof that the
// LIO process changed generation and the complete post-recovery localization
// chain is fresh and healthy.  Missing evidence always fails closed.
struct RecoveryHealthEvidence {
  bool algorithm_status_fresh = false;
  bool algorithm_fault_free = false;
  bool algorithm_restart_idle = false;
  bool fast_lio_initialized = false;
  bool algorithm_lio_good = false;
  bool alignment_ready = false;
  bool map_valid = false;
  bool position_valid = false;
  bool manager_lio_stream_fresh = false;
  bool manager_lio_quality_good = false;
  bool manager_lio_static_ready = false;
};

inline bool QueuedFullRecoveryIsStale(
    const GenerationToken &before_lio_restart,
    const GenerationToken &current_lio_generation,
    const RecoveryHealthEvidence &evidence) {
  return GenerationChanged(before_lio_restart, current_lio_generation) &&
         evidence.algorithm_status_fresh && evidence.algorithm_fault_free &&
         evidence.algorithm_restart_idle && evidence.fast_lio_initialized &&
         evidence.algorithm_lio_good && evidence.alignment_ready &&
         evidence.map_valid && evidence.position_valid &&
         evidence.manager_lio_stream_fresh &&
         evidence.manager_lio_quality_good &&
         evidence.manager_lio_static_ready;
}

}  // namespace android_manager_4dof
