#ifndef FAST_LIO_IMU_STATIC_DETECTOR_H
#define FAST_LIO_IMU_STATIC_DETECTOR_H

#include <Eigen/Core>
#include <algorithm>
#include <cmath>
#include <cstddef>

namespace fast_lio
{

// Stateful detector used only during FAST-LIO initialization.  A window is
// accepted iff every sample passes the instantaneous limits and the complete,
// uninterrupted window passes the statistical limits.  One bad sample, a
// timestamp reversal, or an excessive IMU gap restarts the confirmation time.
class ImuStaticDetector
{
public:
    struct Config
    {
        double confirmation_time_sec = 2.0;
        // The lifecycle manager already requests and latches a vehicle stop
        // throughout LIO initialization. These limits are therefore a
        // backstop against obvious motion, not a precision vibration test.
        // Keep adequate margin over normal stationary IMU noise. Acceleration
        // variance remains useful telemetry, but is not a portable static
        // criterion: mounting vibration and sensor bandwidth vary widely. A
        // zero limit disables that optional, hardware-specific gate.
        double max_gyro_mean_norm = 0.08; // rad/s
        double max_gyro_norm = 0.20;      // rad/s, any sample
        double min_acc_norm = 8.0;        // m/s^2, any sample
        double max_acc_norm = 11.5;       // m/s^2, any sample
        double max_acc_variance = 0.0;    // <=0: diagnostic only (no hard gate)
        double max_sample_gap_sec = 0.10;
        std::size_t min_samples = 50;
    };

    enum class Result
    {
        kCollecting,
        kWindowRejected,
        kConfirmed
    };

    enum class RejectReason
    {
        kNone,
        kNonFiniteSample,
        kTimestampDiscontinuity,
        kGyroMaximum,
        kAccelerationMagnitude,
        kGyroMean,
        kAccelerationVariance
    };

    ImuStaticDetector()
    {
        Reset();
    }

    explicit ImuStaticDetector(const Config &config) : config_(config)
    {
        Reset();
    }

    bool Configure(const Config &config)
    {
        if (!ValidConfig(config))
            return false;
        config_ = config;
        Reset();
        return true;
    }

    void Reset()
    {
        ResetWindow();
        last_reject_reason_ = RejectReason::kNone;
    }

    void ResetWindow()
    {
        confirmed_ = false;
        sample_count_ = 0;
        start_time_ = -1.0;
        last_time_ = -1.0;
        max_gyro_norm_seen_ = 0.0;
        mean_acc_.setZero();
        mean_gyro_.setZero();
        m2_acc_.setZero();
        m2_gyro_.setZero();
    }

    Result AddSample(double stamp, const Eigen::Vector3d &acc,
                     const Eigen::Vector3d &gyro)
    {
        if (confirmed_)
            return Result::kConfirmed;

        const double acc_norm = acc.norm();
        const double gyro_norm = gyro.norm();
        const bool finite = std::isfinite(stamp) && acc.allFinite() &&
                            gyro.allFinite() && std::isfinite(acc_norm) &&
                            std::isfinite(gyro_norm);
        const bool time_ok = last_time_ < 0.0 ||
                             (stamp > last_time_ &&
                              stamp - last_time_ <= config_.max_sample_gap_sec);
        if (!finite)
            return Reject(RejectReason::kNonFiniteSample);
        if (!time_ok)
            return Reject(RejectReason::kTimestampDiscontinuity);
        if (gyro_norm > config_.max_gyro_norm)
            return Reject(RejectReason::kGyroMaximum);
        if (acc_norm < config_.min_acc_norm ||
            acc_norm > config_.max_acc_norm)
            return Reject(RejectReason::kAccelerationMagnitude);

        if (sample_count_ == 0)
            start_time_ = stamp;
        last_time_ = stamp;
        ++sample_count_;
        max_gyro_norm_seen_ = std::max(max_gyro_norm_seen_, gyro_norm);

        // Vector Welford update. m2/count is population variance; using the
        // population form is intentional because this is the complete window.
        const Eigen::Vector3d acc_delta = acc - mean_acc_;
        mean_acc_ += acc_delta / static_cast<double>(sample_count_);
        m2_acc_ += acc_delta.cwiseProduct(acc - mean_acc_);
        const Eigen::Vector3d gyro_delta = gyro - mean_gyro_;
        mean_gyro_ += gyro_delta / static_cast<double>(sample_count_);
        m2_gyro_ += gyro_delta.cwiseProduct(gyro - mean_gyro_);

        if (elapsed_sec() + 1e-9 < config_.confirmation_time_sec ||
            sample_count_ < config_.min_samples)
            return Result::kCollecting;

        const bool mean_gyro_ok =
            mean_gyro_.norm() <= config_.max_gyro_mean_norm;
        const bool acc_variance_ok = config_.max_acc_variance <= 0.0 ||
            (acc_variance().array() <= config_.max_acc_variance).all();
        if (!mean_gyro_ok || !acc_variance_ok)
        {
            return Reject(!mean_gyro_ok
                              ? RejectReason::kGyroMean
                              : RejectReason::kAccelerationVariance);
        }

        confirmed_ = true;
        return Result::kConfirmed;
    }

