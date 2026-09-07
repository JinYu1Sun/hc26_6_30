cmake_minimum_required(VERSION 3.8)
project(mqtt_bridge)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 设置C++17标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(angmen_ad_msgs REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)

# 查找 yaml-cpp
find_package(yaml-cpp REQUIRED)

# 查找 nlohmann_json
find_package(nlohmann_json 3.2.0 REQUIRED)

# 设置 CMake 模块路径
list(APPEND CMAKE_PREFIX_PATH /usr/local/lib/cmake)

# 查找 Paho MQTT C
find_package(eclipse-paho-mqtt-c QUIET)
if(eclipse-paho-mqtt-c_FOUND)
  message(STATUS "Found Paho MQTT C via CMake config")
endif()

# 无论是否找到 CMake 配置，都直接查找库文件
find_library(PAHO_MQTT_C_LIB NAMES paho-mqtt3as libpaho-mqtt3as
  PATHS /usr/local/lib /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu
  PATH_SUFFIXES "" "paho-mqtt-c")

if(PAHO_MQTT_C_LIB)
  set(PAHO_MQTT_C_LIBRARIES ${PAHO_MQTT_C_LIB})
  set(PAHO_MQTT_C_INCLUDE_DIRS /usr/local/include /usr/include)
  message(STATUS "Found Paho MQTT C library: ${PAHO_MQTT_C_LIB}")
else()
  message(FATAL_ERROR 
    "Paho MQTT C library not found.\n"
    "Searched in: /usr/local/lib, /usr/lib\n"
    "Please verify installation with: ls -la /usr/local/lib/libpaho-mqtt3*")
endif()

# 查找 Paho MQTT C++
find_library(PAHO_MQTT_CPP_LIB NAMES paho-mqttpp3
  PATHS /usr/local/lib /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu)
if(PAHO_MQTT_CPP_LIB)
  set(PAHO_MQTT_CPP_LIBRARIES ${PAHO_MQTT_CPP_LIB})
  set(PAHO_MQTT_CPP_INCLUDE_DIRS /usr/local/include)
  message(STATUS "Found Paho MQTT C++ library: ${PAHO_MQTT_CPP_LIB}")
else()
  message(FATAL_ERROR "Paho MQTT C++ library not found. Please install from source.")
endif()

message(STATUS "Paho MQTT C libraries: ${PAHO_MQTT_C_LIBRARIES}")
message(STATUS "Paho MQTT C++ libraries: ${PAHO_MQTT_CPP_LIBRARIES}")

# 创建库
add_library(${PROJECT_NAME}_lib SHARED
  src/mqtt_client.cpp
  src/config_manager.cpp
  src/coordinate_converter.cpp
  src/sm2_crypto.cpp
)

target_include_directories(${PROJECT_NAME}_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${PAHO_MQTT_C_INCLUDE_DIRS}
  ${PAHO_MQTT_CPP_INCLUDE_DIRS}
)

ament_target_dependencies(${PROJECT_NAME}_lib
  rclcpp
  std_msgs
  angmen_ad_msgs
)

target_link_libraries(${PROJECT_NAME}_lib
  ${PAHO_MQTT_C_LIBRARIES}
  ${PAHO_MQTT_CPP_LIBRARIES}
  nlohmann_json::nlohmann_json
  yaml-cpp
  OpenSSL::Crypto
)

# 创建可执行文件
# 创建可执行文件
add_executable(mqtt_bridge_node
  src/mqtt_bridge_node.cpp
)

ament_target_dependencies(mqtt_bridge_node
  rclcpp
  std_msgs
  angmen_ad_msgs
)

target_include_directories(mqtt_bridge_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(mqtt_bridge_node
  ${PROJECT_NAME}_lib
)

# 安装
install(TARGETS mqtt_bridge_node
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}_lib
  EXPORT ${PROJECT_NAME}_targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include
)

install(DIRECTORY config/
  DESTINATION share/${PROJECT_NAME}/config
)

install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}/launch
)

install(PROGRAMS
  scripts/mock_bridge_services.py
  scripts/publish_car_events.py
  scripts/publish_takeover_request.py
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(sm2_crypto_test test/sm2_crypto_test.cpp)
  target_link_libraries(sm2_crypto_test
    ${PROJECT_NAME}_lib
    yaml-cpp
  )
  target_compile_definitions(sm2_crypto_test PRIVATE
    SM2_TEST_KEY_FILE="${CMAKE_CURRENT_SOURCE_DIR}/config/sm2_test_keys.yaml"
  )

  ament_add_gtest(coordinate_converter_test test/coordinate_converter_test.cpp)
  target_link_libraries(coordinate_converter_test ${PROJECT_NAME}_lib)

  add_executable(sm2_compat_tool test/sm2_compat_tool.cpp)
  target_link_libraries(sm2_compat_tool
    ${PROJECT_NAME}_lib
    yaml-cpp
  )
  target_compile_definitions(sm2_compat_tool PRIVATE
    SM2_TEST_KEY_FILE="${CMAKE_CURRENT_SOURCE_DIR}/config/sm2_test_keys.yaml"
  )
endif()

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME}_lib)
ament_export_dependencies(rclcpp std_msgs angmen_ad_msgs nlohmann_json OpenSSL)

ament_package()
