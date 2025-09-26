# Include the same directories as the main project
include_directories(${GLOBAL_PROJ_INCLUDE_DIR})

# Compile interface sources to static library
add_library(libfreebsdnet++_interface STATIC
    lib.cpp
    manager.cpp
    statistics.cpp
    base.cpp
    vnet.cpp
    ethernet.cpp
    bridge.cpp
    vlan.cpp
    tunnel.cpp
    vxlan.cpp
    gif.cpp
    tun.cpp
    tap.cpp
    lagg.cpp
    pfsync.cpp
    pflog.cpp
    carp.cpp
    l2vlan.cpp
    wireless.cpp
    epair.cpp
    loopback.cpp
)
