cmake_minimum_required (VERSION 3.10)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set (CMAKE_CXX_STANDARD 17)

project (vca)

add_subdirectory(source/lib)
add_subdirectory(source/apps/vca)

option(ENABLE_PERFORMANCE_TEST "Enable Performance Test" OFF)
if(ENABLE_PERFORMANCE_TEST)
	add_subdirectory(source/apps/vcaPerformanceTest)
endif(ENABLE_PERFORMANCE_TEST)
