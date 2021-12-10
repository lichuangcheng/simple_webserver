add_executable(html_server_conf "day10/html_server_conf.cpp")
target_link_libraries(html_server_conf PRIVATE ${PROJECT_NAME})

add_executable(http_server_select "day11/http_server_select.cpp")
target_link_libraries(http_server_select PRIVATE ${PROJECT_NAME})

add_executable(http_header_parse "day13/http_header_parse.cpp")
target_link_libraries(http_header_parse PRIVATE ${PROJECT_NAME})
