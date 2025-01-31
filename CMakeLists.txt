cmake_minimum_required(VERSION 3.10)
project(MultiProcessCounter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(initiator initiator.cpp)
add_executable(receiver receiver.cpp)

target_link_libraries(initiator rt)
target_link_libraries(receiver rt)

target_link_libraries(initiator pthread)
target_link_libraries(receiver pthread)