#pragma once
#ifndef _CONSOLIX_SYSTEM_UTILS_HPP_INCLUDED
#define _CONSOLIX_SYSTEM_UTILS_HPP_INCLUDED

/// \file system_utils.hpp
/// \brief Provides system-related utility functions such as clipboard handling, OS detection, and system information retrieval.

#include <string>
#include <cstdlib>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <pwd.h>
#endif

namespace consolix {

    /// \brief Copies the given text to the system clipboard.
    /// \param text The text to copy.
    /// \return True if the operation was successful, false otherwise.
    bool copy_to_clipboard(const std::string& text) {
#       ifdef _WIN32
        if (OpenClipboard(nullptr)) {
            EmptyClipboard();
            size_t size = (text.size() + 1) * sizeof(char);
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
            if (!hMem) {
                CloseClipboard();
                return false;
            }
            memcpy(GlobalLock(hMem), text.c_str(), size);
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
            CloseClipboard();
            return true;
        }
        return false;
#       elif defined(__APPLE__)
        std::string command = "echo \"" + text + "\" | pbcopy";
        return system(command.c_str()) == 0;
#       elif defined(__linux__)
        std::string command = "echo \"" + text + "\" | xclip -selection clipboard";
        return system(command.c_str()) == 0;
#       else
        return false;
#       endif
    }

    /// \brief Retrieves text from the system clipboard.
    /// \return Clipboard contents as a string, or an empty string on failure.
    std::string get_clipboard_text() {
#       ifdef _WIN32
        if (!OpenClipboard(nullptr)) return "";
        HANDLE hData = GetClipboardData(CF_TEXT);
        if (!hData) {
            CloseClipboard();
            return "";
        }
        char* text = static_cast<char*>(GlobalLock(hData));
        std::string result(text ? text : "");
        GlobalUnlock(hData);
        CloseClipboard();
        return result;
#       elif defined(__APPLE__)
        return system("pbpaste") == 0 ? "(clipboard contents)" : "";
#       elif defined(__linux__)
        return system("xclip -selection clipboard -o") == 0 ? "(clipboard contents)" : "";
#       else
        return "";
#       endif
    }

    /// \brief Gets the name of the operating system.
    /// \return A string representing the OS name.
    std::string get_os_name() {
#       ifdef _WIN32
        return "Windows";
#       elif defined(__APPLE__)
        return "macOS";
#       elif defined(__linux__)
        return "Linux";
#       else
        return "Unknown OS";
#       endif
    }

    /// \brief Gets the current system time in milliseconds.
    /// \return System time in milliseconds.
    uint64_t get_system_time_ms() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    /// \brief Gets the number of logical CPU cores.
    /// \return The number of logical CPU cores.
    int get_cpu_count() {
#       ifdef _WIN32
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return sysinfo.dwNumberOfProcessors;
#       elif defined(__linux__) || defined(__APPLE__)
        return sysconf(_SC_NPROCESSORS_ONLN);
#       else
        return 1;
#       endif
    }

    /// \brief Gets the user's home directory path.
    /// \return A string containing the home directory path.
    std::string get_home_directory() {
#       ifdef _WIN32
        char* home = getenv("USERPROFILE");
        return home ? std::string(home) : "";
#       elif defined(__linux__) || defined(__APPLE__)
        struct passwd* pw = getpwuid(getuid());
        return pw ? std::string(pw->pw_dir) : "";
#       else
        return "";
#       endif
    }

    /// \brief Gets the system temporary directory path.
    /// \return A string containing the temporary directory path.
    std::string get_temp_directory() {
#       ifdef _WIN32
        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        return std::string(tempPath);
#       elif defined(__linux__) || defined(__APPLE__)
        char* temp = getenv("TMPDIR");
        return temp ? std::string(temp) : "/tmp";
#       else
        return "";
#       endif
    }

    /// \brief Retrieves an environment variable's value.
    /// \param var_name The name of the environment variable.
    /// \return The value of the environment variable, or an empty string if not found.
    std::string get_env_var(const std::string& var_name) {
        const char* value = getenv(var_name.c_str());
        return value ? std::string(value) : "";
    }

}; // namespace consolix

#endif // _CONSOLIX_SYSTEM_UTILS_HPP_INCLUDED
