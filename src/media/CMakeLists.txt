# Include the same directories as the main project
include_directories(${GLOBAL_PROJ_INCLUDE_DIR})

# Compile media sources to static library
add_library(libfreebsdnet++_media STATIC
    lib.cpp
    manager.cpp
)
