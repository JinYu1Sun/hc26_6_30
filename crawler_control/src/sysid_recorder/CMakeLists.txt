cmake_minimum_required(VERSION 3.0.2)
project(sysid_recorder)

add_compile_options(-std=c++14)   # nlohmann/json 需要 C++11 以上

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  mower_msgs
  minibus_msg_util
)

# nlohmann_json（apt: nlohmann-json3-dev），与 cloud_bridge 相同的查找逻辑
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
  find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp)
  if(NOT NLOHMANN_JSON_INCLUDE_DIR)
    message(FATAL_ERROR "nlohmann/json.hpp not found. Install: sudo apt install nlohmann-json3-dev")
  endif()
endif()

catkin_package(
  CATKIN_DEPENDS roscpp std_msgs mower_msgs minibus_msg_util
)

include_directories(${catkin_INCLUDE_DIRS})

add_executable(sysid_recorder_node src/sysid_recorder_node.cpp)

# 关键：保证 mower_msgs / minibus_msg_util 的消息头文件
# 在编译本节点【之前】生成，否则报找不到 .h 的错误
add_dependencies(sysid_recorder_node ${catkin_EXPORTED_TARGETS})

target_link_libraries(sysid_recorder_node ${catkin_LIBRARIES})

if(nlohmann_json_FOUND)
  target_link_libraries(sysid_recorder_node nlohmann_json::nlohmann_json)
else()
  target_include_directories(sysid_recorder_node PRIVATE ${NLOHMANN_JSON_INCLUDE_DIR})
endif()

install(TARGETS sysid_recorder_node
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(DIRECTORY launch/
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/launch
)
