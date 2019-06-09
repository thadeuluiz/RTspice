cmake_minimum_required(VERSION 3.2)
project(rtspice LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)

add_subdirectory(third_party/)
add_subdirectory(src/)
add_subdirectory(test/)
