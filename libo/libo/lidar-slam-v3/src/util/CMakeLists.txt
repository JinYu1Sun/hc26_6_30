cmake_minimum_required(VERSION 2.8.3)
project(util)

add_compile_options(-std=c++14)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  nav_msgs
  message_generation
)

add_message_files( FILES
    GpsPosition.msg
    ImuStaticCalibration.msg
    Vslam.msg
    GPS.msg
    LIOPose.msg
    Pose6D.msg
    Position.msg
    err.msg
    LocalPose.msg
    LocalizationStatus.msg
)

add_service_files(
  FILES
  Intervene.srv
  ManageLocalizationMap.srv
)

generate_messages(
  DEPENDENCIES
  std_msgs
  nav_msgs
)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES util
  CATKIN_DEPENDS roscpp std_msgs message_runtime
  DEPENDS system_lib
)

include_directories(
# include
  ${catkin_INCLUDE_DIRS}
)
