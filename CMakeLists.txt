cmake_minimum_required(VERSION 3.2)
project(rtspice LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)

# external packages
find_package(CUDA   REQUIRED)
find_package(OpenMP REQUIRED)
pkg_search_module(Jack REQUIRED jack)

include(CTest)

add_subdirectory(third_party/)
add_subdirectory(lib/)
add_subdirectory(test/)

add_executable(rtspice main.cpp)
target_link_libraries(rtspice PUBLIC circuit jack)
