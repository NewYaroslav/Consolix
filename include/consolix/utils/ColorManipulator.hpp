#pragma once
#ifndef _CONSOLIX_COLOR_MANIPULATOR_HPP_INCLUDED
#define _CONSOLIX_COLOR_MANIPULATOR_HPP_INCLUDED

/// \file ColorManipulator.hpp
/// \brief Provides utilities for changing text color in streams.

#include <iostream>

#ifndef CONSOLIX_DEFAULT_COLOR
#define CONSOLIX_DEFAULT_COLOR consolix::TextColor::LightGray ///< Default text color.
#endif

namespace consolix {

    /// \class ColorManipulator
    /// \brief A utility class for managing text color in streams.
    ///
    /// This class allows dynamic control of text color when writing to output streams.
    /// It resets the color to the default upon destruction.
    class ColorManipulator {
    public:
        /// \brief Constructor.
        /// \param color The desired text color.
        explicit ColorManipulator(TextColor color) : m_color(color) {}

        /// \brief Retrieves the current text color.
        /// \return The color associated with the manipulator.
        TextColor color() const { return m_color; }

        /// \brief Destructor that resets the text color to the default.
        ~ColorManipulator() {
#           if CONSOLIX_USE_LOGIT == 0
#           if defined(_WIN32) || defined(_WIN64)
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hConsole, to_windows_color(CONSOLIX_DEFAULT_COLOR));
#           else
            if (isatty(STDOUT_FILENO)) {
                std::cout << "\033[0m"; // Reset color for ANSI-compatible terminals.
            }
#           endif
#           endif
        }

    private:
        TextColor m_color; ///< The current text color.
    }; // ColorManipulator

    /// \brief Creates a color manipulator for use in output streams.
    /// \param color The desired text color.
    /// \return A `ColorManipulator` object for stream manipulation.
    inline ColorManipulator color(TextColor color) {
        return ColorManipulator(color);
    }

    /// \brief Overloads the stream operator to apply text color.
    /// \param os The output stream.
    /// \param manip The `ColorManipulator` controlling the text color.
    /// \return The modified output stream.
    inline std::ostream& operator<<(std::ostream& os, const ColorManipulator& manip) {
        os << to_c_str(manip.color()); // Apply ANSI escape sequence or equivalent.
        return os;
    }

} // namespace consolix

#endif // _CONSOLIX_COLOR_MANIPULATOR_HPP_INCLUDED
