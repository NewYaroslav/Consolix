#pragma once
#ifndef _CONSOLIX_CONSOLE_APPLICATION_HPP_INCLUDED
#define _CONSOLIX_CONSOLE_APPLICATION_HPP_INCLUDED

/// \file ConsoleApplication.hpp
/// \brief Manages the lifecycle of a console application.
/// \ingroup Core

#include <csignal>
#include <cstdlib>
#include <iostream>

#if !defined(_WIN32) && !defined(_WIN64)
#include <signal.h>
#endif

namespace consolix {

    /// \class ConsoleApplication
    /// \brief Singleton class to manage the lifecycle of a console application.
    ///
    /// Provides functionality to initialize, execute, and gracefully shut down components
    /// in a structured lifecycle. On POSIX systems, signal handlers only request shutdown,
    /// and the actual component cleanup runs later in the normal execution path.
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
                while (!stop_requested() && !m_manager.initialize()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                cleanup_if_stopping();
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
                while (!stop_requested() && !m_manager.initialize()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                cleanup_if_stopping();
                init_action();
                cleanup_if_stopping();
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
            m_stopping = true;
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
            reset_signal_state();

            struct sigaction action = {};
            action.sa_handler = signal_handler;
            sigemptyset(&action.sa_mask);
            sigaddset(&action.sa_mask, SIGINT);
            sigaddset(&action.sa_mask, SIGTERM);
            action.sa_flags = 0;

            sigaction(SIGINT, &action, nullptr);
            sigaction(SIGTERM, &action, nullptr);
#           endif
            std::atexit(on_exit_handler);
        }

        /// \brief Cleans up the application and shuts down all components.
        /// \param exit_code Exit code indicating the reason for cleanup.
        /// \param wait_for_press Flag indicating if the program should wait for a key press before exiting.
        void cleanup(int exit_code, bool wait_for_press = false) {
            bool expected = false;
            if (!m_cleanup.compare_exchange_strong(expected, true)) return;

            m_stopping = true;

#           if !defined(_WIN32) && !defined(_WIN64)
            PosixTerminationSignalMaskGuard posix_signal_mask_guard;
#           endif

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
                while (!stop_requested()) {
                    m_manager.process();
                    iteration_action();
                }
                cleanup(resolve_stop_exit_code(0));
            } catch (const std::exception& e) {
                handle_fatal_exception(e);
            }
        }

        /// \brief The main lifecycle loop.
        void lifecycle_loop() {
            try {
                while (!stop_requested()) {
                    m_manager.process();
                }
                cleanup(resolve_stop_exit_code(0));
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
            ConsoleApplication::get_instance().shutdown(event_to_exit_code(win_event));
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

        class PosixTerminationSignalMaskGuard {
        public:
            PosixTerminationSignalMaskGuard() {
                sigemptyset(&m_mask);
                sigaddset(&m_mask, SIGINT);
                sigaddset(&m_mask, SIGTERM);

                if (sigprocmask(SIG_BLOCK, &m_mask, &m_old_mask) == 0) {
                    m_active = true;
                }
            }

            ~PosixTerminationSignalMaskGuard() {
                if (m_active) {
                    sigprocmask(SIG_SETMASK, &m_old_mask, nullptr);
                }
            }

        private:
            sigset_t m_mask{};
            sigset_t m_old_mask{};
            bool     m_active{false};
        };

        /// \brief Handles a POSIX signal by recording a deferred shutdown request.
        /// \param exit_code POSIX signal code.
        static void signal_handler(int exit_code) {
            pending_signal_code() = static_cast<std::sig_atomic_t>(exit_code);
            signal_stop_requested() = 1;
        }

        /// \brief Resets POSIX signal state before installing handlers.
        static void reset_signal_state() {
            pending_signal_code() = 0;
            signal_stop_requested() = 0;
        }

        /// \brief Checks whether a POSIX signal requested shutdown and synchronizes the runtime state.
        bool stop_requested() {
            if (signal_stop_requested() != 0) {
                m_stopping = true;
            }
            return m_stopping.load();
        }

        /// \brief Resolves the exit code for the active stop request.
        int resolve_stop_exit_code(int fallback_exit_code) const {
            const int pending_exit_code = static_cast<int>(pending_signal_code());
            if (pending_exit_code != 0) {
                return pending_exit_code;
            }
            return fallback_exit_code;
        }

        /// \brief Executes cleanup immediately if a stop request is already pending.
        void cleanup_if_stopping() {
            if (stop_requested()) {
                cleanup(resolve_stop_exit_code(0));
            }
        }

        /// \brief Storage for the last requested POSIX shutdown signal.
        static volatile std::sig_atomic_t& pending_signal_code() {
            static volatile std::sig_atomic_t value = 0;
            return value;
        }

        /// \brief Storage for the POSIX stop-request flag.
        static volatile std::sig_atomic_t& signal_stop_requested() {
            static volatile std::sig_atomic_t value = 0;
            return value;
        }

#       endif

#       if defined(_WIN32) || defined(_WIN64)
        bool stop_requested() {
            return m_stopping.load();
        }

        int resolve_stop_exit_code(int fallback_exit_code) const {
            return fallback_exit_code;
        }

        void cleanup_if_stopping() {
            if (m_stopping) {
                cleanup(0);
            }
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
