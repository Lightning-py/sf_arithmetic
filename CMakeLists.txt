project(SF_ARITHMETIC C) 

set(CMAKE_C_STANDARD 11)  
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(SF_ARITHMETIC_LIB STATIC src/sf_arithmetic.c)
target_include_directories(SF_ARITHMETIC_LIB PUBLIC include)
target_compile_options(SF_ARITHMETIC_LIB PUBLIC -Wall -Wextra)

option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE)
    # Только для GCC/Clang
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # Флаги для gcov
        add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
        add_link_options(--coverage)
    endif()
endif()

enable_testing()
add_subdirectory(tests)