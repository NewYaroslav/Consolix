cmake_minimum_required(VERSION 3.14)
project(ConsolixExamples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# === Опции (можно переопределить через -D)
option(CONSOLIX_USE_LOGIT "Use LogIt++ logging library" ON)
option(CONSOLIX_USE_JSON "Use nlohmann/json" ON)
option(CONSOLIX_USE_CXXOPTS "Use cxxopts for argument parsing" ON)

# === Базовые include
set(CONSOLIX_BASE_PATH "${CMAKE_SOURCE_DIR}")
include_directories(${CMAKE_SOURCE_DIR}/include)

# === Добавляем директории зависимостей
if(CONSOLIX_USE_LOGIT)
    include_directories(${CMAKE_SOURCE_DIR}/libs/log-it-cpp/include/logit_cpp)
    include_directories(${CMAKE_SOURCE_DIR}/libs/time-shield-cpp/include/time_shield_cpp)
endif()

if(CONSOLIX_USE_JSON)
    include_directories(${CMAKE_SOURCE_DIR}/libs/json/include)
endif()

if(CONSOLIX_USE_CXXOPTS)
    include_directories(${CMAKE_SOURCE_DIR}/libs/cxxopts/include)
endif()

# === Собираем все .cpp файлы из examples/
file(GLOB_RECURSE EXAMPLE_SOURCES "${CMAKE_SOURCE_DIR}/examples/*.cpp")

foreach(example_file ${EXAMPLE_SOURCES})
    get_filename_component(example_name ${example_file} NAME_WE)

    add_executable(${example_name} ${example_file})

    target_compile_definitions(${example_name} PRIVATE
        CONSOLIX_USE_LOGIT=$<BOOL:${CONSOLIX_USE_LOGIT}>
        CONSOLIX_USE_JSON=$<BOOL:${CONSOLIX_USE_JSON}>
        CONSOLIX_USE_CXXOPTS=$<BOOL:${CONSOLIX_USE_CXXOPTS}>
        CONSOLIX_BASE_PATH="${CONSOLIX_BASE_PATH}"
    )

    target_include_directories(${example_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)

    if(MSVC)
        target_compile_options(${example_name} PRIVATE /W4 /WX)
    else()
        target_compile_options(${example_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    # === Копируем config.json, если он существует
    set(CONFIG_JSON_PATH "${CMAKE_SOURCE_DIR}/examples/config.json")
    if(EXISTS "${CONFIG_JSON_PATH}")
        add_custom_command(TARGET ${example_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                "${CONFIG_JSON_PATH}"
                "$<TARGET_FILE_DIR:${example_name}>/config.json"
        )
    endif()
endforeach()

# === Цель для сохранения логов сборки
add_custom_target(save_logs
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} > ${CMAKE_BINARY_DIR}/build_log.txt 2>&1
    COMMENT "Building and saving logs to build_log.txt"
)
