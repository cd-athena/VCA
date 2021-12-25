cmake_minimum_required(VERSION 3.10)

file(GLOB_RECURSE vcaLib_source *.cpp)
file(GLOB_RECURSE vcaLib_header *.h)
file(GLOB_RECURSE vcaLib_asm *.asm)

include_directories(./)

if (VCA_STATIC_BUILD)
  add_library(vcaLib STATIC ${vcaLib_source} ${vcaLib_header})
else()
  add_library(vcaLib SHARED ${vcaLib_source} ${vcaLib_header})
endif()
set_target_properties(vcaLib PROPERTIES OUTPUT_NAME vcaLib)
set_target_properties(vcaLib PROPERTIES PUBLIC_HEADER "vcaLib.h")

install(TARGETS vcaLib LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries PUBLIC_HEADER DESTINATION include)
