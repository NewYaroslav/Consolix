#pragma once
#ifndef _CONSOLIX_LOGGER_COMPONENT_HPP_INCLUDED
#define _CONSOLIX_LOGGER_COMPONENT_HPP_INCLUDED

/// \file LoggerComponent.hpp
/// \brief Logger component for managing application logging with LogIt.
/// \ingroup Components

#if CONSOLIX_USE_LOGIT == 1

/// \brief Defines the base path used for relative path conversion in logs.
/// \note Set this macro to the root path of your project or a meaningful base directory.
#ifndef CONSOLIX_BASE_PATH
#define CONSOLIX_BASE_PATH {}
#endif

#ifndef LOGIT_BASE_PATH
#define LOGIT_BASE_PATH CONSOLIX_BASE_PATH
#endif

/// \brief Defines the console log stream index for LogIt.
/// Used to direct logs to the main console backend.
#define CONSOLIX_LOGIT_CONSOLE_INDEX        0

/// \brief Defines the logo log stream index for LogIt.
/// Used to direct logo-specific logs to a dedicated stream.
#define CONSOLIX_LOGIT_LOGO_INDEX           1

/// \brief Defines the debug log stream index for LogIt.
/// Used for logging debug-specific information.
#define CONSOLIX_LOGIT_DEBUG_INDEX          2

/// \brief Defines the logger stream index for general logs.
/// This stream is used for regular application logging.
#define CONSOLIX_LOGIT_LOGGER_INDEX         3

/// \brief Defines the unique file log stream index for LogIt.
/// Used for creating unique log files for large or isolated log entries.
#define CONSOLIX_LOGIT_UNIQUE_FILE_INDEX    4

/// \brief Defines the general log stream for the application.
/// Redirects logs to the main console backend with detailed context.
#define CONSOLIX_STREAM()                                \
    consolix::MultiStream(                               \
        logit::LogLevel::LOG_LVL_TRACE,                  \
        logit::make_relative(__FILE__, LOGIT_BASE_PATH), \
        __LINE__,                                        \
        LOGIT_FUNCTION)

/// \brief Defines a stderr-only diagnostic stream at a fixed level.
/// \param level LogIt level to assume for the stderr-side diagnostic message.
/// \details Writes exclusively to std::cerr. No LogIt backends are touched.
///          Pass `logit::LogLevel::LOG_LVL_ERROR` (or any other level) explicitly.
#define CONSOLIX_STDERR_STREAM(level)                              \
    consolix::StderrStream(                                        \
        (level),                                                   \
        logit::make_relative(__FILE__, LOGIT_BASE_PATH),           \
        __LINE__,                                                  \
        LOGIT_FUNCTION,                                            \
        {})

/// \brief Defines a stderr diagnostic stream that also fans out to the
///        Consolix default LogIt backend set.
/// \param level LogIt level for the targeted backends.
/// \details The set of duplicated LogIt backends is taken from the
///          `CONSOLIX_LOGIT_DEFAULT_BACKENDS` macro (empty by default).
///          Override that macro (e.g. `-DCONSOLIX_LOGIT_DEFAULT_BACKENDS=8,9`)
///          to redirect the duplicated output.
#define CONSOLIX_STDERR_LOG_STREAM(level)                          \
    consolix::StderrStream(                                        \
        (level),                                                   \
        logit::make_relative(__FILE__, LOGIT_BASE_PATH),           \
        __LINE__,                                                  \
        LOGIT_FUNCTION,                                            \
        {CONSOLIX_LOGIT_DEFAULT_BACKENDS})

/// \brief Defines a stderr diagnostic stream that also fans out to an
///        explicit inline list of LogIt backend indices.
/// \param level LogIt level for the targeted backends.
/// \param ... Comma-separated LogIt backend indices (e.g. `0,3`).
#define CONSOLIX_STDERR_LOG_STREAM_EX(level, ...)                  \
    consolix::StderrStream(                                        \
        (level),                                                   \
        logit::make_relative(__FILE__, LOGIT_BASE_PATH),           \
        __LINE__,                                                  \
        LOGIT_FUNCTION,                                            \
        {__VA_ARGS__})

/// \brief Defines a multi-target log stream (file logger + console + default LogIt backends).
/// \param level LogIt level for the multi-target stream.
/// \details Uses `CONSOLIX_LOGIT_DEFAULT_BACKENDS` (empty by default) to select
///          additional backends; override that macro to extend the set.
#define CONSOLIX_LOG_STREAM(level)                                 \
    consolix::MultiStream(                                         \
        (level),                                                   \
        logit::make_relative(__FILE__, LOGIT_BASE_PATH),           \
        __LINE__,                                                  \
        LOGIT_FUNCTION,                                            \
        {CONSOLIX_LOGIT_DEFAULT_BACKENDS})

