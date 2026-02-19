cmake_minimum_required(VERSION 3.13)

# Version header
configure_file("${PROJECT_SOURCE_DIR}/source/lib/vca_config.h.in"
    "${PROJECT_BINARY_DIR}/vca_config.h")

find_package(Git QUIET) # present in 2.8.8

# defaults, in case everything below fails
set(VCA_VERSION "unknown")
set(VCA_LATEST_TAG "0.0")
set(VCA_TAG_DISTANCE "0")

set(LIB_SOURCE_DIR ${CMAKE_SOURCE_DIR}/source/lib)

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

# Architecture detection
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" SYSPROC)
set(X86_ALIASES x86 i386 i686 x86_64 amd64)
set(ARM_ALIASES armv6l armv7l aarch64 arm64)
set(POWER_ALIASES ppc64 ppc64le)
list(FIND X86_ALIASES "${SYSPROC}" X86MATCH)
list(FIND ARM_ALIASES "${SYSPROC}" ARMMATCH)
list(FIND POWER_ALIASES "${SYSPROC}" POWERMATCH)

if("${SYSPROC}" STREQUAL "" OR X86MATCH GREATER "-1")
    add_definitions(-DX86_64=1)
    add_definitions(-DVCA_ARCH_X86=1)
    set(VCA_ARCH_X86 1)
    message(STATUS "Detected x86 target processor")
elseif(POWERMATCH GREATER "-1")
    message(STATUS "Detected POWER target processor")
elseif(ARMMATCH GREATER "-1")
    message(STATUS "Detected ARM target processor")
    add_definitions(-DVCA_ARCH_ARM=1)
    set(VCA_ARCH_ARM 1)
else()
    message(STATUS "CMAKE_SYSTEM_PROCESSOR value `${CMAKE_SYSTEM_PROCESSOR}` is unknown")
    message(STATUS "Please add this value near ${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE}")
endif()

# SIMD / Assembler availability (parent owns options; we just detect)

include(CheckLanguage)

# Default off until proven available (or intrinsics-only on ARM)
set(SIMD_ENABLED OFF)
set(NASM_ENABLED OFF)

if (ENABLE_SIMD)
    if (VCA_ARCH_X86)
        # Try NASM -> then YASM -> then MASM (for MSVC)
        if (ENABLE_NASM)
            check_language(ASM_NASM)
            if (CMAKE_ASM_NASM_COMPILER)
                enable_language(ASM_NASM)
                set(SIMD_ENABLED ON)
                set(NASM_ENABLED ON)
                message(STATUS "SIMD: enabling x86 assembly with NASM (${CMAKE_ASM_NASM_COMPILER})")
            else()
                # Optional: fall back to ASM_YASM if your code supports it.
                check_language(ASM_YASM)
                if (CMAKE_ASM_YASM_COMPILER)
                    enable_language(ASM_YASM)
                    set(SIMD_ENABLED ON)
                    set(NASM_ENABLED ON) # treat YASM equivalently for downstream guards
                    message(STATUS "SIMD: enabling x86 assembly with YASM (${CMAKE_ASM_YASM_COMPILER})")
                elseif (MSVC)
                    check_language(ASM_MASM)
                    if (CMAKE_ASM_MASM_COMPILER)
                        enable_language(ASM_MASM)
                        set(SIMD_ENABLED ON)
                        # Use NASM_ENABLED=false to signal different dialect if needed
                        message(STATUS "SIMD: enabling x86 assembly with MASM (${CMAKE_ASM_MASM_COMPILER})")
                    else()
                        message(WARNING "SIMD: no assembler (NASM/YASM/MASM) found on x86 -> disabling SIMD")
                    endif()
                else()
                    message(WARNING "SIMD: no assembler (NASM/YASM) found on x86 -> disabling SIMD")
                endif()
            endif()
        else()
            message(STATUS "SIMD: NASM explicitly disabled by user (-DENABLE_NASM=OFF)")
        endif()

    elseif (VCA_ARCH_ARM)
        if (DEFINED VCA_ARM_INTRINSICS_ONLY AND VCA_ARM_INTRINSICS_ONLY)
            # Intrinsics path does not require an assembler
            set(SIMD_ENABLED ON)
            message(STATUS "SIMD: enabling ARM NEON intrinsics (no assembler required)")
        else()
            # If you have .S files, require generic ASM
            check_language(ASM)
            if (CMAKE_ASM_COMPILER)
                enable_language(ASM)
                set(SIMD_ENABLED ON)
                message(STATUS "SIMD: enabling ARM assembly (${CMAKE_ASM_COMPILER})")
            else()
                message(WARNING "SIMD: no ARM assembler found -> disabling SIMD")
            endif()
        endif()
    else()
        message(STATUS "SIMD: unsupported/unknown arch for SIMD -> leaving disabled")
    endif()
else()
    message(STATUS "SIMD: disabled by user (-DENABLE_SIMD=OFF)")
endif()

# Export these to the analyzer (subdirectory can read them)
set(SIMD_ENABLED ${SIMD_ENABLED} PARENT_SCOPE) # if someone includes this dir as subdir
set(NASM_ENABLED ${NASM_ENABLED} PARENT_SCOPE)

# Also make available to children of this directory
set(SIMD_ENABLED ${SIMD_ENABLED} CACHE INTERNAL "SIMD enabled after toolchain detection")
set(NASM_ENABLED ${NASM_ENABLED} CACHE INTERNAL "NASM (or equivalent) enabled on x86")

# Libraries in this directory
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

# Add analyzer (it will conditionally add simd/ and SIMD targets)
add_subdirectory(analyzer)

# Tests
if (ENABLE_TEST)
    message(STATUS "Enable building of unit tests")
    add_subdirectory(test)
else()
    message(STATUS "Not building unit tests")
endif()

# Includes & compile definitions
target_include_directories(vcaInternal PRIVATE ${LIB_SOURCE_DIR})
target_include_directories(vcaLib PRIVATE ${LIB_SOURCE_DIR})

# If SIMD is not enabled, let sources choose scalar paths
if (NOT SIMD_ENABLED)
    target_compile_definitions(vcaInternal PRIVATE VCA_DISABLE_SIMD=1)
    target_compile_definitions(vcaLib       PRIVATE VCA_DISABLE_SIMD=1)
endif()

# Linking (safe: only link SIMD libs if they exist)
# analyzer/simd creates vcaLibSimd{8,10,12} only when SIMD_ENABLED=ON
target_link_libraries(vcaLib
    vcaInternal
)

# If you want vcaLib to link SIMD libs directly (optional), guard with TARGET_EXISTS:
if (SIMD_ENABLED)
    if (TARGET vcaLibSimd8bit OR TARGET vcaLibSimd10bit OR TARGET vcaLibSimd12bit)
        target_link_libraries(vcaLib
            $<$<TARGET_EXISTS:vcaLibSimd8bit>:vcaLibSimd8bit>
            $<$<TARGET_EXISTS:vcaLibSimd10bit>:vcaLibSimd10bit>
            $<$<TARGET_EXISTS:vcaLibSimd12bit>:vcaLibSimd12bit>
        )
    endif()
endif()

set_target_properties(vcaLib PROPERTIES OUTPUT_NAME vcaLib)
set_target_properties(vcaLib PROPERTIES PUBLIC_HEADER "vcaLib.h")

install(
    TARGETS vcaLib
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    COMPONENT libraries
    PUBLIC_HEADER DESTINATION include
)
