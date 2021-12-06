cmake_minimum_required (VERSION 3.10)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set (CMAKE_CXX_STANDARD 17)

project (vca)

add_subdirectory(source/lib)
add_subdirectory(source/app)
