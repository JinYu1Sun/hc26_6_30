#ifndef GLOBAL_FUSION_H
#define GLOBAL_FUSION_H

#include <iostream>
#include <thread>
#include <map>
#include <vector>
#include <mutex>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>
#include <ceres/cost_function.h>
#include <ceres/ceres.h>
#include <ceres/cost_function.h>
#include <nav_msgs/Odometry.h>
#include <nav_msgs/Path.h>
#include "LocalCartesian.hpp"
#include "Factors.h"

// using namespace std;
// using namespace ceres;

class GlobalOptimization
{
public:
    GlobalOptimization();
    ~GlobalOptimization();

    void InputGPS(double t, double latitude, double longitude, double altitude,double gauss_yaw, double posAccuracy);
    void InputOdom(double t, Eigen::Vector3d OdomP, Eigen::Quaterniond OdomQ);
    void GetGlobalOdom(Eigen::Vector3d &odomP, Eigen::Quaterniond &odomQ);

    // TODO: debug.chen
    bool location_mode;
    double origin_latitude, origin_longitude, origin_altitude,origin_gauss_yaw;
    bool clear_data;
private:
    void GPS2XYZ(double latitude, double longitude, double altitude,double gauss_yaw, double *xyz);
    void optimize();

    bool mbInitGPS;
    bool mbGPSCtrl;
    bool mbThreadCtrl;

    bool mbLIOCtrl;

    std::map<double, std::vector<double>> localPoseMap;
    std::map<double, std::vector<double>> globalPoseMap;
    std::map<double, std::vector<double>> GPSPositionMap;

    GeographicLib::LocalCartesian geoConverter;
    Eigen::Matrix4d WGPS_T_WVIO;
    Eigen::Vector3d lastP;
    Eigen::Quaterniond lastQ;

    std::mutex mtxOdm;
    std::thread optThread;
};

#endif // GLOBAL_FUSION_H
