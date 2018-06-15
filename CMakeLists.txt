cmake_minimum_required(VERSION 3.10)
project(DynamicPriorityQueue CXX)

set(CXX clang++)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -Wextra")

add_executable(dynamic_prioirty_queue_test 
        test/dynamic_priority_queue_test.cpp 
        include/dynamic_priority_queue.hpp
        )