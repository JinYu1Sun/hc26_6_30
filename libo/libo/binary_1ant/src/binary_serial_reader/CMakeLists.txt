cmake_minimum_required(VERSION 3.0.2)
project(binary_serial_reader)

find_package(catkin REQUIRED COMPONENTS
        roscpp
        rospy
        std_msgs
        serial
        message_generation
        util
)
add_message_files(
        FILES
        #gpsPosition.msg
)
#generate_messages(
#        DEPENDENCIES
#        std_msgs
#)
catkin_package(
        #  INCLUDE_DIRS include
        #  LIBRARIES serial_reader
        #  CATKIN_DEPENDS roscpp rospy std_msgs
        #  DEPENDS system_lib
)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../TinyGPS)
include_directories(
        # include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${catkin_INCLUDE_DIRS}
	
)
add_executable(read_serial src/binary_pub_position.cpp
        src/gausskruger.cpp
        include/pub_position.h)
target_link_libraries(read_serial ${catkin_LIBRARIES} serial)

