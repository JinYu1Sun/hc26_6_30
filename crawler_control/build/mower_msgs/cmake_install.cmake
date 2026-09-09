# Install script for directory: /home/huilian/hc26_6_30/crawler_control/src/mower_msgs

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/huilian/hc26_6_30/crawler_control/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  
      if (NOT EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}")
        file(MAKE_DIRECTORY "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}")
      endif()
      if (NOT EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/.catkin")
        file(WRITE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/.catkin" "")
      endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/huilian/hc26_6_30/crawler_control/install/_setup_util.py")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/huilian/hc26_6_30/crawler_control/install" TYPE PROGRAM FILES "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/catkin_generated/installspace/_setup_util.py")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/huilian/hc26_6_30/crawler_control/install/env.sh")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/huilian/hc26_6_30/crawler_control/install" TYPE PROGRAM FILES "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/catkin_generated/installspace/env.sh")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/huilian/hc26_6_30/crawler_control/install/setup.bash;/home/huilian/hc26_6_30/crawler_control/install/local_setup.bash")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/huilian/hc26_6_30/crawler_control/install" TYPE FILE FILES
    "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/catkin_generated/installspace/setup.bash"
    "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/catkin_generated/installspace/local_setup.bash"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/huilian/hc26_6_30/crawler_control/install/setup.sh;/home/huilian/hc26_6_30/crawler_control/install/local_setup.sh")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/huilian/hc26_6_30/crawler_control/install" TYPE FILE FILES
    "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/catkin_generated/installspace/setup.sh"
    "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/catkin_generated/installspace/local_setup.sh"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/huilian/hc26_6_30/crawler_control/install/setup.zsh;/home/huilian/hc26_6_30/crawler_control/install/local_setup.zsh")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/huilian/hc26_6_30/crawler_control/install" TYPE FILE FILES
    "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/catkin_generated/installspace/setup.zsh"
    "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/catkin_generated/installspace/local_setup.zsh"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/huilian/hc26_6_30/crawler_control/install/setup.fish;/home/huilian/hc26_6_30/crawler_control/install/local_setup.fish")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/huilian/hc26_6_30/crawler_control/install" TYPE FILE FILES
    "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/catkin_generated/installspace/setup.fish"
    "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/catkin_generated/installspace/local_setup.fish"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/huilian/hc26_6_30/crawler_control/install/.rosinstall")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/huilian/hc26_6_30/crawler_control/install" TYPE FILE FILES "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/catkin_generated/installspace/.rosinstall")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/mower_msgs/msg" TYPE FILE FILES
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/LocalPath.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/CheckResult.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/TaskStatus.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/LidarSelfDtect.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/PerceptionSelfDetect.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/MultiMapSelfDetect.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/Manual_Set.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/Manual_Driving_Cmd.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/Fault_Code.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/Direct_Control.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/VehicleCmd.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/VehicleInfo.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/VehicleStatus.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/PlaningOK.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/PlanType.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/CamerargbState.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/ControlOk.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/ControlState.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/SegState.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/GnssOK.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/VslamState.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/CameraState.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/Vslam.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/TimeAreaLeft.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/Position.msg"
    "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/msg/Monitor.msg"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/mower_msgs/srv" TYPE FILE FILES "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/srv/Intervene.srv")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/mower_msgs/cmake" TYPE FILE FILES "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/catkin_generated/installspace/mower_msgs-msg-paths.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "/home/huilian/hc26_6_30/crawler_control/devel/include/mower_msgs")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/roseus/ros" TYPE DIRECTORY FILES "/home/huilian/hc26_6_30/crawler_control/devel/share/roseus/ros/mower_msgs")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/common-lisp/ros" TYPE DIRECTORY FILES "/home/huilian/hc26_6_30/crawler_control/devel/share/common-lisp/ros/mower_msgs")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/gennodejs/ros" TYPE DIRECTORY FILES "/home/huilian/hc26_6_30/crawler_control/devel/share/gennodejs/ros/mower_msgs")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  execute_process(COMMAND "/usr/bin/python3" -m compileall "/home/huilian/hc26_6_30/crawler_control/devel/lib/python3/dist-packages/mower_msgs")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/python3/dist-packages" TYPE DIRECTORY FILES "/home/huilian/hc26_6_30/crawler_control/devel/lib/python3/dist-packages/mower_msgs")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/catkin_generated/installspace/mower_msgs.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/mower_msgs/cmake" TYPE FILE FILES "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/catkin_generated/installspace/mower_msgs-msg-extras.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/mower_msgs/cmake" TYPE FILE FILES
    "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/catkin_generated/installspace/mower_msgsConfig.cmake"
    "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/catkin_generated/installspace/mower_msgsConfig-version.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/mower_msgs" TYPE FILE FILES "/home/huilian/hc26_6_30/crawler_control/src/mower_msgs/package.xml")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/gtest/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/huilian/hc26_6_30/crawler_control/build/mower_msgs/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
