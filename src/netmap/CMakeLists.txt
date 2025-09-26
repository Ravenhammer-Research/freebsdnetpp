# Include the same directories as the main project
include_directories(${GLOBAL_PROJ_INCLUDE_DIR})

# Compile netmap sources to static library
add_library(libfreebsdnet++_netmap STATIC
    lib.cpp
    ring.cpp
    interface.cpp
)
