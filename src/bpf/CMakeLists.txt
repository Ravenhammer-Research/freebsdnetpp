# Include the same directories as the main project
include_directories(${GLOBAL_PROJ_INCLUDE_DIR})

# Compile bpf sources to static library
add_library(libfreebsdnet++_bpf STATIC
    lib.cpp
    filter.cpp
    capture.cpp
)
