# CMake generated Testfile for 
# Source directory: /home/nvidia/libo/lidar-slam-v3/src/fast-lio
# Build directory: /home/nvidia/libo/lidar-slam-v3/build/fast-lio
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(gravity_leveling_test "/home/nvidia/libo/lidar-slam-v3/devel/lib/fast_lio/gravity_leveling_test")
set_tests_properties(gravity_leveling_test PROPERTIES  _BACKTRACE_TRIPLES "/home/nvidia/libo/lidar-slam-v3/src/fast-lio/CMakeLists.txt;109;add_test;/home/nvidia/libo/lidar-slam-v3/src/fast-lio/CMakeLists.txt;0;")
add_test(imu_static_detector_test "/home/nvidia/libo/lidar-slam-v3/devel/lib/fast_lio/imu_static_detector_test")
set_tests_properties(imu_static_detector_test PROPERTIES  _BACKTRACE_TRIPLES "/home/nvidia/libo/lidar-slam-v3/src/fast-lio/CMakeLists.txt;111;add_test;/home/nvidia/libo/lidar-slam-v3/src/fast-lio/CMakeLists.txt;0;")
subdirs("Thirdparty/GeographicLib")
