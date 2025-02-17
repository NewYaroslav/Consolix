#pragma once
#ifndef _CONSOLIX_APP_COMPONENT_MANAGER_HPP_INCLUDED
#define _CONSOLIX_APP_COMPONENT_MANAGER_HPP_INCLUDED

/// \file AppComponentManager.hpp
/// \brief Manages application components with lifecycle support.
/// \ingroup Core

#include <vector>

namespace consolix {

    /// \class AppComponentManager
    /// \brief Manages a collection of application components with lifecycle support.
    ///
    /// The manager provides controlled initialization, execution, and shutdown for all
    /// registered components. It ensures:
    /// - `initialize()`: Prepares each component for execution.
    /// - `process()`: Executes the main loop logic for each component.
    /// - `shutdown(signal)`: Implements a "soft shutdown" by continuing to shut down other
    ///   components even if one fails, and logs aggregated errors if any occur.
    ///
    /// Components are stored as `std::shared_ptr` for memory safety and compatibility with
    /// the `ServiceLocator` pattern.
    class AppComponentManager {
    public:

        /// \brief Constructs an empty component manager.
        AppComponentManager() = default;

        /// \brief Destroys the component manager and clears all components.
        ~AppComponentManager() {
            m_components.clear(); // Ensures the container is empty.
        }

        /// \brief Adds a new component to the manager.
        ///
        /// Constructs a new component of type `Component` and registers it with the manager.
        /// \tparam Component The type of the component to create.
        /// \tparam Args Argument types for the component's constructor.
        /// \param args Arguments for constructing the component.
        /// \return A shared pointer to the newly created component.
        template <typename Component, typename... Args>
        std::shared_ptr<Component> add(Args&&... args) {
            auto ptr = std::make_shared<Component>(std::forward<Args>(args)...);
            m_components.push_back(ptr);
            return ptr;
        }

        /// \brief Adds an existing component to the manager.
        /// \param component A `std::shared_ptr` to the component to add.
        void add(std::shared_ptr<IAppComponent> component) {
            m_components.push_back(std::move(component));
        }

        /// \brief Initializes all registered components.
        ///
        /// Skips components that are already initialized.
        /// \return `true` if all components are initialized successfully, `false` otherwise.
        /// \throws std::exception If any component fails during initialization.
        bool initialize() {
            try {
                for (const auto& component : m_components) {
                    if (component->is_initialized()) continue;
                    component->initialize();
                }
                return is_initialized();
            } catch (const std::exception& e) {
#               if CONSOLIX_USE_LOGIT == 1
                LOGIT_PRINT_FATAL("Initialization error: ", e.what());
#               endif
                throw;
            }
        }

        /// \brief Checks if all components are initialized.
        /// \return `true` if all components are initialized, `false` otherwise.
        bool is_initialized() const {
            for (const auto& component : m_components) {
                if (!component->is_initialized()) return false;
            }
            return true;
        }

        /// \brief Executes the main functionality of all components.
        ///
        /// Calls `process()` on each managed component.
        /// \throws std::exception If any component fails during execution.
        void process() {
            try {
                for (const auto& component : m_components) {
                    component->process();
                }
            } catch (const std::exception& e) {
#               if CONSOLIX_USE_LOGIT == 1
                LOGIT_PRINT_FATAL("Processing error: ", e.what());
#               endif
                throw;
            }
        }

        /// \brief Shuts down all components with "soft shutdown" support.
        ///
        /// Calls `shutdown(signal)` on components implementing the `IShutdownable` interface.
        /// If any component throws an exception during shutdown, the error is logged and
        /// stored, and the shutdown process continues for the remaining components.
        /// At the end, a summary of all errors is logged, and a `std::runtime_error` is thrown
        /// if any errors occurred.
        ///
        /// This mechanism ensures that a failure in one component does not prevent others
        /// from shutting down gracefully.
        ///
        /// \param signal The signal to pass to the shutdown method.
        /// \throws std::runtime_error If one or more components fail during shutdown.
        void shutdown(int signal) {
#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_PRINT_INFO("Starting shutdown with signal: ", signal);
#           endif
            std::vector<std::string> errors; // Собираем ошибки
            for (size_t index = 0; index < m_components.size(); ++index) {
                const auto& component = m_components[index];
                try {
                    if (auto shutdownable = std::dynamic_pointer_cast<IShutdownable>(component)) {
                        shutdownable->shutdown(signal);
                    }
                } catch (const std::exception& e) {
#                   if CONSOLIX_USE_LOGIT == 1
                    LOGIT_PRINT_FATAL("Component [", index, "] shutdown error: ", e.what());
#                   endif
                    errors.push_back("Component [" + std::to_string(index) + "] error: " + e.what());
                } catch (...) {
#                   if CONSOLIX_USE_LOGIT == 1
                    LOGIT_PRINT_FATAL("Component [", index, "] shutdown error: Unknown error");
#                   endif
                    errors.push_back("Component [" + std::to_string(index) + "] error: Unknown error");
                }
            }

            if (!errors.empty()) {
                // Обрабатываем собранные ошибки
#               if CONSOLIX_USE_LOGIT == 1
                std::string summary = std::accumulate(
                    errors.begin(), errors.end(), std::string(),
                    [](const std::string& acc, const std::string& error) {
                        return acc.empty() ? error : acc + "; " + error;
                    }
                );
                LOGIT_PRINT_FATAL("Shutdown completed with errors. Summary: ", summary);
#               endif
                //throw std::runtime_error("Shutdown completed with errors. See logs for details.");
            }
        }

    private:
        /// \brief List of managed application components.
        std::vector<std::shared_ptr<IAppComponent>> m_components;
    }; // AppComponentManager

}; // namespace consolix

#endif // _CONSOLIX_APP_COMPONENT_MANAGER_HPP_INCLUDED
