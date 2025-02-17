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
            static ConsoleApplication* instance = new ConsoleApplication();
            return *instance;
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
                while (!m_stopping && !m_manager.initialize()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                if (m_stopping) {
                    cleanup(0);
                }
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
                while (!m_stopping && !m_manager.initialize()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                if (m_stopping) {
                    cleanup(0);
                }
                init_action();
                if (m_stopping) {
                    cleanup(0);
                }
            } catch (const std::exception& e) {
                handle_fatal_exception(e);
            }
        }

        /// \brief Runs the application with the registered components.
        void run() {
            if (m_running) return;
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
            if (m_running) return;
            m_running = true;
            setup_signal_handlers();
            init();
            lifecycle_loop(iteration_action);
        }

        /// \brief Stops the application's main loop.
        void stop() {
            if (m_stopping) return;
            m_stopping = true;
        }

        /// \brief
        void shutdown(int signal) {
            cleanup(signal);
        }

    private:
        AppComponentManager m_manager;
        std::atomic<bool>   m_init{false};
        std::atomic<bool>   m_running{false};  ///< Flag indicating whether the loop is running.
        std::atomic<bool>   m_stopping{false};
        std::atomic<bool>   m_cleanup{false}; ///<

        /// \brief Sets up signal handlers for graceful application termination.
        void setup_signal_handlers() {
#           if defined(_WIN32) || defined(_WIN64)
            SetConsoleCtrlHandler(console_handler, TRUE);
#           else
            std::signal(SIGINT, signal_handler);
            std::signal(SIGTERM, signal_handler);
#           endif
            std::atexit(on_exit_handler);
        }

        /// \brief Cleans up the application and shuts down all components.
        /// \param exit_code Exit code indicating the reason for cleanup.
        /// \param wait_for_press Flag indicating if the program should wait for a key press before exiting.
        void cleanup(int exit_code, bool wait_for_press = false) {
            if (m_cleanup) return;
            m_cleanup = true;

#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_PRINT_INFO("Cleaning up application for exit code: ", exit_code);
            try {
                m_manager.shutdown(exit_code);
            } catch (const std::exception& e) {
                LOGIT_FATAL(e);
            }
            try {
                ServiceLocator::get_instance().clear_all();
            } catch (const std::exception& e) {
                LOGIT_FATAL(e);
            }
            LOGIT_SHUTDOWN();
#           else
            try {
                m_manager.shutdown(exit_code);
            } catch (...) {}
            try {
                ServiceLocator::get_instance().clear_all();
            } catch (...) {}
#           endif
            if (wait_for_press) {
                CONSOLIX_STREAM() << "Press Enter to exit..." << std::endl;
                std::cin.get();
            }
            std::exit(exit_code);
        }

        /// \brief Handles fatal exceptions by logging the error and terminating the application.
        /// \param e The exception that caused the fatal error.
        void handle_fatal_exception(const std::exception& e) {
#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_PRINT_FATAL("Unhandled exception: ", e.what());
#           endif //
            cleanup(-1, static_cast<bool>(CONSOLIX_WAIT_ON_ERROR));
        }

        /// \brief The main lifecycle loop with a custom action.
        /// \tparam IterationAction A callable executed within the loop.
        /// \param iteration_action The action to execute in each iteration.
        template <typename IterationAction>
        void lifecycle_loop(IterationAction iteration_action) {
            try {
                while (!m_stopping) {
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
                while (!m_stopping) {
                    m_manager.process();
                }
                cleanup(0);
            } catch (const std::exception& e) {
                handle_fatal_exception(e);
            }
        }

        /// \brief Called upon normal program termination.
        static void on_exit_handler() {
            ConsoleApplication::get_instance().shutdown(0);
        }

#       if defined(_WIN32) || defined(_WIN64)

        /// \brief Handles Windows console events and triggers application shutdown.
        /// \param win_event Windows console event code.
        /// \return FALSE to allow further processing by the system.
        static BOOL WINAPI console_handler(DWORD win_event) {
            std::cout << "-con1" << std::endl;
            switch (win_event) {
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
            std::cout << "-con2" << std::endl;
            ConsoleApplication::get_instance().shutdown(event_to_exit_code(win_event));
            std::cout << "-con3" << std::endl;
            return FALSE;
        }

        /// \brief Logs a Windows console event.
        /// \param event_name Name of the console event.
        static void log_event(const char* event_name) {
#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_PRINT_INFO("Console event received: ", event_name);
#           endif
        }

        /// \brief Maps a Windows console event to a corresponding POSIX exit code.
        /// \param win_event Windows console event code.
        /// \return Corresponding POSIX exit code.
        static int event_to_exit_code(DWORD win_event) {
            switch (win_event) {
            case CTRL_C_EVENT:
                return SIGINT;
            default:
                return SIGTERM;
            };
        }

#       else

        /// \brief Handles a POSIX signal and delegates to signal handler.
        /// \param exit_code POSIX signal code.
        static void signal_handler(int exit_code) {
            switch (exit_code) {
            case SIGINT:
                handle_signal("SIGINT", exit_code);
                break;
            case SIGTERM:
                handle_signal("SIGTERM", exit_code);
                break;
            default:
                handle_signal("UNKNOWN_SIGNAL", exit_code);
                break;
            }
        }

        /// \brief Logs and processes a POSIX signal by initiating application cleanup.
        /// \param signal_name Name of the POSIX signal.
        /// \param exit_code POSIX signal code.
        static void handle_signal(const char* signal_name, int exit_code) {
#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_PRINT_INFO("POSIX signal received: ", signal_name, ", exit code: ", exit_code);
#           endif
            cleanup(exit_code);
        }

#       endif

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
