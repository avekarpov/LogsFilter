cmake_minimum_required(VERSION 3.19)
project(LogsFilter)

set(CMAKE_CXX_STANDARD 17)

add_executable(LogsFilter main.cpp ThreadPool.cpp ThreadPool.h)