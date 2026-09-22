#include "gravity_leveling.h"

#include <Eigen/Geometry>
#include <cmath>
#include <iostream>
#include <limits>

namespace
{
constexpr double kTolerance = 1e-10;
constexpr double kPi = 3.14159265358979323846;

bool Near(const Eigen::Vector3d &a, const Eigen::Vector3d &b,
          double tolerance = kTolerance)
{
    return (a - b).norm() <= tolerance;
}

int Fail(const char *message)
{
    std::cerr << "gravity_leveling_test: " << message << std::endl;
    return 1;
}
} // namespace

int main()
{
    // Synthetic 12 degree installation pitch: W is the initial IMU frame and
    // H is horizontal.  The recovered leveling rotation must equal the known
    // pitch (gravity alone cannot recover an additional yaw, hence yaw=0 here).
    const double pitch = 12.0 * kPi / 180.0;
    const Eigen::Matrix3d known_R_H_W =
        Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()).toRotationMatrix();
    const Eigen::Vector3d gravity_H(0.0, 0.0, -9.81);
    const Eigen::Vector3d gravity_W = known_R_H_W.transpose() * gravity_H;

    Eigen::Matrix3d recovered_R_H_W = Eigen::Matrix3d::Identity();
    double error = 0.0;
    double tilt = 0.0;
    if (!fast_lio::ComputeGravityLevelRotation(
            gravity_W, &recovered_R_H_W, 5.0, 15.0, &error, &tilt))
        return Fail("valid 12 degree gravity vector was rejected");
    if (!Near(recovered_R_H_W * gravity_W, gravity_H))
        return Fail("gravity was not mapped to H down axis");
    if ((recovered_R_H_W - known_R_H_W).norm() > kTolerance)
        return Fail("recovered pitch rotation differs from the known rotation");
    if (std::fabs(tilt - pitch) > kTolerance || error > kTolerance)
        return Fail("reported tilt/alignment error is inconsistent");

    // A rigid world rotation must preserve the full 3D displacement norm.  A
    // level-ground displacement that looked like XY+Z in W must become z=0 in H.
    const Eigen::Vector3d displacement_H(12.767, 0.0, 0.0);
    const Eigen::Vector3d displacement_W =
        known_R_H_W.transpose() * displacement_H;
    const Eigen::Vector3d recovered_displacement_H =
        recovered_R_H_W * displacement_W;
    if (!Near(recovered_displacement_H, displacement_H, 1e-9))
        return Fail("level-ground displacement was not recovered");
    if (std::fabs(displacement_W.norm() - recovered_displacement_H.norm()) >
        kTolerance)
        return Fail("rigid leveling changed the 3D metric scale");

    // Full pose composition: if the IMU is mounted at +12 degrees and the
    // calibrated IMU->base rotation is -12 degrees, the base is level.
    const Eigen::Matrix3d R_H_I = recovered_R_H_W;
    const Eigen::Matrix3d R_I_B =
        Eigen::AngleAxisd(-pitch, Eigen::Vector3d::UnitY()).toRotationMatrix();
    if ((R_H_I * R_I_B - Eigen::Matrix3d::Identity()).norm() > kTolerance)
        return Fail("world leveling and IMU->base extrinsic compose incorrectly");

    // The surveyed lever arm is LiDAR->base in base axes, whereas FAST-LIO's
    // state origin is IMU and its configured offset is IMU->LiDAR in IMU axes.
    // Verify the deployed folded IMU->base value and its pose composition.
    const Eigen::Vector3d t_I_L_I(-0.011, -0.02329, 0.04412);
    const Eigen::Vector3d t_L_B_B(-0.9001752, 0.0, -0.2589463);
    const Eigen::Matrix3d R_B_I = R_I_B.transpose();
    const Eigen::Vector3d t_I_B_B = t_L_B_B + R_B_I * t_I_L_I;
    const Eigen::Vector3d expected_t_I_B_B(-0.9017618, -0.02329,
                                            -0.2135034);
    if (!Near(t_I_B_B, expected_t_I_B_B, 1e-7))
        return Fail("folded IMU->base lever arm differs from launch value");
    const Eigen::Vector3d t_I_B_I = R_I_B * t_I_B_B;
    if (!Near(R_H_I * t_I_B_I, t_I_B_B, 1e-10))
        return Fail("IMU->base lever-arm pose composition is inconsistent");

    Eigen::Matrix3d unused;
    if (fast_lio::ComputeGravityLevelRotation(Eigen::Vector3d::Zero(), &unused))
        return Fail("zero gravity vector was accepted");
    if (fast_lio::ComputeGravityLevelRotation(
            Eigen::Vector3d(std::numeric_limits<double>::quiet_NaN(), 0.0, -9.81),
            &unused))
        return Fail("non-finite gravity vector was accepted");

    std::cout << "gravity_leveling_test: PASS" << std::endl;
    return 0;
}
