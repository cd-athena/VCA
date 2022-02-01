cmake_minimum_required(VERSION 3.10)

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

find_package (Threads REQUIRED)

# System architecture detection
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" SYSPROC)
set(X86_ALIASES x86 i386 i686 x86_64 amd64)
list(FIND X86_ALIASES "${SYSPROC}" X86MATCH)
if("${SYSPROC}" STREQUAL "" OR X86MATCH GREATER "-1")
    set(X86 1)
    add_definitions(-DVCA_ARCH_X86=1)
    if(CMAKE_CXX_FLAGS STREQUAL "-m32")
        message(STATUS "Detected x86 target processor")
    elseif("${CMAKE_SIZEOF_VOID_P}" MATCHES 8)
        set(X64 1)
        add_definitions(-DX86_64=1)
        message(STATUS "Detected x86_64 target processor")
    endif()
else()
    message(STATUS "CMAKE_SYSTEM_PROCESSOR value `${CMAKE_SYSTEM_PROCESSOR}` is not supported")
    message(STATUS "Please add this value near ${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE}")
endif()

if(NOT X64)
    message(FATAL_ERROR "Only 64bit architecture is supported")
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set(GCC 1)
endif()

find_program(NASM_EXECUTABLE 
 NAMES nasm nasm-2.13.0-win32 nasm-2.13.0-win64 nasm nasm-2.13.0-win32 nasm-2.13.0-win64
 HINTS $ENV{NASM_ROOT} ${NASM_ROOT}
 PATH_SUFFIXES bin
)

if(NASM_EXECUTABLE)
        execute_process(COMMAND ${NASM_EXECUTABLE} -version
            OUTPUT_VARIABLE nasm_version
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )
    if(nasm_version MATCHES "^NASM version ([0-9\\.]*)")
        set(NASM_VERSION_STRING "${CMAKE_MATCH_1}")
    endif()
    unset(nasm_version)
endif()

if(NASM_VERSION_STRING AND X86)
    message(STATUS "Found Nasm ${NASM_VERSION_STRING} to build assembly primitives")
    option(ENABLE_ASSEMBLY "Enable use of assembly coded primitives" ON)
else()
    option(ENABLE_ASSEMBLY "Enable use of assembly coded primitives" OFF)
endif()

if(ENABLE_ASSEMBLY)
    set_source_files_properties(analyzer/Analyzer.cpp analyzer/EnergyCalculation.cpp analyzer/simd/cpu.cpp PROPERTIES COMPILE_FLAGS -DENABLE_ASSEMBLY=1)
    message(STATUS "Activating nasm assembly")
    enable_language(ASM_NASM)
    set(A_SRCS 
        analyzer/simd/dct8.asm 
        analyzer/simd/const-a.asm
        analyzer/simd/cpu-a.asm)
    
    if(APPLE)
        set(CMAKE_ASM_NASM_FLAGS "-I\"${CMAKE_CURRENT_SOURCE_DIR}/analyzer/simd/\" -DPIC -DARCH_X86_64=1 -DPREFIX -DHIGH_BIT_DEPTH=0 -DBIT_DEPTH=8 -DVCA_NS=vca")
    else()
        set(CMAKE_ASM_NASM_FLAGS "-I\"${CMAKE_CURRENT_SOURCE_DIR}/analyzer/simd/\" -DPIC -DARCH_X86_64=1 -DHIGH_BIT_DEPTH=0 -DBIT_DEPTH=8 -DVCA_NS=vca")	
    endif()

    if (GCC)
        set_source_files_properties(analyzer/simd/dct-ssse3.cpp PROPERTIES COMPILE_FLAGS "-mssse3")
    endif(GCC)
endif(ENABLE_ASSEMBLY)

file(GLOB_RECURSE vcaLib_source *.cpp)
file(GLOB_RECURSE vcaLib_header *.h)

include_directories(./)

add_library(vcaLib ${vcaLib_source} ${vcaLib_header} ${A_SRCS})
target_link_libraries(vcaLib Threads::Threads)
set_target_properties(vcaLib PROPERTIES OUTPUT_NAME vcaLib)
set_target_properties(vcaLib PROPERTIES PUBLIC_HEADER "vcaLib.h")

install(TARGETS vcaLib LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries PUBLIC_HEADER DESTINATION include)
