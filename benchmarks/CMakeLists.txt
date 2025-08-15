cmake_minimum_required(VERSION 3.14)

project(SF_ARITHMETIC_TEST LANGUAGES CXX)

add_executable(addition_bench addition_bench.cpp)
target_link_libraries(addition_bench PRIVATE SF_ARITHMETIC_LIB gtest_main)

set_target_properties(SF_ARITHMETIC_LIB PROPERTIES LINKER_LANGUAGE C)