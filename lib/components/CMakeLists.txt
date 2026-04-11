add_library(components INTERFACE)

target_include_directories(components
  INTERFACE
  include/)

target_link_libraries(components
  INTERFACE
  circuit)


if(BUILD_TESTING)
  add_subdirectory(test/)
endif()
