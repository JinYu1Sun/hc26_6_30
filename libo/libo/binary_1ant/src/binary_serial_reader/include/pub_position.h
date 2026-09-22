#ifndef PUB_POSITION_H
#define PUB_POSITION_H

#include <ros/ros.h>
#include <std_msgs/String.h>
#include <string>
#include <iomanip>
#include <sstream>
#include <serial/serial.h>
//#include "serial_reader/gpsPosition.h"
#include "gausskruger.h"
#include <boost/bind/bind.hpp>
#include "gausskruger.h"
#include <cstdio>
class PubPosition {
public:
    PubPosition();
    ~PubPosition();
    double compute_gps(double a);
    void processSerialData(const std::string &serial_data, ros::Publisher &pub, bool imugood_flag);

private:
    double old_x;
    double old_y;

};

#endif // PUB_POSITION_H

