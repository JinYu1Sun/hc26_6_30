# generated from genmsg/cmake/pkg-genmsg.cmake.em

message(STATUS "read_waypoints: 1 messages, 1 services")

set(MSG_I_FLAGS "-Iread_waypoints:/home/nvidia/Mower/src/planning/read_waypoints/msg;-Istd_msgs:/opt/ros/noetic/share/std_msgs/cmake/../msg")

# Find all generators
find_package(gencpp REQUIRED)
find_package(geneus REQUIRED)
find_package(genlisp REQUIRED)
find_package(gennodejs REQUIRED)
find_package(genpy REQUIRED)

add_custom_target(read_waypoints_generate_messages ALL)

# verify that message/service dependencies have not changed since configure



get_filename_component(_filename "/home/nvidia/Mower/src/planning/read_waypoints/msg/LocalPath.msg" NAME_WE)
add_custom_target(_read_waypoints_generate_messages_check_deps_${_filename}
  COMMAND ${CATKIN_ENV} ${PYTHON_EXECUTABLE} ${GENMSG_CHECK_DEPS_SCRIPT} "read_waypoints" "/home/nvidia/Mower/src/planning/read_waypoints/msg/LocalPath.msg" "std_msgs/Header"
)

get_filename_component(_filename "/home/nvidia/Mower/src/planning/read_waypoints/srv/TurnCompleted.srv" NAME_WE)
add_custom_target(_read_waypoints_generate_messages_check_deps_${_filename}
  COMMAND ${CATKIN_ENV} ${PYTHON_EXECUTABLE} ${GENMSG_CHECK_DEPS_SCRIPT} "read_waypoints" "/home/nvidia/Mower/src/planning/read_waypoints/srv/TurnCompleted.srv" ""
)

#
#  langs = gencpp;geneus;genlisp;gennodejs;genpy
#

### Section generating for lang: gencpp
### Generating Messages
_generate_msg_cpp(read_waypoints
  "/home/nvidia/Mower/src/planning/read_waypoints/msg/LocalPath.msg"
  "${MSG_I_FLAGS}"
  "/opt/ros/noetic/share/std_msgs/cmake/../msg/Header.msg"
  ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/read_waypoints
)

### Generating Services
_generate_srv_cpp(read_waypoints
  "/home/nvidia/Mower/src/planning/read_waypoints/srv/TurnCompleted.srv"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/read_waypoints
)

### Generating Module File
_generate_module_cpp(read_waypoints
  ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/read_waypoints
  "${ALL_GEN_OUTPUT_FILES_cpp}"
)

add_custom_target(read_waypoints_generate_messages_cpp
  DEPENDS ${ALL_GEN_OUTPUT_FILES_cpp}
)
add_dependencies(read_waypoints_generate_messages read_waypoints_generate_messages_cpp)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/nvidia/Mower/src/planning/read_waypoints/msg/LocalPath.msg" NAME_WE)
add_dependencies(read_waypoints_generate_messages_cpp _read_waypoints_generate_messages_check_deps_${_filename})
get_filename_component(_filename "/home/nvidia/Mower/src/planning/read_waypoints/srv/TurnCompleted.srv" NAME_WE)
add_dependencies(read_waypoints_generate_messages_cpp _read_waypoints_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(read_waypoints_gencpp)
add_dependencies(read_waypoints_gencpp read_waypoints_generate_messages_cpp)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS read_waypoints_generate_messages_cpp)

### Section generating for lang: geneus
### Generating Messages
_generate_msg_eus(read_waypoints
  "/home/nvidia/Mower/src/planning/read_waypoints/msg/LocalPath.msg"
  "${MSG_I_FLAGS}"
  "/opt/ros/noetic/share/std_msgs/cmake/../msg/Header.msg"
  ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/read_waypoints
)

### Generating Services
_generate_srv_eus(read_waypoints
  "/home/nvidia/Mower/src/planning/read_waypoints/srv/TurnCompleted.srv"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/read_waypoints
)

### Generating Module File
_generate_module_eus(read_waypoints
  ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/read_waypoints
  "${ALL_GEN_OUTPUT_FILES_eus}"
)

add_custom_target(read_waypoints_generate_messages_eus
  DEPENDS ${ALL_GEN_OUTPUT_FILES_eus}
)
add_dependencies(read_waypoints_generate_messages read_waypoints_generate_messages_eus)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/nvidia/Mower/src/planning/read_waypoints/msg/LocalPath.msg" NAME_WE)
add_dependencies(read_waypoints_generate_messages_eus _read_waypoints_generate_messages_check_deps_${_filename})
get_filename_component(_filename "/home/nvidia/Mower/src/planning/read_waypoints/srv/TurnCompleted.srv" NAME_WE)
add_dependencies(read_waypoints_generate_messages_eus _read_waypoints_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(read_waypoints_geneus)
add_dependencies(read_waypoints_geneus read_waypoints_generate_messages_eus)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS read_waypoints_generate_messages_eus)

### Section generating for lang: genlisp
### Generating Messages
_generate_msg_lisp(read_waypoints
  "/home/nvidia/Mower/src/planning/read_waypoints/msg/LocalPath.msg"
  "${MSG_I_FLAGS}"
  "/opt/ros/noetic/share/std_msgs/cmake/../msg/Header.msg"
  ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/read_waypoints
)

### Generating Services
_generate_srv_lisp(read_waypoints
  "/home/nvidia/Mower/src/planning/read_waypoints/srv/TurnCompleted.srv"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/read_waypoints
)

