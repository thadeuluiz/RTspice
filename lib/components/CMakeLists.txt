add_library(components src/resistor.cpp src/source.cpp)
target_include_directories(components PUBLIC include/)

target_link_libraries(components PUBLIC circuit)

add_subdirectory(test/)
