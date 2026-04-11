add_library(circuit src/circuit.cpp)
set_target_properties(circuit PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(circuit
  PUBLIC
  include/)

target_include_directories(circuit
  SYSTEM PUBLIC
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

if(BUILD_TESTING)
  add_subdirectory(test/)
endif()
