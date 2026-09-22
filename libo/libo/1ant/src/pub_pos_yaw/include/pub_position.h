#ifndef PUB_POSITION_H
#define PUB_POSITION_H

#include <ros/ros.h>
#include <std_msgs/String.h>
#include <string>
#include <iomanip>
#include <sstream>
#include <serial/serial.h>
//#include "serial_reader/GpsInfo.h"
#include <util/GpsPosition.h>
#include "gausskruger.h"
#include <boost/bind/bind.hpp>
#include "gausskruger.h"
#include <cstdio>
#include "ins_protocol.h"
class PubPosition {
public:
    PubPosition();
    ~PubPosition();
    double compute_gps(double a);
    void processSerialData(const std::string &serial_data, ros::Publisher &pub,
                           const ins_protocol::ImuAttaData &imu,
                           double imu_age_sec,
                           const ins_protocol::GgaData &gga,
                           double gga_age_sec,
                           const ros::Time &frame_rx_ros_stamp,
                           double frame_rx_steady_sec);

private:
    double old_x;
    double old_y;

};

#endif // PUB_POSITION_H
