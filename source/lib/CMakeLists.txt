cmake_minimum_required(VERSION 3.13)

# Version header
configure_file("${PROJECT_SOURCE_DIR}/source/lib/vca_config.h.in"
    "${PROJECT_BINARY_DIR}/vca_config.h")

find_package(Git QUIET) # present in 2.8.8

# Defaults
set(VCA_VERSION "unknown")
set(VCA_BASE_TAG "0.0.0")
set(VCA_TAG_DISTANCE "0")
set(VCA_REVISION_ID "")
set(VCA_DIRTY "")

set(LIB_SOURCE_DIR ${CMAKE_SOURCE_DIR}/source/lib)

if(GIT_EXECUTABLE AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # nearest tag reachable from HEAD
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        OUTPUT_VARIABLE VCA_BASE_TAG
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # distance from tag to HEAD
    if(NOT VCA_BASE_TAG STREQUAL "")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-list ${VCA_BASE_TAG}..HEAD --count --first-parent
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
            OUTPUT_VARIABLE VCA_TAG_DISTANCE
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()

    # revision id (short)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short=12 HEAD
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        OUTPUT_VARIABLE VCA_REVISION_ID
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # dirty flag (optional)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} diff --quiet
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        RESULT_VARIABLE GIT_DIFF_EXIT
        ERROR_QUIET
    )
    if(GIT_DIFF_EXIT EQUAL 1)
        set(VCA_DIRTY "-dirty")
    endif()
endif()

# Compose version string
if(VCA_BASE_TAG STREQUAL "" OR VCA_BASE_TAG STREQUAL "unknown")
    set(VCA_VERSION "unknown")
else()
    if(VCA_TAG_DISTANCE STREQUAL "0")
        # exact tag
        set(VCA_VERSION "${VCA_BASE_TAG}${VCA_DIRTY}")
    else()
        # tag+N-g<sha>[-dirty]
        set(VCA_VERSION "${VCA_BASE_TAG}+${VCA_TAG_DISTANCE}-g${VCA_REVISION_ID}${VCA_DIRTY}")
    endif()
endif()

message(STATUS "VCA version: ${VCA_VERSION}")

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

# Analyzer (it will conditionally add simd/ and SIMD targets)
add_subdirectory(analyzer)

# Tests
if (ENABLE_TEST)
    message(STATUS "Enable building of unit tests")
    add_subdirectory(test)
else()
    message(STATUS "Not building unit tests")
endif()

# Ensure core has includes and scalar define
target_include_directories(vcaInternal PRIVATE ${LIB_SOURCE_DIR})
if (NOT SIMD_ENABLED)
    target_compile_definitions(vcaInternal PRIVATE VCA_DISABLE_SIMD=1)
endif()

# --- vcaLib variants (shared/static) ---
include(GNUInstallDirs)

set(VCA_LIB_PUBLIC_HEADERS
    vcaLib.h
)
set(VCA_LIB_SOURCES
    vcaLib.cpp
)

function(vca_configure_lib tgt)
    target_include_directories(${tgt} PUBLIC ${LIB_SOURCE_DIR})
    if (NOT SIMD_ENABLED)
        target_compile_definitions(${tgt} PRIVATE VCA_DISABLE_SIMD=1)
    endif()
    target_link_libraries(${tgt} PUBLIC vcaInternal)
    if (SIMD_ENABLED)
        if (TARGET vcaLibSimd8bit OR TARGET vcaLibSimd10bit OR TARGET vcaLibSimd12bit)
            target_link_libraries(${tgt} PUBLIC
                $<$<TARGET_EXISTS:vcaLibSimd8bit>:vcaLibSimd8bit>
                $<$<TARGET_EXISTS:vcaLibSimd10bit>:vcaLibSimd10bit>
                $<$<TARGET_EXISTS:vcaLibSimd12bit>:vcaLibSimd12bit>
            )
        endif()
    endif()
    set_target_properties(${tgt} PROPERTIES OUTPUT_NAME vcaLib)
endfunction()

# Static build
if (ENABLE_STATIC_BUILD)
    add_library(vcaLib_static STATIC ${VCA_LIB_SOURCES} ${VCA_LIB_PUBLIC_HEADERS})
    vca_configure_lib(vcaLib_static)
    add_library(vcaLib::static ALIAS vcaLib_static)
endif()

# Shared build
if (ENABLE_SHARED_BUILD)
    add_library(vcaLib_shared SHARED ${VCA_LIB_SOURCES} ${VCA_LIB_PUBLIC_HEADERS})
    vca_configure_lib(vcaLib_shared)
    add_library(vcaLib::shared ALIAS vcaLib_shared)
    # On Windows we define export macro
    target_compile_definitions(vcaLib_shared PRIVATE VCA_LIB_SHARED=1 VCA_LIB_EXPORTS=1)
endif()

# Ensure the string is quoted for the compiler, regardless of special chars
if (TARGET vcaLib_shared)
  target_compile_definitions(vcaLib_shared PRIVATE VCA_VERSION_STR="${VCA_VERSION}")
endif()

if (TARGET vcaLib_static)
  target_compile_definitions(vcaLib_static PRIVATE VCA_VERSION_STR="${VCA_VERSION}")
endif()


# Backward-compatible logical target that existing code links to
if (ENABLE_SHARED_BUILD)
    add_library(vcaLib INTERFACE)
    target_link_libraries(vcaLib INTERFACE vcaLib_shared)
elseif(ENABLE_STATIC_BUILD)
    add_library(vcaLib INTERFACE)
    target_link_libraries(vcaLib INTERFACE vcaLib_static)
else()
    message(FATAL_ERROR "Both ENABLE_SHARED_BUILD and ENABLE_STATIC_BUILD are OFF - nothing to build.")
endif()

# Install: put both libs (if built) into standard dirs; install headers once
if (TARGET vcaLib_shared)
    install(TARGETS vcaLib_shared
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT libraries
    )
endif()
if (TARGET vcaLib_static)
    install(TARGETS vcaLib_static
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT libraries
    )
endif()
install(FILES ${VCA_LIB_PUBLIC_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT libraries)
