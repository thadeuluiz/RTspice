add_executable(component_parser_test component_parser_test.cpp)
target_link_libraries(component_parser_test
  PRIVATE
    test_main
    components)

