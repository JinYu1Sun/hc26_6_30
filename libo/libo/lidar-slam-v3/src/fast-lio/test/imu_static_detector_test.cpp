#include "imu_static_detector.h"

#include <Eigen/Core>
#include <cmath>
#include <iostream>

namespace
{
int Fail(const char *message)
{
    std::cerr << "imu_static_detector_test: " << message << std::endl;
    return 1;
}

fast_lio::ImuStaticDetector::Result AddStationary(
    fast_lio::ImuStaticDetector *detector, double start, double duration,
    double dt = 0.01)
{
    fast_lio::ImuStaticDetector::Result result =
        fast_lio::ImuStaticDetector::Result::kCollecting;
    for (double t = start; t <= start + duration + 1e-9; t += dt)
    {
        const Eigen::Vector3d acc(0.01 * std::sin(t), 0.01 * std::cos(t),
                                  -9.81);
        result = detector->AddSample(t, acc, Eigen::Vector3d::Zero());
    }
    return result;
}
} // namespace

int main()
{
    using Detector = fast_lio::ImuStaticDetector;
    Detector detector;

    if (AddStationary(&detector, 10.0, 1.99) == Detector::Result::kConfirmed)
        return Fail("confirmed before two continuous seconds");
    detector.Reset();
    if (AddStationary(&detector, 12.0, 2.01) != Detector::Result::kConfirmed)
        return Fail("valid stationary window was not confirmed");

    detector.Reset();
    for (int i = 0; i <= 200; ++i)
        detector.AddSample(i * 0.01, Eigen::Vector3d(0, 0, -9.81),
                           Eigen::Vector3d(0.09, 0, 0));
    if (detector.confirmed())
        return Fail("gyro mean above threshold was accepted");

    detector.Reset();
    AddStationary(&detector, 0.0, 1.0);
    if (detector.AddSample(1.01, Eigen::Vector3d(0, 0, -9.81),
                           Eigen::Vector3d(0.21, 0, 0)) !=
        Detector::Result::kWindowRejected)
        return Fail("gyro maximum violation did not reset the window");
    if (detector.last_reject_reason() !=
        Detector::RejectReason::kGyroMaximum)
        return Fail("gyro maximum rejection reason was not retained");
    if (AddStationary(&detector, 1.02, 1.98) == Detector::Result::kConfirmed)
        return Fail("pre-spike samples counted toward continuous confirmation");
    if (AddStationary(&detector, 3.01, 0.04) != Detector::Result::kConfirmed)
        return Fail("detector did not recover after a gyro spike");

    detector.Reset();
    if (detector.AddSample(0.0, Eigen::Vector3d(0, 0, -12.0),
                           Eigen::Vector3d::Zero()) !=
        Detector::Result::kWindowRejected)
        return Fail("acceleration magnitude violation was accepted");
    if (detector.last_reject_reason() !=
        Detector::RejectReason::kAccelerationMagnitude)
        return Fail("acceleration magnitude rejection reason was not retained");

    detector.Reset();
    if (detector.AddSample(0.0, Eigen::Vector3d(0, 0, -9.81),
                           Eigen::Vector3d(0.201, 0, 0)) !=
        Detector::Result::kWindowRejected)
        return Fail("obvious-motion gyro spike above 0.20 rad/s was accepted");
    if (detector.last_reject_reason() != Detector::RejectReason::kGyroMaximum)
        return Fail("manual-motion rejection reason was not gyro maximum");

    // Acceleration variance depends strongly on the IMU, mounting, vibration,
    // bandwidth and sample rate. The portable default keeps reporting it but
    // must not turn it into a startup interlock.
    Detector field_noise_detector;
    for (int i = 0; i <= 400; ++i)
    {
        const double x = (i % 2 == 0) ? 0.8 : -0.8; // variance ~= 0.64
        field_noise_detector.AddSample(
            i * 0.005, Eigen::Vector3d(x, 0, -9.81),
            Eigen::Vector3d(0.01, -0.01, 0.03));
    }
    if (!field_noise_detector.confirmed())
        return Fail("diagnostic-only acceleration variance blocked startup");

    Detector::Config variance_config;
    // A positive installation-specific limit explicitly enables the optional
    // hard gate and must retain the old strict behavior.
    variance_config.max_acc_variance = 0.01;
    Detector variance_detector(variance_config);
    for (int i = 0; i <= 200; ++i)
    {
        const double x = (i % 2 == 0) ? 0.5 : -0.5;
        variance_detector.AddSample(i * 0.01,
                                    Eigen::Vector3d(x, 0, -9.81),
                                    Eigen::Vector3d::Zero());
    }
    if (variance_detector.confirmed())
        return Fail("acceleration variance above threshold was accepted");

    detector.Reset();
    AddStationary(&detector, 0.0, 1.0);
    if (detector.AddSample(1.2, Eigen::Vector3d(0, 0, -9.81),
                           Eigen::Vector3d::Zero()) !=
        Detector::Result::kWindowRejected)
        return Fail("IMU gap did not reset continuous confirmation");

    std::cout << "imu_static_detector_test: PASS" << std::endl;
    return 0;
}
