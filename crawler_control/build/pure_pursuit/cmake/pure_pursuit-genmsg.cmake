# generated from genmsg/cmake/pkg-genmsg.cmake.em

message(STATUS "pure_pursuit: 0 messages, 1 services")

set(MSG_I_FLAGS "-Istd_msgs:/opt/ros/noetic/share/std_msgs/cmake/../msg")

# Find all generators
find_package(gencpp REQUIRED)
find_package(geneus REQUIRED)
find_package(genlisp REQUIRED)
find_package(gennodejs REQUIRED)
find_package(genpy REQUIRED)

add_custom_target(pure_pursuit_generate_messages ALL)

# verify that message/service dependencies have not changed since configure



get_filename_component(_filename "/home/huilian/hc26_6_30/crawler_control/src/pure_pursuit/srv/TurnCompleted.srv" NAME_WE)
add_custom_target(_pure_pursuit_generate_messages_check_deps_${_filename}
  COMMAND ${CATKIN_ENV} ${PYTHON_EXECUTABLE} ${GENMSG_CHECK_DEPS_SCRIPT} "pure_pursuit" "/home/huilian/hc26_6_30/crawler_control/src/pure_pursuit/srv/TurnCompleted.srv" ""
)

#
#  langs = gencpp;geneus;genlisp;gennodejs;genpy
#

### Section generating for lang: gencpp
### Generating Messages

### Generating Services
_generate_srv_cpp(pure_pursuit
  "/home/huilian/hc26_6_30/crawler_control/src/pure_pursuit/srv/TurnCompleted.srv"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/pure_pursuit
)

### Generating Module File
_generate_module_cpp(pure_pursuit
  ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/pure_pursuit
  "${ALL_GEN_OUTPUT_FILES_cpp}"
)

add_custom_target(pure_pursuit_generate_messages_cpp
  DEPENDS ${ALL_GEN_OUTPUT_FILES_cpp}
)
add_dependencies(pure_pursuit_generate_messages pure_pursuit_generate_messages_cpp)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/huilian/hc26_6_30/crawler_control/src/pure_pursuit/srv/TurnCompleted.srv" NAME_WE)
add_dependencies(pure_pursuit_generate_messages_cpp _pure_pursuit_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(pure_pursuit_gencpp)
add_dependencies(pure_pursuit_gencpp pure_pursuit_generate_messages_cpp)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS pure_pursuit_generate_messages_cpp)

### Section generating for lang: geneus
### Generating Messages

### Generating Services
_generate_srv_eus(pure_pursuit
  "/home/huilian/hc26_6_30/crawler_control/src/pure_pursuit/srv/TurnCompleted.srv"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/pure_pursuit
)

### Generating Module File
_generate_module_eus(pure_pursuit
  ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/pure_pursuit
  "${ALL_GEN_OUTPUT_FILES_eus}"
)

add_custom_target(pure_pursuit_generate_messages_eus
  DEPENDS ${ALL_GEN_OUTPUT_FILES_eus}
)
add_dependencies(pure_pursuit_generate_messages pure_pursuit_generate_messages_eus)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/huilian/hc26_6_30/crawler_control/src/pure_pursuit/srv/TurnCompleted.srv" NAME_WE)
add_dependencies(pure_pursuit_generate_messages_eus _pure_pursuit_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(pure_pursuit_geneus)
add_dependencies(pure_pursuit_geneus pure_pursuit_generate_messages_eus)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS pure_pursuit_generate_messages_eus)

### Section generating for lang: genlisp
### Generating Messages

### Generating Services
_generate_srv_lisp(pure_pursuit
  "/home/huilian/hc26_6_30/crawler_control/src/pure_pursuit/srv/TurnCompleted.srv"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/pure_pursuit
)

### Generating Module File
_generate_module_lisp(pure_pursuit
  ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/pure_pursuit
  "${ALL_GEN_OUTPUT_FILES_lisp}"
)

