#include "fusion/global_fusion.h"
#include "fusion/inertial_dead_reckoning.h"

#include <Eigen/Geometry>
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
  GlobalOptimization estimator;
  estimator.SetParams(40, 10.0, 0.02, 0.2, 0.05);
  estimator.SetVerticalParams(1, 0.1, 0.3, 100.0);

  // Old-map state contains both a committed transform and a readable
  // seed-only transform.
  estimator.InputOdom(1.0, Eigen::Vector3d(2.0, 3.0, 0.5),
                      Eigen::Quaterniond::Identity());
  estimator.InputGPS(1.0, 12.0, 23.0, 1.5, 0.4, true, true, true);
  double theta = 0.0, tx = 0.0, ty = 0.0, tz = 0.0;
  assert(estimator.GetSeedOnlyTransform4D(theta, tx, ty, tz));

  fusion_4dof::InertialDeadReckoning imu;
  assert(imu.SetStaticCalibration(Eigen::Vector3d(0.0, 0.0, 9.80665),
                                  Eigen::Vector3d::Zero()));
  assert(imu.SetMainPose(1.0, Eigen::Vector3d(12.0, 23.0, 1.5),
                         Eigen::Quaterniond::Identity(),
                         Eigen::Vector3d::Zero(), true));
  assert(imu.initialized());

  estimator.ResetForMapSwitch();
  imu.ResetGlobalAnchor();
  assert(!estimator.Is4DAligned());
  assert(!estimator.GetSeedOnlyTransform4D(theta, tx, ty, tz));
  assert(imu.has_static_calibration());
  assert(!imu.initialized());

  // New-map samples must be the only possible seed after the reset.
  const Eigen::Vector3d new_lio(2.1, 3.0, 0.5);
  const double new_theta = -0.2;
  const double c = std::cos(new_theta), s = std::sin(new_theta);
  estimator.InputOdom(2.0, new_lio,
                      Eigen::Quaterniond::Identity());
  estimator.InputGPS(2.0,
                     c * new_lio.x() - s * new_lio.y() - 10.0,
                     s * new_lio.x() + c * new_lio.y() + 5.0,
                     new_lio.z() + 2.0, new_theta, true, true, true);
  assert(estimator.GetSeedOnlyTransform4D(theta, tx, ty, tz));
  assert(std::fabs(theta + 0.2) < 1e-12);
  assert(std::fabs(tx + 10.0) < 1e-12);
  assert(std::fabs(ty - 5.0) < 1e-12);
  assert(std::fabs(tz - 2.0) < 1e-12);
  std::cout << "map_switch_state_test: PASS" << std::endl;
  return 0;
}
