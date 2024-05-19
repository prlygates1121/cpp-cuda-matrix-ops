cmake_minimum_required(VERSION 3.28)

project(Project4)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -march=native")

add_executable(Project4 matrix.h
        matmul_performance_test.cpp
        matmul_correctness_test.cpp
        test.cpp
        matrix.tpp
)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()


target_link_libraries(Project4 GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(Project4)
