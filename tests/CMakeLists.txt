project(SF_ARITHMETIC_TEST LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(test_base main.cpp)
target_link_libraries(test_base PRIVATE SF_ARITHMETIC_LIB gtest_main)

set_target_properties(SF_ARITHMETIC_LIB PROPERTIES LINKER_LANGUAGE C)

add_test(NAME test_base_ex COMMAND test_base)
