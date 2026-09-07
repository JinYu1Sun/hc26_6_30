#include "mqtt_bridge/coordinate_converter.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>

TEST(CoordinateConverter, RoundTripWithinOneCentimeter)
{
  constexpr double latitude = 28.23622640;
  constexpr double longitude = 112.87798410;
  const auto gcj = mqtt_bridge::wgs84ToGcj02(latitude, longitude);
  const auto wgs = mqtt_bridge::gcj02ToWgs84(gcj.first, gcj.second);

  EXPECT_NEAR(wgs.first, latitude, 1e-7);
  EXPECT_NEAR(wgs.second, longitude, 1e-7);
  EXPECT_GT(std::abs(gcj.first - latitude) + std::abs(gcj.second - longitude), 1e-4);
}

TEST(CoordinateConverter, OutsideChinaUnchanged)
{
  const auto gcj = mqtt_bridge::wgs84ToGcj02(48.8566, 2.3522);
  EXPECT_DOUBLE_EQ(gcj.first, 48.8566);
  EXPECT_DOUBLE_EQ(gcj.second, 2.3522);
}

TEST(CoordinateConverter, RejectsInvalidCoordinate)
{
  EXPECT_THROW(mqtt_bridge::wgs84ToGcj02(91.0, 112.0), std::invalid_argument);
  EXPECT_THROW(mqtt_bridge::gcj02ToWgs84(28.0, 181.0), std::invalid_argument);
}
