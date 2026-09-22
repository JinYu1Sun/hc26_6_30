#ifndef FAST_LIO_GRAVITY_LEVELING_H
#define FAST_LIO_GRAVITY_LEVELING_H

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <algorithm>
#include <cmath>

namespace fast_lio
{

// Compute the fixed rotation R_H_W which expresses a vector from FAST-LIO's
// per-process world W in a gravity-level local world H.  FAST-LIO stores the
// gravity vector as "down" in W, so a valid result satisfies
//
//     R_H_W * gravity_W = [0, 0, -|g|].
//
// Gravity leaves yaw unobservable.  Eigen's FromTwoVectors supplies the
// minimum-angle representative; the remaining yaw gauge is intentionally left
// to fusion_se2's H->ENU SE(2) alignment.
inline bool ComputeGravityLevelRotation(
    const Eigen::Vector3d &gravity_W, Eigen::Matrix3d *R_H_W,
    double min_gravity_norm = 5.0, double max_gravity_norm = 15.0,
    double *alignment_error = nullptr, double *tilt_rad = nullptr)
{
    if (R_H_W == nullptr || !gravity_W.allFinite() ||
        !std::isfinite(min_gravity_norm) ||
        !std::isfinite(max_gravity_norm) || min_gravity_norm <= 0.0 ||
        max_gravity_norm <= min_gravity_norm)
        return false;

    const double gravity_norm = gravity_W.norm();
    if (!std::isfinite(gravity_norm) || gravity_norm < min_gravity_norm ||
        gravity_norm > max_gravity_norm)
        return false;

    const Eigen::Vector3d down_W = gravity_W / gravity_norm;
    const Eigen::Vector3d down_H(0.0, 0.0, -1.0);
    Eigen::Quaterniond q_H_W =
        Eigen::Quaterniond::FromTwoVectors(down_W, down_H);
    if (!q_H_W.coeffs().allFinite() || q_H_W.norm() < 1e-12)
        return false;
    q_H_W.normalize();

    const Eigen::Matrix3d result = q_H_W.toRotationMatrix();
    const double error = (result * down_W - down_H).norm();
    const double determinant = result.determinant();
    if (!result.allFinite() || !std::isfinite(error) || error > 1e-9 ||
        !std::isfinite(determinant) || std::fabs(determinant - 1.0) > 1e-9)
        return false;

    *R_H_W = result;
    if (alignment_error != nullptr)
        *alignment_error = error;
    if (tilt_rad != nullptr)
    {
        double cosine = down_W.dot(down_H);
        cosine = std::max(-1.0, std::min(1.0, cosine));
        *tilt_rad = std::acos(cosine);
    }
    return true;
}

} // namespace fast_lio

#endif // FAST_LIO_GRAVITY_LEVELING_H
