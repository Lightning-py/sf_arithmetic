cmake_minimum_required(VERSION 3.14)  
project(SF_ARITHMETIC C) 

set(CMAKE_C_STANDARD 11)  
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(SF_ARITHMETIC_LIB STATIC src/sf_arithmetic.c)
target_include_directories(SF_ARITHMETIC_LIB PUBLIC include)

enable_testing()
add_subdirectory(tests)
