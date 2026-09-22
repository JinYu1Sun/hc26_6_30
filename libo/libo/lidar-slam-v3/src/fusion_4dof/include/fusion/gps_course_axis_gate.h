#pragma once

#include <algorithm>
#include <cmath>
#include <deque>

namespace fusion_4dof {

enum class GpsCourseAxisResult { kInsufficientEvidence, kTurning,
                                 kAlignedForward, kAlignedReverse,
                                 kSuspect, kRejected };

struct GpsCourseAxisConfig {
  double min_window_sec = 0.30;
  double max_window_sec = 0.65;
  double min_displacement_m = 0.25;
  double max_turn_rate_radps = 0.25;
  double max_axis_error_rad = 40.0 * M_PI / 180.0;
  int reject_after_windows = 3;
};

class GpsCourseAxisGate {
 public:
  explicit GpsCourseAxisGate(const GpsCourseAxisConfig &config = {})
      : config_(config) {}

  void Reset() {
    samples_.clear();
    mismatch_windows_ = 0;
    have_last_mismatch_evidence_ = false;
  }

  GpsCourseAxisResult Observe(double stamp, double x, double y, double yaw,
                              bool heading_trusted,
                              double independent_turn_rate_radps,
                              bool turn_rate_trusted) {
    if (!heading_trusted || !turn_rate_trusted ||
        !std::isfinite(independent_turn_rate_radps) ||
        independent_turn_rate_radps < 0.0 ||
        !std::isfinite(stamp) || !std::isfinite(x) ||
        !std::isfinite(y) || !std::isfinite(yaw)) {
      Reset();
      return GpsCourseAxisResult::kInsufficientEvidence;
    }
    // Clear the displacement window at the turn boundary; a curved segment
    // must not later be evaluated as a straight-line course merely because
    // the latest angular-rate sample fell below threshold.
    if (independent_turn_rate_radps >= config_.max_turn_rate_radps) {
      Reset();
      samples_.push_back({stamp, x, y});
      return GpsCourseAxisResult::kTurning;
    }
    if (!samples_.empty() &&
        (stamp <= samples_.back().stamp ||
         stamp - samples_.back().stamp > config_.max_window_sec))
      Reset();
    samples_.push_back({stamp, x, y});
    while (samples_.size() > 1 &&
           stamp - samples_.front().stamp > config_.max_window_sec)
      samples_.pop_front();
    const double dt = stamp - samples_.front().stamp;
    if (dt < config_.min_window_sec)
      return GpsCourseAxisResult::kInsufficientEvidence;

    const double dx = x - samples_.front().x;
    const double dy = y - samples_.front().y;
    if (std::hypot(dx, dy) < config_.min_displacement_m) {
      mismatch_windows_ = 0;
      have_last_mismatch_evidence_ = false;
      return GpsCourseAxisResult::kInsufficientEvidence;
    }
    const double difference =
        std::fabs(Wrap(std::atan2(dy, dx) - yaw));
    const double axis_error = std::min(difference, M_PI - difference);
    if (axis_error > config_.max_axis_error_rad) {
      // Overlapping windows are correlated. Do not count the same isolated
      // GPS side-step on three adjacent callbacks as three independent
      // mismatch observations: require fresh spatial progress each time.
      const double independent_step_m =
          config_.min_displacement_m / config_.reject_after_windows;
      if (!have_last_mismatch_evidence_ ||
          std::hypot(x - last_mismatch_evidence_x_,
                     y - last_mismatch_evidence_y_) >= independent_step_m) {
        ++mismatch_windows_;
        have_last_mismatch_evidence_ = true;
        last_mismatch_evidence_x_ = x;
        last_mismatch_evidence_y_ = y;
      }
      return mismatch_windows_ >= config_.reject_after_windows
                 ? GpsCourseAxisResult::kRejected
                 : GpsCourseAxisResult::kSuspect;
    }
    mismatch_windows_ = 0;
    have_last_mismatch_evidence_ = false;
    return difference <= M_PI / 2.0
               ? GpsCourseAxisResult::kAlignedForward
               : GpsCourseAxisResult::kAlignedReverse;
  }

 private:
  static double Wrap(double angle) {
    return std::atan2(std::sin(angle), std::cos(angle));
  }
  struct Sample { double stamp, x, y; };
  GpsCourseAxisConfig config_;
  std::deque<Sample> samples_;
  int mismatch_windows_ = 0;
  bool have_last_mismatch_evidence_ = false;
  double last_mismatch_evidence_x_ = 0.0;
  double last_mismatch_evidence_y_ = 0.0;
};

}  // namespace fusion_4dof
