# We assemble the same code 3 times with different settings for BIT_DEPTH
# Each compilation will create functions with different names that we can
# then all link to the library

if (NOT SIMD_ENABLED)
    message(STATUS "SIMD: disabled: Skipping simd library targets")
    return()
endif()

# Helper to avoid repetition
function(vca_add_simd_lib target_name bitdepth)
    add_library(${target_name} STATIC "")
    set_target_properties(${target_name} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_include_directories(${target_name} PRIVATE ${LIB_SOURCE_DIR})
    target_compile_definitions(${target_name} PRIVATE BIT_DEPTH=${bitdepth})

    if (VCA_ARCH_X86 AND NASM_ENABLED)
        # x86 + NASM path
        # (ASM_NASM should already be enabled by the parent; do not error if called again)
        enable_language(ASM_NASM)

        # Common NASM flags
        if (APPLE)
            # Prefix is required on macOS; keep ARCH and namespace defines
            set(CMAKE_ASM_NASM_FLAGS
                "${CMAKE_ASM_NASM_FLAGS} -I\"${CMAKE_CURRENT_SOURCE_DIR}/\" -DPIC -DARCH_X86_64=1 -DPREFIX -DVCA_NS=vca")
        else()
            set(CMAKE_ASM_NASM_FLAGS
                "${CMAKE_ASM_NASM_FLAGS} -I\"${CMAKE_CURRENT_SOURCE_DIR}/\" -DPIC -DARCH_X86_64=1 -DVCA_NS=vca")
        endif()

        target_sources(${target_name}
            PRIVATE
                dct8.asm
                const-a.asm
                cpu-a.asm
                dct-ssse3.cpp
                entropy.cpp
        )

        # GCC/Clang need -mssse3 for the .cpp sources; MSVC does not understand this flag
        if (GCC)
            set_source_files_properties(dct-ssse3.cpp PROPERTIES COMPILE_FLAGS "-mssse3")
            set_source_files_properties(entropy.cpp   PROPERTIES COMPILE_FLAGS "-mssse3")
        endif()

    elseif (VCA_ARCH_ARM)
        # ARM NEON intrinsics path (no assembler required if your code is intrinsics-only)
        message(STATUS "SIMD: ${target_name} -> building ARM NEON intrinsics")
        target_sources(${target_name}
            PRIVATE
                arm/dct-neon.cpp
                arm/entropy-neon.cpp
        )

        # If you need extra NEON flags for GCC/Clang, you can add them here:
        # target_compile_options(${target_name} PRIVATE -mfpu=neon)

    else()
        # Defensive fallback (should not normally happen because parent only adds this dir when SIMD_ENABLED=ON)
        message(WARNING "SIMD: unknown arch or assembler missing; using scalar no-ASM fallback for ${target_name}")
        if (bitdepth EQUAL 8)
            target_sources(${target_name} PRIVATE noAsmImpl8bit.cpp)
        elseif (bitdepth EQUAL 10)
            target_sources(${target_name} PRIVATE noAsmImpl10bit.cpp)
        elseif (bitdepth EQUAL 12)
            target_sources(${target_name} PRIVATE noAsmImpl12bit.cpp)
        endif()
    endif()
endfunction()

# Create the three bit-depth variants
vca_add_simd_lib(vcaLibSimd8bit  8)
vca_add_simd_lib(vcaLibSimd10bit 10)
vca_add_simd_lib(vcaLibSimd12bit 12)

