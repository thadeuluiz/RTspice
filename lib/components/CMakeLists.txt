add_library(components INTERFACE)

target_include_directories(components
  INTERFACE
  include/)

target_link_libraries(components
  INTERFACE
  circuit)


add_subdirectory(test/)
