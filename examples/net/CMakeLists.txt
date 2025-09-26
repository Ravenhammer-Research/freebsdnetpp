# Net tool example
cmake_minimum_required(VERSION 3.20)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(READLINE REQUIRED readline)

# Source files
set(NET_SOURCES
    src/main.cpp
    src/net_tool.cpp
    src/interface_commands.cpp
    src/interface_show_commands.cpp
    src/interface_show_bridge_commands.cpp
    src/interface_show_lagg_commands.cpp
    src/interface_show_gif_commands.cpp
    src/interface_show_ethernet_commands.cpp
    src/interface_set_commands.cpp
    src/interface_delete_commands.cpp
    src/route_commands.cpp
    src/help.cpp
    src/utils.cpp
    src/shell.cpp
    src/parser.cpp
)

# Create executable
add_executable(net ${NET_SOURCES})

# Include directories
target_include_directories(net
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${READLINE_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(net
    PRIVATE
        libfreebsdnet++
        ${READLINE_LIBRARIES}
        jail
)

# Link directories
target_link_directories(net
    PRIVATE
        ${READLINE_LIBRARY_DIRS}
)

# Compile definitions
target_compile_definitions(net
    PRIVATE
        ${READLINE_CFLAGS_OTHER}
)

# Set compile options
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(net PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wformat=2
        -Wformat-security
        -Wstack-protector
        -fstack-protector
        -D_FORTIFY_SOURCE=2
        -fPIC
        -fPIE
    )
    
    target_link_options(net PRIVATE
        -Wl,-z,relro
        -Wl,-z,now
    )
endif()

# Set compile features
target_compile_features(net PRIVATE cxx_std_23)

# Install target
install(TARGETS net DESTINATION bin)
