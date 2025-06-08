cmake_minimum_required(VERSION 3.12)
project(OrderBookEngine VERSION 1.0.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags for optimization
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(src)

# Source files
set(SOURCES
    src/order_book.cpp
    src/parser.cpp
    src/logger.cpp
)

# Main executable
add_executable(order_engine src/main.cpp ${SOURCES})
target_link_libraries(order_engine Threads::Threads)

# Test executable
enable_testing()
add_executable(test_order_book tests/test_order_book.cpp ${SOURCES})
target_link_libraries(test_order_book Threads::Threads)
add_test(NAME OrderBookTests COMMAND test_order_book)

# Create run script
file(WRITE ${CMAKE_BINARY_DIR}/run_demo.sh 
"#!/bin/bash\n"
"echo 'Starting Order Book Engine Demo...'\n"
"echo 'Connect via: telnet localhost 8080'\n"
"echo 'Send JSON orders or use console commands'\n"
"./order_engine 8080\n"
)

execute_process(COMMAND chmod +x ${CMAKE_BINARY_DIR}/run_demo.sh)
