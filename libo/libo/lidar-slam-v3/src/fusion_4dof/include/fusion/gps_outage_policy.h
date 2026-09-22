#pragma once

#include <algorithm>
#include <cmath>

namespace fusion_4dof {

enum class GpsObservation {
  kMissing,
  kBaselineOnly,
  kHeadingSuspect,
  kAccepted,
  kRejected
};

inline GpsObservation ClassifyGpsObservation(bool has_message,
                                             bool receiver_quality_good,
                                             bool motion_baseline_only,
                                             bool sample_accepted,
                                             bool heading_suspect = false) {
  if (!has_message)
    return GpsObservation::kMissing;
  if (receiver_quality_good && heading_suspect)
    return GpsObservation::kHeadingSuspect;
  if (receiver_quality_good && motion_baseline_only)
    return GpsObservation::kBaselineOnly;
  return sample_accepted ? GpsObservation::kAccepted
                         : GpsObservation::kRejected;
}

struct GpsOutageState {
  int stable_count = 0;
  int failed_count = 0;
  int missing_count = 0;
  int transient_reacquire_count = 0;
  bool failed = false;
  bool transient_fallback = false;
};

struct GpsOutageResult {
  bool clear_candidate_window = false;
  bool entered_full_failure = false;
  bool transient_recovered = false;
};

// No single unusable RTK observation may switch the production source. A bad
// sample is quarantined (never published) while the caller coasts from the
// last trusted RTK pose with same-generation LIO increments. Missing packets,
// cadence/baseline gaps, and received rejections share one bounded consecutive
// grace. During full-failure recovery every non-accepted observation still
// breaks the consecutive accepted sequence.
inline GpsOutageResult AdvanceGpsOutageState(
    GpsObservation observation, bool system_initialized, int stable_threshold,
    int recovery_floor, int missing_grace_frames,
    int transient_reacquire_frames, GpsOutageState *state) {
  GpsOutageResult result;
  if (state == nullptr || stable_threshold < 1 || recovery_floor < 0 ||
      recovery_floor >= stable_threshold || missing_grace_frames < 1 ||
      transient_reacquire_frames < 1)
    return result;

  if (!system_initialized) {
    // Baseline establishment/timestamp discontinuity and transport holes do
    // not contradict receiver quality, so preserve cold-start evidence. Only
    // an explicit quality/motion rejection invalidates the accumulated run.
    if (observation == GpsObservation::kAccepted) {
      state->stable_count =
          std::min(state->stable_count + 1, stable_threshold);
    } else if (observation == GpsObservation::kRejected) {
      state->stable_count = 0;
    }
    state->failed_count = 0;
    state->missing_count = 0;
    state->transient_reacquire_count = 0;
    state->transient_fallback = false;
    return result;
  }

  if (observation == GpsObservation::kMissing ||
      observation == GpsObservation::kBaselineOnly ||
      observation == GpsObservation::kRejected) {
    state->failed_count = std::min(state->failed_count + 1, 1000000);
    state->missing_count = std::min(state->missing_count + 1, 1000000);
    state->transient_reacquire_count = 0;
    // A rejected measurement must not remain in a recovery candidate window,
    // even though one rejection is not sufficient to change output source.
    if (observation == GpsObservation::kRejected)
      result.clear_candidate_window = true;
    if (state->failed) {
      // Full recovery is explicitly consecutive: every unusable observation
      // restarts it, regardless of whether transport or validation caused it.
      state->stable_count = recovery_floor;
      state->transient_fallback = false;
      result.clear_candidate_window = true;
      return result;
    }
    // Never publish the unusable sample. Keep the already healthy RTK mode
    // through the complete grace window by using the caller's trusted RTK/LIO
    // coast. Only a consecutive run beyond the grace may change source.
    if (state->missing_count > missing_grace_frames) {
      state->stable_count = recovery_floor;
      state->failed = true;
      state->transient_fallback = false;
      result.clear_candidate_window = true;
      result.entered_full_failure = true;
    }
    return result;
  }

  if (observation == GpsObservation::kHeadingSuspect) {
    // Direction disagreement is not yet a hard RTK conflict. Do not spend
    // the transport-missing three-frame budget on correlated evidence.
    // The caller instead enforces an independent measurement-time coast cap.
    state->missing_count = 0;
    state->transient_reacquire_count = 0;
    if (state->failed) {
      state->stable_count = recovery_floor;
      state->transient_fallback = false;
      result.clear_candidate_window = true;
    }
    return result;
  }

  state->missing_count = 0;
  state->failed_count = 0;
  if (observation == GpsObservation::kAccepted) {
    if (state->failed) {
      state->stable_count =
          std::min(state->stable_count + 1, stable_threshold);
    } else if (state->transient_fallback) {
      state->transient_reacquire_count = std::min(
          state->transient_reacquire_count + 1, transient_reacquire_frames);
      if (state->transient_reacquire_count >= transient_reacquire_frames) {
        state->transient_fallback = false;
        state->transient_reacquire_count = 0;
        result.transient_recovered = true;
      }
    } else {
      state->stable_count =
          std::min(state->stable_count + 1, stable_threshold);
    }
    return result;
  }

  // All enum values are handled above. Keep a fail-closed fallback for future
  // extensions rather than silently accepting an unknown observation.
  state->stable_count = recovery_floor;
  state->failed = true;
  result.clear_candidate_window = true;
  result.entered_full_failure = true;
  return result;
}

inline bool ShouldCommitAcceptedGps(const GpsOutageState &state,
                                    int stable_threshold,
                                    int transient_reacquire_frames) {
  if (!state.failed && !state.transient_fallback)
    return true;
  if (state.transient_fallback)
    return state.transient_reacquire_count + 1 >=
           transient_reacquire_frames;
  return std::min(state.stable_count + 1, stable_threshold) >=
         stable_threshold;
}

inline bool IsGpsUnavailableWithinGrace(const GpsOutageState &state,
                                        int unavailable_grace_frames) {
  return !state.failed && !state.transient_fallback &&
         state.missing_count > 0 &&
         state.missing_count <= unavailable_grace_frames;
}

// This switch changes output policy only; it must never bypass outage
// counting or promote an unusable RTK measurement. HeadingSuspect has its
// own measurement-time limit and therefore does not use missing_count.
inline bool ShouldPublishGpsGraceCoast(bool coast_output_enabled,
                                       GpsObservation observation,
                                       const GpsOutageState &state,
                                       int unavailable_grace_frames) {
  if (!coast_output_enabled || state.failed || state.transient_fallback)
    return false;
  return observation == GpsObservation::kHeadingSuspect ||
         IsGpsUnavailableWithinGrace(state, unavailable_grace_frames);
}

inline bool GpsCoastTimeExpired(double lio_measurement_stamp,
                                double trusted_rtk_anchor_stamp,
                                double max_duration_sec) {
  return !std::isfinite(lio_measurement_stamp) ||
         !std::isfinite(trusted_rtk_anchor_stamp) ||
         !std::isfinite(max_duration_sec) || max_duration_sec <= 0.0 ||
         lio_measurement_stamp < trusted_rtk_anchor_stamp ||
         lio_measurement_stamp - trusted_rtk_anchor_stamp > max_duration_sec;
}

}  // namespace fusion_4dof
