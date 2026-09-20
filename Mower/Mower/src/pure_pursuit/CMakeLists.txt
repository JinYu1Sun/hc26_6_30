cmake_minimum_required(VERSION 3.0.2)
project(pure_pursuit)

find_package(catkin REQUIRED COMPONENTS
  geometry_msgs
  roscpp
  std_msgs
  mower_msgs
  util
  message_generation
)

find_package(yaml-cpp CONFIG REQUIRED)  # 查找 yaml-cpp 库

## Generate services in the 'srv' folder
add_service_files(
  FILES
  TurnCompleted.srv
)

generate_messages(
  DEPENDENCIES
  std_msgs
)

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES pure_pursuit
  CATKIN_DEPENDS geometry_msgs roscpp std_msgs message_runtime
#  DEPENDS system_lib
)

include_directories(
 include
  ${catkin_INCLUDE_DIRS}
  ${YAML_CPP_INCLUDE_DIRS} # 添加 yaml-cpp 头文件目录
)

 add_executable(${PROJECT_NAME} src/main.cpp src/pure_pursuit.cpp)

install(DIRECTORY launch/
    DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/launch
)

 target_link_libraries(${PROJECT_NAME}
   ${catkin_LIBRARIES}
   ${YAML_CPP_LIBRARIES}
 )
