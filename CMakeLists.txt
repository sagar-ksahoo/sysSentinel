cmake_minimum_required(VERSION 3.15)
project(SysSentinel CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# Include directories
include_directories(include)

# Create executable
add_executable(SysSentinel 
src/main
src/monitor.cpp
)
