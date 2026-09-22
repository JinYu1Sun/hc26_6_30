cmake_minimum_required(VERSION 2.8.3)
project(gps_imu)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -g -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")
set(DCMAKE_BUILD_TYPE "Release")

find_package(catkin REQUIRED COMPONENTS
        pcl_conversions
        pcl_ros
        roscpp
        rospy
        std_msgs
        genmsg
        util
        )

include_directories(
        ${catkin_INCLUDE_DIRS}
        ${PCL_INCLUDE_DIRS}
        )

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES imu_gps
#  CATKIN_DEPENDS other_catkin_pkg
#  DEPENDS system_lib
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
# include
# ${catkin_INCLUDE_DIRS}
)

add_executable(mkt
       
               src/AHRS.h
               src/GNSS_IMU.cpp
               src/gauss-kruger-cpp/gausskruger.cpp
)

target_link_libraries(mkt ${catkin_LIBRARIES})