### Generating Module File
_generate_module_lisp(read_waypoints
  ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/read_waypoints
  "${ALL_GEN_OUTPUT_FILES_lisp}"
)

add_custom_target(read_waypoints_generate_messages_lisp
  DEPENDS ${ALL_GEN_OUTPUT_FILES_lisp}
)
add_dependencies(read_waypoints_generate_messages read_waypoints_generate_messages_lisp)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/nvidia/Mower/src/planning/read_waypoints/msg/LocalPath.msg" NAME_WE)
add_dependencies(read_waypoints_generate_messages_lisp _read_waypoints_generate_messages_check_deps_${_filename})
get_filename_component(_filename "/home/nvidia/Mower/src/planning/read_waypoints/srv/TurnCompleted.srv" NAME_WE)
add_dependencies(read_waypoints_generate_messages_lisp _read_waypoints_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(read_waypoints_genlisp)
add_dependencies(read_waypoints_genlisp read_waypoints_generate_messages_lisp)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS read_waypoints_generate_messages_lisp)

### Section generating for lang: gennodejs
### Generating Messages
_generate_msg_nodejs(read_waypoints
  "/home/nvidia/Mower/src/planning/read_waypoints/msg/LocalPath.msg"
  "${MSG_I_FLAGS}"
  "/opt/ros/noetic/share/std_msgs/cmake/../msg/Header.msg"
  ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/read_waypoints
)

### Generating Services
_generate_srv_nodejs(read_waypoints
  "/home/nvidia/Mower/src/planning/read_waypoints/srv/TurnCompleted.srv"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/read_waypoints
)

### Generating Module File
_generate_module_nodejs(read_waypoints
  ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/read_waypoints
  "${ALL_GEN_OUTPUT_FILES_nodejs}"
)

add_custom_target(read_waypoints_generate_messages_nodejs
  DEPENDS ${ALL_GEN_OUTPUT_FILES_nodejs}
)
add_dependencies(read_waypoints_generate_messages read_waypoints_generate_messages_nodejs)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/nvidia/Mower/src/planning/read_waypoints/msg/LocalPath.msg" NAME_WE)
add_dependencies(read_waypoints_generate_messages_nodejs _read_waypoints_generate_messages_check_deps_${_filename})
get_filename_component(_filename "/home/nvidia/Mower/src/planning/read_waypoints/srv/TurnCompleted.srv" NAME_WE)
add_dependencies(read_waypoints_generate_messages_nodejs _read_waypoints_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(read_waypoints_gennodejs)
add_dependencies(read_waypoints_gennodejs read_waypoints_generate_messages_nodejs)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS read_waypoints_generate_messages_nodejs)

### Section generating for lang: genpy
### Generating Messages
_generate_msg_py(read_waypoints
  "/home/nvidia/Mower/src/planning/read_waypoints/msg/LocalPath.msg"
  "${MSG_I_FLAGS}"
  "/opt/ros/noetic/share/std_msgs/cmake/../msg/Header.msg"
  ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/read_waypoints
)

### Generating Services
_generate_srv_py(read_waypoints
  "/home/nvidia/Mower/src/planning/read_waypoints/srv/TurnCompleted.srv"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/read_waypoints
)

### Generating Module File
_generate_module_py(read_waypoints
  ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/read_waypoints
  "${ALL_GEN_OUTPUT_FILES_py}"
)

add_custom_target(read_waypoints_generate_messages_py
  DEPENDS ${ALL_GEN_OUTPUT_FILES_py}
)
add_dependencies(read_waypoints_generate_messages read_waypoints_generate_messages_py)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/nvidia/Mower/src/planning/read_waypoints/msg/LocalPath.msg" NAME_WE)
add_dependencies(read_waypoints_generate_messages_py _read_waypoints_generate_messages_check_deps_${_filename})
get_filename_component(_filename "/home/nvidia/Mower/src/planning/read_waypoints/srv/TurnCompleted.srv" NAME_WE)
add_dependencies(read_waypoints_generate_messages_py _read_waypoints_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(read_waypoints_genpy)
add_dependencies(read_waypoints_genpy read_waypoints_generate_messages_py)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS read_waypoints_generate_messages_py)



if(gencpp_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/read_waypoints)
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/read_waypoints
    DESTINATION ${gencpp_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_cpp)
  add_dependencies(read_waypoints_generate_messages_cpp std_msgs_generate_messages_cpp)
endif()

if(geneus_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/read_waypoints)
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/read_waypoints
    DESTINATION ${geneus_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_eus)
  add_dependencies(read_waypoints_generate_messages_eus std_msgs_generate_messages_eus)
endif()

if(genlisp_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/read_waypoints)
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/read_waypoints
    DESTINATION ${genlisp_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_lisp)
  add_dependencies(read_waypoints_generate_messages_lisp std_msgs_generate_messages_lisp)
endif()

if(gennodejs_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/read_waypoints)
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/read_waypoints
    DESTINATION ${gennodejs_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_nodejs)
  add_dependencies(read_waypoints_generate_messages_nodejs std_msgs_generate_messages_nodejs)
endif()

if(genpy_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/read_waypoints)
  install(CODE "execute_process(COMMAND \"/usr/bin/python3\" -m compileall \"${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/read_waypoints\")")
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/read_waypoints
    DESTINATION ${genpy_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_py)
  add_dependencies(read_waypoints_generate_messages_py std_msgs_generate_messages_py)
endif()
