cmake_minimum_required(VERSION 3.0.2)
project(serial_reader)

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
        #GpsInfo.msg
)
generate_messages(
        DEPENDENCIES
        std_msgs
)
catkin_package(
        CATKIN_DEPENDS roscpp rospy std_msgs serial util
)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../TinyGPS)
include_directories(
        # include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${catkin_INCLUDE_DIRS}
	
)
add_executable(read_serial src/pub_position.cpp
        src/gausskruger.cpp
        include/pub_position.h)
add_dependencies(read_serial ${catkin_EXPORTED_TARGETS})
target_link_libraries(read_serial ${catkin_LIBRARIES} serial)

if(CATKIN_ENABLE_TESTING)
        catkin_add_gtest(test_ins_protocol test/test_ins_protocol.cpp)
endif()
