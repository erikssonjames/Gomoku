cmake_minimum_required(VERSION 3.22)
project(gomoku)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-O3")
add_executable(gomoku main.cc gomoku.cc
        Timer.cpp
        Timer.h)
