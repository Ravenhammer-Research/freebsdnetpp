# CMakeLists.txt for types module

add_library(libfreebsdnet++_types STATIC
  address.cpp
)

target_include_directories(libfreebsdnet++_types PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(libfreebsdnet++_types PUBLIC
  # No external dependencies for now
)