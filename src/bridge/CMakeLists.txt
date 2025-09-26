# Include the same directories as the main project
include_directories(${GLOBAL_PROJ_INCLUDE_DIR})

# Compile bridge sources to static library
add_library(libfreebsdnet++_bridge STATIC
    lib.cpp
    manager.cpp
    port.cpp
    stp.cpp
)
