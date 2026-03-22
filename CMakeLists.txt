cmake_minimum_required(VERSION 3.14)
project(Consolix LANGUAGES CXX)

set(CONSOLIX_CXX_STANDARD "17" CACHE STRING "C++ standard for Consolix consumers, examples, and tests")
set_property(CACHE CONSOLIX_CXX_STANDARD PROPERTY STRINGS 11 14 17)

set(_consolix_supported_standards 11 14 17)
if(NOT CONSOLIX_CXX_STANDARD IN_LIST _consolix_supported_standards)
    message(FATAL_ERROR "CONSOLIX_CXX_STANDARD must be one of: 11, 14, 17")
endif()

option(CONSOLIX_USE_LOGIT "Use LogIt++ logging library" ON)
option(CONSOLIX_USE_JSON "Use nlohmann/json" ON)
option(CONSOLIX_USE_CXXOPTS "Use cxxopts for argument parsing" ON)
option(CONSOLIX_BUILD_EXAMPLES "Build Consolix examples" ON)
option(CONSOLIX_BUILD_TESTS "Build Consolix tests" ON)

set(CMAKE_CXX_STANDARD ${CONSOLIX_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(Consolix INTERFACE)
add_library(Consolix::Consolix ALIAS Consolix)

target_compile_features(Consolix INTERFACE cxx_std_${CONSOLIX_CXX_STANDARD})

target_include_directories(Consolix
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_definitions(Consolix
    INTERFACE
        CONSOLIX_USE_LOGIT=$<BOOL:${CONSOLIX_USE_LOGIT}>
        CONSOLIX_USE_JSON=$<BOOL:${CONSOLIX_USE_JSON}>
        CONSOLIX_USE_CXXOPTS=$<BOOL:${CONSOLIX_USE_CXXOPTS}>
        CONSOLIX_BASE_PATH="${CMAKE_CURRENT_SOURCE_DIR}"
)

if(CONSOLIX_USE_LOGIT)
    target_include_directories(Consolix
        SYSTEM INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/log-it-cpp/include/logit_cpp>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/time-shield-cpp/include>
    )
endif()

if(CONSOLIX_USE_JSON)
    target_include_directories(Consolix
        SYSTEM INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/json/include>
    )
endif()

if(CONSOLIX_USE_CXXOPTS)
    target_include_directories(Consolix
        SYSTEM INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/cxxopts/include>
    )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CONSOLIX_CXX_STANDARD LESS 17)
    target_link_libraries(Consolix INTERFACE stdc++fs)
endif()

function(consolix_apply_common_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

function(consolix_prepare_target target_name)
    target_link_libraries(${target_name} PRIVATE Consolix::Consolix)
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD ${CONSOLIX_CXX_STANDARD}
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
    consolix_apply_common_warnings(${target_name})
endfunction()

function(consolix_add_test target_name source_file)
    add_executable(${target_name} "${source_file}")
    consolix_prepare_target(${target_name})
    add_test(NAME ${target_name} COMMAND ${target_name})
endfunction()

if(CONSOLIX_BUILD_EXAMPLES)
    file(GLOB_RECURSE CONSOLIX_EXAMPLE_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp"
    )

    foreach(example_file IN LISTS CONSOLIX_EXAMPLE_SOURCES)
        get_filename_component(example_name "${example_file}" NAME_WE)

        add_executable(${example_name} "${example_file}")
        consolix_prepare_target(${example_name})

        set(example_config "${CMAKE_CURRENT_SOURCE_DIR}/examples/config.json")
        if(EXISTS "${example_config}")
            add_custom_command(TARGET ${example_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                    "${example_config}"
                    "$<TARGET_FILE_DIR:${example_name}>/config.json"
            )
        endif()
    endforeach()
endif()

if(CONSOLIX_BUILD_TESTS)
    enable_testing()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_strip_json_comments.cpp")
        consolix_add_test(test_strip_json_comments "tests/test_strip_json_comments.cpp")
        target_compile_definitions(test_strip_json_comments
            PRIVATE
                CONSOLIX_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests"
        )
    endif()

    if(UNIX AND NOT WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_posix_signal_shutdown.cpp")
        consolix_add_test(test_posix_signal_shutdown "tests/test_posix_signal_shutdown.cpp")
        target_compile_definitions(test_posix_signal_shutdown PRIVATE CONSOLIX_WAIT_ON_ERROR=0)
        set_tests_properties(test_posix_signal_shutdown PROPERTIES TIMEOUT 15)
    endif()
endif()