/// \brief Defines a multi-target log stream with an explicit inline list of
///        additional LogIt backend indices.
/// \param level LogIt level for the multi-target stream.
/// \param ... Comma-separated LogIt backend indices (e.g. `0,3`).
#define CONSOLIX_LOG_STREAM_EX(level, ...)                         \
    consolix::MultiStream(                                         \
        (level),                                                   \
        logit::make_relative(__FILE__, LOGIT_BASE_PATH),           \
        __LINE__,                                                  \
        LOGIT_FUNCTION,                                            \
        {__VA_ARGS__})

/// \note The legacy `_ERROR_STREAM` and `_FILE_*` stderr variants have been
///       removed. To colorize a stderr message, prepend the color manipulator
///       in the chain, e.g.
///       `CONSOLIX_STDERR_STREAM(logit::LogLevel::LOG_LVL_ERROR) << consolix::color(consolix::TextColor::Red)`.

/// \brief Defines the log stream for application logos.
/// This stream is dedicated to rendering logo-specific logs.
#define CONSOLIX_LOGO_STREAM() \
    LOGIT_STREAM_TRACE_TO(CONSOLIX_LOGIT_LOGO_INDEX)

/// \brief Defines the unique file log stream.
/// Redirects logs to a unique log file for isolated entries.
#define CONSOLIX_UNIQUE_FILE_STREAM() \
    LOGIT_STREAM_TRACE_TO(CONSOLIX_LOGIT_UNIQUE_FILE_INDEX)

/// \brief Retrieves the name of the most recent unique log file.
/// Useful for referencing unique log file paths in other operations.
#define CONSOLIX_UNIQUE_FILE_NAME() \
    LOGIT_GET_LAST_FILE_NAME(CONSOLIX_LOGIT_UNIQUE_FILE_INDEX)

/// \brief Console log message pattern.
/// Defines the default format for console log messages.
#ifndef CONSOLIX_CONSOLE_PATTERN
#define CONSOLIX_CONSOLE_PATTERN LOGIT_CONSOLE_PATTERN
#endif

/// \brief Debug console log message pattern.
/// A specialized format for detailed debug messages.
#ifndef CONSOLIX_CONSOLE_DEBUG_PATTERN
#define CONSOLIX_CONSOLE_DEBUG_PATTERN "%H:%M:%S.%e | [%25!g:%#] [%l] %^%v%$"
#endif

/// \brief File log message pattern.
/// Specifies the default format for log messages written to files.
#ifndef CONSOLIX_FILE_LOGGER_PATTERN
#define CONSOLIX_FILE_LOGGER_PATTERN LOGIT_FILE_LOGGER_PATTERN
#endif

/// \brief Default file logger path.
/// Specifies the directory where general log files are stored.
#ifndef CONSOLIX_FILE_LOGGER_PATH
#define CONSOLIX_FILE_LOGGER_PATH LOGIT_FILE_LOGGER_PATH
#endif

/// \brief Default unique file logger path.
/// Specifies the directory where unique log files are stored.
#ifndef CONSOLIX_UNIQUE_FILE_LOGGER_PATH
#define CONSOLIX_UNIQUE_FILE_LOGGER_PATH LOGIT_UNIQUE_FILE_LOGGER_PATH
#endif

/// \brief Auto-delete log files older than specified days.
/// Automatically removes log files that exceed the defined retention period.
#ifndef CONSOLIX_FILE_LOGGER_AUTO_DELETE_DAYS
#define CONSOLIX_FILE_LOGGER_AUTO_DELETE_DAYS LOGIT_FILE_LOGGER_AUTO_DELETE_DAYS
#endif

/// \brief Default hash length for unique file names.
/// Determines the number of characters in the hash portion of unique file names.
#ifndef CONSOLIX_UNIQUE_FILE_LOGGER_HASH_LENGTH
#define CONSOLIX_UNIQUE_FILE_LOGGER_HASH_LENGTH LOGIT_UNIQUE_FILE_LOGGER_HASH_LENGTH
#endif

/// \brief Enables or disables the debug log stream.
/// \param mode A boolean indicating whether the debug stream is active.
#define CONSOLIX_SET_DEBUG_MODE(mode) \
    LOGIT_SET_LOGGER_ENABLED(CONSOLIX_LOGIT_DEBUG_INDEX, mode)

#include <logit.hpp>
#include "LoggerComponent/MultiStream.hpp"

namespace consolix {

    /// \class LoggerComponent
    /// \brief Configures and initializes logging for the application using LogIt.
    ///
    /// This component handles multiple logging streams, including console, debug,
    /// and unique file loggers. It ensures proper setup and integration with LogIt.
    class LoggerComponent : public IAppComponent {
    public:

