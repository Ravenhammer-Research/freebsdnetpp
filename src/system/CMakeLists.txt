# System module
add_library(libfreebsdnet++_system STATIC
  config.cpp
)

target_include_directories(libfreebsdnet++_system PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(libfreebsdnet++_system PUBLIC
  libfreebsdnet++_types
)
