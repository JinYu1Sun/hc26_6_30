#include "fusion/global_fusion.h"

#include <Eigen/Geometry>
#include <cassert>
#include <cmath>
#include <iostream>

namespace {
bool Near(double a, double b, double eps = 1e-9) {
  return std::fabs(a - b) <= eps;
}

void ExpectTransform(const GlobalOptimization &estimator, double theta,
                     double tx, double ty) {
  double actual_theta = 0.0, actual_tx = 0.0, actual_ty = 0.0;
  assert(estimator.GetTransform(actual_theta, actual_tx, actual_ty));
  assert(Near(actual_theta, theta));
  assert(Near(actual_tx, tx));
  assert(Near(actual_ty, ty));
}
} // namespace

int main() {
  GlobalOptimization estimator;
  estimator.SetParams(400, 45.0, 0.05, 0.5, 0.35);
  estimator.SetRobustParams(15, 3, 0.5);

  const Eigen::Quaterniond identity = Eigen::Quaterniond::Identity();

  // 建立初始生产变换 T=(theta=0, tx=0, ty=0)。
  estimator.InputOdom(0.0, Eigen::Vector3d(0.0, 0.0, 0.0), identity);
  estimator.InputGPS(0.0, 0.0, 0.0, 2.3, 0.0, true, true, true);
  ExpectTransform(estimator, 0.0, 0.0, 0.0);

  // 模拟 RTK 长时间失效后恢复：GPS 相对冻结的 LIO 回退位置恒偏 1.14m。
  // 每个样本都必须在入窗前被拒绝，不得用“连续且内部一致”的
  // 错误RTK重写生产变换。
  estimator.ClearGpsWindowKeepTransform();
  for (int i = 1; i <= 22; ++i) {
    const double t = static_cast<double>(i);
    estimator.InputOdom(t, Eigen::Vector3d(t, 0.0, 0.0), identity);
    assert(!estimator.InputGPS(t, t + 1.14, 0.0, 2.3, 0.0, true,
                               true, i == 22));
    ExpectTransform(estimator, 0.0, 0.0, 0.0);

    Eigen::Vector3d output;
    Eigen::Quaterniond output_q;
    estimator.GetGlobalOdom(output, output_q);
    assert(Near(output.x(), t));
  }

  // 构造一个多数点无法由同一刚体变换解释的候选窗。即使请求提交，生产
  // 变换也必须保持在上一次可信值，不能重现 outlier_ratio=1 时仍改 tx/ty。
  estimator.ClearGpsWindowKeepTransform();
  estimator.SetRobustParams(4, 1, 0.05);
  const double gps_x[] = {100.0, -80.0, 60.0, -40.0};
  const double gps_y[] = {30.0, 70.0, -90.0, -50.0};
  for (int i = 0; i < 4; ++i) {
    const double t = 30.0 + i;
    estimator.InputOdom(t, Eigen::Vector3d(i, 0.0, 0.0), identity);
    estimator.InputGPS(t, gps_x[i], gps_y[i], 2.3, 0.0, true, true,
                       i == 3);
  }
  ExpectTransform(estimator, 0.0, 0.0, 0.0);

  // A genuinely locked transform is immutable in all four components. Small
  // RTK residuals may continue to be observed, but cannot move production.
  GlobalOptimization locked;
  locked.SetParams(100, 30.0, 0.05, 0.2, 0.05);
  locked.SetRobustParams(6, 3, 0.5);
  locked.SetLockIndependenceParams(0.1, 0.1);
  locked.SetVerticalParams(6, 0.05, 0.30, 100.0);
  for (int i = 0; i < 12; ++i) {
    const double t = 0.2 * i;
    const Eigen::Vector3d p(0.3 * i, (i % 3) * 0.3, 0.05 * i);
    const double local_yaw = 0.08 * i;
    const Eigen::Quaterniond q(
        Eigen::AngleAxisd(local_yaw, Eigen::Vector3d::UnitZ()));
    locked.InputOdom(t, p, q);
    assert(locked.InputGPS(t, p.x() + 4.0, p.y() - 2.0, p.z() + 1.5,
                           local_yaw, true, true, true));
  }
  assert(locked.IsLocked());
  double theta, tx, ty, tz;
  assert(locked.GetTransform4D(theta, tx, ty, tz));
  locked.InputOdom(3.0, Eigen::Vector3d(4.5, 0.3, 0.8), identity);
  assert(locked.InputGPS(3.0, 4.5 + 4.2, 0.3 - 2.0, 0.8 + 1.7,
                         0.0, true, true, true));
  double theta_after, tx_after, ty_after, tz_after;
  assert(locked.GetTransform4D(theta_after, tx_after, ty_after, tz_after));
  assert(Near(theta_after, theta) && Near(tx_after, tx) &&
         Near(ty_after, ty) && Near(tz_after, tz));

  // Rapid callbacks over nearly the same sliding window are not independent
  // lock evidence. The initial geometric hypothesis needs two later,
  // time/space-separated observations that agree with that frozen hypothesis.
  GlobalOptimization independent_lock;
  independent_lock.SetParams(100, 30.0, 0.05, 0.05, 0.02);
  independent_lock.SetRobustParams(4, 3, 0.5);
  // This case isolates confirmation independence; condition-number behavior
  // is covered separately below with the production limit of 50.
  independent_lock.SetLockGeometryParams(0.70, 0.05, 1.0e6,
                                         30.0 * M_PI / 180.0);
  independent_lock.SetLockIndependenceParams(1.0, 0.30);
  independent_lock.SetVerticalParams(4, 0.05, 0.30, 100.0);
  for (int i = 0; i < 20; ++i) {
    const double t = 0.05 * i;
    const Eigen::Vector3d p(0.10 * i, (i % 2) * 0.20, 0.0);
    const double local_yaw = 0.05 * i;
    const Eigen::Quaterniond q(
        Eigen::AngleAxisd(local_yaw, Eigen::Vector3d::UnitZ()));
    independent_lock.InputOdom(t, p, q);
    assert(independent_lock.InputGPS(t, p.x(), p.y(), 1.0, local_yaw, true,
                                     true, true));
  }
  assert(!independent_lock.IsLocked());
  const Eigen::Quaterniond q_120(
      Eigen::AngleAxisd(1.0, Eigen::Vector3d::UnitZ()));
  independent_lock.InputOdom(1.20, Eigen::Vector3d(2.4, 0.2, 0.0), q_120);
  assert(independent_lock.InputGPS(1.20, 2.4, 0.2, 1.0, 1.0, true, true,
                                   true));
  assert(!independent_lock.IsLocked());
  const Eigen::Quaterniond q_230(
      Eigen::AngleAxisd(1.1, Eigen::Vector3d::UnitZ()));
  independent_lock.InputOdom(2.30, Eigen::Vector3d(3.0, 0.0, 0.0), q_230);
  assert(independent_lock.InputGPS(2.30, 3.0, 0.0, 1.0, 1.1, true, true,
                                   true));
  assert(!independent_lock.IsLocked());
  const Eigen::Quaterniond q_340(
      Eigen::AngleAxisd(1.2, Eigen::Vector3d::UnitZ()));
  independent_lock.InputOdom(3.40, Eigen::Vector3d(3.6, 0.2, 0.0), q_340);
  assert(independent_lock.InputGPS(3.40, 3.6, 0.2, 1.0, 1.2, true, true,
                                   true));
  assert(independent_lock.IsLocked());

  // Two-dimensional position scatter alone is insufficient: without a real
  // LIO heading excursion, static position noise/side slip must not lock yaw.
  // Alternating +/-1 degree yaw explicitly verifies that jitter is not summed
  // indefinitely into the default 30-degree turn requirement.
  GlobalOptimization no_turn;
  no_turn.SetParams(100, 30.0, 0.05, 0.05, 0.02);
  no_turn.SetRobustParams(4, 3, 0.5);
  no_turn.SetLockIndependenceParams(0.1, 0.1);
  no_turn.SetVerticalParams(4, 0.05, 0.30, 100.0);
  for (int i = 0; i < 30; ++i) {
    const double t = 0.2 * i;
    const Eigen::Vector3d p(0.15 * i, (i % 2) * 0.20, 0.0);
    const double jitter_yaw = (i % 2 == 0 ? 1.0 : -1.0) * M_PI / 180.0;
    const Eigen::Quaterniond jitter_q(
        Eigen::AngleAxisd(jitter_yaw, Eigen::Vector3d::UnitZ()));
    no_turn.InputOdom(t, p, jitter_q);
    assert(no_turn.InputGPS(t, p.x(), p.y(), 1.0, jitter_yaw, true, true,
                            true));
  }
  assert(!no_turn.IsLocked());

  // Default production lock sample threshold: 14 geometrically good robust
  // inliers are insufficient; the 15th makes the one-confirmation test lock.
  // Only confirmation count is shortened here so the sample boundary can be
  // isolated; the production min_samples=15 and geometry defaults are kept.
  GlobalOptimization min_sample_boundary;
  min_sample_boundary.SetParams(100, 30.0, 0.05, 0.05, 0.10);
  min_sample_boundary.SetRobustParams(15, 1, 0.5);
  min_sample_boundary.SetVerticalParams(10, 0.05, 0.30, 100.0);
  for (int i = 0; i < 15; ++i) {
    const double t = 0.2 * i;
    const Eigen::Vector3d p(0.30 * i, 0.40 * (i % 3), 0.0);
    const double yaw = 0.05 * i;
    const Eigen::Quaterniond q(
        Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()));
    min_sample_boundary.InputOdom(t, p, q);
    assert(min_sample_boundary.InputGPS(t, p.x(), p.y(), 1.0, yaw, true,
                                        true, true));
    if (i == 13)
      assert(!min_sample_boundary.IsLocked());
  }
  assert(min_sample_boundary.GetHorizontalInlierCount() == 15);
  assert(min_sample_boundary.IsLocked());

  // Exercise the production 70% robust-inlier requirement independently of
  // the 15-point minimum. Symmetric +/-1m outliers do not bias the all-point
  // reference transform; they are then deterministically trimmed at 0.5m.
  struct LockCaseResult {
    std::size_t inliers;
    double condition_number;
    bool locked;
  };
  const auto build_inlier_ratio_case = [](int good_count) {
    GlobalOptimization estimator;
    estimator.SetParams(100, 30.0, 0.05, 0.05, 0.10);
    estimator.SetRobustParams(15, 1, 0.5);
    estimator.SetInnovationParams(5.0, 5.0, M_PI, 5.0);
    estimator.SetVerticalParams(10, 0.05, 0.30, 100.0);
    int sample = 0;
    for (int i = 0; i < good_count; ++i, ++sample) {
      const double t = 0.2 * sample;
      const Eigen::Vector3d p(0.25 * i, 0.40 * (i % 3), 0.0);
      const double yaw = 0.05 * i;
      const Eigen::Quaterniond q(
          Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()));
      estimator.InputOdom(t, p, q);
      assert(estimator.InputGPS(t, p.x(), p.y(), 1.0, yaw, true, true,
                                false));
    }
    for (int pair = 0; pair < 4; ++pair) {
      const Eigen::Vector3d p(0.50 * pair, 0.40 * (pair % 2), 0.0);
      const double yaw = 0.05 * (good_count - 1);
      for (int sign : {-1, 1}) {
        const double t = 0.2 * sample++;
        const Eigen::Quaterniond q(
            Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()));
        estimator.InputOdom(t, p, q);
        const bool final_sample = pair == 3 && sign == 1;
        assert(estimator.InputGPS(t, p.x(), p.y() + sign * 1.0, 1.0,
                                  yaw, true, true, final_sample));
      }
    }
    return LockCaseResult{estimator.GetHorizontalInlierCount(),
                          estimator.GetLockConditionNumber(),
                          estimator.IsLocked()};
  };
  const LockCaseResult below_ratio = build_inlier_ratio_case(18); // 18/26
  assert(below_ratio.inliers == 18);
  assert(!below_ratio.locked);
  const LockCaseResult above_ratio = build_inlier_ratio_case(19); // 19/27
  assert(above_ratio.inliers == 19);
  assert(above_ratio.locked);

  // True covariance condition-number boundary. For the symmetric 5x3 grid,
  // lambda_x/lambda_y is constructed to be exactly the requested value.
  const auto build_condition_case = [](double target_condition) {
    GlobalOptimization estimator;
    estimator.SetParams(100, 30.0, 0.05, 0.05, 0.10);
    estimator.SetRobustParams(15, 1, 0.5);
    estimator.SetVerticalParams(10, 0.05, 0.30, 100.0);
    const double b = 0.20;
    const double a = std::sqrt(target_condition * b * b / 3.0);
    int sample = 0;
    for (int ix = -2; ix <= 2; ++ix) {
      for (int iy = -1; iy <= 1; ++iy, ++sample) {
        const double t = 0.2 * sample;
        const Eigen::Vector3d p(a * ix, b * iy, 0.0);
        const double yaw = 0.05 * sample;
        const Eigen::Quaterniond q(
            Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()));
        estimator.InputOdom(t, p, q);
        assert(estimator.InputGPS(t, p.x(), p.y(), 1.0, yaw, true, true,
                                  true));
      }
    }
    return LockCaseResult{estimator.GetHorizontalInlierCount(),
                          estimator.GetLockConditionNumber(),
                          estimator.IsLocked()};
  };
  const LockCaseResult condition_49 = build_condition_case(49.0);
  assert(std::fabs(condition_49.condition_number - 49.0) < 1e-8);
  assert(condition_49.locked);
  const LockCaseResult condition_51 = build_condition_case(51.0);
  assert(std::fabs(condition_51.condition_number - 51.0) < 1e-8);
  assert(!condition_51.locked);

  std::cout << "global_fusion recovery regression tests passed" << std::endl;
  return 0;
}
