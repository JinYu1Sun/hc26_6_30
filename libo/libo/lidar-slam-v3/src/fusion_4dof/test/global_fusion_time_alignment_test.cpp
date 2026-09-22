#include "fusion/global_fusion.h"

#include <Eigen/Geometry>
#include <cassert>
#include <cmath>

int main() {
  GlobalOptimization estimator;
  estimator.SetParams(100, 30.0, 0.05, 0.5, 0.1);
  estimator.SetTimeAlignmentParams(0.50, 0.05);
  estimator.SetInnovationParams(0.03, 0.03, 0.2, 0.5);
  const auto q = Eigen::Quaterniond::Identity();

  estimator.InputOdom(1.0, Eigen::Vector3d(0.0, 0.0, 0.0), q);
  assert(estimator.InputGPS(1.0, 0.0, 0.0, 0.0, 0.0, true, true));
  estimator.InputOdom(1.1, Eigen::Vector3d(1.0, 0.0, 0.0), q);

  // The nearest raw LIO frame is 0.5 m away. Exact-time interpolation must
  // pass the tight 3 cm innovation limit and produce an unbiased seed.
  assert(estimator.InputGPS(1.05, 0.5, 0.0, 0.0, 0.0, true, true));
  double theta = 0.0, tx = 0.0, ty = 0.0;
  assert(estimator.GetSeedOnlyTransform(theta, tx, ty));
  assert(std::fabs(tx) < 1e-8);
  assert(std::fabs(ty) < 1e-8);

  // A receiver sample slightly after the newest LIO may be extrapolated,
  // but the 50 ms bound remains strict.
  assert(estimator.InputGPS(1.12, 1.2, 0.0, 0.0, 0.0, true, true));
  assert(estimator.GetSeedOnlyTransform(theta, tx, ty));
  assert(std::fabs(tx) < 1e-8);
  assert(!estimator.InputGPS(1.16, 1.6, 0.0, 0.0, 0.0, true, true));

  // A resource-constrained 5 Hz LIO still provides a valid interpolation
  // bracket; the match tolerance constrains the nearest sample separately.
  estimator.InputOdom(1.3, Eigen::Vector3d(3.0, 0.0, 0.0), q);
  assert(estimator.InputGPS(1.28, 2.8, 0.0, 0.0, 0.0, true, true));

  // Clear must remove the former generation's interpolation history.
  estimator.ResetLioCoupled();
  assert(!estimator.InputGPS(1.05, 0.5, 0.0, 0.0, 0.0, true, true));
  return 0;
}
