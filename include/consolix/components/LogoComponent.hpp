#pragma once
#ifndef _CONSOLIX_LOGO_COMPONENT_HPP_INCLUDED
#define _CONSOLIX_LOGO_COMPONENT_HPP_INCLUDED

/// \file LogoComponent.hpp
/// \brief Defines a component for displaying a customizable ASCII logo.
/// \ingroup Components

#include <iostream>
#include <string>

/// \brief Enables or disables LogIt for logo rendering.
#ifndef CONSOLIX_USE_LOGIT
#define CONSOLIX_USE_LOGIT 0
#endif

namespace consolix {

    /// \class LogoComponent
    /// \brief Component responsible for displaying a customizable ASCII logo.
    ///
    /// This component allows for displaying an ASCII logo with optional color customization.
    /// It supports direct console output or integration with LogIt for styled logging.
    class LogoComponent : public IAppComponent {
    public:
        /// \brief Constructs a `LogoComponent` with a default logo and color.
        /// \param color The color to use for the logo (default: DarkYellow).
        LogoComponent(TextColor color = TextColor::DarkYellow) :
                m_color(color) {
            const uint8_t logo[] =
            /*
               █████████                                       ████   ███
              ███░░░░░███                                     ░░███  ░░░
             ███     ░░░   ██████  ████████    █████   ██████  ░███  ████  █████ █████
            ░███          ███░░███░░███░░███  ███░░   ███░░███ ░███ ░░███ ░░███ ░░███
            ░███         ░███ ░███ ░███ ░███ ░░█████ ░███ ░███ ░███  ░███  ░░░█████░
            ░░███     ███░███ ░███ ░███ ░███  ░░░░███░███ ░███ ░███  ░███   ███░░░███
             ░░█████████ ░░██████  ████ █████ ██████ ░░██████  █████ █████ █████ █████
              ░░░░░░░░░   ░░░░░░  ░░░░ ░░░░░ ░░░░░░   ░░░░░░  ░░░░░ ░░░░░ ░░░░░ ░░░░░
              https://www.asciiart.eu/text-to-ascii-art
              https://www.rapidtables.com/convert/number/ascii-to-hex.html
              https://www.mimastech.com/charset-converter-free-online-text-files-charset-converter/
            */
            {
                0x20,0x20,0x20,0xDB,0xDB,0xDB,0xDB,0xDB,0xDB,0xDB,0xDB,0xDB,0x20,0x20,0x20,0x20,
                0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
                0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
                0x20,0x20,0x20,0xDB,0xDB,0xDB,0xDB,0x20,0x20,0x20,0xDB,0xDB,0xDB,0x20,0x20,0x20,
                0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x0A,0x20,0x20,0xDB,0xDB,0xDB,
                0xB0,0xB0,0xB0,0xB0,0xB0,0xDB,0xDB,0xDB,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
                0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
                0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0xB0,0xB0,0xDB,
                0xDB,0xDB,0x20,0x20,0xB0,0xB0,0xB0,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
                0x20,0x20,0x20,0x20,0x20,0x0A,0x20,0xDB,0xDB,0xDB,0x20,0x20,0x20,0x20,0x20,0xB0,
                0xB0,0xB0,0x20,0x20,0x20,0xDB,0xDB,0xDB,0xDB,0xDB,0xDB,0x20,0x20,0xDB,0xDB,0xDB,
                0xDB,0xDB,0xDB,0xDB,0xDB,0x20,0x20,0x20,0x20,0xDB,0xDB,0xDB,0xDB,0xDB,0x20,0x20,
                0x20,0xDB,0xDB,0xDB,0xDB,0xDB,0xDB,0x20,0x20,0xB0,0xDB,0xDB,0xDB,0x20,0x20,0xDB,
                0xDB,0xDB,0xDB,0x20,0x20,0xDB,0xDB,0xDB,0xDB,0xDB,0x20,0xDB,0xDB,0xDB,0xDB,0xDB,
                0x0A,0xB0,0xDB,0xDB,0xDB,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0xDB,
                0xDB,0xDB,0xB0,0xB0,0xDB,0xDB,0xDB,0xB0,0xB0,0xDB,0xDB,0xDB,0xB0,0xB0,0xDB,0xDB,
                0xDB,0x20,0x20,0xDB,0xDB,0xDB,0xB0,0xB0,0x20,0x20,0x20,0xDB,0xDB,0xDB,0xB0,0xB0,
                0xDB,0xDB,0xDB,0x20,0xB0,0xDB,0xDB,0xDB,0x20,0xB0,0xB0,0xDB,0xDB,0xDB,0x20,0xB0,
                0xB0,0xDB,0xDB,0xDB,0x20,0xB0,0xB0,0xDB,0xDB,0xDB,0x20,0x0A,0xB0,0xDB,0xDB,0xDB,
                0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0xB0,0xDB,0xDB,0xDB,0x20,0xB0,0xDB,
                0xDB,0xDB,0x20,0xB0,0xDB,0xDB,0xDB,0x20,0xB0,0xDB,0xDB,0xDB,0x20,0xB0,0xB0,0xDB,
                0xDB,0xDB,0xDB,0xDB,0x20,0xB0,0xDB,0xDB,0xDB,0x20,0xB0,0xDB,0xDB,0xDB,0x20,0xB0,
                0xDB,0xDB,0xDB,0x20,0x20,0xB0,0xDB,0xDB,0xDB,0x20,0x20,0xB0,0xB0,0xB0,0xDB,0xDB,
                0xDB,0xDB,0xDB,0xB0,0x20,0x20,0x0A,0xB0,0xB0,0xDB,0xDB,0xDB,0x20,0x20,0x20,0x20,
                0x20,0xDB,0xDB,0xDB,0xB0,0xDB,0xDB,0xDB,0x20,0xB0,0xDB,0xDB,0xDB,0x20,0xB0,0xDB,
                0xDB,0xDB,0x20,0xB0,0xDB,0xDB,0xDB,0x20,0x20,0xB0,0xB0,0xB0,0xB0,0xDB,0xDB,0xDB,
                0xB0,0xDB,0xDB,0xDB,0x20,0xB0,0xDB,0xDB,0xDB,0x20,0xB0,0xDB,0xDB,0xDB,0x20,0x20,
                0xB0,0xDB,0xDB,0xDB,0x20,0x20,0x20,0xDB,0xDB,0xDB,0xB0,0xB0,0xB0,0xDB,0xDB,0xDB,
                0x20,0x0A,0x20,0xB0,0xB0,0xDB,0xDB,0xDB,0xDB,0xDB,0xDB,0xDB,0xDB,0xDB,0x20,0xB0,
                0xB0,0xDB,0xDB,0xDB,0xDB,0xDB,0xDB,0x20,0x20,0xDB,0xDB,0xDB,0xDB,0x20,0xDB,0xDB,
                0xDB,0xDB,0xDB,0x20,0xDB,0xDB,0xDB,0xDB,0xDB,0xDB,0x20,0xB0,0xB0,0xDB,0xDB,0xDB,
                0xDB,0xDB,0xDB,0x20,0x20,0xDB,0xDB,0xDB,0xDB,0xDB,0x20,0xDB,0xDB,0xDB,0xDB,0xDB,
                0x20,0xDB,0xDB,0xDB,0xDB,0xDB,0x20,0xDB,0xDB,0xDB,0xDB,0xDB,0x0A,0x20,0x20,0xB0,
                0xB0,0xB0,0xB0,0xB0,0xB0,0xB0,0xB0,0xB0,0x20,0x20,0x20,0xB0,0xB0,0xB0,0xB0,0xB0,
                0xB0,0x20,0x20,0xB0,0xB0,0xB0,0xB0,0x20,0xB0,0xB0,0xB0,0xB0,0xB0,0x20,0xB0,0xB0,
                0xB0,0xB0,0xB0,0xB0,0x20,0x20,0x20,0xB0,0xB0,0xB0,0xB0,0xB0,0xB0,0x20,0x20,0xB0,
                0xB0,0xB0,0xB0,0xB0,0x20,0xB0,0xB0,0xB0,0xB0,0xB0,0x20,0xB0,0xB0,0xB0,0xB0,0xB0,
                0x20,0xB0,0xB0,0xB0,0xB0,0xB0,0x20,0x00
            };
            m_logo = std::string(reinterpret_cast<const char*>(logo));
        }

