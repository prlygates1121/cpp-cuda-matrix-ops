cmake_minimum_required(VERSION 3.27)
project(Project3 C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -fopenmp")

set(OpenBLAS_DIR "D:/OpenBLAS-0.3.27-x64")

include_directories(${OpenBLAS_DIR}/include)

link_directories(${OpenBLAS_DIR}/lib)

add_executable(Project3 main.c
        operation.h
        operation.c)

target_link_libraries(Project3 libopenblas.dll.a)