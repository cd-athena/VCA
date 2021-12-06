cmake_minimum_required(VERSION 3.10)
include(CheckIncludeFiles)

file(GLOB_RECURSE vca_app_source *.cpp)
file(GLOB_RECURSE vca_app_header *.h)

check_include_files(getopt.h HAVE_GETOPT_H)
if(NOT HAVE_GETOPT_H)
    if(MSVC)
        set_source_files_properties(getopt/getopt.c PROPERTIES COMPILE_FLAGS "/wd4100 /wd4131 -DHAVE_STRING_H=1")
    endif(MSVC)
    include_directories(getopt)
    set(GETOPT getopt/getopt.c getopt/getopt.h)
    message("Using compatibility getopt")
endif(NOT HAVE_GETOPT_H)

include_directories("${CMAKE_SOURCE_DIR}/source")
include_directories("${CMAKE_SOURCE_DIR}/source/app")

add_executable(vca ${vca_app_source} ${vca_app_header} ${GETOPT})
target_link_libraries (vca vcaLib)

install(TARGETS vca RUNTIME DESTINATION bin COMPONENT applications)
