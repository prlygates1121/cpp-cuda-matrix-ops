cmake_minimum_required(VERSION 3.28)
project(Project4)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp ")

add_executable(Project4 Matrix.cpp Matrix.h
        test1.cpp
        test2.cpp
        test3.cpp
        test3.cpp
)


