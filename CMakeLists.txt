cmake_minimum_required(VERSION 3.2)
project(rtspice LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)

# external packages
find_package(Boost  REQUIRED)
find_package(Qt5    COMPONENTS Core Widgets REQUIRED)
find_package(CUDA   REQUIRED)
find_package(OpenMP REQUIRED)
#find_package(TBB    REQUIRED)
pkg_search_module(Jack REQUIRED jack)

include(CTest)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_subdirectory(third_party/)
add_subdirectory(lib/)
add_subdirectory(test/)

add_executable(rtspice main.cpp)
target_link_libraries(rtspice PUBLIC circuit jack gui Qt5::Widgets)
