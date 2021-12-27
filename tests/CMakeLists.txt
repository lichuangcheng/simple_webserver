add_executable(ini_config_test "ini_config_test.cpp")
target_link_libraries(ini_config_test PRIVATE ${PROJECT_NAME})

add_executable(test_string_utils "test_string_utils.cpp")
target_link_libraries(test_string_utils PRIVATE ${PROJECT_NAME})

add_executable(tcp_server_event "tcp_server_event.cpp")
target_link_libraries(tcp_server_event PRIVATE ${PROJECT_NAME})

add_executable(test_http_request "test_http_request.cpp")
target_link_libraries(test_http_request PRIVATE ${PROJECT_NAME})
