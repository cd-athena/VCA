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

# set_target_properties can't do list expansion
string(REPLACE ";" " " VERSION_FLAGS "${VFLAGS}")
set_source_files_properties(version.cpp PROPERTIES COMPILE_FLAGS ${VERSION_FLAGS})

if(WIN32)
    set(WINXP winxp.h winxp.cpp)
endif(WIN32)

add_library(common OBJECT
    ${WINXP}
    primitives.cpp primitives.h
    pixel.cpp dct.cpp lowpassdct.cpp
    constants.cpp constants.h
    version.cpp
    threading.cpp threading.h
    threadpool.cpp threadpool.h
    common.cpp common.h)