        /// \brief Constructs the LoggerComponent with custom logging patterns.
        /// \param console_pattern The log pattern for console output.
        /// \param console_debug_pattern The log pattern for debug console output.
        /// \param file_pattern The log pattern for file logs.
        /// \param auto_delete_days Number of days after which old log files are auto-deleted.
        LoggerComponent(
                const std::string& console_pattern = CONSOLIX_CONSOLE_PATTERN,
                const std::string& console_debug_pattern = CONSOLIX_CONSOLE_DEBUG_PATTERN,
                const std::string& file_pattern = CONSOLIX_FILE_LOGGER_PATTERN,
                const int auto_delete_days = CONSOLIX_FILE_LOGGER_AUTO_DELETE_DAYS)
            : m_debug_pattern(console_debug_pattern) {
            init_logger(console_pattern, file_pattern, auto_delete_days);
        }

        /// \brief Virtual destructor.
        virtual ~LoggerComponent() = default;

    protected:

        /// \brief Initializes the logging component.
        /// \return `true` if initialization is successful, `false` otherwise.
        bool initialize() override {
#           if CONSOLIX_USE_CXXOPTS == 1
            if (!has_service<CliOptions>()) {
                m_is_init = true;
                return true;
            }
            if (!has_service<CliArguments>()) return false;
            auto args = get_service<CliArguments>();
            if (args.count("debug") || args.count("d")) {
                CONSOLIX_SET_DEBUG_MODE(true);
            }
#           endif
            m_is_init = true;
            return true;
        }

        /// \brief Checks if the component is initialized.
        /// \return `true` if the component is initialized, `false` otherwise.
        bool is_initialized() const override {
            return m_is_init;
        }

        /// \brief Executes the main functionality (no-op for LoggerComponent).
        void process() override {}

    private:
        std::string       m_debug_pattern;  ///< Pattern for debug log messages.
        std::atomic<bool> m_is_init{false}; ///< Indicates whether the component is initialized.

        /// \brief Initializes the logging system.
        /// \param console_pattern The log pattern for console output.
        /// \param file_pattern The log pattern for file logs.
        /// \param auto_delete_days Number of days after which old log files are auto-deleted.
        void init_logger(
                const std::string& console_pattern,
                const std::string& file_pattern,
                int auto_delete_days) {
            static bool is_once = false;
            if (is_once) return;
            is_once = true;

            LOGIT_ADD_CONSOLE_SINGLE_MODE(console_pattern, true);
            LOGIT_ADD_CONSOLE_SINGLE_MODE("%^%v%$", true); // Logo log stream
            LOGIT_ADD_CONSOLE(m_debug_pattern, true);      // Debug log stream
            CONSOLIX_SET_DEBUG_MODE(false);

            // Файловый лорггер
            LOGIT_ADD_FILE_LOGGER(
                CONSOLIX_FILE_LOGGER_PATH,
                true,
                auto_delete_days,
                file_pattern);

            // Файловый логгер для отедельных больших записей
            LOGIT_ADD_UNIQUE_FILE_LOGGER_SINGLE_MODE(
                CONSOLIX_UNIQUE_FILE_LOGGER_PATH,
                true,
                auto_delete_days,
                CONSOLIX_UNIQUE_FILE_LOGGER_HASH_LENGTH,
                "%v");
        }
    }; // LoggerComponent

}; // namespace consolix

#else // Fallback for when LogIt is not enabled

#include "LoggerComponent/MultiStream.hpp"

/// \brief Fallback for general logging.
#define CONSOLIX_STREAM() \
    consolix::MultiStream()

/// \brief Fallback stderr-only diagnostic stream. The `level` argument is
///        ignored when LogIt is disabled.
#define CONSOLIX_STDERR_STREAM(level) \
    consolix::StderrStream()

/// \brief Fallback stderr diagnostic stream. Both `level` and backend
///        indices are ignored when LogIt is disabled.
#define CONSOLIX_STDERR_LOG_STREAM(level) \
    consolix::StderrStream()

/// \brief Fallback stderr diagnostic stream with an ignored backend list.
#define CONSOLIX_STDERR_LOG_STREAM_EX(level, ...) \
    consolix::StderrStream()

/// \brief Fallback multi-target log stream. The `level` argument is
///        ignored when LogIt is disabled.
#define CONSOLIX_LOG_STREAM(level) \
    consolix::MultiStream()

/// \brief Fallback multi-target log stream with an ignored backend list.
#define CONSOLIX_LOG_STREAM_EX(level, ...) \
    consolix::MultiStream()

/// \brief Fallback for logo logging.
#define CONSOLIX_LOGO_STREAM() \
    consolix::MultiStream(false)

namespace consolix {

    /// \class LoggerComponent
    /// \brief Fallback LoggerComponent when LogIt is disabled.
    class LoggerComponent : public IAppComponent {
    public:

        LoggerComponent() = default;
        virtual ~LoggerComponent() = default;

    protected:

        bool initialize() override {return true;}
        bool is_initialized() const override {return true;}
        void process() override {}

    }; // LoggerComponent

}; // namespace consolix

#endif

#endif // _CONSOLIX_LOGGER_COMPONENT_HPP_INCLUDED
