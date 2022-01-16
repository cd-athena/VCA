cmake_minimum_required (VERSION 3.10)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project (vca)

option(ENABLE_ASSEMBLY "Enable use of nasm assembly" ON)
option(ENABLE_PERFORMANCE_TEST "Enable Performance Test" OFF)

add_subdirectory(source/lib)
add_subdirectory(source/apps/vca)

if(ENABLE_PERFORMANCE_TEST)
  add_subdirectory(source/apps/vcaPerformanceTest)
endif(ENABLE_PERFORMANCE_TEST)
