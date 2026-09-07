#include "mqtt_bridge/coordinate_converter.hpp"

#include <cmath>
#include <stdexcept>

namespace mqtt_bridge
{
namespace
{

constexpr double kPi = 3.14159265358979323846;

void validateCoordinate(double latitude, double longitude)
{
  if (!std::isfinite(latitude) || !std::isfinite(longitude) ||
    latitude < -90.0 || latitude > 90.0 ||
    longitude < -180.0 || longitude > 180.0)
  {
    throw std::invalid_argument("invalid geographic coordinate");
  }
}

bool outsideChina(double latitude, double longitude)
{
  return longitude < 72.004 || longitude > 137.8347 ||
         latitude < 0.8293 || latitude > 55.8271;
}

double transformLatitude(double x, double y)
{
  double value = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y +
    0.1 * x * y + 0.2 * std::sqrt(std::abs(x));
  value += (20.0 * std::sin(6.0 * x * kPi) + 20.0 * std::sin(2.0 * x * kPi)) *
    2.0 / 3.0;
  value += (20.0 * std::sin(y * kPi) + 40.0 * std::sin(y * kPi / 3.0)) * 2.0 / 3.0;
  value += (160.0 * std::sin(y * kPi / 12.0) + 320.0 * std::sin(y * kPi / 30.0)) *
    2.0 / 3.0;
  return value;
}

double transformLongitude(double x, double y)
{
  double value = 300.0 + x + 2.0 * y + 0.1 * x * x +
    0.1 * x * y + 0.1 * std::sqrt(std::abs(x));
  value += (20.0 * std::sin(6.0 * x * kPi) + 20.0 * std::sin(2.0 * x * kPi)) *
    2.0 / 3.0;
  value += (20.0 * std::sin(x * kPi) + 40.0 * std::sin(x * kPi / 3.0)) * 2.0 / 3.0;
  value += (150.0 * std::sin(x * kPi / 12.0) + 300.0 * std::sin(x * kPi / 30.0)) *
    2.0 / 3.0;
  return value;
}

}  // namespace

std::pair<double, double> wgs84ToGcj02(double latitude, double longitude)
{
  validateCoordinate(latitude, longitude);
  if (outsideChina(latitude, longitude)) {
    return {latitude, longitude};
  }

  constexpr double axis = 6378245.0;
  constexpr double eccentricity_squared = 0.00669342162296594323;
  double delta_latitude = transformLatitude(longitude - 105.0, latitude - 35.0);
  double delta_longitude = transformLongitude(longitude - 105.0, latitude - 35.0);
  const double latitude_radians = latitude * kPi / 180.0;
  double magic = std::sin(latitude_radians);
  magic = 1.0 - eccentricity_squared * magic * magic;
  const double sqrt_magic = std::sqrt(magic);
  delta_latitude = delta_latitude * 180.0 /
    ((axis * (1.0 - eccentricity_squared)) / (magic * sqrt_magic) * kPi);
  delta_longitude = delta_longitude * 180.0 /
    (axis / sqrt_magic * std::cos(latitude_radians) * kPi);
  return {latitude + delta_latitude, longitude + delta_longitude};
}

std::pair<double, double> gcj02ToWgs84(double latitude, double longitude)
{
  validateCoordinate(latitude, longitude);
  if (outsideChina(latitude, longitude)) {
    return {latitude, longitude};
  }

  double wgs_latitude = latitude;
  double wgs_longitude = longitude;
  for (int iteration = 0; iteration < 6; ++iteration) {
    const auto gcj = wgs84ToGcj02(wgs_latitude, wgs_longitude);
    wgs_latitude += latitude - gcj.first;
    wgs_longitude += longitude - gcj.second;
  }
  return {wgs_latitude, wgs_longitude};
}

}  // namespace mqtt_bridge
