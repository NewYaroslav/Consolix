#pragma once
#ifndef _CONSOLIX_CLI_COMPONENT_HPP_INCLUDED
#define _CONSOLIX_CLI_COMPONENT_HPP_INCLUDED

/// \file CliComponent.hpp
/// \brief Component for handling command-line arguments using cxxopts library.
/// \ingroup Components

#if CONSOLIX_USE_CXXOPTS == 1

#include <cxxopts.hpp>
#include <string>
#include <memory>

namespace consolix {

    /// \class CliComponent
    /// \brief Component for parsing and managing command-line arguments.
    ///
    /// This component leverages the `cxxopts` library to parse command-line arguments
    /// and integrates with the service locator pattern to provide global access to parsed options.
    class CliComponent : public IAppComponent {
    public:

        /// \brief Constructs a `CliComponent` with automatic service registration.
        /// \param name Name displayed in `--help` (does not affect the actual program name).
        /// \param description Brief description displayed in the help message.
        /// \param creator A function to initialize command-line options.
        CliComponent(
                const std::string& name,
                const std::string& description,
                std::function<void(CliOptions&)> creator)
            : m_creator(std::move(creator)) {
            register_service<CliOptions>([&name, &description]() {
                return std::make_shared<CliOptions>(name, description);
            });
        }

        /// \brief Constructs a `CliComponent` with pre-parsed arguments.
        /// \param name Name displayed in `--help` (does not affect the actual program name).
        /// \param description Brief description displayed in the help message.
        /// \param creator A function to initialize command-line options.
        /// \param argc The number of command-line arguments.
        /// \param argv The command-line argument values.
        CliComponent(
                const std::string& name,
                const std::string& description,
                std::function<void(CliOptions&)> creator,
                int argc,
                const char* argv[])
            : m_creator(std::move(creator)) {
            register_service<CliOptions>([&name, &description](){
                return std::make_shared<CliOptions>(name, description);
            });
            m_argc = argc;
            for (int i = 0; i < argc; ++i) {
                m_argv_c.push_back(argv[i]);
            }
        }

        /// \brief Adds a command-line option without a default value.
        /// \tparam T The type of the option value.
        /// \param key The option key.
        /// \param description The description of the option.
        template <typename T>
        void add_option(const std::string& key, const std::string& description) {
            get_service<CliOptions>().add_options()(key, description, cxxopts::value<T>());
        }

        /// \brief Adds a command-line option with a default value.
        /// \tparam T The type of the option value.
        /// \param key The option key.
        /// \param description The description of the option.
        /// \param default_value The default value of the option.
        template <typename T>
        void add_option(const std::string& key, const std::string& description, T default_value) {
            get_service<CliOptions>().add_options()(key, description, cxxopts::value<T>()->default_value(std::to_string(default_value)));
        }

        /// \brief Adds a command-line option with an implicit value.
        /// \tparam T The type of the option value.
        /// \param key The option key.
        /// \param description The description of the option.
        /// \param implicit_value The implicit value of the option.
        template <typename T>
        void add_option_implicit(const std::string& key, const std::string& description, T implicit_value) {
            get_service<CliOptions>().add_options()(key, description, cxxopts::value<T>()->implicit_value(std::to_string(implicit_value)));
        }

        /// \brief Parses the command-line arguments.
        /// \param argc The number of command-line arguments.
        /// \param argv The command-line argument values.
        void parse(int argc, const char* argv[]) {
            try {
                auto result = get_service<CliOptions>().parse(argc, argv);
                register_service<CliArguments>([&result](){
                    return std::make_shared<CliArguments>(result);
                });
            } catch (const std::exception& e) {
                CONSOLIX_STREAM() << "Error parsing command-line arguments: " << e.what() << std::endl;
#               if CONSOLIX_USE_LOGIT == 1
                LOGIT_PRINT_ERROR("Error parsing command-line arguments: ", e.what());
#               endif
                throw;
            }
        }

        /// \brief Retrieves the value of a parsed option.
        /// \tparam T The type of the option value.
        /// \param key The option key.
        /// \return The value of the option.
        /// \throws std::runtime_error If the option is not found.
        template <typename T>
        T get(const std::string& key) const {
            if (!get_service<CliArguments>().count(key)) {
#               if CONSOLIX_USE_LOGIT == 1
                LOGIT_PRINT_ERROR("Option not found: ", key);
#               endif
                throw std::runtime_error("Option not found: " + key);
            }
            return get_service<CliArguments>()[key].as<T>();
        }

    protected:

        /// \brief Initializes the `CliComponent`.
        /// \return `true` if initialization is successful, `false` otherwise.
        bool initialize() override {
            try {
                auto &options = get_service<CliOptions>();
                if (!m_creator) {
                    throw std::runtime_error("CLI options creator function is null. Ensure you provide a valid function to initialize CLI options.");
                }
                m_creator(options);
#               if defined(_WIN32) || defined(_WIN64)
                parse_windows();
#               else
                if (m_argc) {
                    parse(m_argc, m_argv_c.data());
                }
#               endif
            } catch (const std::exception& e) {
#               if CONSOLIX_USE_LOGIT == 1
                LOGIT_PRINT_ERROR("CLI component initialization failed: ", e.what());
                throw;
#               else
                CONSOLIX_STREAM() << "Error initializing CLI component: " << e.what() << std::endl;
                throw;
#               endif
            }
            m_is_init = true;
            return true;
        }

        /// \brief Checks if the `CliComponent` is initialized.
        /// \return `true` if initialized, `false` otherwise.
        bool is_initialized() const override {
            return m_is_init;
        }

        /// \brief Processes the component (no-op for `CliComponent`).
        void process() override {}

    private:
        int m_argc = 0;                             ///< The number of command-line arguments passed to the program.
        std::vector<const char*> m_argv_c;          ///< A copy of command-line arguments in `const char*` format, compatible with cxxopts.
        std::function<void(CliOptions&)> m_creator; ///< A function to initialize and configure command-line options.
        std::atomic<bool> m_is_init{false};         ///< Initialization flag. `true` if the component is successfully initialized.

#       if defined(_WIN32) || defined(_WIN64)
        /// \brief Parse arguments using Windows API.
        void parse_windows() {
            int argc = 0;
            LPWSTR* argv_w = CommandLineToArgvW(GetCommandLineW(), &argc);

            std::vector<std::string> args;
            for (int i = 0; i < argc; ++i) {
                args.push_back(utf16_to_utf8(argv_w[i]));
            }

            // Convert to standard `char*` for cxxopts
            std::vector<const char*> argv_c;
            for (const auto& arg : args) {
                argv_c.push_back(arg.c_str());
            }

            parse(argc, argv_c.data());
        }
#       endif

    }; // CliComponent

}; // namespace consolix

#endif // #if CONSOLIX_USE_CXXOPTS == 1

#endif // _CONSOLIX_CLI_COMPONENT_HPP_INCLUDED
