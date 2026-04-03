cmake_minimum_required(VERSION 3.15...4.0.0)

project(memory_allocation LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(mem_allocation src/main.cpp)