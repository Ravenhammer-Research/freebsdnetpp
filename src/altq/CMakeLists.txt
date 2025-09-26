# Include the same directories as the main project
include_directories(${GLOBAL_PROJ_INCLUDE_DIR})

# Compile altq sources to static library
add_library(libfreebsdnet++_altq STATIC
    lib.cpp
    queue.cpp
    scheduler.cpp
)
