cmake_minimum_required(VERSION 3.10)

file(GLOB_RECURSE vcaLib_source *.cpp)
file(GLOB_RECURSE vcaLib_header *.h)
file(GLOB_RECURSE vcaLib_asm *.asm)

include_directories(./)

add_library(vcaLib SHARED ${vcaLib_source} ${vcaLib_header})
set_target_properties(vcaLib PROPERTIES OUTPUT_NAME vcaLib)
set_target_properties(vcaLib PROPERTIES PUBLIC_HEADER "vcaLib.h")

install(TARGETS vcaLib LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries PUBLIC_HEADER DESTINATION include)
