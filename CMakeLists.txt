cmake_minimum_required(VERSION 3.10)
project(MultiProcessCounter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(pingpongcounter pingpongcounter.cpp)
