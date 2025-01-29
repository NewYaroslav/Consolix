#pragma once
#ifndef _CONSOLIX_TITLE_COMPONENT_HPP_INCLUDED
#define _CONSOLIX_TITLE_COMPONENT_HPP_INCLUDED

/// \file TitleComponent.hpp
/// \brief Component for managing console window titles in a platform-independent way.
/// \ingroup Components

#include <string>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <iostream>
#endif


namespace consolix {

    /// \class TitleComponent
    /// \brief Handles setting and retrieving the console window title.
    ///
    /// This component provides a platform-independent way to change the console title.
    /// - On Windows, it uses `SetConsoleTitleW()`.
    /// - On Linux/macOS, it prints an ANSI escape sequence `\033]0;TITLE\007`.
    /// - The title is also stored internally and can be retrieved later.
    class TitleComponent : public IAppComponent {
    public:
        /// \brief Constructs the component and optionally sets the console title.
        /// \param title The title to set for the console window.
        explicit TitleComponent(const std::string& title = std::string()) {
            if (!title.empty()) {
                set_title(title);
            }
        }

        /// \brief Virtual destructor.
        virtual ~TitleComponent() override = default;

        /// \brief Sets the console window title.
        /// \param title The new console title.
        void set_title(const std::string& title) {
            m_title = title;
#           if defined(_WIN32) || defined(_WIN64)
            std::wstring wtitle = utf8_to_utf16(title);
            SetConsoleTitleW(wtitle.c_str());
#           else
            std::cout << "\033]0;" << title << "\007";
#           endif
        }

        /// \brief Gets the last set console title.
        /// \return The current console title.
        std::string get_title() const {
            return m_title;
        }

        /// \brief Retrieves the executable name.
        /// \return The executable filename (without path).
        static std::string get_executable_name() {
            return get_file_name(get_exec_path());
        }

        /// \brief Initializes the component (no-op).
        bool initialize() override {
            return true;
        }

        /// \brief Checks if the component is initialized.
        bool is_initialized() const override {
            return true;
        }

        /// \brief Executes the component (no-op).
        void process() override {}

    private:
        std::string m_title; ///< Stores the last set title.
    };

};

#endif // _CONSOLIX_TITLE_COMPONENT_HPP_INCLUDED
