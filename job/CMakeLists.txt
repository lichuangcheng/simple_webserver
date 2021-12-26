add_executable(html_server_conf "day10/html_server_conf.cpp")
target_link_libraries(html_server_conf PRIVATE ${PROJECT_NAME})

add_executable(http_server_select "day11/http_server_select.cpp")
target_link_libraries(http_server_select PRIVATE ${PROJECT_NAME})

add_executable(http_header_parse "day13/http_header_parse.cpp")
target_link_libraries(http_header_parse PRIVATE ${PROJECT_NAME})

add_executable(http_server_file "day14/http_server_file.cpp")
target_link_libraries(http_server_file PRIVATE ${PROJECT_NAME})

add_executable(http_server_file_event "day14/http_server_file_event.cpp")
target_link_libraries(http_server_file_event PRIVATE ${PROJECT_NAME})

add_executable(tcp_server_event "day14/tcp_server_event.cpp")
target_link_libraries(tcp_server_event PRIVATE ${PROJECT_NAME})