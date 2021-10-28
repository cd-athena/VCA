# vim: syntax=cmake

list(APPEND VFLAGS "-DVCA_VERSION=${VCA_VERSION}")
if(EXTRA_LIB)
    if(LINKED_8BIT)
        list(APPEND VFLAGS "-DLINKED_8BIT=1")
    endif(LINKED_8BIT)
    if(LINKED_10BIT)
        list(APPEND VFLAGS "-DLINKED_10BIT=1")
    endif(LINKED_10BIT)
    if(LINKED_12BIT)
        list(APPEND VFLAGS "-DLINKED_12BIT=1")
    endif(LINKED_12BIT)
endif(EXTRA_LIB)

if(ENABLE_ASSEMBLY)
    set_source_files_properties(threading.cpp primitives.cpp PROPERTIES COMPILE_FLAGS -DENABLE_ASSEMBLY=1)
    list(APPEND VFLAGS "-DENABLE_ASSEMBLY=1")
endif(ENABLE_ASSEMBLY)

if(ENABLE_ASSEMBLY AND X86)
    set(SSE3  vec/dct-sse3.cpp)
    set(SSSE3 vec/dct-ssse3.cpp)

    if(MSVC)
        set(PRIMITIVES ${SSE3} ${SSSE3})
        set(WARNDISABLE "/wd4100") # unreferenced formal parameter
        if(INTEL_CXX)
            add_definitions(/Qwd111) # statement is unreachable
            add_definitions(/Qwd128) # loop is unreachable
            add_definitions(/Qwd177) # declared function is unused
            add_definitions(/Qwd185) # dynamic initialization in unreachable code
            add_definitions(/Qwd280) # conditional expression is constant
        endif()
        if(X64)
            set_source_files_properties(${SSE3} ${SSSE3} PROPERTIES COMPILE_FLAGS "${WARNDISABLE}")
        else()
            # x64 implies SSE4, so only add /arch:SSE2 if building for Win32
            set_source_files_properties(${SSE3} ${SSSE3} PROPERTIES COMPILE_FLAGS "${WARNDISABLE} /arch:SSE2")
        endif()
    endif()
    if(GCC)
        if(CLANG)
            # llvm intrinsic headers cause shadow warnings
            set(WARNDISABLE "-Wno-shadow -Wno-unused-parameter")
        else()
            set(WARNDISABLE "-Wno-unused-parameter")
        endif()
        if(INTEL_CXX OR CLANG OR (NOT CC_VERSION VERSION_LESS 4.3))
            set(PRIMITIVES ${SSE3} ${SSSE3})
            set_source_files_properties(${SSE3}  PROPERTIES COMPILE_FLAGS "${WARNDISABLE} -msse3")
            set_source_files_properties(${SSSE3} PROPERTIES COMPILE_FLAGS "${WARNDISABLE} -mssse3")
        endif()
    endif()
    set(VEC_PRIMITIVES vec/vec-primitives.cpp ${PRIMITIVES})
    source_group(Intrinsics FILES ${VEC_PRIMITIVES})
	
	set(C_SRCS asm-primitives.cpp dct8.h)
    set(A_SRCS const-a.asm cpu-a.asm dct8.asm)
	
	if(MSVC_IDE OR XCODE)
    # MSVC requires custom build rules in the main cmake script for nasm
        set(MSVC_ASMS "${A_SRCS}" CACHE INTERNAL "nasm sources")
        set(A_SRCS)
    endif()

    enable_language(ASM_NASM)

    foreach(SRC ${A_SRCS} ${C_SRCS})
        set(ASM_PRIMITIVES ${ASM_PRIMITIVES} x86/${SRC})
    endforeach()
    source_group(Assembly FILES ${ASM_PRIMITIVES})
endif(ENABLE_ASSEMBLY AND X86)

# set_target_properties can't do list expansion
string(REPLACE ";" " " VERSION_FLAGS "${VFLAGS}")
set_source_files_properties(version.cpp PROPERTIES COMPILE_FLAGS ${VERSION_FLAGS})

if(GCC AND CC_HAS_NO_NARROWING)
    set_source_files_properties(cpu.cpp PROPERTIES COMPILE_FLAGS -Wno-narrowing)
endif()

if(WIN32)
    set(WINXP winxp.h winxp.cpp)
endif(WIN32)

add_library(common OBJECT
    ${ASM_PRIMITIVES} ${VEC_PRIMITIVES} ${WINXP}
    primitives.cpp primitives.h
    pixel.cpp dct.cpp lowpassdct.cpp
    constants.cpp constants.h
    cpu.cpp cpu.h version.cpp
    threading.cpp threading.h
    threadpool.cpp threadpool.h
    common.cpp common.h)
