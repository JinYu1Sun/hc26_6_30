# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if("/home/nvidia/Mower/src/polygon_coverage_planning/polygon_coverage_solvers/include/InstancesBinary.zip" STREQUAL "")
  message(FATAL_ERROR "LOCAL can't be empty")
endif()

if(NOT EXISTS "/home/nvidia/Mower/src/polygon_coverage_planning/polygon_coverage_solvers/include/InstancesBinary.zip")
  message(FATAL_ERROR "File not found: /home/nvidia/Mower/src/polygon_coverage_planning/polygon_coverage_solvers/include/InstancesBinary.zip")
endif()

if("MD5" STREQUAL "")
  message(WARNING "File will not be verified since no URL_HASH specified")
  return()
endif()

if("255831bd47de71df8419a54741f0a6be" STREQUAL "")
  message(FATAL_ERROR "EXPECT_VALUE can't be empty")
endif()

message(STATUS "verifying file...
     file='/home/nvidia/Mower/src/polygon_coverage_planning/polygon_coverage_solvers/include/InstancesBinary.zip'")

file("MD5" "/home/nvidia/Mower/src/polygon_coverage_planning/polygon_coverage_solvers/include/InstancesBinary.zip" actual_value)

if(NOT "${actual_value}" STREQUAL "255831bd47de71df8419a54741f0a6be")
  message(FATAL_ERROR "error: MD5 hash of
  /home/nvidia/Mower/src/polygon_coverage_planning/polygon_coverage_solvers/include/InstancesBinary.zip
does not match expected value
  expected: '255831bd47de71df8419a54741f0a6be'
    actual: '${actual_value}'
")
endif()

message(STATUS "verifying file... done")
