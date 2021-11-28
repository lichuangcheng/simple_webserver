cmake_minimum_required(VERSION 3.15)
project(webserver)
    
add_executable(tcp_server "tcp_server.c")
target_compile_options(tcp_server PUBLIC -Wall -Wextra -pedantic)
target_link_libraries(tcp_server PUBLIC -lpthread)

message(STATUS "[cmake] build type: ${CMAKE_BUILD_TYPE}")