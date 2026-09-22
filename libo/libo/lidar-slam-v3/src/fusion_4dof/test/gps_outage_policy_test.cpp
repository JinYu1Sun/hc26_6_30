#include "fusion/gps_outage_policy.h"

#include <cassert>
#include <iostream>

int main() {
  using fusion_4dof::AdvanceGpsOutageState;
  using fusion_4dof::ClassifyGpsObservation;
  using fusion_4dof::GpsObservation;
  using fusion_4dof::GpsOutageState;
  using fusion_4dof::IsGpsUnavailableWithinGrace;
  using fusion_4dof::ShouldCommitAcceptedGps;
  using fusion_4dof::ShouldPublishGpsGraceCoast;
  using fusion_4dof::GpsCoastTimeExpired;

  assert(!GpsCoastTimeExpired(10.6, 10.0, 0.75)); // 5 Hz, third hole
  assert(GpsCoastTimeExpired(11.0, 10.0, 0.75));  // slow callbacks: time wins
  assert(!GpsCoastTimeExpired(11.4, 10.0, 1.50));
  assert(GpsCoastTimeExpired(11.6, 10.0, 1.50));
  assert(GpsCoastTimeExpired(9.9, 10.0, 1.50));

  GpsOutageState state;
  assert(ClassifyGpsObservation(false, false, false, false) ==
         GpsObservation::kMissing);
  assert(ClassifyGpsObservation(true, true, true, false) ==
         GpsObservation::kBaselineOnly);
  assert(ClassifyGpsObservation(true, true, false, true) ==
         GpsObservation::kAccepted);
  assert(ClassifyGpsObservation(true, false, false, false) ==
         GpsObservation::kRejected);
  assert(ClassifyGpsObservation(true, true, true, false, true) ==
         GpsObservation::kHeadingSuspect);

  // Cold start preserves evidence across non-contradictory cadence/transport
  // gaps. Only an explicit quality or motion rejection clears the count.
  state.stable_count = 7;
  AdvanceGpsOutageState(GpsObservation::kBaselineOnly, false, 50, 28, 3, 3,
                        &state);
  assert(state.stable_count == 7);
  AdvanceGpsOutageState(GpsObservation::kMissing, false, 50, 28, 3, 3,
                        &state);
  assert(state.stable_count == 7);
  AdvanceGpsOutageState(GpsObservation::kAccepted, false, 50, 28, 3, 3,
                        &state);
  assert(state.stable_count == 8);
  AdvanceGpsOutageState(GpsObservation::kRejected, false, 50, 28, 3, 3,
                        &state);
  assert(state.stable_count == 0);

  state.stable_count = 50;
  // A three-frame grace means three consecutive missing observations remain
  // in RTK mode. An accepted observation breaks the run and needs no
  // re-acquisition because the production source never changed.
  AdvanceGpsOutageState(GpsObservation::kMissing, true, 50, 28, 3, 3,
                        &state);
  assert(!state.failed && !state.transient_fallback);
  assert(state.missing_count == 1);
  assert(IsGpsUnavailableWithinGrace(state, 3));
  assert(ShouldPublishGpsGraceCoast(true, GpsObservation::kMissing, state, 3));
  assert(!ShouldPublishGpsGraceCoast(false, GpsObservation::kMissing, state, 3));
  assert(ShouldCommitAcceptedGps(state, 50, 3));
  AdvanceGpsOutageState(GpsObservation::kAccepted, true, 50, 28, 3, 3,
                        &state);
  assert(!state.transient_fallback && !state.failed);
  assert(state.missing_count == 0);

  // The first three consecutive holes are coasted. The fourth confirms a
  // transport outage and atomically enters the existing full-failure path.
  for (int i = 0; i < 3; ++i) {
    const auto grace = AdvanceGpsOutageState(
        GpsObservation::kMissing, true, 50, 28, 3, 3, &state);
    assert(!grace.entered_full_failure);
    assert(IsGpsUnavailableWithinGrace(state, 3));
  }
  const auto expired = AdvanceGpsOutageState(
      GpsObservation::kMissing, true, 50, 28, 3, 3, &state);
  assert(expired.entered_full_failure && expired.clear_candidate_window);
  assert(state.failed && !state.transient_fallback);
  assert(!IsGpsUnavailableWithinGrace(state, 3));

  // Complete hard recovery before exercising the baseline-only transient
  // path below.
  for (int i = 0; i < 22; ++i)
    AdvanceGpsOutageState(GpsObservation::kAccepted, true, 50, 28, 3, 3,
                          &state);
  // The ROS caller clears failed atomically with the threshold-reaching
  // estimator commit; this header deliberately owns only evidence counting.
  state.failed = false;
  assert(!state.failed && !state.transient_fallback);

  // A baseline-only frame after missing transport shares the same bounded
  // non-accepted-evidence grace. It never changes the source by itself.
  AdvanceGpsOutageState(GpsObservation::kMissing, true, 50, 28, 3, 3,
                        &state);
  AdvanceGpsOutageState(GpsObservation::kMissing, true, 50, 28, 3, 3,
                        &state);
  assert(IsGpsUnavailableWithinGrace(state, 3));
  AdvanceGpsOutageState(GpsObservation::kBaselineOnly, true, 50, 28, 3, 3,
                        &state);
  assert(!state.failed && !state.transient_fallback);
  assert(state.missing_count == 3);
  AdvanceGpsOutageState(GpsObservation::kAccepted, true, 50, 28, 3, 3,
                        &state);
  assert(!state.transient_fallback && !state.failed &&
         state.missing_count == 0);

  // Four consecutive non-accepted frames, whether missing or baseline-only,
  // enter full outage.
  for (int i = 0; i < 3; ++i)
    AdvanceGpsOutageState(GpsObservation::kBaselineOnly, true, 50, 28, 3, 3,
                          &state);
  const auto baseline_expired = AdvanceGpsOutageState(
      GpsObservation::kMissing, true, 50, 28, 3, 3, &state);
  assert(baseline_expired.entered_full_failure && state.failed);
  state.failed = false;
  state.stable_count = 50;
  state.missing_count = 0;

  // A long run of provisional direction evidence must not silently use up
  // the three-frame transport grace. Its separate real-time limit is checked
  // by the production caller against the last trusted RTK anchor.
  for (int i = 0; i < 8; ++i) {
    const auto suspect = AdvanceGpsOutageState(
        GpsObservation::kHeadingSuspect, true, 50, 28, 3, 3, &state);
    assert(!suspect.entered_full_failure && !state.failed);
    assert(state.missing_count == 0 && state.stable_count == 50);
    assert(ShouldPublishGpsGraceCoast(
        true, GpsObservation::kHeadingSuspect, state, 3));
    assert(!ShouldPublishGpsGraceCoast(
        false, GpsObservation::kHeadingSuspect, state, 3));
  }
  AdvanceGpsOutageState(GpsObservation::kMissing, true, 50, 28, 3, 3,
                        &state);
  assert(state.missing_count == 1 && !state.failed);
  AdvanceGpsOutageState(GpsObservation::kAccepted, true, 50, 28, 3, 3,
                        &state);

  // A received-but-rejected RTK sample is quarantined, but one bad sample may
  // not switch source. It shares the same three-consecutive-frame grace. An
  // accepted sample breaks the run without requiring the 22-frame recovery.
  auto rejected = AdvanceGpsOutageState(
      GpsObservation::kRejected, true, 50, 28, 3, 3, &state);
  assert(!rejected.entered_full_failure && rejected.clear_candidate_window);
  assert(!state.failed && state.missing_count == 1);
  AdvanceGpsOutageState(GpsObservation::kAccepted, true, 50, 28, 3, 3,
                        &state);
  assert(!state.failed && state.missing_count == 0 &&
         state.stable_count == 50);

  // The fourth consecutive rejection confirms an outage and then requires
  // the existing 22-consecutive-accepted recovery.
  for (int i = 0; i < 3; ++i) {
    rejected = AdvanceGpsOutageState(
        GpsObservation::kRejected, true, 50, 28, 3, 3, &state);
    assert(!rejected.entered_full_failure && !state.failed);
  }
  rejected = AdvanceGpsOutageState(
      GpsObservation::kRejected, true, 50, 28, 3, 3, &state);
  assert(rejected.entered_full_failure && rejected.clear_candidate_window);
  assert(state.failed && state.stable_count == 28 &&
         state.missing_count == 4);
  for (int i = 0; i < 10; ++i)
    AdvanceGpsOutageState(GpsObservation::kAccepted, true, 50, 28, 3, 3,
                          &state);
  assert(state.stable_count == 38);
  // A packet hole during hard recovery breaks consecutiveness.
  const auto recovery_hole = AdvanceGpsOutageState(
      GpsObservation::kMissing, true, 50, 28, 3, 3, &state);
  assert(state.failed && state.stable_count == 28);
  assert(recovery_hole.clear_candidate_window);
  for (int i = 0; i < 21; ++i)
    AdvanceGpsOutageState(GpsObservation::kAccepted, true, 50, 28, 3, 3,
                          &state);
  // The 22nd prospective accepted sample is the atomic commit boundary.
  assert(ShouldCommitAcceptedGps(state, 50, 3));
  AdvanceGpsOutageState(GpsObservation::kAccepted, true, 50, 28, 3, 3,
                        &state);
  assert(state.stable_count == 50);

  std::cout << "gps_outage_policy_test: PASS" << std::endl;
  return 0;
}
