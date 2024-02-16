# We assemble the same code 3 times with different settings for BIT_DEPTH
# Each compilation will create functions with different names that we can
# then all link to the library

add_library(vcaLibSimd8bit STATIC "")
add_library(vcaLibSimd10bit STATIC "")
add_library(vcaLibSimd12bit STATIC "")

target_include_directories(vcaLibSimd8bit PRIVATE ${LIB_SOURCE_DIR})
target_include_directories(vcaLibSimd10bit PRIVATE ${LIB_SOURCE_DIR})
target_include_directories(vcaLibSimd12bit PRIVATE ${LIB_SOURCE_DIR})

if(BUILD_WITH_NASM)
    enable_language(ASM_NASM)

    target_sources(vcaLibSimd8bit
        PRIVATE
        dct8.asm
        const-a.asm
        cpu-a.asm
        dct-ssse3.cpp
		entropy.cpp
    )
    target_sources(vcaLibSimd10bit
        PRIVATE
        dct8.asm
        const-a.asm
        cpu-a.asm
        dct-ssse3.cpp
		entropy.cpp
    )
    target_sources(vcaLibSimd12bit
        PRIVATE
        dct8.asm
        const-a.asm
        cpu-a.asm
        dct-ssse3.cpp
		entropy.cpp
    )

    set_property(TARGET vcaLibSimd8bit PROPERTY COMPILE_FLAGS -DBIT_DEPTH=8)
    set_property(TARGET vcaLibSimd10bit PROPERTY COMPILE_FLAGS -DBIT_DEPTH=10)
    set_property(TARGET vcaLibSimd12bit PROPERTY COMPILE_FLAGS -DBIT_DEPTH=12)

    if(APPLE)
        set(CMAKE_ASM_NASM_FLAGS "-I\"${CMAKE_CURRENT_SOURCE_DIR}/\" -DPIC -DARCH_X86_64=1 -DPREFIX -DVCA_NS=vca")
    else()
        set(CMAKE_ASM_NASM_FLAGS "-I\"${CMAKE_CURRENT_SOURCE_DIR}/\" -DPIC -DARCH_X86_64=1 -DVCA_NS=vca")
    endif()

    if(GCC)
        set_source_files_properties(dct-ssse3.cpp PROPERTIES COMPILE_FLAGS "-mssse3")
		set_source_files_properties(entropy.cpp PROPERTIES COMPILE_FLAGS "-mssse3")
    endif(GCC)
else()
    target_sources(vcaLibSimd8bit
        PRIVATE
        noAsmImpl8bit.cpp
    )
    target_sources(vcaLibSimd10bit
        PRIVATE
        noAsmImpl10bit.cpp
    )
    target_sources(vcaLibSimd12bit
        PRIVATE
        noAsmImpl12bit.cpp
    )
endif(BUILD_WITH_NASM)
