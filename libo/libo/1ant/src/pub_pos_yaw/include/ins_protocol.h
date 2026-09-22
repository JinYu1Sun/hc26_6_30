#ifndef INS_PROTOCOL_H
#define INS_PROTOCOL_H

#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <string>
#include <vector>

namespace ins_protocol {

// UM98x ASCII logs used by this node.  Keeping the field mapping in a
// ROS-independent header makes it possible to test the protocol conversion
// without a receiver or a ROS master.
struct ImuAttaData {
  bool valid = false;
  bool solution_good = false;
  uint8_t ins_status = 0;
  uint8_t position_type = 0;
  // IMUATTA SOL age: total time in an INS solution, unit 0.001 s.
  uint32_t solution_age_millisecond = 0;
  double roll_deg = 0.0;
  double pitch_deg = 0.0;
  double azimuth_deg = 0.0;
  double acc_x_mps2 = 0.0;
  double acc_y_mps2 = 0.0;
  double acc_z_mps2 = 0.0;
  double gyro_x_dps = 0.0;
  double gyro_y_dps = 0.0;
  double gyro_z_dps = 0.0;
};

struct DrpvaData {
  bool valid = false;
  bool solution_computed = false;
  bool rtk_fixed = false;
  uint32_t gps_week = 0;
  uint32_t gps_millisecond = 0;
  double latitude_deg = 0.0;
  double longitude_deg = 0.0;
  double height_m = 0.0;
  double latitude_std_m = 0.0;
  double longitude_std_m = 0.0;
  double height_std_m = 0.0;
  double east_velocity_mps = 0.0;
  double north_velocity_mps = 0.0;
  double up_velocity_mps = 0.0;
  double heading_deg = 0.0; // Receiver convention: north=0, clockwise positive.
  double pitch_deg = 0.0;
  double roll_deg = 0.0;
  double heading_std_deg = 0.0;
};

struct GgaData {
  bool valid = false;
  int quality = 0;
  uint8_t satellites = 0;
  double differential_age_sec = 0.0;
};

inline std::vector<std::string> Split(const std::string &text, char delimiter) {
  std::vector<std::string> fields;
  std::size_t begin = 0;
  while (true) {
    const std::size_t end = text.find(delimiter, begin);
    fields.push_back(text.substr(begin, end - begin));
    if (end == std::string::npos) {
      break;
    }
    begin = end + 1;
  }
  return fields;
}

inline bool SplitHeaderAndBody(const std::string &line,
                               std::vector<std::string> *header,
                               std::vector<std::string> *body) {
  if (header == nullptr || body == nullptr) {
    return false;
  }
  const std::size_t semicolon = line.find(';');
  const std::size_t checksum = line.find('*', semicolon);
  if (semicolon == std::string::npos || checksum == std::string::npos ||
      checksum <= semicolon + 1) {
    return false;
  }
  *header = Split(line.substr(0, semicolon), ',');
  *body = Split(line.substr(semicolon + 1, checksum - semicolon - 1), ',');
  return true;
}

inline bool ParseDouble(const std::string &text, double *value) {
  if (value == nullptr || text.empty()) {
    return false;
  }
  char *end = nullptr;
  errno = 0;
  const double parsed = std::strtod(text.c_str(), &end);
  if (errno != 0 || end == text.c_str() || *end != '\0' ||
      !std::isfinite(parsed)) {
    return false;
  }
  *value = parsed;
  return true;
}

inline bool ParseLong(const std::string &text, long *value) {
  if (value == nullptr || text.empty()) {
    return false;
  }
  char *end = nullptr;
  errno = 0;
  const long parsed = std::strtol(text.c_str(), &end, 10);
  if (errno != 0 || end == text.c_str() || *end != '\0') {
    return false;
  }
  *value = parsed;
  return true;
}

inline uint8_t InsStatusCode(const std::string &status) {
  if (status == "INS_INACTIVE")
    return 0;
  if (status == "INS_ALIGNING")
    return 1;
  if (status == "INS_HIGH_VARIANCE")
    return 2;
  if (status == "INS_SOLUTION_GOOD")
    return 3;
  if (status == "INS_SOLUTION_FREE")
    return 6;
  if (status == "INS_ALIGNMENT_COMPLETE")
    return 7;
  return 0;
}

inline uint8_t InsPositionTypeCode(const std::string &type) {
  if (type == "INS")
    return 52;
  if (type == "INS_PSRSP")
    return 53;
  if (type == "INS_PSRDIFF")
    return 54;
  if (type == "INS_RTKFLOAT")
    return 55;
  if (type == "INS_RTKFIXED")
    return 56;
  return 0;
}

inline bool ParseImuAtta(const std::string &line, ImuAttaData *output) {
  if (output == nullptr || line.find("#IMUATTA") != 0) {
    return false;
  }
  std::vector<std::string> header;
  std::vector<std::string> body;
  // PDF table 2-6: 10 ASCII header fields and 15 data fields (including the
  // two reserved fields). Requiring the complete layout prevents a valid CRC
  // over a truncated frame from being accepted.
  if (!SplitHeaderAndBody(line, &header, &body) || header.size() < 10 ||
      body.size() < 15) {
    return false;
  }

  long solution_age_millisecond = 0;
  long roll_raw = 0;
  long pitch_raw = 0;
  long azimuth_raw = 0;
  long acc_x_raw = 0;
  long acc_y_raw = 0;
  long acc_z_raw = 0;
  long gyro_x_raw = 0;
  long gyro_y_raw = 0;
  long gyro_z_raw = 0;
  if (!ParseLong(body[2], &solution_age_millisecond) ||
      solution_age_millisecond < 0 ||
      solution_age_millisecond >
          static_cast<long>(std::numeric_limits<uint32_t>::max()) ||
      !ParseLong(body[4], &roll_raw) || !ParseLong(body[5], &pitch_raw) ||
      !ParseLong(body[6], &azimuth_raw) || !ParseLong(body[7], &acc_x_raw) ||
      !ParseLong(body[8], &acc_y_raw) || !ParseLong(body[9], &acc_z_raw) ||
      !ParseLong(body[10], &gyro_x_raw) || !ParseLong(body[11], &gyro_y_raw) ||
      !ParseLong(body[12], &gyro_z_raw)) {
    return false;
  }

  const long raw_values[] = {roll_raw,   pitch_raw,  azimuth_raw,
                             acc_x_raw,  acc_y_raw,  acc_z_raw,
                             gyro_x_raw, gyro_y_raw, gyro_z_raw};
  for (const long raw : raw_values) {
    if (raw < -32768 || raw > 32767) {
      return false;
    }
  }

  constexpr double kAttitudeScale = 360.0 / 32767.0;
  constexpr double kAccelerationScale = 80.0 / 32767.0;
  constexpr double kGyroScale = 500.0 / 32767.0;
  ImuAttaData parsed;
  parsed.valid = true;
  parsed.solution_good = body[0] == "INS_SOLUTION_GOOD";
  parsed.ins_status = InsStatusCode(body[0]);
  parsed.position_type = InsPositionTypeCode(body[1]);
  parsed.solution_age_millisecond =
      static_cast<uint32_t>(solution_age_millisecond);
  parsed.roll_deg = roll_raw * kAttitudeScale;
  parsed.pitch_deg = pitch_raw * kAttitudeScale;
  parsed.azimuth_deg = azimuth_raw * kAttitudeScale;
  parsed.acc_x_mps2 = acc_x_raw * kAccelerationScale;
  parsed.acc_y_mps2 = acc_y_raw * kAccelerationScale;
  parsed.acc_z_mps2 = acc_z_raw * kAccelerationScale;
  parsed.gyro_x_dps = gyro_x_raw * kGyroScale;
  parsed.gyro_y_dps = gyro_y_raw * kGyroScale;
  parsed.gyro_z_dps = gyro_z_raw * kGyroScale;
  *output = parsed;
  return true;
}

inline bool ParseDrpva(const std::string &line, DrpvaData *output) {
  if (output == nullptr || line.find("#DRPVA") != 0) {
    return false;
  }
  std::vector<std::string> header;
  std::vector<std::string> body;
  // PDF table 2-16: 10 ASCII header fields and 40 data fields.
  if (!SplitHeaderAndBody(line, &header, &body) || header.size() < 10 ||
      body.size() < 40) {
    return false;
  }

  long gps_week = 0;
  double seconds_of_week = 0.0;
  DrpvaData parsed;
  if (!ParseLong(header[5], &gps_week) || gps_week < 0 ||
      gps_week > static_cast<long>(std::numeric_limits<uint32_t>::max()) ||
      !ParseDouble(header[6], &seconds_of_week) || seconds_of_week < 0.0 ||
      seconds_of_week >= 604800.0 ||
      !ParseDouble(body[9], &parsed.latitude_deg) ||
      !ParseDouble(body[10], &parsed.longitude_deg) ||
      !ParseDouble(body[11], &parsed.height_m) ||
      !ParseDouble(body[13], &parsed.latitude_std_m) ||
      !ParseDouble(body[14], &parsed.longitude_std_m) ||
      !ParseDouble(body[15], &parsed.height_std_m) ||
      !ParseDouble(body[16], &parsed.east_velocity_mps) ||
      !ParseDouble(body[17], &parsed.north_velocity_mps) ||
      !ParseDouble(body[18], &parsed.up_velocity_mps) ||
      !ParseDouble(body[22], &parsed.heading_deg) ||
      !ParseDouble(body[23], &parsed.pitch_deg) ||
      !ParseDouble(body[24], &parsed.roll_deg) ||
      !ParseDouble(body[25], &parsed.heading_std_deg)) {
    return false;
  }

  const long long milliseconds = std::llround(seconds_of_week * 1000.0);
  if (milliseconds < 0 || milliseconds > 604800000LL) {
    return false;
  }
  parsed.valid = true;
  parsed.solution_computed = body[0] == "SOL_COMPUTED";
  // Current firmware reports NARROW_INT; table 2-16 refers to the INS
  // position-type table where the equivalent combined solution is
  // INS_RTKFIXED. Accept both spellings, while GNGGA quality=4 remains the
  // independent RTK-fixed gate in the publisher.
  parsed.rtk_fixed = body[1] == "NARROW_INT" || body[1] == "INS_RTKFIXED";
  parsed.gps_week = static_cast<uint32_t>(gps_week);
  parsed.gps_millisecond = static_cast<uint32_t>(milliseconds);
  *output = parsed;
  return true;
}

// UM981 DRPVA heading is north=0 and clockwise positive, with the documented
// inclusive range [0, 360]. GpsPosition expects east=0 and CCW positive.
inline bool
HeadingNorthClockwiseToEastCounterClockwise(double heading_deg,
                                            double *enu_heading_deg) {
  if (enu_heading_deg == nullptr || !std::isfinite(heading_deg) ||
      heading_deg < 0.0 || heading_deg > 360.0) {
    return false;
  }
  double converted = std::fmod(90.0 - heading_deg, 360.0);
  if (converted < 0.0) {
    converted += 360.0;
  }
  *enu_heading_deg = converted;
  return true;
}

inline int HexDigit(char value) {
  if (value >= '0' && value <= '9')
    return value - '0';
  if (value >= 'a' && value <= 'f')
    return value - 'a' + 10;
  if (value >= 'A' && value <= 'F')
    return value - 'A' + 10;
  return -1;
}

inline bool VerifyNmeaChecksum(const std::string &line) {
  if (line.empty() || line[0] != '$') {
    return false;
  }
  const std::size_t checksum = line.find('*');
  if (checksum == std::string::npos || checksum + 2 >= line.size()) {
    return false;
  }
  uint8_t calculated = 0;
  for (std::size_t i = 1; i < checksum; ++i) {
    calculated ^= static_cast<uint8_t>(line[i]);
  }
  const int high = HexDigit(line[checksum + 1]);
  const int low = HexDigit(line[checksum + 2]);
  return high >= 0 && low >= 0 &&
         calculated == static_cast<uint8_t>((high << 4) | low);
}

inline bool ParseGga(const std::string &line, GgaData *output) {
  if (output == nullptr || line.find("$GNGGA") != 0 ||
      !VerifyNmeaChecksum(line)) {
    return false;
  }
  const std::size_t checksum = line.find('*');
  const std::string payload = line.substr(0, checksum);
  const std::vector<std::string> fields = Split(payload, ',');
  if (fields.size() < 15) {
    return false;
  }
  long quality = 0;
  long satellites = 0;
  double differential_age = 0.0;
  if (!ParseLong(fields[6], &quality) || quality < 0 || quality > 9 ||
      !ParseLong(fields[7], &satellites) || satellites < 0 ||
      satellites > 255) {
    return false;
  }
  if (!fields[13].empty() && !ParseDouble(fields[13], &differential_age)) {
    return false;
  }

  GgaData parsed;
  parsed.valid = true;
  parsed.quality = static_cast<int>(quality);
  parsed.satellites = static_cast<uint8_t>(satellites);
  parsed.differential_age_sec = differential_age;
  *output = parsed;
  return true;
}

} // namespace ins_protocol

#endif // INS_PROTOCOL_H