add_custom_target(pure_pursuit_generate_messages_lisp
  DEPENDS ${ALL_GEN_OUTPUT_FILES_lisp}
)
add_dependencies(pure_pursuit_generate_messages pure_pursuit_generate_messages_lisp)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/huilian/hc26_6_30/crawler_control/src/pure_pursuit/srv/TurnCompleted.srv" NAME_WE)
add_dependencies(pure_pursuit_generate_messages_lisp _pure_pursuit_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(pure_pursuit_genlisp)
add_dependencies(pure_pursuit_genlisp pure_pursuit_generate_messages_lisp)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS pure_pursuit_generate_messages_lisp)

### Section generating for lang: gennodejs
### Generating Messages

### Generating Services
_generate_srv_nodejs(pure_pursuit
  "/home/huilian/hc26_6_30/crawler_control/src/pure_pursuit/srv/TurnCompleted.srv"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/pure_pursuit
)

### Generating Module File
_generate_module_nodejs(pure_pursuit
  ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/pure_pursuit
  "${ALL_GEN_OUTPUT_FILES_nodejs}"
)

add_custom_target(pure_pursuit_generate_messages_nodejs
  DEPENDS ${ALL_GEN_OUTPUT_FILES_nodejs}
)
add_dependencies(pure_pursuit_generate_messages pure_pursuit_generate_messages_nodejs)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/huilian/hc26_6_30/crawler_control/src/pure_pursuit/srv/TurnCompleted.srv" NAME_WE)
add_dependencies(pure_pursuit_generate_messages_nodejs _pure_pursuit_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(pure_pursuit_gennodejs)
add_dependencies(pure_pursuit_gennodejs pure_pursuit_generate_messages_nodejs)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS pure_pursuit_generate_messages_nodejs)

### Section generating for lang: genpy
### Generating Messages

### Generating Services
_generate_srv_py(pure_pursuit
  "/home/huilian/hc26_6_30/crawler_control/src/pure_pursuit/srv/TurnCompleted.srv"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/pure_pursuit
)

### Generating Module File
_generate_module_py(pure_pursuit
  ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/pure_pursuit
  "${ALL_GEN_OUTPUT_FILES_py}"
)

add_custom_target(pure_pursuit_generate_messages_py
  DEPENDS ${ALL_GEN_OUTPUT_FILES_py}
)
add_dependencies(pure_pursuit_generate_messages pure_pursuit_generate_messages_py)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/huilian/hc26_6_30/crawler_control/src/pure_pursuit/srv/TurnCompleted.srv" NAME_WE)
add_dependencies(pure_pursuit_generate_messages_py _pure_pursuit_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(pure_pursuit_genpy)
add_dependencies(pure_pursuit_genpy pure_pursuit_generate_messages_py)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS pure_pursuit_generate_messages_py)



if(gencpp_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/pure_pursuit)
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/pure_pursuit
    DESTINATION ${gencpp_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_cpp)
  add_dependencies(pure_pursuit_generate_messages_cpp std_msgs_generate_messages_cpp)
endif()

if(geneus_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/pure_pursuit)
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/pure_pursuit
    DESTINATION ${geneus_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_eus)
  add_dependencies(pure_pursuit_generate_messages_eus std_msgs_generate_messages_eus)
endif()

if(genlisp_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/pure_pursuit)
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/pure_pursuit
    DESTINATION ${genlisp_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_lisp)
  add_dependencies(pure_pursuit_generate_messages_lisp std_msgs_generate_messages_lisp)
endif()

if(gennodejs_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/pure_pursuit)
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/pure_pursuit
    DESTINATION ${gennodejs_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_nodejs)
  add_dependencies(pure_pursuit_generate_messages_nodejs std_msgs_generate_messages_nodejs)
endif()

if(genpy_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/pure_pursuit)
  install(CODE "execute_process(COMMAND \"/usr/bin/python3\" -m compileall \"${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/pure_pursuit\")")
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/pure_pursuit
    DESTINATION ${genpy_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_py)
  add_dependencies(pure_pursuit_generate_messages_py std_msgs_generate_messages_py)
endif()
