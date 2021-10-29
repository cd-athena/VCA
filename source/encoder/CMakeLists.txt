# vim: syntax=cmake

if(GCC)
    add_definitions(-Wno-uninitialized)
    if(CC_HAS_NO_STRICT_OVERFLOW)
        # GCC 4.9.2 gives warnings we know we can ignore in this file
        set_source_files_properties(slicetype.cpp PROPERTIES COMPILE_FLAGS -Wno-strict-overflow)
    endif(CC_HAS_NO_STRICT_OVERFLOW)
endif()
if(MSVC)
   add_definitions(/wd4701) # potentially uninitialized local variable 'foo' used
endif()

check_symbol_exists(strtok_r "string.h" HAVE_STRTOK_R)
if(HAVE_STRTOK_R)
    set_source_files_properties(encoder.cpp PROPERTIES COMPILE_FLAGS -DHAVE_STRTOK_R=1)
endif()

add_library(encoder OBJECT ../vca.h
    encoder.cpp encoder.h)
