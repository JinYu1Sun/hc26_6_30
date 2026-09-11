cmake_minimum_required(VERSION 3.0.2)
project(cloud_bridge)

add_compile_options(-std=c++17)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  mower_msgs
)

find_package(Threads REQUIRED)

# nlohmann_json（apt: nlohmann-json3-dev）
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
  find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp)
  if(NOT NLOHMANN_JSON_INCLUDE_DIR)
    message(FATAL_ERROR "nlohmann/json.hpp not found. Install: sudo apt install nlohmann-json3-dev")
  endif()
endif()

# Paho MQTT C++（apt: libpaho-mqttpp-dev，或源码安装到 /usr/local）
find_library(PAHO_MQTT_CPP_LIB NAMES paho-mqttpp3
  PATHS /usr/local/lib /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu)

if(NOT PAHO_MQTT_CPP_LIB)
  message(FATAL_ERROR
    "Paho MQTT C++ library not found.\n"
    "Install: sudo apt install libpaho-mqtt-dev libpaho-mqttpp-dev\n"
    "or build from source: https://github.com/eclipse/paho.mqtt.cpp")
endif()
message(STATUS "Paho MQTT C++: ${PAHO_MQTT_CPP_LIB}")

# Paho MQTT C 库（C++ 库的底层依赖，必须显式链接）
find_library(PAHO_MQTT_C_LIB NAMES paho-mqtt3as paho-mqtt3a
  PATHS /usr/local/lib /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu)

if(NOT PAHO_MQTT_C_LIB)
  message(FATAL_ERROR
    "Paho MQTT C library not found.\n"
    "Install: sudo apt install libpaho-mqtt-dev")
endif()
message(STATUS "Paho MQTT C: ${PAHO_MQTT_C_LIB}")

catkin_package(
  INCLUDE_DIRS include
  CATKIN_DEPENDS roscpp std_msgs mower_msgs
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_executable(cloud_bridge_node
  src/cloud_bridge_node.cpp
  src/mqtt_client.cpp
)

add_dependencies(cloud_bridge_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

target_link_libraries(cloud_bridge_node
  ${catkin_LIBRARIES}
  ${PAHO_MQTT_CPP_LIB}
  ${PAHO_MQTT_C_LIB}
  Threads::Threads
)

if(nlohmann_json_FOUND)
  target_link_libraries(cloud_bridge_node nlohmann_json::nlohmann_json)
else()
  target_include_directories(cloud_bridge_node PRIVATE ${NLOHMANN_JSON_INCLUDE_DIR})
endif()

install(TARGETS cloud_bridge_node
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(DIRECTORY launch
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)
