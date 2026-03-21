#pragma once
#ifndef _CONSOLIX_ENCODING_UTILS_HPP_INCLUDED
#define _CONSOLIX_ENCODING_UTILS_HPP_INCLUDED

/// \file encoding_utils.hpp
/// \brief Utilities for working with character encodings and string transformations.
/// \ingroup Utilities

#if defined(_WIN32)

#include <string>

namespace consolix {

    /// \brief Converts a UTF-8 string to an ANSI string (Windows-specific).
    /// \param utf8 The UTF-8 encoded string.
    /// \return The converted ANSI string.
    inline std::string utf8_to_ansi(const std::string& utf8) noexcept {
        const int wide_length = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
        if (wide_length <= 0) return {};

        std::wstring wide_string(static_cast<std::size_t>(wide_length), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wide_string[0], wide_length);

        const int ansi_length = WideCharToMultiByte(CP_ACP, 0, wide_string.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (ansi_length <= 0) return {};

        std::string ansi_string(static_cast<std::size_t>(ansi_length), '\0');
        WideCharToMultiByte(CP_ACP, 0, wide_string.c_str(), -1, &ansi_string[0], ansi_length, nullptr, nullptr);
        if (!ansi_string.empty()) {
            ansi_string.pop_back();
        }

        return ansi_string;
    }

    /// \brief Converts an ANSI string to a UTF-8 string (Windows-specific).
    /// \param ansi The ANSI encoded string.
    /// \return The converted UTF-8 string.
    inline std::string ansi_to_utf8(const std::string& ansi) noexcept {
        const int wide_length = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, nullptr, 0);
        if (wide_length <= 0) return {};

        std::wstring wide_string(static_cast<std::size_t>(wide_length), L'\0');
        MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, &wide_string[0], wide_length);

        const int utf8_length = WideCharToMultiByte(CP_UTF8, 0, wide_string.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (utf8_length <= 0) return {};

        std::string utf8_string(static_cast<std::size_t>(utf8_length), '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide_string.c_str(), -1, &utf8_string[0], utf8_length, nullptr, nullptr);
        if (!utf8_string.empty()) {
            utf8_string.pop_back();
        }

        return utf8_string;
    }

    /// \brief Converts a UTF-8 string to a CP866 string (DOS-specific, Windows).
    /// \param utf8 The UTF-8 encoded string.
    /// \return The converted CP866 string.
    inline std::string utf8_to_cp866(const std::string& utf8) noexcept {
        std::string temp = utf8_to_ansi(utf8);
        if (!temp.empty()) {
            CharToOemA(temp.c_str(), &temp[0]);
        }
        return temp;
    }

    /// \brief Validates whether a string is a valid UTF-8 string.
    /// \param message The string to validate.
    /// \return `true` if the string is valid UTF-8, `false` otherwise.
    inline bool is_valid_utf8(const char* message) {
        if (!message) return true;

        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(message);
        while (*bytes != 0x00) {
            int num = 0;
            unsigned int cp = 0;
            if ((*bytes & 0x80) == 0x00) { cp = (*bytes & 0x7F); num = 1; }
            else if ((*bytes & 0xE0) == 0xC0) { cp = (*bytes & 0x1F); num = 2; }
            else if ((*bytes & 0xF0) == 0xE0) { cp = (*bytes & 0x0F); num = 3; }
            else if ((*bytes & 0xF8) == 0xF0) { cp = (*bytes & 0x07); num = 4; }
            else return false;

            ++bytes;
            for (int i = 1; i < num; ++i) {
                if ((*bytes & 0xC0) != 0x80) return false;
                cp = (cp << 6) | (*bytes & 0x3F);
                ++bytes;
            }

            if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) return false;
        }

        return true;
    }

    /// \brief Converts a CP1251 string to UTF-8.
    /// \param cp1251_string The CP1251 encoded string.
    /// \return The converted UTF-8 string.
    inline std::string cp1251_to_utf8(const std::string& cp1251_string) {
        const int wide_length = MultiByteToWideChar(1251, 0, cp1251_string.c_str(), -1, nullptr, 0);
        if (wide_length <= 0) return {};

        std::wstring wide_string(static_cast<std::size_t>(wide_length), L'\0');
        MultiByteToWideChar(1251, 0, cp1251_string.c_str(), -1, &wide_string[0], wide_length);

        const int utf8_length = WideCharToMultiByte(CP_UTF8, 0, wide_string.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (utf8_length <= 0) return {};

        std::string utf8_string(static_cast<std::size_t>(utf8_length), '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide_string.c_str(), -1, &utf8_string[0], utf8_length, nullptr, nullptr);
        if (!utf8_string.empty()) {
            utf8_string.pop_back();
        }

        return utf8_string;
    }

    /// \brief Converts a UTF-16 string to UTF-8.
    /// \param utf16_string A wide character string.
    /// \return The converted UTF-8 string.
    inline std::string utf16_to_utf8(const wchar_t* utf16_string) {
        const int buffer_size = WideCharToMultiByte(CP_UTF8, 0, utf16_string, -1, nullptr, 0, nullptr, nullptr);
        if (buffer_size <= 0) return {};

        std::string utf8_string(static_cast<std::size_t>(buffer_size), '\0');
        WideCharToMultiByte(CP_UTF8, 0, utf16_string, -1, &utf8_string[0], buffer_size, nullptr, nullptr);
        if (!utf8_string.empty()) {
            utf8_string.pop_back();
        }
        return utf8_string;
    }

    /// \brief Converts a UTF-8 string to a UTF-16 wide string.
    /// \param utf8 The UTF-8 encoded string.
    /// \return The converted UTF-16 string.
    inline std::wstring utf8_to_utf16(const std::string& utf8) noexcept {
        const int utf16_length = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
        if (utf16_length <= 0) return {};

        std::wstring utf16_string(static_cast<std::size_t>(utf16_length), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &utf16_string[0], utf16_length);
        if (!utf16_string.empty()) {
            utf16_string.pop_back();
        }

        return utf16_string;
    }

} // namespace consolix

#endif

#endif // _CONSOLIX_ENCODING_UTILS_HPP_INCLUDED
