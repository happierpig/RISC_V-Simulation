cmake_minimum_required(VERSION 3.16)
project(code)

set(CMAKE_CXX_STANDARD 14)

add_executable(code main.cpp opReader.hpp parser.hpp program.hpp highLevelBranchPredictor.hpp)