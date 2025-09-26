# Include the same directories as the main project
include_directories(${GLOBAL_PROJ_INCLUDE_DIR})

# Compile vlan sources to static library
add_library(libfreebsdnet++_vlan STATIC
    lib.cpp
    manager.cpp
    interface.cpp
)
