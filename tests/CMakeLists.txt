add_executable(ini_config_test "ini_config_test.cpp")
target_link_libraries(ini_config_test PRIVATE ${PROJECT_NAME})

add_executable(test_string_utils "test_string_utils.cpp")
target_link_libraries(test_string_utils PRIVATE ${PROJECT_NAME})

add_executable(epoll_http_server "epoll_http_server.cpp")
target_link_libraries(epoll_http_server PRIVATE ${PROJECT_NAME})
