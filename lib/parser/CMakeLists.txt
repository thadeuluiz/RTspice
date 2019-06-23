add_library(parser INTERFACE)

target_include_directories(parser
  INTERFACE
  include/)

target_link_libraries(parser
  INTERFACE
  components
  Boost::boost) # for Boost.Spirit

add_subdirectory(test/)
