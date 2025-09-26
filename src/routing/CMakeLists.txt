# Include the same directories as the main project
include_directories(${GLOBAL_PROJ_INCLUDE_DIR})

# Compile routing sources to static library
add_library(libfreebsdnet++_routing STATIC
    lib.cpp
    table.cpp
    entry.cpp
)
