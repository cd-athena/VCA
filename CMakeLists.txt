cmake_minimum_required(VERSION 3.13)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(VCA)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR "Building for 32 bit targets is not supported.")
endif()

option(ENABLE_NASM "Enable use of nasm assembly" ON)
option(ENABLE_PERFORMANCE_TEST "Enable Performance Test" OFF)
option(ENABLE_TEST "Enable build of Tests" OFF)

add_subdirectory(source/lib)
add_subdirectory(source/apps/vca)

if(ENABLE_PERFORMANCE_TEST)
  add_subdirectory(source/apps/vcaPerformanceTest)
endif(ENABLE_PERFORMANCE_TEST)
