cmake_minimum_required(VERSION 3.20)
project(coroutines)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0")

add_executable(coroutines main.cpp)
