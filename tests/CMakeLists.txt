include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG a05c0915074bcd1b82f232e081da9bb6c205c28d
)
FetchContent_MakeAvailable(googletest)

add_executable(test_base main.cpp regressions.cpp)
target_link_libraries(test_base PRIVATE SF_ARITHMETIC_LIB gtest_main)
add_test(NAME test_base_ex COMMAND test_base)

add_executable(test_allocations allocation_failures.c)
target_include_directories(test_allocations PRIVATE ../include)
# Exercise automatic FFT dispatch failures with small, inexpensive inputs.
target_compile_definitions(test_allocations PRIVATE SF_ARITHMETIC_FFT_THRESHOLD=4)
add_test(NAME allocation_failures COMMAND test_allocations)

find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
    add_executable(arithmetic_driver arithmetic_driver.c)
    target_link_libraries(arithmetic_driver PRIVATE SF_ARITHMETIC_LIB)
    add_test(NAME python_reference
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/reference.py"
                "$<TARGET_FILE:arithmetic_driver>")
endif()
