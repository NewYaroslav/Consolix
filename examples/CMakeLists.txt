# Minimum CMake version required
cmake_minimum_required(VERSION 3.14)

# Project name
project(ConsolixExample LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add test executable
add_executable(example_application_main_loop example_application_main_loop.cpp)

# Define macros for dependencies
target_compile_definitions(example_application_main_loop PRIVATE 
    CONSOLIX_USE_LOGIT=1
    CONSOLIX_USE_JSON=1
    CONSOLIX_USE_CXXOPTS=1
	CONSOLIX_BASE_PATH="${CMAKE_SOURCE_DIR}"
)

include_directories(../libs/log-it-cpp/include)
include_directories(../libs/time-shield-cpp/include)
include_directories(../libs/cxxopts/include)
include_directories(../libs/nlohmann-json/include)

# Include directories
target_include_directories(example_application_main_loop PRIVATE ../include)

# Include LogIt and its dependency if enabled
if(CONSOLIX_USE_LOGIT)
    target_include_directories(example_application_main_loop PRIVATE ../libs/log-it-cpp/include)
    target_include_directories(example_application_main_loop PRIVATE ../libs/time-shield-cpp/include)
endif()

# Include cxxopts if enabled
if(CONSOLIX_USE_CXXOPTS)
	target_include_directories(example_application_main_loop PRIVATE ../libs/cxxopts/include)
endif()

# Include nlohmann/json if enabled
if(CONSOLIX_USE_JSON)
    target_include_directories(example_application_main_loop PRIVATE ../libs/nlohmann-json/include)
endif()

# Add optional flags if needed (adjust based on your platform or compiler)
if(MSVC)
    target_compile_options(example_application_main_loop PRIVATE /W4 /WX)
else()
    target_compile_options(example_application_main_loop PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Copy config.json to the build directory
add_custom_command(
    TARGET example_application_main_loop POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/tests/config.json # Путь к config.json, проверьте!
        $<TARGET_FILE_DIR:example_application_main_loop>/config.json
)

add_custom_target(save_logs
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} > ${CMAKE_BINARY_DIR}/build_log.txt 2>&1
    COMMENT "Building and saving logs to build_log.txt"
)