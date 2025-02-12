#pragma once
#ifndef _CONSOLIX_ENUMS_HPP_INCLUDED
#define _CONSOLIX_ENUMS_HPP_INCLUDED

/// \file enums.hpp
/// \brief Contains enumerations and utility functions for text colors.

#include <array>

namespace consolix {

    /// \enum TextColor
    /// \brief Represents text colors for console output.
    ///
    /// The enumeration defines a set of standard text colors compatible with
    /// both ANSI escape codes (Linux/macOS) and Windows API (Windows).
    enum class TextColor {
        Black,
        DarkRed,
        DarkGreen,
        DarkYellow,
        DarkBlue,
        DarkMagenta,
        DarkCyan,
        LightGray,
        DarkGray,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
    };

    /// \brief Converts a `TextColor` to an ANSI escape code string.
    /// \param color The text color to convert.
    /// \return A C-style string representing the ANSI escape code for the color.
    inline const char* to_c_str(const TextColor& color) {
        static const std::array<const char*, 16> ansi_codes = {
            "\033[30m",   // Black
            "\033[31m",   // DarkRed
            "\033[32m",   // DarkGreen
            "\033[33m",   // DarkYellow
            "\033[34m",   // DarkBlue
            "\033[35m",   // DarkMagenta
            "\033[36m",   // DarkCyan
            "\033[37m",   // LightGray
            "\033[90m",   // DarkGray
            "\033[91m",   // Red
            "\033[92m",   // Green
            "\033[93m",   // Yellow
            "\033[94m",   // Blue
            "\033[95m",   // Magenta
            "\033[96m",   // Cyan
            "\033[97m"    // White
        };

        return ansi_codes[static_cast<int>(color)];
    }

#   if defined(_WIN32) || defined(_WIN64)
    /// \brief Converts a `TextColor` to a Windows console color attribute.
    /// \param color The text color to convert.
    /// \return A `WORD` value representing the Windows console color attribute.
    inline WORD to_windows_color(const TextColor& color) {
        static const std::array<WORD, 16> windows_colors = {
            0,                            // Black
            FOREGROUND_RED,               // DarkRed
            FOREGROUND_GREEN,             // DarkGreen
            FOREGROUND_RED | FOREGROUND_GREEN, // DarkYellow
            FOREGROUND_BLUE,              // DarkBlue
            FOREGROUND_RED | FOREGROUND_BLUE, // DarkMagenta
            FOREGROUND_GREEN | FOREGROUND_BLUE, // DarkCyan
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, // LightGray
            FOREGROUND_INTENSITY,         // DarkGray
            FOREGROUND_RED | FOREGROUND_INTENSITY, // Red
            FOREGROUND_GREEN | FOREGROUND_INTENSITY, // Green
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, // Yellow
            FOREGROUND_BLUE | FOREGROUND_INTENSITY, // Blue
            FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY, // Magenta
            FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, // Cyan
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY // White
        };

        return windows_colors[static_cast<int>(color)];
    }
#   endif

}; // namespace consolix

#endif // _CONSOLIX_ENUMS_HPP_INCLUDED
