cmake_minimum_required(VERSION 3.10)

message("ENABLE_ASSEMBLY " ${ENABLE_ASSEMBLY})
if(ENABLE_ASSEMBLY)
    message("Activating nasm assembly")
    enable_language(ASM_NASM)
    set(A_SRCS analyzer/simd/dct8.asm analyzer/simd/const-a.asm)
    
    set(CMAKE_ASM_NASM_FLAGS "-I\"${CMAKE_CURRENT_SOURCE_DIR}/analyzer/simd\" -DARCH_X86_64=1 -DHIGH_BIT_DEPTH=0 -DBIT_DEPTH=8 -DVCA_NS=vca")
endif(ENABLE_ASSEMBLY)

file(GLOB_RECURSE vcaLib_source *.cpp)
file(GLOB_RECURSE vcaLib_header *.h)

include_directories(./)

add_library(vcaLib ${vcaLib_source} ${vcaLib_header} ${A_SRCS})
set_target_properties(vcaLib PROPERTIES OUTPUT_NAME vcaLib)
set_target_properties(vcaLib PROPERTIES PUBLIC_HEADER "vcaLib.h")

install(TARGETS vcaLib LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries PUBLIC_HEADER DESTINATION include)
