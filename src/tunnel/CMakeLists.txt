# Include the same directories as the main project
include_directories(${GLOBAL_PROJ_INCLUDE_DIR})

# Compile tunnel sources to static library
add_library(libfreebsdnet++_tunnel STATIC
    lib.cpp
    manager.cpp
)
