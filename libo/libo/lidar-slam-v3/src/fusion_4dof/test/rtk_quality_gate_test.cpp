#include "fusion/rtk_quality_gate.h"

#include <cassert>
#include <limits>

int main() {
  fusion_4dof::RtkQualityConfig config;
  fusion_4dof::RtkQualityObservation sample;
  sample.gps_flag = 4;
  sample.ins_status = 3;
  sample.ins_position_type = 56;
  sample.satellites = 18;
  sample.differential_age_sec = 1;
  sample.position_status = 1;
  sample.latitude_std_m = 0.05;
  sample.longitude_std_m = 0.06;
  sample.height_std_m = 0.10;
  sample.heading_std_deg = 1.5;
  assert(fusion_4dof::IsRtkPositionQualityGood(sample, config));
  assert(fusion_4dof::IsRtkHeadingQualityGood(sample, config));

  sample.differential_age_sec = 10;
  assert(fusion_4dof::IsRtkPositionQualityGood(sample, config));
  sample.differential_age_sec = 11;
  assert(!fusion_4dof::IsRtkPositionQualityGood(sample, config));
  sample.differential_age_sec = 1;

  sample.ins_position_type = 55; // raw INS_RTKFLOAT
  assert(!fusion_4dof::IsRtkPositionQualityGood(sample, config));
  sample.ins_position_type = 56;
  sample.position_status = 0;
  assert(fusion_4dof::IsRtkPositionQualityGood(sample, config));
  // The 1ant moving-heading confirmation is diagnostic only.  A stationary
  // cold start must not be blocked when all receiver quality fields pass.
  assert(fusion_4dof::IsRtkHeadingQualityGood(sample, config));
  sample.position_status = 1;
  sample.latitude_std_m = 0.0; // old/unpopulated ROS field must fail closed
  assert(!fusion_4dof::IsRtkPositionQualityGood(sample, config));
  sample.latitude_std_m = 0.05;
  sample.heading_std_deg = 0.0;
  assert(!fusion_4dof::IsRtkHeadingQualityGood(sample, config));
  sample.heading_std_deg = 1.5;
  sample.latitude_std_m = std::numeric_limits<double>::quiet_NaN();
  assert(!fusion_4dof::IsRtkPositionQualityGood(sample, config));
  return 0;
}
