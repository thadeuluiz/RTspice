add_executable(line_parser_test line_parser_test.cpp)
target_link_libraries(line_parser_test PRIVATE parser test_main)

add_executable(component_parser_test component_parser_test.cpp)
target_link_libraries(component_parser_test PRIVATE parser test_main)
