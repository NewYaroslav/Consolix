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
    std::string get_exec_path() {
#       ifdef _WIN32
        std::vector<wchar_t> buffer(MAX_PATH);
        HMODULE hModule = GetModuleHandle(NULL);

        DWORD size = GetModuleFileNameW(hModule, buffer.data(), (DWORD)buffer.size());

        while (size == buffer.size() && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            buffer.resize(buffer.size() * 2);
            size = GetModuleFileNameW(hModule, buffer.data(), (DWORD)buffer.size());
        }

        if (size == 0) {
            throw std::runtime_error("Failed to get executable path.");
        }

        std::wstring exe_path(buffer.begin(), buffer.begin() + size);
        return std::filesystem::path(exe_path).string();
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
    std::string get_exec_dir() {
        return std::filesystem::path(get_exec_path()).parent_path().string();
    }

    /// \brief Extracts the file name from a full file path.
    /// \param file_path The full file path as a string.
    /// \return The extracted file name, or the full string if no directory separator is found.
    std::string get_file_name(const std::string& file_path) {
        std::size_t pos = file_path.find_last_of("/\\");
        return (pos == std::string::npos) ? file_path : file_path.substr(pos + 1);
    }

    /// \brief Computes the relative path from base_path to file_path.
    /// \param file_path The target file path.
    /// \param base_path The base path from which to compute the relative path.
    /// \return A string representing the relative path from base_path to file_path.
    inline std::string make_relative(const std::string& file_path, const std::string& base_path) {
        if (base_path.empty()) return file_path;
        std::filesystem::path fileP = std::filesystem::u8path(file_path);
        std::filesystem::path baseP = std::filesystem::u8path(base_path);
        std::error_code ec;
        std::filesystem::path relativeP = std::filesystem::relative(fileP, baseP, ec);
        return ec ? file_path : relativeP.u8string();
    }

    /// \brief Constructs an absolute path relative to the executable's directory.
    /// \param relative_path The relative file path.
    /// \return The absolute file path.
    std::string resolve_exec_path(const std::string& relative_path) {
        return std::filesystem::absolute(get_exec_dir() + "/" + relative_path).string();
    }

    /// \brief Creates directories recursively for the given path.
    /// \param path The directory path to create.
    /// \throws std::runtime_error if the directories cannot be created.
    void create_directories(const std::string& path) {
        std::filesystem::path dir(path);
        if (!std::filesystem::exists(dir)) {
            std::error_code ec;
            if (!std::filesystem::create_directories(dir, ec)) {
                throw std::runtime_error("Failed to create directories for path: " + path);
            }
        }
    }

}; // namespace consolix

#endif // _CONSOLIX_PATH_UTILS_HPP_INCLUDED
