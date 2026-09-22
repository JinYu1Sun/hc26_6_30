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
    Vslam.msg
)

add_service_files(
  FILES
  Intervene.srv
)

generate_messages(
  DEPENDENCIES
  std_msgs
  nav_msgs
)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES util
  CATKIN_DEPENDS roscpp std_msgs
  DEPENDS system_lib
)

include_directories(
# include
  ${catkin_INCLUDE_DIRS}
)

