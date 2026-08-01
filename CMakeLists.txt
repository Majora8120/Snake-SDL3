cmake_minimum_required(VERSION 3.31)
project(Snake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_executable(Snake src/main.cpp)

find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(Snake PRIVATE SDL3::SDL3)