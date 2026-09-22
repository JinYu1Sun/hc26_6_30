#include "android_manager_4dof/lifecycle_mode.h"

#include <cassert>
#include <iostream>
#include <limits>

int main() {
  using android_manager_4dof::RecoveryRuntime;
  using android_manager_4dof::AggregateLocalizationStop;
  using android_manager_4dof::LowFrequencyNeedsFullRestart;
  using android_manager_4dof::MeasurementStampFresh;
  using android_manager_4dof::PositionTimeoutNeedsFullRestart;
  using android_manager_4dof::FreshOutputWatchdogEligible;
  using android_manager_4dof::PositionWatchdogAllowed;
  using android_manager_4dof::GenerationToken;
  using android_manager_4dof::ExpectedRestartGenerationsChanged;
  using android_manager_4dof::ExpectedRestartGenerationsObserved;
  using android_manager_4dof::GenerationRegressed;
  using android_manager_4dof::UsablePositionInvariant;
  using android_manager_4dof::RequiredStopInvariant;
  using android_manager_4dof::QueuedFullRecoveryIsStale;
  using android_manager_4dof::RecoveryHealthEvidence;
  using android_manager_4dof::RestartGenerationVerificationTimedOut;
  using android_manager_4dof::PostRestartReadinessHeartbeatStale;
  using android_manager_4dof::SelectAutomaticRecoveryRuntime;

  assert(RecoveryRuntime::kUnknown != RecoveryRuntime::kCreateOrigin);
  assert(!AggregateLocalizationStop(false, false));
  assert(AggregateLocalizationStop(true, false));
  assert(AggregateLocalizationStop(false, true));
  assert(AggregateLocalizationStop(true, true));
  assert(RecoveryRuntime::kUnknown != RecoveryRuntime::kTrackSavedOrigin);
  assert(SelectAutomaticRecoveryRuntime(true, false) ==
         RecoveryRuntime::kCreateOrigin);
  assert(SelectAutomaticRecoveryRuntime(true, true) ==
         RecoveryRuntime::kTrackSavedOrigin);
  assert(SelectAutomaticRecoveryRuntime(false, false) ==
         RecoveryRuntime::kTrackSavedOrigin);
  assert(SelectAutomaticRecoveryRuntime(false, true) ==
         RecoveryRuntime::kTrackSavedOrigin);
  assert(!LowFrequencyNeedsFullRestart(true, true, true));
  assert(LowFrequencyNeedsFullRestart(false, true, true));
  assert(LowFrequencyNeedsFullRestart(true, false, true));
  assert(LowFrequencyNeedsFullRestart(true, true, false));
  assert(MeasurementStampFresh(99.6, 100.0, 0.5, 0.05));
  assert(MeasurementStampFresh(100.05, 100.0, 0.5, 0.05));
  assert(!MeasurementStampFresh(99.49, 100.0, 0.5, 0.05));
  assert(!MeasurementStampFresh(100.051, 100.0, 0.5, 0.05));
  assert(!MeasurementStampFresh(0.0, 100.0, 0.5, 0.05));
  assert(!MeasurementStampFresh(std::numeric_limits<double>::quiet_NaN(),
                                100.0, 0.5, 0.05));
  assert(!MeasurementStampFresh(100.0,
                                std::numeric_limits<double>::infinity(),
                                0.5, 0.05));
  assert(!PositionTimeoutNeedsFullRestart(true, false));
  assert(PositionTimeoutNeedsFullRestart(false, false));
  assert(PositionTimeoutNeedsFullRestart(true, true));
  assert(FreshOutputWatchdogEligible(true, true, true, false, true));
  assert(!FreshOutputWatchdogEligible(false, true, true, false, true));
  assert(!FreshOutputWatchdogEligible(true, false, true, false, true));
  assert(!FreshOutputWatchdogEligible(true, true, false, false, true));
  assert(!FreshOutputWatchdogEligible(true, true, true, true, true));
  assert(!FreshOutputWatchdogEligible(true, true, true, false, false));
  assert(PositionWatchdogAllowed(false, false, false, false));
  assert(PositionWatchdogAllowed(true, true, false, false));
  assert(!PositionWatchdogAllowed(true, true, false, true));
  assert(!PositionWatchdogAllowed(false, false, true, false));
  const GenerationToken old_generation{true, 10, 20};
  const GenerationToken new_generation{true, 11, 30};
  const GenerationToken stale_generation{true, 9, 999};
  const GenerationToken request_floor{true, 10, 500};
  const GenerationToken post_request_generation{true, 10, 501};
  const GenerationToken invalid_generation{};
  assert(ExpectedRestartGenerationsChanged(
      true, false, old_generation, new_generation, old_generation,
      old_generation));
  assert(ExpectedRestartGenerationsChanged(
      false, true, old_generation, old_generation, old_generation,
      new_generation));
  assert(ExpectedRestartGenerationsChanged(
      true, true, old_generation, new_generation, old_generation,
      new_generation));
  assert(!ExpectedRestartGenerationsChanged(
      true, true, old_generation, new_generation, old_generation,
      old_generation));
  assert(!ExpectedRestartGenerationsChanged(
      false, false, old_generation, new_generation, old_generation,
      new_generation));
  assert(!ExpectedRestartGenerationsChanged(
      true, false, old_generation, stale_generation, old_generation,
      new_generation));
  assert(ExpectedRestartGenerationsObserved(
      true, false, invalid_generation, post_request_generation,
      invalid_generation, invalid_generation, request_floor));
  assert(!ExpectedRestartGenerationsObserved(
      true, false, invalid_generation, old_generation,
      invalid_generation, invalid_generation, request_floor));
  assert(GenerationRegressed(new_generation, old_generation));
  assert(GenerationRegressed(new_generation, invalid_generation));
  assert(!GenerationRegressed(old_generation, new_generation));
  assert(UsablePositionInvariant(true, false, false, 0, 0));
  assert(!UsablePositionInvariant(true, true, false, 0, 0));
  assert(!UsablePositionInvariant(true, false, true, 0, 0));
  assert(!UsablePositionInvariant(true, false, false, 1, 0));
  assert(!UsablePositionInvariant(true, false, false, 0, 1));
  assert(RequiredStopInvariant(false, false, 0, 0));
  assert(RequiredStopInvariant(true, true, 0, 0));
  assert(!RequiredStopInvariant(false, true, 0, 0));
  assert(!RequiredStopInvariant(false, false, 1, 0));
  assert(!RequiredStopInvariant(false, false, 0, 1));
  assert(RestartGenerationVerificationTimedOut(true, false, 30.0, 30.1));
  assert(!RestartGenerationVerificationTimedOut(true, true, 30.0, 300.0));
  assert(!RestartGenerationVerificationTimedOut(false, false, 30.0, 300.0));
  assert(PostRestartReadinessHeartbeatStale(
      true, false, false, true, true, true));
  assert(!PostRestartReadinessHeartbeatStale(
      true, false, false, true, true, false));
  assert(!PostRestartReadinessHeartbeatStale(
      true, true, false, true, true, true));
  RecoveryHealthEvidence healthy;
  healthy.algorithm_status_fresh = true;
  healthy.algorithm_fault_free = true;
  healthy.algorithm_restart_idle = true;
  healthy.fast_lio_initialized = true;
  healthy.algorithm_lio_good = true;
  healthy.alignment_ready = true;
  healthy.map_valid = true;
  healthy.position_valid = true;
  healthy.manager_lio_stream_fresh = true;
  healthy.manager_lio_quality_good = true;
  healthy.manager_lio_static_ready = true;
  assert(QueuedFullRecoveryIsStale(old_generation, new_generation, healthy));
  assert(!QueuedFullRecoveryIsStale(old_generation, old_generation, healthy));
  bool RecoveryHealthEvidence::*gates[] = {
      &RecoveryHealthEvidence::algorithm_status_fresh,
      &RecoveryHealthEvidence::algorithm_fault_free,
      &RecoveryHealthEvidence::algorithm_restart_idle,
      &RecoveryHealthEvidence::fast_lio_initialized,
      &RecoveryHealthEvidence::algorithm_lio_good,
      &RecoveryHealthEvidence::alignment_ready,
      &RecoveryHealthEvidence::map_valid,
      &RecoveryHealthEvidence::position_valid,
      &RecoveryHealthEvidence::manager_lio_stream_fresh,
      &RecoveryHealthEvidence::manager_lio_quality_good,
      &RecoveryHealthEvidence::manager_lio_static_ready};
  for (bool RecoveryHealthEvidence::*gate : gates) {
    RecoveryHealthEvidence missing_one_gate = healthy;
    missing_one_gate.*gate = false;
    assert(!QueuedFullRecoveryIsStale(
        old_generation, new_generation, missing_one_gate));
  }
  std::cout << "lifecycle_mode_test: PASS" << std::endl;
  return 0;
}
