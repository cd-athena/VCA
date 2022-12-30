cmake_minimum_required(VERSION 3.13)

configure_file("${PROJECT_SOURCE_DIR}/source/lib/vca_config.h.in"
    "${PROJECT_BINARY_DIR}/vca_config.h")

find_package(Git QUIET) # present in 2.8.8

# defaults, in case everything below fails
set(VCA_VERSION "unknown")
set(VCA_LATEST_TAG "0.0")
set(VCA_TAG_DISTANCE "0")

if(GIT_EXECUTABLE AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../.git)
    execute_process(
        COMMAND
        ${GIT_EXECUTABLE} rev-list --tags --max-count=1
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE VCA_LATEST_TAG_COMMIT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND
        ${GIT_EXECUTABLE} describe --tags ${VCA_LATEST_TAG_COMMIT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE VCA_LATEST_TAG
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND
        ${GIT_EXECUTABLE} rev-list ${VCA_LATEST_TAG}.. --count --first-parent
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE VCA_TAG_DISTANCE
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND
        ${GIT_EXECUTABLE} log -1 --format=g%h
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE VCA_REVISION_ID
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

if(VCA_TAG_DISTANCE STREQUAL "0")
    set(VCA_VERSION "${VCA_LATEST_TAG}")
else()
    set(VCA_VERSION "${VCA_LATEST_TAG}+${VCA_TAG_DISTANCE}-${VCA_REVISION_ID}")
endif()

message(STATUS "VCA version ${VCA_VERSION}")
list(APPEND VFLAG "-DVCA_VERSION=${VCA_VERSION}")
set_source_files_properties(vcaLib.cpp PROPERTIES COMPILE_FLAGS ${VFLAG})

find_package(Threads REQUIRED)

# System architecture detection
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" SYSPROC)
set(X86_ALIASES x86 i386 i686 x86_64 amd64)
set(ARM_ALIASES armv6l armv7l aarch64)
set(POWER_ALIASES ppc64 ppc64le)
list(FIND X86_ALIASES "${SYSPROC}" X86MATCH)
list(FIND ARM_ALIASES "${SYSPROC}" ARMMATCH)
list(FIND POWER_ALIASES "${SYSPROC}" POWERMATCH)

if("${SYSPROC}" STREQUAL "" OR X86MATCH GREATER "-1")
    add_definitions(-DX86_64=1)
    add_definitions(-DVCA_ARCH_X86=1)
    message(STATUS "Detected x86 target processor")
elseif(POWERMATCH GREATER "-1")
    message(STATUS "Detected POWER target processor")
elseif(ARMMATCH GREATER "-1")
    message(STATUS "Detected ARM target processor")
else()
    message(STATUS "CMAKE_SYSTEM_PROCESSOR value `${CMAKE_SYSTEM_PROCESSOR}` is unknown")
    message(STATUS "Please add this value near ${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE}")
endif()

add_library(vcaLib "")
target_sources(vcaLib
    PRIVATE
    vcaLib.cpp
    PUBLIC
    vcaLib.h
)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set(GCC 1)
endif()

add_subdirectory(analyzer)

include_directories(./)

target_link_libraries(vcaLib vcaInternal)
set_target_properties(vcaLib PROPERTIES OUTPUT_NAME vcaLib)
set_target_properties(vcaLib PROPERTIES PUBLIC_HEADER "vcaLib.h")

install(TARGETS vcaLib LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries PUBLIC_HEADER DESTINATION include)
