#include "fusion/global_fusion.h"

#include <Eigen/Geometry>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

namespace
{
bool Near(double a, double b, double eps = 1e-8)
{
    return std::fabs(a - b) <= eps;
}
} // namespace

int main()
{
    GlobalOptimization estimator;
    estimator.SetParams(100, 30.0, 0.02, 0.2, 0.05);
    estimator.SetRobustParams(6, 1, 0.5);
    estimator.SetVerticalParams(6, 0.05, 0.30, 100.0);

    const double theta = 0.30;
    const double tx = 4.0, ty = -2.0, tz = 1.75;
    const double c = std::cos(theta), s = std::sin(theta);
    for (int i = 0; i < 12; ++i)
    {
        const double t = 0.1 * i;
        const Eigen::Vector3d lio(0.4 * i, (i % 3) * 0.3, 0.05 * i);
        const double local_yaw = 0.20 + 0.07 * i;
        const Eigen::Quaterniond q(
            Eigen::AngleAxisd(0.10, Eigen::Vector3d::UnitX()) *
            Eigen::AngleAxisd(-0.06, Eigen::Vector3d::UnitY()) *
            Eigen::AngleAxisd(local_yaw, Eigen::Vector3d::UnitZ()));
        estimator.InputOdom(t, lio, q);
        double gps_z = lio.z() + tz;
        if (i == 8)
            gps_z += 4.0; // must be rejected before it contaminates the window
        estimator.InputGPS(t, c * lio.x() - s * lio.y() + tx,
                           s * lio.x() + c * lio.y() + ty, gps_z,
                           local_yaw + theta, true, true, true);
    }

    double got_theta = 0.0, got_tx = 0.0, got_ty = 0.0, got_tz = 0.0;
    assert(estimator.GetTransform4D(got_theta, got_tx, got_ty, got_tz));
    assert(Near(got_theta, theta));
    assert(Near(got_tx, tx));
    assert(Near(got_ty, ty));
    assert(Near(got_tz, tz));
    // Once horizontal lock is committed, later RTK height samples are shadow
    // observations only and cannot update either tz or its production fit.
    assert(Near(estimator.GetVerticalOutlierRatio(), 0.0));

    // Defensive API checks: a non-finite sensor sample must not poison the
    // already committed transform or output quaternion.
    estimator.InputOdom(
        1.3,
        Eigen::Vector3d(std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0),
        Eigen::Quaterniond::Identity());
    estimator.InputGPS(1.3, std::numeric_limits<double>::infinity(), 0.0,
                       0.0, 0.0, true, true, true);
    assert(estimator.GetTransform4D(got_theta, got_tx, got_ty, got_tz));
    assert(Near(got_theta, theta) && Near(got_tx, tx) && Near(got_ty, ty) &&
           Near(got_tz, tz));

    Eigen::Vector3d global;
    Eigen::Quaterniond global_q;
    estimator.GetGlobalOdom(global, global_q);
    const Eigen::Vector3d last_lio(4.4, 0.6, 0.55);
    assert(Near(global.x(), c * last_lio.x() - s * last_lio.y() + tx));
    assert(Near(global.y(), s * last_lio.x() + c * last_lio.y() + ty));
    assert(Near(global.z(), last_lio.z() + tz));
    const Eigen::Quaterniond local_q(
        Eigen::AngleAxisd(0.10, Eigen::Vector3d::UnitX()) *
        Eigen::AngleAxisd(-0.06, Eigen::Vector3d::UnitY()) *
        Eigen::AngleAxisd(0.20 + 0.07 * 11,
                          Eigen::Vector3d::UnitZ()));
    const Eigen::Quaterniond expected_q =
        (Eigen::Quaterniond(Eigen::AngleAxisd(
             theta, Eigen::Vector3d::UnitZ())) *
         local_q)
            .normalized();
    assert(global_q.angularDistance(expected_q) < 1e-10);

    // Recovery candidates must not modify committed tz until commit=true.
    estimator.ClearGpsWindowKeepTransform();
    for (int i = 0; i < 6; ++i)
    {
        const double t = 2.0 + 0.1 * i;
        const Eigen::Vector3d lio(i, 0.0, 0.1 * i);
        estimator.InputOdom(t, lio, Eigen::Quaterniond::Identity());
        estimator.InputGPS(t, c * lio.x() - s * lio.y() + tx,
                           s * lio.x() + c * lio.y() + ty,
                           lio.z() + tz + 0.4, theta, true, true, false);
    }
    assert(estimator.GetTransform4D(got_theta, got_tx, got_ty, got_tz));
    assert(Near(got_tz, tz));

    // The threshold-crossing recovery sample may be accepted as an
    // observation, but a locked production 4DoF remains immutable. A 0.4 m
    // vertical datum change requires an explicit map/datum transaction.
    {
        const double t = 2.6;
        const Eigen::Vector3d lio(6.0, 0.0, 0.6);
        estimator.InputOdom(t, lio, Eigen::Quaterniond::Identity());
        estimator.InputGPS(t, c * lio.x() - s * lio.y() + tx,
                           s * lio.x() + c * lio.y() + ty,
                           lio.z() + tz + 0.4, theta, true, true, true);
    }
    assert(estimator.GetTransform4D(got_theta, got_tx, got_ty, got_tz));
    assert(Near(got_tz, tz));

    double seed_theta = 0.0, seed_tx = 0.0, seed_ty = 0.0, seed_tz = 0.0;
    assert(estimator.GetSeedOnlyTransform4D(seed_theta, seed_tx, seed_ty,
                                            seed_tz));
    assert(Near(seed_theta, theta) && Near(seed_tx, tx) &&
           Near(seed_ty, ty) && Near(seed_tz, tz + 0.4));
    // A newer, unmatched LIO callback must not change the diagnostic seed.
    // The seed is defined by one accepted same-time RTK/LIO observation pair.
    estimator.InputOdom(2.61, Eigen::Vector3d(99.0, -20.0, 8.0),
                        Eigen::Quaterniond(Eigen::AngleAxisd(
                            -1.0, Eigen::Vector3d::UnitZ())));
    double seed_theta_after = 0.0, seed_tx_after = 0.0, seed_ty_after = 0.0,
           seed_tz_after = 0.0;
    assert(estimator.GetSeedOnlyTransform4D(
        seed_theta_after, seed_tx_after, seed_ty_after, seed_tz_after));
    assert(Near(seed_theta_after, seed_theta) &&
           Near(seed_tx_after, seed_tx) && Near(seed_ty_after, seed_ty) &&
           Near(seed_tz_after, seed_tz));
    estimator.ResetForMapSwitch();
    assert(!estimator.Is4DAligned());
    assert(!estimator.GetSeedOnlyTransform4D(seed_theta, seed_tx, seed_ty,
                                             seed_tz));
    assert(estimator.RestoreTransform4D(theta, tx, ty, tz, true));
    assert(estimator.GetTransform4D(got_theta, got_tx, got_ty, got_tz));
    assert(Near(got_tz, tz));

    std::cout << "global_fusion_4d_test: PASS" << std::endl;
    return 0;
}
