#pragma once
#ifndef _CONSOLIX_MULTI_STREAM_HPP_INCLUDED
#define _CONSOLIX_MULTI_STREAM_HPP_INCLUDED

/// \file MultiStream.hpp
/// \brief MultiStream class for unified log output handling.
///
/// This class provides platform-independent handling of multi-target log streams.
/// It supports UTF-8 output for consoles and integrates with LogIt (if enabled).
/// On Windows, it converts UTF-8 strings to CP866 for compatibility with legacy console encoding,
/// while on Linux/macOS, it directly outputs UTF-8 strings with ANSI color codes.

#include <sstream>

namespace consolix {

    /// \class MultiStream
	/// \brief A class for handling multi-target log streams.
	///
	/// MultiStream ensures consistent and platform-independent logging output:
	/// - On **Windows**, UTF-8 strings are automatically converted to CP866 to support legacy console encoding.
	/// - On **Linux/macOS**, UTF-8 strings are directly output to the console.
	/// - Supports ANSI color codes for enhanced readability.
	/// - Integrates with LogIt logging library for advanced log management (if enabled).
	/// - Supports LogIt integration (if enabled) and platform-specific console handling.
    class MultiStream {
    public:

#if     CONSOLIX_USE_LOGIT == 1
        /// \brief Default constructor for MultiStream with LogIt integration.
        MultiStream()
            : m_level(logit::LogLevel::LOG_LVL_TRACE), m_file(__FILE__),
            m_line(__LINE__),
            m_function(logit::make_relative(__FILE__, LOGIT_BASE_PATH)) {
        }

        /// \brief Parameterized constructor for LogIt integration.
        /// \param level The log level.
        /// \param file Source file name.
        /// \param line Line number in the source file.
        /// \param function Function name.
        MultiStream(
            logit::LogLevel level,
            const std::string& file,
            int line,
            const std::string& function)
            : m_level(level), m_file(file), m_line(line), m_function(function) {
        }
#       else

        /// \brief Default constructor for MultiStream without LogIt integration.
        /// \param use_utf8 Flag to indicate if UTF-8 encoding should be used.
        MultiStream(bool use_utf8 = true) : use_utf8(use_utf8) {};

#       endif

        /// \brief Destructor to flush the accumulated log content.
        ~MultiStream() {
#           if defined(_WIN32)
            auto str = use_utf8 ? utf8_to_cp866(m_stream.str()) : m_stream.str();
#           else
            auto str = m_stream.str();
#           endif // if defined(_WIN32) || defined(_WIN64)

#           if CONSOLIX_USE_LOGIT == 1
            if (LOGIT_IS_SINGLE_MODE(CONSOLIX_LOGIT_CONSOLE_INDEX)) {
                LOGIT_STREAM_TRACE_TO(CONSOLIX_LOGIT_CONSOLE_INDEX) << str;
            }
            logit::LogStream(m_level, m_file, m_line, m_function, CONSOLIX_LOGIT_LOGGER_INDEX) << str;
#           else

#           if defined(_WIN32) || defined(_WIN64)
            handle_ansi_colors_windows(str);
#           else
            flush_to_console(str);
#           endif // if defined(_WIN32) || defined(_WIN64)

#           endif // if CONSOLIX_USE_LOGIT == 1
        }

        /// \brief Overloaded operator<< for adding content to the stream.
        /// \tparam T The type of the content to log.
        /// \param value The value to log.
        /// \return Reference to the current MultiStream object.
        template <typename T>
        MultiStream& operator<<(const T& value) {
            m_stream << value;
            return *this;
        }

        /// \brief Overloaded operator<< for manipulators (e.g., std::endl).
        /// \param manip Manipulator function (e.g., std::endl).
        /// \return Reference to the current MultiStream object.
        MultiStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
            m_stream << manip;
            return *this;
        }

