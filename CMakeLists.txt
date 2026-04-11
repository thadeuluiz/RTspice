cmake_minimum_required(VERSION 3.15)
project(rtspice LANGUAGES C CXX)
project(rtspice VERSION 0.1.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# external packages
find_package(Boost  REQUIRED)
find_package(CUDA   REQUIRED)
find_package(OpenMP REQUIRED)

include(CTest)
if(BUILD_TESTING)
  FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
  )
  FetchContent_MakeAvailable(catch2)
  list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
endif()

FetchContent_Declare(
  juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.6
)
FetchContent_MakeAvailable(juce)

add_subdirectory(lib/)
if(BUILD_TESTING)
  add_subdirectory(test/)
endif()
