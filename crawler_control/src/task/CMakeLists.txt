cmake_minimum_required(VERSION 3.0.2)
project(task)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
  util
  mower_msgs

)

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES task
#  CATKIN_DEPENDS roscpp rospy std_msgs
#  DEPENDS system_lib
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME} src/task_node.cpp)


  
  
 target_link_libraries(${PROJECT_NAME}
   ${catkin_LIBRARIES}
 )
