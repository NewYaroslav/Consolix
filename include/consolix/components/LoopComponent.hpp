#pragma once
#ifndef _CONSOLIX_LOOP_COMPONENT_HPP_INCLUDED
#define _CONSOLIX_LOOP_COMPONENT_HPP_INCLUDED

/// \file LoopComponent.hpp
/// \brief Defines a component for running a customizable execution loop.
/// \ingroup Components

#include <thread>
#include <atomic>
#include <chrono>
#include <functional>

namespace consolix {

    /// \class LoopComponent
    /// \brief Component for managing a customizable execution loop.
    ///
    /// This component allows for running a loop with user-defined behavior.
    /// It supports three customizable functions:
    /// - `on_initialize`: Called once during initialization.
    /// - `on_execute`: Called repeatedly while the loop is running.
    /// - `on_shutdown`: Called during shutdown.
    class LoopComponent :
        public IAppComponent,
        public IShutdownable  {
    public:

        /// \brief Default constructor.
        LoopComponent() = default;

        /// \brief Constructor with customizable callbacks.
        /// \param on_initialize Function to execute during initialization.
        /// \param on_execute Function to execute in the loop.
        /// \param on_shutdown Function to execute during shutdown.
        LoopComponent(
                std::function<bool()>    on_initialize,
                std::function<void()>    on_execute,
                std::function<void(int)> on_shutdown) :
            m_on_initialize(std::move(on_initialize)),
            m_on_execute(std::move(on_execute)),
            m_on_shutdown(std::move(on_shutdown)) {
        }

        /// \brief Virtual destructor.
        virtual ~LoopComponent() override = default;

        /// \brief Access the initialization function.
        /// \return Reference to the initialization function.
        std::function<bool()>& on_initialize() {
            return m_on_initialize;
        }

        /// \brief Access the execution function.
        /// \return Reference to the execution function.
        std::function<void()>& on_execute() {
            return m_on_execute;
        }

        /// \brief Access the shutdown function.
        /// \return Reference to the shutdown function.
        std::function<void(int)>& on_shutdown() {
            return m_on_shutdown;
        }

    protected:

        /// \brief Initializes the component.
        /// \return `true` if initialization succeeds, `false` otherwise.
        bool initialize() override {
            try {
                if (m_on_initialize) {
                    m_is_init = m_on_initialize();
                } else m_is_init = true;
                return m_is_init;
            } catch (const std::exception& e) {
#               if CONSOLIX_USE_LOGIT == 1
                LOGIT_PRINT_ERROR("Unhandled exception during initialization: ", e.what());
#               else
                CONSOLIX_STREAM() << "Unhandled exception during initialization: " << e.what() << std::endl;
#               endif
                stop();
                throw;
            }
        }

        /// \brief Checks if the component is initialized.
        /// \return `true` if the component is initialized, `false` otherwise.
        bool is_initialized() override const {
            return m_is_init;
        }

        /// \brief Executes the main loop logic.
        ///
        /// Calls the `on_execute` function repeatedly. If no function is provided,
        /// the loop idles with a short sleep.
        void execute() override {
            try {
                if (m_on_execute) m_on_execute();
                else std::this_thread::sleep_for(std::chrono::milliseconds(1));
            } catch (const std::exception& e) {
#               if CONSOLIX_USE_LOGIT == 1
                LOGIT_PRINT_ERROR("Unhandled exception during execution: , e.what());
#               else
                CONSOLIX_STREAM() << "Unhandled exception during execution:  << e.what() << std::endl;
#               endif
                stop();
                throw;
            }
        }

        /// \brief Shuts down the component.
        /// \param signal The shutdown signal.
        void shutdown(int signal) override {
            try {
                if (m_on_shutdown) m_on_shutdown(signal);
                stop();
            } catch (const std::exception& e) {
#               if CONSOLIX_USE_LOGIT == 1
                LOGIT_PRINT_ERROR("Unhandled exception during shutdown: ", e.what());
#               else
                CONSOLIX_STREAM() << "Unhandled exception during shutdown: " << e.what() << std::endl;
#               endif
                throw;
            }
        }

    private:
        std::function<bool()> m_on_initialize;   ///< Initialization function.
        std::function<void()> m_on_execute;      ///< Execution function for the loop.
        std::function<void(int)> m_on_shutdown;  ///< Shutdown function.
        std::atomic<bool> m_is_init{false};      ///< Indicates whether the component is initialized.
    }; // LoopComponent

}; // namespace consolix

#endif // _CONSOLIX_LOOP_COMPONENT_HPP_INCLUDED
