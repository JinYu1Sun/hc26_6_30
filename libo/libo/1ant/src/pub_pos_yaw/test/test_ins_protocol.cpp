#include <gtest/gtest.h>

#include "ins_protocol.h"

namespace {

TEST(InsProtocol, ParsesRecordedDrpvaFrame) {
  const std::string frame =
      "#DRPVAA,COM1,0,36.0,FINE,2408,373920.500,0,4507553,0;"
      "SOL_COMPUTED,NARROW_INT,WGS84,0,127,0,46,0.070,4493.480,"
      "28.23549385509,112.87775033449,64.7682,-16.6124,0.0166,"
      "0.0211,0.1091,-0.0000,0.0005,-0.0020,0.0278,0.0256,0.0297,"
      "13.362958,0.244302,1.500840,1.3480,0.4736,0.3175,1,304,583,"
      "209000,209.0000,-0.1002,1.5218,1.0000,13.362958,1.000000,"
      "0.000000,0.000000*364dde91\r\n";

  ins_protocol::DrpvaData data;
  ASSERT_TRUE(ins_protocol::ParseDrpva(frame, &data));
  EXPECT_TRUE(data.valid);
  EXPECT_TRUE(data.solution_computed);
  EXPECT_TRUE(data.rtk_fixed);
  EXPECT_EQ(2408U, data.gps_week);
  EXPECT_EQ(373920500U, data.gps_millisecond);
  EXPECT_NEAR(28.23549385509, data.latitude_deg, 1e-12);
  EXPECT_NEAR(112.87775033449, data.longitude_deg, 1e-12);
  EXPECT_DOUBLE_EQ(64.7682, data.height_m);
  EXPECT_DOUBLE_EQ(0.0166, data.latitude_std_m);
  EXPECT_DOUBLE_EQ(0.0211, data.longitude_std_m);
  EXPECT_DOUBLE_EQ(0.1091, data.height_std_m);
  EXPECT_DOUBLE_EQ(0.0005, data.north_velocity_mps);
  EXPECT_DOUBLE_EQ(0.244302, data.pitch_deg);
  EXPECT_DOUBLE_EQ(1.500840, data.roll_deg);
  EXPECT_DOUBLE_EQ(13.362958, data.heading_deg);
  EXPECT_DOUBLE_EQ(1.3480, data.heading_std_deg);

  std::string ins_fixed_frame = frame;
  const std::string narrow_int = "NARROW_INT";
  const std::size_t position = ins_fixed_frame.find(narrow_int);
  ASSERT_NE(std::string::npos, position);
  ins_fixed_frame.replace(position, narrow_int.size(), "INS_RTKFIXED");
  ASSERT_TRUE(ins_protocol::ParseDrpva(ins_fixed_frame, &data));
  EXPECT_TRUE(data.rtk_fixed);
}

TEST(InsProtocol, ParsesRecordedImuAttaFrameAndScalesRawData) {
  const std::string frame =
      "#IMUATTA,51,GPS,FINE,2408,373923000,0,0,18,53604;"
      "INS_SOLUTION_GOOD,INS_RTKFIXED,4495980,1,136,22,1216,20,-104,"
      "-4013,-5,-11,-2,9,0*23c50fc6\r\n";

  ins_protocol::ImuAttaData data;
  ASSERT_TRUE(ins_protocol::ParseImuAtta(frame, &data));
  EXPECT_TRUE(data.valid);
  EXPECT_TRUE(data.solution_good);
  EXPECT_EQ(3U, data.ins_status);
  EXPECT_EQ(56U, data.position_type);
  EXPECT_EQ(4495980U, data.solution_age_millisecond);
  EXPECT_NEAR(136.0 * 360.0 / 32767.0, data.roll_deg, 1e-12);
  EXPECT_NEAR(22.0 * 360.0 / 32767.0, data.pitch_deg, 1e-12);
  EXPECT_NEAR(20.0 * 80.0 / 32767.0, data.acc_x_mps2, 1e-12);
  EXPECT_NEAR(-4013.0 * 80.0 / 32767.0, data.acc_z_mps2, 1e-12);
  EXPECT_NEAR(-11.0 * 500.0 / 32767.0, data.gyro_y_dps, 1e-12);
}

TEST(InsProtocol, VerifiesAndParsesRecordedGgaFrame) {
  const std::string frame =
      "$GNGGA,075143.00,2814.12968355,N,11252.66503046,E,4,33,0.5,"
      "64.7700,M,-16.6124,M,2.0,0*7A\r\n";
  ins_protocol::GgaData data;
  ASSERT_TRUE(ins_protocol::ParseGga(frame, &data));
  EXPECT_EQ(4, data.quality);
  EXPECT_EQ(33U, data.satellites);
  EXPECT_DOUBLE_EQ(2.0, data.differential_age_sec);

  std::string corrupt = frame;
  corrupt[10] = corrupt[10] == '0' ? '1' : '0';
  EXPECT_FALSE(ins_protocol::ParseGga(corrupt, &data));
}

TEST(InsProtocol, ConvertsAllDocumentedCardinalHeadingBoundaries) {
  double heading = -1.0;
  ASSERT_TRUE(
      ins_protocol::HeadingNorthClockwiseToEastCounterClockwise(0.0, &heading));
  EXPECT_DOUBLE_EQ(90.0, heading);
  ASSERT_TRUE(ins_protocol::HeadingNorthClockwiseToEastCounterClockwise(
      90.0, &heading));
  EXPECT_DOUBLE_EQ(0.0, heading);
  ASSERT_TRUE(ins_protocol::HeadingNorthClockwiseToEastCounterClockwise(
      360.0, &heading));
  EXPECT_DOUBLE_EQ(90.0, heading);
  EXPECT_FALSE(ins_protocol::HeadingNorthClockwiseToEastCounterClockwise(
      -0.01, &heading));
  EXPECT_FALSE(ins_protocol::HeadingNorthClockwiseToEastCounterClockwise(
      360.01, &heading));
}

TEST(InsProtocol, RejectsTruncatedFrames) {
  ins_protocol::DrpvaData drpva;
  ins_protocol::ImuAttaData imu;
  EXPECT_FALSE(ins_protocol::ParseDrpva("#DRPVAA,COM1;SOL_COMPUTED*0", &drpva));
  EXPECT_FALSE(
      ins_protocol::ParseImuAtta("#IMUATTA,1;INS_SOLUTION_GOOD*0", &imu));
}

} // namespace
