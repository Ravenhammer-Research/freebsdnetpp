# Netlink module
set(NETLINK_SOURCES
    manager.cpp
)

add_library(libfreebsdnet++_netlink STATIC ${NETLINK_SOURCES})

target_include_directories(libfreebsdnet++_netlink
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(libfreebsdnet++_netlink
    PRIVATE
        pthread
)

# Set compile options
target_compile_options(libfreebsdnet++_netlink PRIVATE ${COMPILE_OPTIONS})

# Set compile features
target_compile_features(libfreebsdnet++_netlink PRIVATE cxx_std_23)
