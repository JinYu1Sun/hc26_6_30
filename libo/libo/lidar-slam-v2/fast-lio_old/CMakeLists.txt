cmake_minimum_required(VERSION 2.8.3)
project(fast_lio)

SET(CMAKE_BUILD_TYPE "RELEASE")

ADD_COMPILE_OPTIONS(-std=c++14 )
set( CMAKE_CXX_FLAGS "-std=c++14 -O3" )

add_definitions(-DROOT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/\")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fexceptions" )
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread -std=c++0x -std=c++14 -fexceptions")

message("Current CPU archtecture: ${CMAKE_SYSTEM_PROCESSOR}")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)" )
  include(ProcessorCount)
  ProcessorCount(N)
  message("Processer number:  ${N}")
  if(N GREATER 4)
    add_definitions(-DMP_EN)
    add_definitions(-DMP_PROC_NUM=3)
    message("core for MP: 3")
  elseif(N GREATER 3)
    add_definitions(-DMP_EN)
    add_definitions(-DMP_PROC_NUM=2)
    message("core for MP: 2")
  else()
    add_definitions(-DMP_PROC_NUM=1)
  endif()
else()
  add_definitions(-DMP_PROC_NUM=1)
endif()

find_package(OpenMP QUIET)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   ${OpenMP_C_FLAGS}")

find_package(PythonLibs REQUIRED)
find_path(MATPLOTLIB_CPP_INCLUDE_DIRS "matplotlibcpp.h")

find_package(catkin REQUIRED COMPONENTS
  geometry_msgs
  nav_msgs
  sensor_msgs
  roscpp
  rospy
  std_msgs
  pcl_ros
  tf
  livox_ros_driver2
  message_generation
  eigen_conversions
  util
)

find_package(Eigen3 REQUIRED)
find_package(PCL REQUIRED)
find_package(Ceres REQUIRED)

message(Eigen: ${EIGEN3_INCLUDE_DIR})

include_directories(
  include
	${catkin_INCLUDE_DIRS} 
  ${EIGEN3_INCLUDE_DIR}
  ${PCL_INCLUDE_DIRS}
  ${CERES_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/Thirdparty/GeographicLib/include
)

add_subdirectory(${PROJECT_SOURCE_DIR}/Thirdparty/GeographicLib)

#add_message_files(
	#FILES
  #util/Pose6D.msg
  #util/LIOPose.msg
  #util/GPS.msg
  # util/Position.msg
  #)

generate_messages(
 DEPENDENCIES
 geometry_msgs
 util
)

catkin_package(
  CATKIN_DEPENDS geometry_msgs nav_msgs roscpp rospy std_msgs message_runtime
  DEPENDS EIGEN3 PCL
  INCLUDE_DIRS
  LIBRARIES libGeographic # Add new.
)

add_executable(fastlio_mapping src/laserMapping.cpp include/ikd-Tree/ikd_Tree.cpp src/preprocess.cpp)
target_link_libraries(fastlio_mapping ${catkin_LIBRARIES} ${PCL_LIBRARIES} ${PYTHON_LIBRARIES})
target_include_directories(fastlio_mapping PRIVATE ${PYTHON_INCLUDE_DIRS})

add_executable(fast_fusion src/fusion.cc src/global_fusion.cc)
target_link_libraries(fast_fusion ${catkin_LIBRARIES} ${CERES_LIBRARIES} ${PROJECT_SOURCE_DIR}/Thirdparty/GeographicLib/lib/libGeographic.so yaml-cpp)

#set(livox_ros_driver2_DIR "/home/nvidia/mid360/ws_livox/build/livox_ros_driver2/catkin_generated/installspace")

set(CMAKE_PREFIX_PATH "/home/nvidia/mid360/ws_livox/build/livox_ros_driver2/catkin_generated/installspace/:${CMAKE_PREFIX_PATH}")

