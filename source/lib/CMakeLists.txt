cmake_minimum_required(VERSION 3.10)

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

message("ENABLE_ASSEMBLY " ${ENABLE_ASSEMBLY})
if(ENABLE_ASSEMBLY)
    message("Activating nasm assembly")
    enable_language(ASM_NASM)
    set(A_SRCS 
        analyzer/simd/dct8.asm 
        analyzer/simd/const-a.asm
        analyzer/simd/cpu-a.asm)
    
    set(CMAKE_ASM_NASM_FLAGS "-I\"${CMAKE_CURRENT_SOURCE_DIR}/analyzer/simd\" -DARCH_X86_64=1 -DHIGH_BIT_DEPTH=0 -DBIT_DEPTH=8 -DVCA_NS=vca")
endif(ENABLE_ASSEMBLY)

file(GLOB_RECURSE vcaLib_source *.cpp)
file(GLOB_RECURSE vcaLib_header *.h)

include_directories(./)

add_library(vcaLib ${vcaLib_source} ${vcaLib_header} ${A_SRCS})
set_target_properties(vcaLib PROPERTIES OUTPUT_NAME vcaLib)
set_target_properties(vcaLib PROPERTIES PUBLIC_HEADER "vcaLib.h")

install(TARGETS vcaLib LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries PUBLIC_HEADER DESTINATION include)
