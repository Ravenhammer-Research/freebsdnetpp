cmake_minimum_required(VERSION 3.31.6)

project(libfreebsdnet++ VERSION 1.0.0 LANGUAGES CXX)

# Set C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Prefer clang++ if available
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Using Clang compiler")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Using GCC compiler")
endif()

# Enable warnings as errors
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

# Compiler hardening options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat=2 -Wformat-security")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstack-protector")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fPIE")

# Linker hardening options
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro -Wl,-z,now")

# Define global include directory
set(GLOBAL_PROJ_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${GLOBAL_PROJ_INCLUDE_DIR})

# Include subdirectories
add_subdirectory(src/interface)
add_subdirectory(src/bpf)
add_subdirectory(src/routing)
add_subdirectory(src/altq)
add_subdirectory(src/ethernet)
add_subdirectory(src/netmap)
add_subdirectory(src/bridge)
add_subdirectory(src/vlan)
add_subdirectory(src/tunnel)
add_subdirectory(src/media)
add_subdirectory(src/netlink)
add_subdirectory(src/types)
add_subdirectory(src/system)

# The subdirectories already create the individual libraries

# Create combined interface library
add_library(libfreebsdnet++ INTERFACE)
target_link_libraries(libfreebsdnet++ INTERFACE
    libfreebsdnet++_interface
    libfreebsdnet++_bpf
    libfreebsdnet++_routing
    libfreebsdnet++_altq
    libfreebsdnet++_ethernet
    libfreebsdnet++_netmap
    libfreebsdnet++_bridge
    libfreebsdnet++_vlan
    libfreebsdnet++_tunnel
    libfreebsdnet++_media
    libfreebsdnet++_netlink
    libfreebsdnet++_types
    libfreebsdnet++_system
)

# Create example executables (optional)
option(BUILD_EXAMPLE "Build example program" OFF)
option(BUILD_NET_TOOL "Build net command-line tool" ON)

if(BUILD_EXAMPLE)
    add_executable(example example.cpp)
    target_link_libraries(example libfreebsdnet++)
endif()

if(BUILD_NET_TOOL)
    add_subdirectory(examples/net)
endif()

# Check if .clang-format exists
if(EXISTS "${CMAKE_SOURCE_DIR}/.clang-format")
    find_program(CLANG_FORMAT clang-format)
    
    if(CLANG_FORMAT)
        # clang-format target
        add_custom_target(clang-format
            COMMAND find . -name "*.cpp" -o -name "*.hpp" | xargs ${CLANG_FORMAT} -i
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Formatting code with clang-format"
        )
        
        # clang-format-check target (preview only)
        add_custom_target(clang-format-check
            COMMAND find . -name "*.cpp" -o -name "*.hpp" | xargs ${CLANG_FORMAT}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Checking code formatting with clang-format"
        )
    else()
        message(WARNING "clang-format not found - formatting targets not available")
    endif()
else()
    message(WARNING ".clang-format file not found - formatting targets not available")
endif()
