#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace fusion_4dof {

// Only fields traceable to verified receiver frames in the current 1ant
// publisher belong here. In particular gps_confidence is currently a constant,
// INS_GpsFlag_Heading is synthesized from solution_good, and VehicleAlign is
// always zero; none of those are receiver-quality evidence.
struct RtkQualityObservation {
  int gps_flag = 0;              // fresh GNGGA quality
  int ins_status = 0;            // raw IMUATTA INS status (3 == GOOD)
  int ins_position_type = 0;     // raw IMUATTA position type (56 == RTK fixed)
  int satellites = 0;            // fresh GNGGA satellites
  int differential_age_sec = 0; // fresh GNGGA differential age
  // Legacy 1ant moving-heading consistency result.  Keep it observable for
  // diagnostics, but do not use it as a 4DoF startup gate: requiring vehicle
  // motion here deadlocks a controller which must remain stopped until
  // localization is ready.
  int position_status = 0;
  double latitude_std_m = 0.0;  // raw DRPVA standard deviations
  double longitude_std_m = 0.0;
  double height_std_m = 0.0;
  double heading_std_deg = 0.0;
};

struct RtkQualityConfig {
  int required_gps_flag = 4;
  int required_ins_status = 3;
  int required_ins_position_type = 56;
  int min_satellites = 8;
  int max_differential_age_sec = 10;
  double max_horizontal_std_m = 0.30;
  double max_height_std_m = 0.50;
  double max_heading_std_deg = 5.0;
};

inline bool FinitePositiveWithin(double value, double limit) {
  // ROS numeric fields default to zero when an older publisher does not fill
  // them.  Treat zero as "missing", not as impossibly perfect receiver data.
  return std::isfinite(value) && value > 0.0 && value <= limit;
}

inline bool IsRtkPositionQualityGood(const RtkQualityObservation &sample,
                                     const RtkQualityConfig &config) {
  return sample.gps_flag == config.required_gps_flag &&
         sample.ins_status == config.required_ins_status &&
         sample.ins_position_type == config.required_ins_position_type &&
         sample.satellites >= config.min_satellites &&
         sample.differential_age_sec >= 0 &&
         sample.differential_age_sec <= config.max_differential_age_sec &&
         FinitePositiveWithin(sample.latitude_std_m,
                              config.max_horizontal_std_m) &&
         FinitePositiveWithin(sample.longitude_std_m,
                              config.max_horizontal_std_m) &&
         FinitePositiveWithin(sample.height_std_m,
                              config.max_height_std_m);
}

inline bool IsRtkHeadingQualityGood(const RtkQualityObservation &sample,
                                    const RtkQualityConfig &config) {
  // position_status is deliberately not part of this decision.  It is a
  // publisher-derived moving-heading check rather than receiver quality, and
  // therefore cannot be satisfied during a stationary cold start.  Heading
  // still fails closed on every traceable RTK/INS position-quality field and
  // on the receiver's DRPVA heading standard deviation.
  return IsRtkPositionQualityGood(sample, config) &&
         FinitePositiveWithin(sample.heading_std_deg,
                              config.max_heading_std_deg);
}

} // namespace fusion_4dof
