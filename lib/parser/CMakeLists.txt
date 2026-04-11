add_library(parser INTERFACE)

target_include_directories(parser
  INTERFACE
  include/)

target_include_directories(parser
  SYSTEM INTERFACE
  ${Boost_INCLUDE_DIRS})

target_link_libraries(parser
  INTERFACE
  components
  Boost::boost) # for Boost.Spirit

if(BUILD_TESTING)
  add_subdirectory(test/)
endif()
