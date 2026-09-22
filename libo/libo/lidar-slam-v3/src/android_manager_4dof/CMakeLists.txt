cmake_minimum_required(VERSION 3.0.2)
project(android_manager_4dof)

## Compile as C++17, required for std::filesystem
set(CMAKE_BUILD_TYPE "RELEASE")
add_compile_options(-std=c++17)
set(CMAKE_CXX_FLAGS "-std=c++17 -O3")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  util
)
find_package(Threads REQUIRED)

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES android_manager_4dof
#  CATKIN_DEPENDS roscpp std_msgs util
#  DEPENDS system_lib
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_executable(android_manager_4dof_node src/android_manager_4dof.cc)
add_dependencies(android_manager_4dof_node ${catkin_EXPORTED_TARGETS})
target_link_libraries(android_manager_4dof_node ${catkin_LIBRARIES} Threads::Threads)

if(CATKIN_ENABLE_TESTING)
  add_executable(lifecycle_mode_test test/lifecycle_mode_test.cpp)
  add_test(NAME lifecycle_mode_test COMMAND lifecycle_mode_test)
endif()
