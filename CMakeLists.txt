cmake_minimum_required(VERSION 3.6)
project(memory_pool)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
aux_source_directory(. SRC_LIST)

add_executable(memory_pool ${SRC_LIST})