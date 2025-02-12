#pragma once
#ifndef _CONSOLIX_ENCODING_UTILS_HPP_INCLUDED
#define _CONSOLIX_ENCODING_UTILS_HPP_INCLUDED

/// \file encoding_utils.hpp
/// \brief Utilities for working with character encodings and string transformations.
/// \ingroup Utilities

#if defined(_WIN32) || defined(_WIN64)

#include <string>

namespace consolix {

    /// \brief Converts a UTF-8 string to an ANSI string (Windows-specific).
    /// \param utf8 The UTF-8 encoded string.
    /// \return The converted ANSI string.
    std::string utf8_to_ansi(const std::string& utf8) noexcept {
        int n_len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
        if (n_len == 0) return {};

        std::wstring wide_string(n_len + 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wide_string.data(), n_len);

        n_len = WideCharToMultiByte(CP_ACP, 0, wide_string.c_str(), -1, NULL, 0, NULL, NULL);
        if (n_len == 0) return {};

        std::string ansi_string(n_len - 1, '\0');
        WideCharToMultiByte(CP_ACP, 0, wide_string.c_str(), -1, ansi_string.data(), n_len, NULL, NULL);
        return ansi_string;
    }

    /// \brief Converts an ANSI string to a UTF-8 string (Windows-specific).
    /// \param ansi The ANSI encoded string.
    /// \return The converted UTF-8 string.
    std::string ansi_to_utf8(const std::string& ansi) noexcept {
        int n_len = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, NULL, 0);
        if (n_len == 0) return {};

        std::wstring wide_string(n_len, L'\0');
        MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, wide_string.data(), n_len);

        n_len = WideCharToMultiByte(CP_UTF8, 0, wide_string.c_str(), -1, NULL, 0, NULL, NULL);
        if (n_len == 0) return {};

        std::string utf8_string(n_len - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide_string.c_str(), -1, utf8_string.data(), n_len, NULL, NULL);
        return utf8_string;
    }

    /// \brief Converts a UTF-8 string to a CP866 string (DOS-specific, Windows).
    /// \param utf8 The UTF-8 encoded string.
    /// \return The converted CP866 string.
    std::string utf8_to_cp866(const std::string& utf8) noexcept {
        std::string temp = utf8_to_ansi(utf8);
        CharToOem((LPSTR)temp.c_str(), temp.data());
        return temp;
    }

    /// \brief Validates whether a string is a valid UTF-8 string.
    /// \param message The string to validate.
    /// \return `true` if the string is valid UTF-8, `false` otherwise.
    bool is_valid_utf8(const char* message) {
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

            bytes++;
            for (int i = 1; i < num; ++i) {
                if ((*bytes & 0xC0) != 0x80) return false;
                cp = (cp << 6) | (*bytes & 0x3F);
                bytes++;
            }

            if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) return false;
        }
        return true;
    }

    /// \brief Converts a CP1251 string to UTF-8.
    /// \param cp1251 The CP1251 encoded string.
    /// \return The converted UTF-8 string.
    std::string cp1251_to_utf8(const std::string& cp1251) {
        int len = MultiByteToWideChar(1251, 0, cp1251.c_str(), -1, NULL, 0);
        if (len == 0) return {};

        std::wstring wideString(len, L'\0');
        MultiByteToWideChar(1251, 0, cp1251.c_str(), -1, wideString.data(), len);

        len = WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), -1, NULL, 0, NULL, NULL);
        if (len == 0) return {};

        std::string utf8String(len, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), -1, utf8String.data(), len, NULL, NULL);
        return utf8String;
    }

    /// \brief Converts a UTF-16 string to UTF-8.
    /// \param utf16String A wide character string.
    /// \return The converted UTF-8 string.
    std::string utf16_to_utf8(LPWSTR utf16String) {
        int bufferSize = WideCharToMultiByte(CP_UTF8, 0, utf16String, -1, nullptr, 0, nullptr, nullptr);
        std::string utf8String(bufferSize, '\0');
        WideCharToMultiByte(CP_UTF8, 0, utf16String, -1, utf8String.data(), bufferSize, nullptr, nullptr);
        return utf8String;
    }

    /// \brief Converts a UTF-8 string to a UTF-16 wide string.
    /// \param utf8 The UTF-8 encoded string.
    /// \return The converted UTF-16 string.
    std::wstring utf8_to_utf16(const std::string& utf8) noexcept {
        int n_len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
        if (n_len == 0) return {};

        std::wstring utf16_string(n_len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, utf16_string.data(), n_len);
        return utf16_string;
    }

} // namespace consolix

#endif

#endif // _CONSOLIX_ENCODING_UTILS_HPP_INCLUDED
