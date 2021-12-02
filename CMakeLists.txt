cmake_minimum_required(VERSION 3.16)
project(simpleweb)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

file(GLOB sources "src/*.cpp" "src/*.c")
file(GLOB headers "src/*.hpp" "src/*.h")
    
add_library(${PROJECT_NAME} SHARED ${sources} ${headers})
target_compile_options(${PROJECT_NAME} 
    PUBLIC
        -Wall -Wextra -pedantic
)
target_link_options(${PROJECT_NAME} 
    PUBLIC
        -pthread
)
target_compile_features(${PROJECT_NAME} 
    PUBLIC 
        cxx_std_17
)
target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(job)
add_subdirectory(tests)

message(STATUS "[cmake] build type: ${CMAKE_BUILD_TYPE}")
