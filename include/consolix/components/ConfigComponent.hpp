#pragma once
#ifndef _CONSOLIX_CONFIG_COMPONENT_HPP_INCLUDED
#define _CONSOLIX_CONFIG_COMPONENT_HPP_INCLUDED

/// \file ConfigComponent.hpp
/// \brief Component for loading and managing configuration data from JSON files.
/// \ingroup Components

#if CONSOLIX_USE_JSON == 1

#include <fstream>
#include <nlohmann/json.hpp>

namespace consolix {

    /// \class ConfigComponent
    /// \brief Loads and manages configuration data from a JSON file.
    ///
    /// This component reads configuration data from a JSON file and stores it
    /// in a user-defined structure. It integrates with the ServiceLocator to provide
    /// global access to the configuration data.
    /// \tparam ConfigType The type of the configuration structure.
    template <typename ConfigType>
    class ConfigComponent : public IAppComponent {
    public:
        /// \brief Constructs the `ConfigComponent`.
        /// \param default_file The default file path for the configuration.
        /// \param cli_flag An optional command-line flag for specifying a custom config file path.
        explicit ConfigComponent(
            const std::string& default_file = "config.json",
            const std::string& cli_flag = "--config")
            : m_default_file(default_file), m_cli_flag(cli_flag) {
        }

        /// \brief Reloads the configuration from the file.
        void reload() {
            load_config();
        }

    protected:

        /// \brief Initializes the component.
        /// \return `true` if initialization succeeds, `false` otherwise.
        bool initialize() override {
            if (!has_service<CliOptions>()) {
                load_config();
                m_is_init = true;
                return true;
            }
            if (!has_service<CliArguments>()) return false;
            load_config();
            m_is_init = true;
            return true;
        }

        /// \brief Checks if the component has been initialized.
        /// \return `true` if the component is initialized, `false` otherwise.
        bool is_initialized() const override {
            return m_is_init;
        }

        /// \brief Executes the component's main functionality (no-op for `ConfigComponent`).
        void process() override {}

    private:
        std::string m_default_file; ///< Default configuration file path.
        std::string m_cli_flag;     ///< Command-line flag for specifying a custom config path.
        ConfigType  m_config_data;  ///< Configuration data structure.
        std::atomic<bool> m_is_init{false}; ///< Tracks whether the component is initialized.
        std::mutex  m_mutex;

        /// \brief Loads the configuration data from the specified file.
        void load_config() {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::string config_path = resolve_file_path();
            std::ifstream file(config_path);

            if (!file) {
#               if CONSOLIX_USE_LOGIT == 1
                LOGIT_PRINT_ERROR("Failed to open config file: ", config_path);
#               endif
                CONSOLIX_STREAM() <<
                    consolix::color(consolix::TextColor::Red) <<
                    "Failed to open config file: " << config_path;
                throw std::runtime_error("Failed to open config file: " + config_path);
            }

            try {
                // Read file content
                std::string json_content((std::istreambuf_iterator<char>(file)),
                                         std::istreambuf_iterator<char>());

                // Remove comments from the JSON string
                json_content = strip_json_comments(json_content);

                // Parse JSON
                nlohmann::json json_data = nlohmann::json::parse(json_content);

                m_config_data = json_data.get<ConfigType>();

                // Register the configuration in the ServiceLocator
                register_service<ConfigType>([this]() {
                    return std::make_shared<ConfigType>(m_config_data);
                });
            } catch (const std::exception& e) {
#               if CONSOLIX_USE_LOGIT == 1
                LOGIT_PRINT_ERROR("Failed to parse config file: ", e.what());
#               endif
                CONSOLIX_STREAM() <<
                    consolix::color(consolix::TextColor::Red) <<
                    "Failed to parse config file: " << e.what();
                throw;
            }
        }

        /// \brief Resolves the file path for the configuration.
        /// \return The resolved file path.
        std::string resolve_file_path() {
            std::string config_path = m_default_file;
            try {
                // Check if CLI arguments are available
                auto args = get_service<CliArguments>();
                if (args.count(m_cli_flag)) {
                    config_path = args[m_cli_flag].as<std::string>();
                }
                // Resolve relative paths to the executable's directory
                return resolve_exec_path(config_path);
            } catch (...) {
                return resolve_exec_path(m_default_file);
            }
        }
    };

} // namespace consolix

#endif // #if CONSOLIX_USE_JSON == 1

#endif // _CONSOLIX_CONFIG_COMPONENT_HPP_INCLUDED
