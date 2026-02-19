# We assemble the same code 3 times with different settings for BIT_DEPTH
# Each compilation will create functions with different names that we can
# then all link to the library

# If SIMD is disabled at configure time, skip creating these targets
if (NOT SIMD_ENABLED)
  message(STATUS "SIMD: disabled -> skipping simd library targets")
  return()
endif()


# Helper to avoid repetition
function(vca_add_simd_lib target_name bitdepth)
  add_library(${target_name} STATIC "")
  set_target_properties(${target_name} PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_include_directories(${target_name} PRIVATE ${LIB_SOURCE_DIR})

  # Pass BIT_DEPTH to C/C++ compilation
  target_compile_definitions(${target_name} PRIVATE BIT_DEPTH=${bitdepth})

  if (VCA_ARCH_X86 AND NASM_ENABLED)
    # --- x86 + NASM (or equivalent) path ---

    # Where your x86inc.asm / x86util.asm live:
    # If they are in a subfolder (e.g., ${CMAKE_CURRENT_SOURCE_DIR}/x86/), change this:
    set(NASM_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

    # Check macro headers; if missing, compile without ASM (use C++ intrinsics only)
    if (NOT EXISTS "${NASM_INCLUDE_DIR}/x86inc.asm" OR
        NOT EXISTS "${NASM_INCLUDE_DIR}/x86util.asm")
      message(WARNING
        "NASM macros (x86inc.asm/x86util.asm) not found in ${NASM_INCLUDE_DIR}; "
        "building ${target_name} without NASM ASM files (C++ intrinsics only).")
      set(HAVE_X86_MACROS OFF)
    else()
      set(HAVE_X86_MACROS ON)
    endif()

    # Common NASM -D defines expected by the x86 macro headers
    set(NASM_DEFS -DARCH_X86_64=1 -DVCA_NS=vca -DPIC)
    if (APPLE)
      list(APPEND NASM_DEFS -DPREFIX)
    endif()
    if (WIN32)
      list(APPEND NASM_DEFS -DWIN64)
    endif()

    # Object format per platform (important on Windows)
    if (WIN32)
      set(NASM_FORMAT "-f" "win64")
    elseif (APPLE)
      set(NASM_FORMAT "-f" "macho64")
    else()
      set(NASM_FORMAT "-f" "elf64")
    endif()

    # Add C++ SIMD sources (available regardless of NASM macros)
    target_sources(${target_name}
      PRIVATE
        dct-ssse3.cpp
        entropy.cpp
    )

    # Add NASM sources only if the macros exist
    if (HAVE_X86_MACROS)
      target_sources(${target_name}
        PRIVATE
          dct8.asm
          const-a.asm
          cpu-a.asm
      )
    endif()

    # Apply NASM compile options only to ASM_NASM sources in this target
    #  - include path
    #  - format (-f win64/macho64/elf64)
    #  - required -D defines
    #  - per-target BIT_DEPTH to NASM as well
    target_compile_options(${target_name} PRIVATE
      $<$<COMPILE_LANGUAGE:ASM_NASM>:-I${NASM_INCLUDE_DIR}>
      $<$<COMPILE_LANGUAGE:ASM_NASM>:${NASM_FORMAT}>
      $<$<COMPILE_LANGUAGE:ASM_NASM>:${NASM_DEFS}>
      $<$<COMPILE_LANGUAGE:ASM_NASM>:-DBIT_DEPTH=${bitdepth}>
    )

    # GCC/Clang need -mssse3 for the .cpp sources; MSVC does not
    if (GCC)
      set_source_files_properties(dct-ssse3.cpp PROPERTIES COMPILE_FLAGS "-mssse3")
      set_source_files_properties(entropy.cpp   PROPERTIES COMPILE_FLAGS "-mssse3")
    endif()

    # Optionally, if we want to be explicit that these files are NASM:
    # set_source_files_properties(dct8.asm const-a.asm cpu-a.asm PROPERTIES LANGUAGE ASM_NASM)

  elseif (VCA_ARCH_ARM)
    # --- ARM NEON intrinsics path (no assembler required) ---
    message(STATUS "SIMD: ${target_name} -> building ARM NEON intrinsics")
    target_sources(${target_name}
      PRIVATE
        arm/dct-neon.cpp
        arm/entropy-neon.cpp
    )

    # If we need extra NEON flags for GCC/Clang, we can add them here:
    # target_compile_options(${target_name} PRIVATE -mfpu=neon)

  else()
    # Defensive fallback (shouldn’t happen if parent gates SIMD correctly)
    message(WARNING
      "SIMD: unknown arch or assembler missing; using scalar no-ASM fallback for ${target_name}")
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
