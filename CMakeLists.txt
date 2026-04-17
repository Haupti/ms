cmake_minimum_required(VERSION 3.13)
project(msl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Enable ccache if available
# This fulfills the requirement for individual file caching.
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

# 2. Dependencies
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# 3. Source file gathering
# We gather all core source files, excluding the main entry point.
file(GLOB_RECURSE SRC_FILES src/*.cpp)
list(REMOVE_ITEM SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# We also include the local utility library files.
file(GLOB_RECURSE LIB_FILES lib/*.cpp)

# 4. Core Library
# This library contains the bulk of the logic and is linked by both the main app and tests.
add_library(msl_core STATIC ${SRC_FILES} ${LIB_FILES})

target_include_directories(msl_core PUBLIC 
    src 
    src/vendor 
    lib
)

target_link_libraries(msl_core PUBLIC 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    Threads::Threads
)

# OpenSSL support for cpp-httplib
target_compile_definitions(msl_core PUBLIC CPPHTTPLIB_OPENSSL_SUPPORT)

# 5. Compiler Flags
if(MSVC)
    target_compile_options(msl_core PRIVATE /W4 /WX)
else()
    # -Wno-pragma-once-outside-header is needed for some vendor headers
    target_compile_options(msl_core PRIVATE -Wall -Wextra -Werror -Wno-pragma-once-outside-header -Wno-deprecated-declarations)
endif()

# Sanitizers for Debug builds (default in asap)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR NOT CMAKE_BUILD_TYPE)
    if(NOT MSVC)
        target_compile_options(msl_core PUBLIC -fsanitize=address -fsanitize=undefined -g)
        target_link_options(msl_core PUBLIC -fsanitize=address -fsanitize=undefined)
    endif()
endif()

# 6. Main Executable
add_executable(msl src/main.cpp)
target_link_libraries(msl PRIVATE msl_core)

# 7. Testing Setup
enable_testing()

# Gather test utilities (shared by all tests)
file(GLOB TEST_UTILS tests/testutil_*.cpp)

# Find all individual test files
file(GLOB TEST_FILES tests/test_*.cpp)

foreach(test_file ${TEST_FILES})
    # Get the filename without extension (e.g., test_parser)
    get_filename_component(test_name ${test_file} NAME_WE)
    
    # Create a separate executable for each test
    add_executable(${test_name} ${test_file} ${TEST_UTILS})
    
    # Link with the core library
    target_link_libraries(${test_name} PRIVATE msl_core)
    
    # Register the test with CTest
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
