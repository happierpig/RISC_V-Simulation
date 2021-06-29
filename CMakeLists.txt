cmake_minimum_required(VERSION 3.19)
project(RISC_V)

set(CMAKE_CXX_STANDARD 14)

add_executable(RISC_V main.cpp opReader.hpp parser.hpp program.hpp)