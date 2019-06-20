add_library(parser INTERFACE)
target_include_directories(parser INTERFACE include/ ${Boost_INCLUDE_DIRS})
target_link_libraries(parser INTERFACE components)

add_subdirectory(test/)
