cmake_minimum_required(VERSION 3.10)
include(CheckIncludeFiles)

file(GLOB_RECURSE vca_apps_common_source ../common/*.cpp)
file(GLOB_RECURSE vca_apps_common_header ../common/*.h)

check_include_files(getopt.h HAVE_GETOPT_H)
if(NOT HAVE_GETOPT_H)
    if(MSVC)
        set_source_files_properties(../common/getopt/getopt.c PROPERTIES COMPILE_FLAGS "/wd4100 /wd4131 -DHAVE_STRING_H=1")
    endif(MSVC)
    include_directories(../common/getopt)
    set(GETOPT ../common/getopt/getopt.c ../common/getopt/getopt.h)
    message(STATUS "Using compatibility getopt")
endif(NOT HAVE_GETOPT_H)

include_directories("${CMAKE_SOURCE_DIR}/source")
include_directories("${CMAKE_SOURCE_DIR}/source/apps")

add_executable(vca vcacli.h vca.cpp ${vca_apps_common_source} ${vca_apps_common_header} ${GETOPT})
target_link_libraries (vca vcaLib)

install(TARGETS vca RUNTIME DESTINATION bin COMPONENT applications)
