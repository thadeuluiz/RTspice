add_executable(circuit_test circuit_test.cpp)
target_link_libraries(
  circuit_test
    PRIVATE
      circuit
      test_main
)
