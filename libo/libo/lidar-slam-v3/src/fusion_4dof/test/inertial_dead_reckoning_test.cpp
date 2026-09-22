#include "fusion/inertial_dead_reckoning.h"

#include <cassert>
#include <cmath>

using fusion_4dof::InertialDeadReckoning;
using fusion_4dof::InertialDeadReckoningConfig;

static void CalibrateAndAnchor(InertialDeadReckoning *dr, double start) {
  const Eigen::Vector3d accel(0.0, 0.0, 9.80665);
  const Eigen::Vector3d gyro = Eigen::Vector3d::Zero();
  // These means represent the one static window already quality-gated by
  // FAST-LIO; the inertial integrator must not run its own timed detector.
  assert(dr->SetStaticCalibration(accel, gyro));
  assert(dr->InputImu(start + 2.0, accel, gyro));
  assert(dr->SetMainPose(start + 2.0, Eigen::Vector3d::Zero(),
                         Eigen::Quaterniond::Identity(),
                         Eigen::Vector3d::Zero(), true));
  dr->InputImu(start + 2.01, accel, gyro);
  dr->InputImu(start + 2.02, accel, gyro);
  assert(dr->bias_ready());
}

int main() {
  InertialDeadReckoningConfig cfg;
  cfg.fallback_max_duration_sec = 10.0;
  cfg.fallback_max_distance_m = 3.0;

  // Raw stationary samples alone must never create a second independent
  // calibration window; only FAST-LIO's exported statistics authorize it.
  InertialDeadReckoning without_authority(cfg);
  for (int i = 0; i <= 300; ++i)
    assert(without_authority.InputImu(
        1.0 + 0.01 * i, Eigen::Vector3d(0.0, 0.0, 9.80665),
        Eigen::Vector3d::Zero()));
  assert(!without_authority.SetMainPose(
      4.01, Eigen::Vector3d::Zero(), Eigen::Quaterniond::Identity(),
      Eigen::Vector3d::Zero(), true));
  assert(!without_authority.SetStaticCalibration(
      Eigen::Vector3d::Constant(std::nan("")), Eigen::Vector3d::Zero()));

  InertialDeadReckoning dr(cfg);
  CalibrateAndAnchor(&dr, 1.0);

  // Stationary integration must not create motion after bias calibration.
  for (int i = 203; i <= 300; ++i)
    assert(dr.InputImu(1.0 + 0.01 * i, Eigen::Vector3d(0.0, 0.0, 9.80665),
                       Eigen::Vector3d::Zero()));
  Eigen::Vector3d p, v;
  Eigen::Quaterniond q;
  assert(dr.GetState(&p, &q, &v));
  assert(p.norm() < 1e-8);
  assert(v.norm() < 1e-8);

  assert(dr.StartFallback(4.0));
  assert(dr.fallback_active());
  dr.CancelFallback();
  assert(!dr.fallback_active());
  assert(dr.GetState(&p, &q, &v));
  assert(dr.StartFallback(4.0));
  assert(dr.SourceStatus(4.0) == InertialDeadReckoning::Status::kUsable);
  assert(dr.SourceStatus(4.2) == InertialDeadReckoning::Status::kStale);

  // A gap during fallback is terminal for this fallback generation.
  assert(!dr.InputImu(5.0, Eigen::Vector3d(0.0, 0.0, 9.80665),
                      Eigen::Vector3d::Zero()));
  dr.InputImu(5.01, Eigen::Vector3d(0.0, 0.0, 9.80665),
              Eigen::Vector3d::Zero());
  assert(dr.SourceStatus(5.01) == InertialDeadReckoning::Status::kStale);

  // Test-mode anchoring is one-shot when allow_reanchor=false.
  InertialDeadReckoning test_mode(cfg);
  CalibrateAndAnchor(&test_mode, 10.0);
  assert(test_mode.SetMainPose(12.1, Eigen::Vector3d(100.0, 0.0, 0.0),
                               Eigen::Quaterniond::Identity(),
                               Eigen::Vector3d::Zero(), false));
  assert(test_mode.GetState(&p, &q, &v));
  assert(p.norm() < 1e-8);

  // The fallback expires on elapsed time even when stationary and IMU-fresh.
  InertialDeadReckoningConfig time_cfg = cfg;
  time_cfg.fallback_max_duration_sec = 0.10;
  InertialDeadReckoning time_limited(time_cfg);
  CalibrateAndAnchor(&time_limited, 20.0);
  assert(time_limited.StartFallback(22.02));
  for (int i = 3; i <= 15; ++i)
    assert(time_limited.InputImu(22.0 + 0.01 * i,
                                 Eigen::Vector3d(0.0, 0.0, 9.80665),
                                 Eigen::Vector3d::Zero()));
  assert(time_limited.SourceStatus(22.15) ==
         InertialDeadReckoning::Status::kExpired);

  // Horizontal specific force is double-integrated and the distance limit is
  // authoritative even before the time limit.
  InertialDeadReckoningConfig distance_cfg = cfg;
  distance_cfg.fallback_max_distance_m = 0.01;
  InertialDeadReckoning distance_limited(distance_cfg);
  CalibrateAndAnchor(&distance_limited, 30.0);
  assert(distance_limited.StartFallback(32.02));
  for (int i = 3; i <= 23; ++i)
    assert(distance_limited.InputImu(
        32.0 + 0.01 * i, Eigen::Vector3d(1.0, 0.0, 9.80665),
        Eigen::Vector3d::Zero()));
  assert(distance_limited.GetState(&p, &q, &v));
  assert(p.x() > 0.015 && v.x() > 0.19);
  assert(distance_limited.SourceStatus(32.23) ==
         InertialDeadReckoning::Status::kExpired);

  // A noisy differentiated main-pose velocity and small residual IMU bias
  // must not make a physically stationary vehicle travel forever in state 6.
  InertialDeadReckoning stationary(cfg);
  CalibrateAndAnchor(&stationary, 35.0);
  assert(stationary.SetMainPose(
      37.03, Eigen::Vector3d(5.0, 6.0, 0.0),
      Eigen::Quaterniond::Identity(), Eigen::Vector3d(0.7, -0.4, 0.0), true));
  for (int i = 4; i <= 104; ++i) {
    assert(stationary.InputImu(
        37.0 + 0.01 * i, Eigen::Vector3d(0.04, -0.03, 9.85665),
        Eigen::Vector3d(0.004, -0.003, 0.002)));
  }
  // Before state 6 begins, the deliberately injected residual bias has made
  // the free integrator drift away from its last trusted anchor.
  assert(stationary.GetState(&p, &q, &v));
  assert((p - Eigen::Vector3d(5.0, 6.0, 0.0)).norm() > 1e-4);
  assert(stationary.StartFallback(38.04));
  Eigen::Vector3d fallback_start_position;
  assert(stationary.GetState(&fallback_start_position, &q, &v));
  assert((fallback_start_position - Eigen::Vector3d(5.0, 6.0, 0.0)).norm() <
         1e-10);
  for (int i = 105; i <= 205; ++i) {
    assert(stationary.InputImu(
        37.0 + 0.01 * i, Eigen::Vector3d(0.04, -0.03, 9.85665),
        Eigen::Vector3d(0.004, -0.003, 0.002)));
    assert(stationary.GetState(&p, &q, &v));
    assert((p - fallback_start_position).norm() < 1e-10);
    assert(v.norm() < 1e-12);
  }
  assert(stationary.stationary_hold_active());
  assert(stationary.GetState(&p, &q, &v));
  assert((p - fallback_start_position).norm() < 1e-10);
  assert(v.norm() < 1e-12);

  // Three-axis gyro integration propagates attitude in the body frame.
  InertialDeadReckoning attitude(cfg);
  CalibrateAndAnchor(&attitude, 40.0);
  for (int i = 3; i <= 22; ++i)
    assert(attitude.InputImu(42.0 + 0.01 * i,
                             Eigen::Vector3d(0.0, 0.0, 9.80665),
                             Eigen::Vector3d(0.0, 0.0, 0.5)));
  assert(attitude.GetState(&p, &q, &v));
  const Eigen::Quaterniond expected_yaw(
      Eigen::AngleAxisd(0.0975, Eigen::Vector3d::UnitZ()));
  assert(q.angularDistance(expected_yaw) < 1e-10);

  // A map switch invalidates the global anchor without discarding the costly
  // static IMU calibration. No old-map state may remain publishable, and the
  // next trusted pose can immediately anchor in the new ENU frame.
  attitude.ResetGlobalAnchor();
  assert(attitude.has_static_calibration());
  assert(!attitude.GetState(&p, &q, &v));
  assert(attitude.SourceStatus(42.23) ==
         InertialDeadReckoning::Status::kUnready);
  assert(attitude.SetMainPose(42.24, Eigen::Vector3d(20.0, 30.0, 1.0),
                              Eigen::Quaterniond::Identity(),
                              Eigen::Vector3d::Zero(), true));
  assert(attitude.GetState(&p, &q, &v));
  assert((p - Eigen::Vector3d(20.0, 30.0, 1.0)).norm() < 1e-12);

  attitude.InvalidateStaticCalibration();
  assert(!attitude.has_static_calibration());
  assert(attitude.SourceStatus(42.23) ==
         InertialDeadReckoning::Status::kUnready);
  return 0;
}
