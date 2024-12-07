#pragma once
#ifndef _CONSOLIX_CONSOLE_APPLICATION_HPP_INCLUDED
#define _CONSOLIX_CONSOLE_APPLICATION_HPP_INCLUDED

/// \file ConsoleApplication.hpp
/// \brief Manages the lifecycle of a console application.
/// \ingroup Core

#include <csignal>

namespace consolix {

    /// \class ConsoleApplication
    /// \brief Singleton class to manage the lifecycle of a console application.
    ///
    /// Provides functionality to initialize, execute, and gracefully shut down components
    /// in a structured lifecycle. Supports signal handling for both Windows and POSIX systems.
    class ConsoleApplication {
    public:

        /// \brief Retrieves the singleton instance of the application.
        /// \return A reference to the `ConsoleApplication` instance.
        static ConsoleApplication& get_instance() {
            static ConsoleApplication instance;
            return instance;
        }

        /// \brief Adds a new component to the application.
        /// \tparam Component The type of the component to create.
        /// \tparam Args The argument types for the component's constructor.
        /// \param args Arguments for constructing the component.
        /// \return A `std::shared_ptr` to the newly added component.
        template <typename Component, typename... Args>
        std::shared_ptr<Component> add(Args&&... args) {
            return m_manager.add<Component>(std::forward<Args>(args)...);
        }

        /// \brief Adds an existing component to the application.
        /// \param component A shared pointer to the component.
        void add(std::shared_ptr<IAppComponent> component) {
            m_manager.add(std::move(component));
        }

        /// \brief Initializes the application and its components.
        /// This method initializes all components in the manager and ensures readiness
        void init() {
            try {
                while (m_running && !m_manager.initialize()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                if (!m_running) cleanup(0);
            } catch (const std::exception& e) {
                handle_fatal_exception(e);
            }
        }

        /// \brief Initializes the application with a custom action.
        /// \tparam InitAction A callable type for the custom initialization action.
        /// \param init_action The custom action to perform during initialization.
        template <typename InitAction>
        void init(InitAction init_action) {
            try {
                while (m_running && !m_manager.initialize()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                if (!m_running) cleanup(0);
                init_action();
                if (!m_running) cleanup(0);
            } catch (const std::exception& e) {
                handle_fatal_exception(e);
            }
        }

        /// \brief Runs the application with the registered components.
        void run() {
            m_running = true;
            setup_signal_handlers();
            init();
            lifecycle_loop();
        }

        /// \brief Runs the application with a custom loop action.
        /// \tparam IterationAction A callable type executed within the main loop.
        /// \param iteration_action The custom action to perform in each loop iteration.
        template <typename IterationAction>
        void run(IterationAction iteration_action) {
            m_running = true;
            setup_signal_handlers();
            init();
            lifecycle_loop(iteration_action);
        }

        /// \brief Cleans up the application and shuts down all components.
        /// \param signal The signal indicating the reason for cleanup.
        void cleanup(int signal) {
#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_PRINT_INFO("Cleaning up application for signal: ", signal);
#           endif
            m_manager.shutdown(signal);
        }

        /// \brief Stops the application's main loop.
        void stop() {
            m_running = false;
        }

    private:
        AppComponentManager m_manager;
        std::atomic<bool>   m_running{false}; ///< Flag indicating whether the loop is running.

        /// \brief Sets up signal handlers for graceful application termination.
        void setup_signal_handlers() {
#           if defined(_WIN32) || defined(_WIN64)
            SetConsoleCtrlHandler(console_handler, TRUE);
#           else
            std::signal(SIGINT, signal_handler);
            std::signal(SIGTERM, signal_handler);
#           endif
        }

        /// \brief The main lifecycle loop with a custom action.
        /// \tparam IterationAction A callable executed within the loop.
        /// \param iteration_action The action to execute in each iteration.
        template <typename IterationAction>
        void lifecycle_loop(IterationAction iteration_action) {
            try {
                while (m_running) {
                    m_manager.process();
                    iteration_action();
                }
                cleanup(0);
            } catch (const std::exception& e) {
                handle_fatal_exception(e);
            }
        }

        /// \brief The main lifecycle loop.
        void lifecycle_loop() {
            try {
                while (m_running) {
                    m_manager.process();
                }
                cleanup(0);
            } catch (const std::exception& e) {
                handle_fatal_exception(e);
            }
        }

#       if defined(_WIN32) || defined(_WIN64)

        /// \brief Handles Windows console events.
        static BOOL WINAPI console_handler(DWORD event) {
            switch (event) {
            case CTRL_C_EVENT:
                log_event("CTRL_C_EVENT");
                break;
            case CTRL_CLOSE_EVENT:
                log_event("CTRL_CLOSE_EVENT");
                break;
            case CTRL_LOGOFF_EVENT:
                log_event("CTRL_LOGOFF_EVENT");
                break;
            case CTRL_SHUTDOWN_EVENT:
                log_event("CTRL_SHUTDOWN_EVENT");
                break;
            default:
                log_event("UNKNOWN_EVENT");
                break;
            }
            get_instance().cleanup(event_to_signal(event));
#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_WAIT();
#           endif
            return TRUE;
        }

        /// \brief Logs Windows console events.
        static void log_event(const char* event_name) {
#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_PRINT_INFO("Console event received: ", event_name);
#           endif
        }

        /// \brief Maps Windows console events to POSIX signals.
        static int event_to_signal(DWORD event) {
            switch (event) {
            case CTRL_C_EVENT: return SIGINT;
            default: return SIGTERM;
            };
        }

#       else

        /// \brief Handles POSIX signals.
        static void signal_handler(int signal) {
            switch (signal) {
            case SIGINT:
                handle_signal("SIGINT", signal);
                break;
            case SIGTERM:
                handle_signal("SIGTERM", signal);
                break;
            default:
                handle_signal("UNKNOWN_SIGNAL", signal);
                break;
            }
        }

        /// \brief Logs and handles POSIX signals.
        static void handle_signal(const char* signal_name, int signal) {
#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_PRINT_INFO("POSIX signal received: ", signal_name, ", signal: ", signal);
#           endif
            get_instance().cleanup(signal);
#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_WAIT();
#           endif
            std::exit(signal);
        }

#       endif

        /// \brief Handles fatal exceptions by logging the error and terminating the application.
        /// \param e The exception that caused the fatal error.
        void handle_fatal_exception(const std::exception& e) {
#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_PRINT_FATAL("Unhandled exception: ", e.what());
            cleanup(-1);
            LOGIT_WAIT();
            std::exit(-1);
#           else
            cleanup(-1);
            std::exit(-1);
#           endif
        }

        /// \brief Handles fatal exceptions without an exception object.
        /// Useful for unknown or non-standard errors.
        void handle_fatal_exception() {
#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_PRINT_FATAL("Unhandled unknown exception occurred.");
            cleanup(-1);
            LOGIT_WAIT();
            std::exit(-1);
#           else
            cleanup(-1);
            std::exit(-1);
#           endif
        }

        ConsoleApplication() = default;
        ~ConsoleApplication() = default;

        // Deleting copy and move constructors and assignment operators to enforce singleton.
        ConsoleApplication(const ConsoleApplication&) = delete;
        ConsoleApplication& operator=(const ConsoleApplication&) = delete;
        ConsoleApplication(ConsoleApplication&&) = delete;
        ConsoleApplication& operator=(ConsoleApplication&&) = delete;
    }; // ConsoleApplication

}; // namespace consolix

#endif // _CONSOLIX_CONSOLE_APPLICATION_HPP_INCLUDED
