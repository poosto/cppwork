cmake_minimum_required(VERSION 3.20)
project(fifo CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # auto-generates compile_commands.json

add_compile_options(-Wall -Wextra -O3)

# Main executable
add_executable(bin main.cpp)

# Tests
add_executable(test_runner
    tests/test_fifo.cpp
    vendor/catch2/catch_amalgamated.cpp
)
target_include_directories(test_runner PRIVATE vendor/catch2)

enable_testing()
add_test(NAME fifo_tests COMMAND test_runner)

target_compile_options(test_runner PRIVATE -fsanitize=thread)
target_link_options(test_runner PRIVATE -fsanitize=thread)
