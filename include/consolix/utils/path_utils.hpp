#pragma once
#ifndef _CONSOLIX_PATH_UTILS_HPP_INCLUDED
#define _CONSOLIX_PATH_UTILS_HPP_INCLUDED

/// \file path_utils.hpp
/// \brief Utilities for working with file and directory paths, including resolving paths relative to the executable.

#include <string>
#include <vector>
#include <filesystem>

namespace consolix {
    namespace fs = std::filesystem;

    /// \brief Retrieves the full path of the executable.
    /// \return A string containing the full path of the executable.
    inline std::string get_exec_path() {
#       ifdef _WIN32
        std::vector<wchar_t> buffer(MAX_PATH);
        HMODULE hModule = GetModuleHandle(NULL);

        // Пробуем получить путь
        DWORD size = GetModuleFileNameW(hModule, buffer.data(), buffer.size());

        // Если путь слишком длинный, увеличиваем буфер
        while (size == buffer.size() && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            buffer.resize(buffer.size() * 2);  // Увеличиваем буфер в два раза
            size = GetModuleFileNameW(hModule, buffer.data(), buffer.size());
        }

        if (size == 0) {
            throw std::runtime_error("Failed to get executable path.");
        }

        std::wstring exe_path(buffer.begin(), buffer.begin() + size);

        // Преобразуем из std::wstring (UTF-16) в std::string (UTF-8)
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(exe_path);
#       else
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);

        if (count == -1) {
            throw std::runtime_error("Failed to get executable path.");
        }

        std::string exe_path(result, count);

        return exe_path;
#       endif
    }

    /// \brief Retrieves the directory of the executable file.
    /// \return A string containing the directory path of the executable.
    inline std::string get_exec_dir() {
        return (fs::u8path(get_exec_path())).parent_path().u8string();
    }

    /// \brief Extracts the file name from a full file path.
    /// \param file_path The full file path as a string.
    /// \return The extracted file name, or the full string if no directory separator is found.
    inline std::string get_file_name(const std::string& file_path) {
        //std::size_t pos = file_path.find_last_of("/\\");
        //return (pos == std::string::npos) ? file_path : file_path.substr(pos + 1);
        return (fs::u8path(file_path)).filename().u8string();
    }

    /// \brief Resolves a relative path to absolute, based on executable location.
    /// \param relative_path Relative path from executable directory.
    /// \return Absolute path string.
    inline std::string resolve_exec_path(const std::string& relative_path) {
        return (fs::u8path(get_exec_dir()) / fs::u8path(relative_path)).u8string();
    }

    /// \brief Computes the relative path from base_path to file_path.
    /// \param file_path The target file path.
    /// \param base_path The base path from which to compute the relative path.
    /// \return A string representing the relative path from base_path to file_path.
    inline std::string make_relative(const std::string& file_path, const std::string& base_path) {
        if (base_path.empty()) return file_path;
        fs::path fileP = fs::u8path(file_path);
        fs::path baseP = fs::u8path(base_path);
        std::error_code ec;
        fs::path relativeP = fs::relative(fileP, baseP, ec);
        return ec ? file_path : relativeP.u8string();
    }

    /// \brief Creates directories recursively for the given path.
    /// \param path The directory path to create.
    /// \throws std::runtime_error if the directories cannot be created.
    inline void create_directories(const std::string& path) {
        fs::path dir = fs::u8path(path);
        if (!std::filesystem::exists(dir)) {
            std::error_code ec;
            if (!std::filesystem::create_directories(dir, ec)) {
                throw std::runtime_error("Failed to create directories for path: " + dir.u8string());
            }
        }
    }

}; // namespace consolix

#endif // _CONSOLIX_PATH_UTILS_HPP_INCLUDED
