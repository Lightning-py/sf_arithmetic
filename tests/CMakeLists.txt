cmake_minimum_required(VERSION 3.14)

project(SF_ARITHMETIC_TEST LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY git@github.com:/google/googletest.git
    GIT_TAG main
)
FetchContent_MakeAvailable(googletest)

add_executable(test_base main.cpp)
target_compile_options(test_base PUBLIC -Wall -Wextra)
target_link_libraries(test_base PRIVATE SF_ARITHMETIC_LIB gtest_main)

set_target_properties(SF_ARITHMETIC_LIB PROPERTIES LINKER_LANGUAGE C)

option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE)
    # Только для GCC/Clang
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # Флаги для gcov
        add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
        add_link_options(--coverage)
    endif()
endif()

add_test(NAME test_base_ex COMMAND test_base)
