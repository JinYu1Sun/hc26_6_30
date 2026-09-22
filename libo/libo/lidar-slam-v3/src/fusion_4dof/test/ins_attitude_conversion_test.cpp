#include "fusion/ins_attitude_conversion.h"

#include <cmath>
#include <iostream>
#include <limits>

int main()
{
    constexpr double kPi = 3.14159265358979323846;
    const double scale = 0.01 * kPi / 180.0;
    double roll = 0.0, pitch = 0.0;
    if (!fusion_4dof::ConvertInsRollPitch(1200.0, 500.0, 1, scale, -scale,
                                         45.0 * kPi / 180.0, true, &roll,
                                         &pitch))
        return 1;
    if (std::fabs(roll - 12.0 * kPi / 180.0) > 1e-12 ||
        std::fabs(pitch + 5.0 * kPi / 180.0) > 1e-12)
        return 2;
    if (fusion_4dof::ConvertInsRollPitch(1.0, 1.0, 0, scale, -scale,
                                        45.0 * kPi / 180.0, true, &roll,
                                        &pitch))
        return 3;
    // positionStatus is a moving-heading consistency gate in 1ant, not an
    // attitude-valid flag. Stationary DRPVA roll/pitch must remain usable
    // when this optional heading gate is disabled.
    if (!fusion_4dof::ConvertInsRollPitch(1200.0, 500.0, 0, scale, -scale,
                                         45.0 * kPi / 180.0, false, &roll,
                                         &pitch))
        return 6;
    if (fusion_4dof::ConvertInsRollPitch(
            std::numeric_limits<double>::quiet_NaN(), 0.0, 1, scale, -scale,
            45.0 * kPi / 180.0, true, &roll, &pitch))
        return 4;
    if (fusion_4dof::ConvertInsRollPitch(5000.0, 0.0, 1, scale, -scale,
                                        45.0 * kPi / 180.0, true, &roll,
                                        &pitch))
        return 5;

    std::cout << "ins_attitude_conversion_test: PASS" << std::endl;
    return 0;
}
