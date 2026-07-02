# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if("/home/nvidia/Mower/src/polygon_coverage_planning/polygon_coverage_solvers/include/gtsp_ma_source_codes.zip" STREQUAL "")
  message(FATAL_ERROR "LOCAL can't be empty")
endif()

if(NOT EXISTS "/home/nvidia/Mower/src/polygon_coverage_planning/polygon_coverage_solvers/include/gtsp_ma_source_codes.zip")
  message(FATAL_ERROR "File not found: /home/nvidia/Mower/src/polygon_coverage_planning/polygon_coverage_solvers/include/gtsp_ma_source_codes.zip")
endif()

if("MD5" STREQUAL "")
  message(WARNING "File will not be verified since no URL_HASH specified")
  return()
endif()

if("765fad8e3746fa3dd9b81be0afb34d35" STREQUAL "")
  message(FATAL_ERROR "EXPECT_VALUE can't be empty")
endif()

message(STATUS "verifying file...
     file='/home/nvidia/Mower/src/polygon_coverage_planning/polygon_coverage_solvers/include/gtsp_ma_source_codes.zip'")

file("MD5" "/home/nvidia/Mower/src/polygon_coverage_planning/polygon_coverage_solvers/include/gtsp_ma_source_codes.zip" actual_value)

if(NOT "${actual_value}" STREQUAL "765fad8e3746fa3dd9b81be0afb34d35")
  message(FATAL_ERROR "error: MD5 hash of
  /home/nvidia/Mower/src/polygon_coverage_planning/polygon_coverage_solvers/include/gtsp_ma_source_codes.zip
does not match expected value
  expected: '765fad8e3746fa3dd9b81be0afb34d35'
    actual: '${actual_value}'
")
endif()

message(STATUS "verifying file... done")
