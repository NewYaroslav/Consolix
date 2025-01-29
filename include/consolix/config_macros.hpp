#pragma once
#ifndef _CONSOLIX_CONFIG_MACROS_HPP_INCLUDED
#define _CONSOLIX_CONFIG_MACROS_HPP_INCLUDED

/// \file config_macros.hpp
/// \brief Configuration macros for the Consolix framework.
///
/// This file provides a centralized definition of macros used for configuring
/// and customizing the behavior of the Consolix framework. By defining these
/// macros, developers can enable or disable certain features and control the
/// formatting of log messages and other behavior.
///
/// ### Purpose:
/// - Centralize configuration macros.
/// - Simplify customization of the Consolix framework.
/// - Provide detailed documentation for each macro.
///
/// ### Usage:
/// Define macros as needed before including Consolix headers:
/// ```cpp
/// #define CONSOLIX_USE_LOGIT 1
/// #define CONSOLIX_FILE_LOGGER_PATH "logs/app.log"
/// #include <consolix/consolix.hpp>
/// ```

/// \def CONSOLIX_USE_LOGIT
/// \brief Enables or disables the use of the LogIt library.
/// \details Set to `1` to enable LogIt, or `0` to disable it.
/// \default `0`
#ifndef CONSOLIX_USE_LOGIT
#define CONSOLIX_USE_LOGIT 0
#endif

/// \def CONSOLIX_USE_CXXOPTS
/// \brief Enables or disables the use of the cxxopts library for command-line arguments.
/// \details Set to `1` to enable cxxopts, or `0` to disable it.
/// \default `0`
#ifndef CONSOLIX_USE_CXXOPTS
#define CONSOLIX_USE_CXXOPTS 0
#endif

/// \def CONSOLIX_USE_JSON
/// \brief Enables or disables the use of the nlohmann/json library for JSON handling.
/// \details Set to `1` to enable JSON functionality, or `0` to disable it.
/// \default `0`
#ifndef CONSOLIX_USE_JSON
#define CONSOLIX_USE_JSON 0
#endif

/// \def CONSOLIX_BASE_PATH
/// \brief Defines the base path for resolving relative paths.
/// \details Set to an empty object `{}` by default.
/// \default `{}`
#ifndef CONSOLIX_BASE_PATH
#define CONSOLIX_BASE_PATH {}
#endif

/// \def CONSOLIX_CONSOLE_PATTERN
/// \brief Console log message pattern.
/// \details Specifies the default format for log messages printed to the console.
/// \default `LOGIT_CONSOLE_PATTERN`
#ifndef CONSOLIX_CONSOLE_PATTERN
#define CONSOLIX_CONSOLE_PATTERN LOGIT_CONSOLE_PATTERN
#endif

/// \def CONSOLIX_CONSOLE_DEBUG_PATTERN
/// \brief Debug console log message pattern.
/// \details A specialized format for detailed debug messages.
/// \default `"%H:%M:%S.%e | [%25!g:%#] [%l] %^%v%$"`
#ifndef CONSOLIX_CONSOLE_DEBUG_PATTERN
#define CONSOLIX_CONSOLE_DEBUG_PATTERN "%H:%M:%S.%e | [%25!g:%#] [%l] %^%v%$"
#endif

/// \def CONSOLIX_FILE_LOGGER_PATTERN
/// \brief File log message pattern.
/// \details Specifies the default format for log messages written to files.
/// \default `LOGIT_FILE_LOGGER_PATTERN`
#ifndef CONSOLIX_FILE_LOGGER_PATTERN
#define CONSOLIX_FILE_LOGGER_PATTERN LOGIT_FILE_LOGGER_PATTERN
#endif

/// \def CONSOLIX_FILE_LOGGER_PATH
/// \brief Default file logger path.
/// \details Specifies the directory where general log files are stored.
/// \default `LOGIT_FILE_LOGGER_PATH`
#ifndef CONSOLIX_FILE_LOGGER_PATH
#define CONSOLIX_FILE_LOGGER_PATH LOGIT_FILE_LOGGER_PATH
#endif

/// \def CONSOLIX_UNIQUE_FILE_LOGGER_PATH
/// \brief Default unique file logger path.
/// \details Specifies the directory where unique log files are stored.
/// \default `LOGIT_UNIQUE_FILE_LOGGER_PATH`
#ifndef CONSOLIX_UNIQUE_FILE_LOGGER_PATH
#define CONSOLIX_UNIQUE_FILE_LOGGER_PATH LOGIT_UNIQUE_FILE_LOGGER_PATH
#endif

/// \def CONSOLIX_FILE_LOGGER_AUTO_DELETE_DAYS
/// \brief Auto-delete log files older than the specified number of days.
/// \details Automatically removes log files that exceed the defined retention period.
/// \default `LOGIT_FILE_LOGGER_AUTO_DELETE_DAYS`
#ifndef CONSOLIX_FILE_LOGGER_AUTO_DELETE_DAYS
#define CONSOLIX_FILE_LOGGER_AUTO_DELETE_DAYS LOGIT_FILE_LOGGER_AUTO_DELETE_DAYS
#endif

/// \def CONSOLIX_UNIQUE_FILE_LOGGER_HASH_LENGTH
/// \brief Default hash length for unique file names.
/// \details Determines the number of characters in the hash portion of unique file names.
/// \default `LOGIT_UNIQUE_FILE_LOGGER_HASH_LENGTH`
#ifndef CONSOLIX_UNIQUE_FILE_LOGGER_HASH_LENGTH
#define CONSOLIX_UNIQUE_FILE_LOGGER_HASH_LENGTH LOGIT_UNIQUE_FILE_LOGGER_HASH_LENGTH
#endif

/// \def CONSOLIX_DEFAULT_COLOR
/// \brief Default text color for console output.
/// \details Sets the default color for console output to LightGray.
/// \default `consolix::TextColor::LightGray`
#ifndef CONSOLIX_DEFAULT_COLOR
#define CONSOLIX_DEFAULT_COLOR consolix::TextColor::LightGray
#endif

/// \def CONSOLIX_WAIT_ON_ERROR
/// \brief Enables or disables waiting for user input before exiting on a fatal error.
/// \details Set to `1` to wait for Enter after a fatal error (useful for debugging),
/// or `0` to exit immediately.
/// \default `0`
#ifndef CONSOLIX_WAIT_ON_ERROR
#define CONSOLIX_WAIT_ON_ERROR 0
#endif

#endif // _CONSOLIX_CONFIG_MACROS_HPP_INCLUDED
