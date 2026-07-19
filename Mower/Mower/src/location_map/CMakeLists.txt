cmake_minimum_required(VERSION 3.10.2)
project(location_map)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  sensor_msgs
  std_msgs
  tf
  tf2_msgs
  util
  rosbag
  roslib 
)
find_package(yaml-cpp CONFIG REQUIRED)

generate_messages(
   DEPENDENCIES
   std_msgs
)

catkin_package(
#   message_runtime
#  INCLUDE_DIRS include
#  LIBRARIES load_map
 # CATKIN_DEPENDS carla_msgs roscpp rospy sensor_msgs std_msgs tf tf2_msgs map_info
#  DEPENDS system_lib
)


include_directories(
  include
  #../include
  ${catkin_INCLUDE_DIRS}
  ${YAML_CPP_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME}_node
  src/main.cpp
  src/location_map.cpp
)

target_link_libraries(${PROJECT_NAME}_node
  ${catkin_LIBRARIES}
  ${YAML_CPP_LIBRARIES}
)
add_dependencies(${PROJECT_NAME}_node ${catkin_EXPORTED_TARGETS})