    private:
        std::ostringstream  m_stream;       ///< Internal stream for accumulating log content.

#if     CONSOLIX_USE_LOGIT == 1
        logit::LogLevel     m_level;        ///< Log level.
        std::string         m_file;         ///< Source file name.
        int                 m_line;         ///< Line number.
        std::string         m_function;     ///< Function name.
#       else
        bool                use_utf8;       ///< Flag indicating whether UTF-8 encoding should be used.
#       endif

        /// \brief Flush the accumulated message to the console on non-Windows platforms.
        /// \param message The message to flush.
        void flush_to_console(const std::string& message) const {
            if (!message.empty()) {
                if (message.back() == '\n') {
                    std::cout << message;
                } else {
                    std::cout << message << std::endl;
                }
            }
        }

#       if CONSOLIX_USE_LOGIT == 0 && (defined(_WIN32) || defined(_WIN64))

        /// \brief Handle ANSI color codes in the message for Windows console.
        /// \param message The message containing ANSI color codes.
        void handle_ansi_colors_windows(const std::string& message) const {
            std::string::size_type start = 0;
            std::string::size_type pos = 0;

            HANDLE handle_stdout = GetStdHandle(STD_OUTPUT_HANDLE);

            while ((pos = message.find("\033[", start)) != std::string::npos) {
                // Output the part of the string before the ANSI code
                if (pos > start) {
                    std::cout << message.substr(start, pos - start);
                }

                // Find the end of the ANSI code
                std::string::size_type end_pos = message.find('m', pos);
                if (end_pos != std::string::npos) {
                    // Extract the ANSI code
                    std::string ansi_code = message.substr(pos + 2, end_pos - pos - 2);
                    apply_color_from_ansi_code(ansi_code, handle_stdout);

                    // Update position
                    start = end_pos + 1;
                } else {
                    break;
                }
            }

            // Output any remaining part of the message
            if (start < message.size()) {
                std::cout << message.substr(start);
            }
            if (!message.empty()) std::cout << std::endl;

            // Reset the console color to default
            SetConsoleTextAttribute(handle_stdout, to_windows_color(CONSOLIX_DEFAULT_COLOR));
        }

        /// \brief Apply color based on ANSI code for Windows console.
        /// \param ansi_code The ANSI code string.
        /// \param handle_stdout The console handle.
        void apply_color_from_ansi_code(const std::string& ansi_code, HANDLE handle_stdout) const {
            WORD color_value = to_windows_color(CONSOLIX_DEFAULT_COLOR); // Default color
            const int code = std::stoi(ansi_code);
            switch (code) {
                case 30: color_value = to_windows_color(TextColor::Black); break;
                case 31: color_value = to_windows_color(TextColor::DarkRed); break;
                case 32: color_value = to_windows_color(TextColor::DarkGreen); break;
                case 33: color_value = to_windows_color(TextColor::DarkYellow); break;
                case 34: color_value = to_windows_color(TextColor::DarkBlue); break;
                case 35: color_value = to_windows_color(TextColor::DarkMagenta); break;
                case 36: color_value = to_windows_color(TextColor::DarkCyan); break;
                case 37: color_value = to_windows_color(TextColor::LightGray); break;
                case 90: color_value = to_windows_color(TextColor::DarkGray); break;
                case 91: color_value = to_windows_color(TextColor::Red); break;
                case 92: color_value = to_windows_color(TextColor::Green); break;
                case 93: color_value = to_windows_color(TextColor::Yellow); break;
                case 94: color_value = to_windows_color(TextColor::Blue); break;
                case 95: color_value = to_windows_color(TextColor::Magenta); break;
                case 96: color_value = to_windows_color(TextColor::Cyan); break;
                case 97: color_value = to_windows_color(TextColor::White); break;
                default:
                    // Unknown code, use default color
                    break;
            };
            // Set the console text attribute to the desired color
            SetConsoleTextAttribute(handle_stdout, color_value);
        }
#       endif
    }; // MultiStream

}; // namespace consolix

#endif // _CONSOLIX_MULTI_STREAM_HPP_INCLUDED
