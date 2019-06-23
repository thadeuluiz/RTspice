add_library(circuit src/circuit.cpp)

target_include_directories(circuit
  PUBLIC
  include/
  ${CUDA_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS})

target_link_libraries(circuit
  PUBLIC
  components
  ${CUDA_LIBRARIES}
  ${CUDA_cusparse_LIBRARY}
  ${CUDA_cusolver_LIBRARY}
  ${TBB_LIBRARIES}
  OpenMP::OpenMP_CXX)

add_subdirectory(test/)
