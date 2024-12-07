# Minimum CMake version required
cmake_minimum_required(VERSION 3.14)

# Project name
project(TestStripJsonComments LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add test executable
add_executable(test_strip_json_comments test_strip_json_comments.cpp)

# Include directories
target_include_directories(test_strip_json_comments PRIVATE ../include)

# Add optional flags if needed (adjust based on your platform or compiler)
if(MSVC)
    target_compile_options(test_strip_json_comments PRIVATE /W4 /WX)
else()
    target_compile_options(test_strip_json_comments PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_command(
	TARGET test_strip_json_comments POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/test_input.json
        $<TARGET_FILE_DIR:test_strip_json_comments>/test_input.json
)