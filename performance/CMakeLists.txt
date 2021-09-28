cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

project(performance VERSION 1.0)
add_executable(performance performance.cpp)

find_package(SEAL)
target_link_libraries(performance SEAL::seal)