    bool confirmed() const { return confirmed_; }
    std::size_t sample_count() const { return sample_count_; }
    double elapsed_sec() const
    {
        return sample_count_ > 1 ? last_time_ - start_time_ : 0.0;
    }
    double max_gyro_norm_seen() const { return max_gyro_norm_seen_; }
    double last_stamp() const { return last_time_; }
    const Eigen::Vector3d &mean_acc() const { return mean_acc_; }
    const Eigen::Vector3d &mean_gyro() const { return mean_gyro_; }
    Eigen::Vector3d acc_variance() const
    {
        if (sample_count_ == 0)
            return Eigen::Vector3d::Zero();
        return m2_acc_ / static_cast<double>(sample_count_);
    }
    Eigen::Vector3d gyro_variance() const
    {
        if (sample_count_ == 0)
            return Eigen::Vector3d::Zero();
        return m2_gyro_ / static_cast<double>(sample_count_);
    }
    const Config &config() const { return config_; }
    RejectReason last_reject_reason() const { return last_reject_reason_; }
    const char *last_reject_reason_name() const
    {
        switch (last_reject_reason_)
        {
        case RejectReason::kNonFiniteSample:
            return "non_finite_sample";
        case RejectReason::kTimestampDiscontinuity:
            return "timestamp_discontinuity";
        case RejectReason::kGyroMaximum:
            return "gyro_maximum";
        case RejectReason::kAccelerationMagnitude:
            return "acceleration_magnitude";
        case RejectReason::kGyroMean:
            return "gyro_mean";
        case RejectReason::kAccelerationVariance:
            return "acceleration_variance";
        case RejectReason::kNone:
        default:
            return "none";
        }
    }

private:
    Result Reject(RejectReason reason)
    {
        ResetWindow();
        last_reject_reason_ = reason;
        return Result::kWindowRejected;
    }

    static bool ValidConfig(const Config &c)
    {
        return std::isfinite(c.confirmation_time_sec) &&
               c.confirmation_time_sec > 0.0 &&
               std::isfinite(c.max_gyro_mean_norm) &&
               c.max_gyro_mean_norm >= 0.0 &&
               std::isfinite(c.max_gyro_norm) && c.max_gyro_norm > 0.0 &&
               c.max_gyro_mean_norm <= c.max_gyro_norm &&
               std::isfinite(c.min_acc_norm) && c.min_acc_norm > 0.0 &&
               std::isfinite(c.max_acc_norm) &&
               c.max_acc_norm > c.min_acc_norm &&
               std::isfinite(c.max_acc_variance) &&
               c.max_acc_variance >= 0.0 &&
               std::isfinite(c.max_sample_gap_sec) &&
               c.max_sample_gap_sec > 0.0 && c.min_samples >= 2;
    }

    Config config_;
    bool confirmed_ = false;
    std::size_t sample_count_ = 0;
    double start_time_ = -1.0;
    double last_time_ = -1.0;
    double max_gyro_norm_seen_ = 0.0;
    Eigen::Vector3d mean_acc_ = Eigen::Vector3d::Zero();
    Eigen::Vector3d mean_gyro_ = Eigen::Vector3d::Zero();
    Eigen::Vector3d m2_acc_ = Eigen::Vector3d::Zero();
    Eigen::Vector3d m2_gyro_ = Eigen::Vector3d::Zero();
    RejectReason last_reject_reason_ = RejectReason::kNone;
};

} // namespace fast_lio

#endif // FAST_LIO_IMU_STATIC_DETECTOR_H
