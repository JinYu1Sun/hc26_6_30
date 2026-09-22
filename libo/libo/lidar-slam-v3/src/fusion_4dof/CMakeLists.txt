cmake_minimum_required(VERSION 3.0.2)
project(fusion_4dof)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-O2)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  nav_msgs
  geometry_msgs
  sensor_msgs
  tf
  util
)
find_package(Eigen3 REQUIRED)

# 复用 fast-lio 内置的 GeographicLib（与在用版完全一致，避免坐标换算差异）。
# 同一工作空间内用相对路径引用，无需另外安装。
set(GEOLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../fast-lio/Thirdparty/GeographicLib)

catkin_package(
  CATKIN_DEPENDS roscpp std_msgs nav_msgs geometry_msgs sensor_msgs tf util
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIR}
  ${GEOLIB_DIR}/include
)

# 重力约束4DoF在线对齐融合定位节点：
#   - global_fusion.*：水平滑窗Umeyama + 稳健高程偏移（取代ceres位姿图）；
#   - fusion_4dof.cc  ：GPS 主发布 + LIO(经4DoF) 回退，状态机(N_RTK/N_LIO/RESET/
#                      RECOVER/FAULT)，纯 ENU 位置帧。
add_executable(fusion_4dof_node
  src/fusion_4dof.cc
  src/global_fusion.cc
  src/map_bundle.cc
)
add_dependencies(fusion_4dof_node ${catkin_EXPORTED_TARGETS})
target_link_libraries(fusion_4dof_node
  ${catkin_LIBRARIES}
  ${GEOLIB_DIR}/lib/libGeographic.so
  yaml-cpp
)

# This is a Bash program, not Python.  catkin_install_python rewrites shebangs
# and would make the installed helper run under Python.  install(PROGRAMS)
# preserves `#!/usr/bin/env bash` while setting executable permissions.
install(PROGRAMS
  scripts/restart_ros_nodes.sh
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

if(CATKIN_ENABLE_TESTING)
  # 不依赖 ROS master 的确定性回归：验证 RTK 恢复候选期冻结生产变换、门限帧
  # 才提交，以及多数异常点不会改写已提交 theta/tx/ty。
  add_executable(global_fusion_recovery_test
    test/global_fusion_recovery_test.cpp
    src/global_fusion.cc
  )
  add_test(NAME global_fusion_recovery_test COMMAND global_fusion_recovery_test)
  add_executable(global_fusion_time_alignment_test
    test/global_fusion_time_alignment_test.cpp
    src/global_fusion.cc
  )
  add_test(NAME global_fusion_time_alignment_test
    COMMAND global_fusion_time_alignment_test)
  add_executable(lio_motion_gate_test test/lio_motion_gate_test.cpp)
  add_test(NAME lio_motion_gate_test COMMAND lio_motion_gate_test)
  add_executable(global_fusion_4d_test
    test/global_fusion_4d_test.cpp
    src/global_fusion.cc
  )
  add_test(NAME global_fusion_4d_test COMMAND global_fusion_4d_test)
  add_executable(ins_attitude_conversion_test
    test/ins_attitude_conversion_test.cpp
  )
  add_test(NAME ins_attitude_conversion_test COMMAND ins_attitude_conversion_test)
  add_executable(alignment_state_test
    test/alignment_state_test.cpp
  )
  add_test(NAME alignment_state_test COMMAND alignment_state_test)
  add_executable(gps_lio_sync_health_test
    test/gps_lio_sync_health_test.cpp
  )
  add_test(NAME gps_lio_sync_health_test COMMAND gps_lio_sync_health_test)
  add_executable(rtk_quality_gate_test
    test/rtk_quality_gate_test.cpp
  )
  add_test(NAME rtk_quality_gate_test COMMAND rtk_quality_gate_test)
  add_executable(pose_continuity_gate_test
    test/pose_continuity_gate_test.cpp
  )
  add_test(NAME pose_continuity_gate_test COMMAND pose_continuity_gate_test)
  add_executable(inertial_dead_reckoning_test
    test/inertial_dead_reckoning_test.cpp
  )
  add_test(NAME inertial_dead_reckoning_test COMMAND inertial_dead_reckoning_test)
  add_executable(fallback_policy_test
    test/fallback_policy_test.cpp
  )
  add_test(NAME fallback_policy_test COMMAND fallback_policy_test)
  add_executable(gps_outage_policy_test
    test/gps_outage_policy_test.cpp
  )
  add_test(NAME gps_outage_policy_test COMMAND gps_outage_policy_test)
  add_executable(gps_motion_gate_test
    test/gps_motion_gate_test.cpp
  )
  add_test(NAME gps_motion_gate_test COMMAND gps_motion_gate_test)
  add_executable(gps_course_axis_gate_test
    test/gps_course_axis_gate_test.cpp
  )
  add_test(NAME gps_course_axis_gate_test COMMAND gps_course_axis_gate_test)
  add_executable(lio_time_alignment_test
    test/lio_time_alignment_test.cpp
  )
  add_test(NAME lio_time_alignment_test COMMAND lio_time_alignment_test)
  add_executable(map_bundle_test
    test/map_bundle_test.cpp
    src/map_bundle.cc
  )
  target_link_libraries(map_bundle_test yaml-cpp)
  add_test(NAME map_bundle_test COMMAND map_bundle_test)
  add_executable(map_switch_state_test
    test/map_switch_state_test.cpp
    src/global_fusion.cc
  )
  add_test(NAME map_switch_state_test COMMAND map_switch_state_test)
  add_test(NAME restart_ros_nodes_test
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/test/restart_ros_nodes_test.sh)
endif()