        /// \brief Constructs a `LogoComponent` with a custom logo and color.
        /// \param logo The ASCII logo to display.
        /// \param color The color to use for the logo (default: DarkYellow).
        LogoComponent(
                const std::string& logo,
                TextColor color = TextColor::DarkYellow) :
            m_logo(logo), m_color(color) {
        }

        /// \brief Virtual destructor.
        virtual ~LogoComponent() = default;

        /// \brief Sets a custom logo and color.
        /// \param logo The custom ASCII logo as a string.
        /// \param color The color to display the logo in.
        void set_logo(const std::string& logo, TextColor color) {
            m_logo = logo;
            m_color = color;
        }

        /// \brief Initializes the component and displays the logo.
        /// \return `true` if initialization succeeds, `false` otherwise.
        bool initialize() override {
            // Настроим консоль на Code Page 437, чтобы символы отображались корректно.
            //SetConsoleOutputCP(437);
            //SetConsoleCP(65001);        //установка кодовой страницы utf-8 (Unicode) для вводного потока
            //SetConsoleOutputCP(65001);  //установка кодовой страницы utf-8 (Unicode)
            if (!m_logo.empty()) {
#               if CONSOLIX_USE_LOGIT == 0
                set_console_color();
                std::cout << m_logo << std::endl;
                reset_console_color();
#               else
                CONSOLIX_LOGO_STREAM() << to_c_str(m_color) << m_logo << to_c_str(TextColor::LightGray);
#               endif
            }
            m_is_init = true;
            return true;
        }

        /// \brief Checks if the component has been initialized.
        /// \return `true` if the component is initialized, `false` otherwise.
        bool is_initialized() const override {
            return m_is_init;
        }

        /// \brief Executes the component (no operation for LogoComponent).
        void process() override {};

    private:
        std::string m_logo;                ///< The ASCII logo text to display.
        TextColor   m_color;               ///< The color of the logo.
        std::atomic<bool> m_is_init{false};///< Indicates whether the component is initialized.

        /// \brief Sets the console color before displaying the logo.
        void set_console_color() {
#           if defined(_WIN32) || defined(_WIN64)
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hConsole, to_windows_color(m_color));
#           else
            if (isatty(STDOUT_FILENO)) {
                std::cout << to_c_str(m_color);
            }
#           endif
        }

        /// \brief Resets the console color to the default.
        void reset_console_color() {
#if         defined(_WIN32) || defined(_WIN64)
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#           else
            if (isatty(STDOUT_FILENO)) {
                std::cout << "\033[0m";
            }
#           endif
        }
    }; // LogoComponent

}; // namespace consolix

#endif // _CONSOLIX_LOGO_COMPONENT_HPP_INCLUDED
