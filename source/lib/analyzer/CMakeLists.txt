cmake_minimum_required(VERSION 3.13)

# Library sources (scalar + shared headers)
add_library(vcaInternal STATIC
    common/common.h
    common/EnumMapper.h
    Analyzer.h
    Analyzer.cpp
    DCTTransform.h
    DCTTransform.cpp
    DCTTransformsNative.h
    DCTTransformsNative.cpp
    EnergyCalculation.h
    EnergyCalculation.cpp
    EntropyNative.h
    EntropyNative.cpp
    EntropyCalculation.h
    EntropyCalculation.cpp
    MultiThreadQueue.h
    MultiThreadQueue.cpp
    ProcessingThread.h
    ProcessingThread.cpp
    ShotDetection.h
    ShotDetection.cpp
    simd/cpu.h
    simd/cpu.cpp
    simd/dct8.h
    simd/entropy.h
)

target_include_directories(vcaInternal PRIVATE ${LIB_SOURCE_DIR})

# SIMD / Assembler detection

set(SIMD_ENABLED OFF)
set(NASM_ENABLED OFF)

if (ENABLE_SIMD)
    # x86 / x64 NASM path
    if (DEFINED VCA_ARCH_X86 AND VCA_ARCH_X86)
        if (ENABLE_NASM)
            include(CheckLanguage)
            check_language(ASM_NASM)
            if (CMAKE_ASM_NASM_COMPILER)
                enable_language(ASM_NASM)
                set(NASM_ENABLED ON)
                set(SIMD_ENABLED ON)
                message(STATUS "SIMD: NASM found- Enabling x86 assembly (${CMAKE_ASM_NASM_COMPILER})")
                # Optional: mark that we build with NASM
                target_compile_definitions(vcaInternal PRIVATE ENABLE_NASM=1)
            else()
                message(WARNING "SIMD: NASM not found- Disabling SIMD on x86")
            endif()
        else()
            message(STATUS "SIMD: NASM explicitly disabled by user (-DENABLE_NASM=OFF)")
        endif()

    # ARM / NEON path (no assembler needed if intrinsics-only)
    elseif (DEFINED VCA_ARCH_ARM AND VCA_ARCH_ARM)
        if (VCA_ARM_INTRINSICS_ONLY)
            set(SIMD_ENABLED ON)
            message(STATUS "SIMD: ARM architecture detected- Enabling NEON intrinsics (no assembler required)")
        else()
            # If you actually have .S files, require generic ASM
            include(CheckLanguage)
            check_language(ASM)
            if (CMAKE_ASM_COMPILER)
                enable_language(ASM)
                set(SIMD_ENABLED ON)
                message(STATUS "SIMD: ARM assembler detected- Enabling ARM assembly (${CMAKE_ASM_COMPILER})")
            else()
                message(WARNING "SIMD: no ARM assembler found- Disabling SIMD on ARM")
            endif()
        endif()

    else()
        message(STATUS "SIMD: Unknown/unsupported arch for SIMD - leaving disabled")
    endif()
else()
    message(STATUS "SIMD: Disabled by user (-DENABLE_SIMD=OFF)")
endif()


# Propagate SIMD off define
if (NOT SIMD_ENABLED)
    target_compile_definitions(vcaInternal PRIVATE VCA_DISABLE_SIMD=1)
    message(STATUS "SIMD: Disabled- compiling scalar fallback (VCA_DISABLE_SIMD=1)")
endif()


# Add SIMD subdir and link only if enabled
if (SIMD_ENABLED)
    add_subdirectory(simd)

    # Link SIMD libs only when they exist
    if (TARGET vcaLibSimd8bit OR TARGET vcaLibSimd10bit OR TARGET vcaLibSimd12bit)
        target_link_libraries(vcaInternal
            Threads::Threads
            $<$<TARGET_EXISTS:vcaLibSimd8bit>:vcaLibSimd8bit>
            $<$<TARGET_EXISTS:vcaLibSimd10bit>:vcaLibSimd10bit>
            $<$<TARGET_EXISTS:vcaLibSimd12bit>:vcaLibSimd12bit>
        )
    else()
        # Fallback: Still link Threads; no SIMD libs
        target_link_libraries(vcaInternal Threads::Threads)
    endif()
else()
    # SIMD disabled: Don't add simd/, don't link simd libs
    target_link_libraries(vcaInternal Threads::Threads)
endif()

