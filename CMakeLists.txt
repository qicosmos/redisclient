cmake_minimum_required(VERSION 3.2)
project(redisclient)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++17")
set(SOURCE_FILES main.cpp main.cpp redisclient.hpp)

INCLUDE_DIRECTORIES("/usr/include/hiredis")

add_executable(redisclient ${SOURCE_FILES})
target_link_libraries(redisclient hiredis)