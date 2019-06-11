add_library(circuit src/circuit.cpp)
target_include_directories(circuit
  PUBLIC
  include/
  ${CUDA_INCLUDE_DIRS}
  ${MAGMA_INCLUDE_DIRS})

target_link_libraries(circuit
  PUBLIC
  components
  ${CUDA_LIBRARIES}
  ${MAGMA_LINK_LIBRARIES})

add_subdirectory(test/)
