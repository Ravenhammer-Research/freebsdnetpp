# Include the same directories as the main project
include_directories(${GLOBAL_PROJ_INCLUDE_DIR})

# Compile ethernet sources to static library
add_library(libfreebsdnet++_ethernet STATIC
    address.cpp
)