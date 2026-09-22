#ifndef FUSION_4DOF_INS_ATTITUDE_CONVERSION_H
#define FUSION_4DOF_INS_ATTITUDE_CONVERSION_H

#include <cmath>

namespace fusion_4dof
{

inline bool ConvertInsRollPitch(double raw_roll, double raw_pitch,
                                int position_status,
                                double roll_scale_rad,
                                double pitch_scale_rad,
                                double max_abs_rad,
                                bool require_position_status,
                                double *roll, double *pitch)
{
    if (roll == nullptr || pitch == nullptr ||
        (require_position_status && position_status == 0) ||
        !std::isfinite(raw_roll) || !std::isfinite(raw_pitch) ||
        !std::isfinite(roll_scale_rad) ||
        !std::isfinite(pitch_scale_rad) || !std::isfinite(max_abs_rad) ||
        max_abs_rad <= 0.0)
        return false;

    const double converted_roll = raw_roll * roll_scale_rad;
    const double converted_pitch = raw_pitch * pitch_scale_rad;
    if (!std::isfinite(converted_roll) || !std::isfinite(converted_pitch) ||
        std::fabs(converted_roll) > max_abs_rad ||
        std::fabs(converted_pitch) > max_abs_rad)
        return false;

    *roll = converted_roll;
    *pitch = converted_pitch;
    return true;
}

} // namespace fusion_4dof

#endif // FUSION_4DOF_INS_ATTITUDE_CONVERSION_H
