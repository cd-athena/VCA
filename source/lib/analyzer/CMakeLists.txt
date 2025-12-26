cmake_minimum_required(VERSION 3.13)

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

if(ENABLE_NASM AND DEFINED VCA_ARCH_X86)
    enable_language(ASM_NASM)

    if(CMAKE_ASM_NASM_COMPILER_LOADED)
        message(STATUS "Nasm found. Activating nasm assembly for x86.")
        set(BUILD_WITH_NASM 1 PARENT_SCOPE)
        set(BUILD_WITH_NASM 1)
    else()
        message(STATUS "Nasm could not be found. Disabling nasm assembly.")
    endif(CMAKE_ASM_NASM_COMPILER_LOADED)

    set_source_files_properties(simd/cpu.cpp PROPERTIES COMPILE_FLAGS -DENABLE_NASM=1)
elseif(DEFINED VCA_ARCH_ARM)
    message(STATUS "ARM architecture detected. Using NEON SIMD instead of NASM.")
else()
    message(STATUS "NASM disabled or not supported on this platform.")
endif()

target_include_directories(vcaInternal PRIVATE ${LIB_SOURCE_DIR})

add_subdirectory(simd)

target_link_libraries(vcaInternal Threads::Threads vcaLibSimd8bit vcaLibSimd10bit vcaLibSimd12bit)
