#pragma once
#ifndef _CONSOLIX_CONSOLE_APPLICATION_RUNNER_HPP_INCLUDED
#define _CONSOLIX_CONSOLE_APPLICATION_RUNNER_HPP_INCLUDED

/// \file ConsoleApplicationRunner.hpp
/// \brief Runs Consolix components and returns an exit code without calling std::exit.
/// \ingroup Core

#include "platform_includes.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <exception>
#include <memory>
#include <mutex>
#include <numeric>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#if CONSOLIX_USE_LOGIT == 1
#include <logit.hpp>
#endif

#include "ServiceLocator.hpp"
#include "AppComponentManager.hpp"
#include "LoopWakeService.hpp"
#include "PosixSignalWakeService.hpp"

#if !defined(_WIN32) && !defined(_WIN64)
#include <signal.h>
#endif

namespace consolix {

    /// \class ConsoleApplicationRunner
    /// \brief Component runner that performs the Consolix lifecycle and returns an exit code.
    ///
    /// The runner initializes components, processes them until a stop request is observed,
    /// shuts them down, clears shared services, shuts down LogIt when enabled, and returns
    /// the resolved exit code. It never calls `std::exit`.
    ///
    /// On Windows, Ctrl+C/Ctrl+Break only request a cooperative stop. Close, logoff,
    /// and shutdown console events request stop and wait briefly for this runner to
    /// complete cleanup on the runner thread.
    ///
    /// A runner instance is single-use. Create a new `AppComponentManager` and
    /// `ConsoleApplicationRunner` for a new application lifecycle.
    class ConsoleApplicationRunner {
    public:
        /// \brief Constructs a runner over an existing component manager.
        /// \param manager The component manager to drive.
        explicit ConsoleApplicationRunner(AppComponentManager& manager) :
            m_manager(manager) {
        }

        /// \brief Runs components until a stop request and returns the exit code.
        /// \return The requested exit code, a signal code, or a non-zero fatal error code.
        int run_for_exit_code() {
            return run_for_exit_code(NoIterationAction());
        }

        /// \brief Runs components with a custom action after every process iteration.
        /// \tparam IterationAction Callable invoked after each `process()` pass.
        /// \param iteration_action The per-iteration action.
        /// \return The requested exit code, a signal code, or a non-zero fatal error code.
        template <typename IterationAction>
        int run_for_exit_code(IterationAction iteration_action) {
            if (m_has_run.exchange(true)) {
                throw std::logic_error("ConsoleApplicationRunner is single-use");
            }

            if (m_running.exchange(true)) {
                throw std::logic_error("ConsoleApplicationRunner is already running");
            }

            m_shutdown_complete.store(false);
            ActiveRunnerGuard active_runner_guard(*this);
            setup_loop_wake_service();
            setup_signal_handlers();

            int exit_code = 0;
            try {
                initialize_components();
                while (!stop_requested()) {
                    m_manager.process();
                    iteration_action();
                }
                exit_code = requested_exit_code();
            } catch (const std::exception& e) {
                exit_code = handle_fatal_exception(e);
            } catch (...) {
                exit_code = handle_unknown_fatal_exception();
            }

            exit_code = cleanup(exit_code);
            mark_shutdown_complete();
            m_running.store(false);
            return exit_code;
        }

        /// \brief Requests the runner to stop and return the provided exit code.
        /// \param exit_code Exit code to return and pass to component shutdown.
        void request_stop(int exit_code = 0) {
            bool expected = false;
            if (m_stopping.compare_exchange_strong(expected, true)) {
                m_requested_exit_code.store(exit_code);
            }
            wake_loop_waiters();
        }

        /// \brief Requests stop and waits for runner-thread cleanup to finish.
        /// \param exit_code Exit code to pass to shutdown and return from the runner.
        /// \param timeout Maximum time to wait for cleanup completion.
        /// \return `true` if cleanup completed before the timeout.
        bool request_forced_stop(
                int exit_code,
                std::chrono::milliseconds timeout) {
            request_stop(exit_code);
            if (is_shutdown_complete()) {
                return true;
            }
            if (!is_running()) {
                return false;
            }
            return wait_for_shutdown(timeout);
        }

        /// \brief Requests a normal zero-code stop.
        void stop() {
            request_stop(0);
        }

        /// \brief Checks whether this runner has received a stop request.
        /// \return `true` when the runner should leave the process loop.
        bool is_stop_requested() {
            return stop_requested();
        }

        /// \brief Checks whether this runner is currently executing.
        /// \return `true` while `run_for_exit_code()` is active.
        bool is_running() const {
            return m_running.load();
        }

        /// \brief Checks whether cleanup has completed for the active run.
        /// \return `true` after shutdown, service cleanup, and logger shutdown finish.
        bool is_shutdown_complete() const {
            return m_shutdown_complete.load();
        }

        /// \brief Requests stop on the currently active runner, if one exists.
        /// \param exit_code Exit code to request.
        /// \return `true` if an active runner accepted the stop request.
        static bool request_current_stop(int exit_code = 0) {
            ConsoleApplicationRunner* runner = current_runner().load();
            if (!runner) {
                return false;
            }
            runner->request_stop(exit_code);
            return true;
        }

        /// \brief Requests forced stop on the currently active runner, if one exists.
        /// \param exit_code Exit code to request.
        /// \param timeout Maximum time to wait for runner-thread cleanup.
        /// \return `true` if an active runner completed cleanup before timeout.
        static bool request_current_forced_stop(
                int exit_code,
                std::chrono::milliseconds timeout) {
            ConsoleApplicationRunner* runner = current_runner().load();
            if (!runner) {
                return false;
            }
            return runner->request_forced_stop(exit_code, timeout);
        }

    private:
        struct NoIterationAction {
            void operator()() const {}
        };

        class ActiveRunnerGuard {
        public:
            explicit ActiveRunnerGuard(ConsoleApplicationRunner& runner) :
                m_previous(current_runner().exchange(&runner)) {
            }

            ~ActiveRunnerGuard() {
                current_runner().store(m_previous);
            }

        private:
            ConsoleApplicationRunner* m_previous;
        };

        AppComponentManager& m_manager;
        std::atomic<bool>   m_has_run{false};
        std::atomic<bool>   m_running{false};
        std::atomic<bool>   m_stopping{false};
        std::atomic<bool>   m_cleanup{false};
        std::atomic<bool>   m_shutdown_complete{false};
        std::atomic<int>    m_requested_exit_code{0};
        std::mutex          m_shutdown_mutex;
        std::condition_variable m_shutdown_complete_cv;
        std::weak_ptr<LoopWakeService> m_loop_wake_service;

        void setup_signal_handlers() {
            reset_signal_state();
#           if defined(_WIN32) || defined(_WIN64)
            SetConsoleCtrlHandler(console_handler, TRUE);
            std::signal(SIGINT, signal_handler);
            std::signal(SIGTERM, signal_handler);
#           else
            struct sigaction action = {};
            action.sa_handler = signal_handler;
            sigemptyset(&action.sa_mask);
            sigaddset(&action.sa_mask, SIGINT);
            sigaddset(&action.sa_mask, SIGTERM);
            action.sa_flags = 0;

            sigaction(SIGINT, &action, nullptr);
            sigaction(SIGTERM, &action, nullptr);
#           endif
        }

        void initialize_components() {
            while (!stop_requested() && !m_manager.initialize()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        void setup_loop_wake_service() {
            auto service = ServiceLocator::get_instance().find_service<LoopWakeService>();
            if (!service) {
                ServiceLocator::get_instance().register_service<LoopWakeService>();
                service = ServiceLocator::get_instance().find_service<LoopWakeService>();
            }
            m_loop_wake_service = service;
        }

        void wake_loop_waiters() {
            if (auto service = m_loop_wake_service.lock()) {
                service->wake_all();
            }
        }

        bool stop_requested() {
            synchronize_signal_stop_request();
            return m_stopping.load();
        }

        int requested_exit_code() {
            synchronize_signal_stop_request();
            return m_requested_exit_code.load();
        }

        void synchronize_signal_stop_request() {
            if (signal_stop_requested() != 0) {
                const int exit_code = static_cast<int>(pending_signal_code());
                request_stop(exit_code);
            }
        }

        bool wait_for_shutdown(std::chrono::milliseconds timeout) {
            std::unique_lock<std::mutex> lock(m_shutdown_mutex);
            return m_shutdown_complete_cv.wait_for(
                lock,
                timeout,
                [this]() {
                    return m_shutdown_complete.load();
                });
        }

        void mark_shutdown_complete() {
            m_shutdown_complete.store(true);
            m_shutdown_complete_cv.notify_all();
        }

        int cleanup(int exit_code) {
            bool expected = false;
            if (!m_cleanup.compare_exchange_strong(expected, true)) {
                return exit_code;
            }

            m_stopping.store(true);
            bool cleanup_failed = false;

#           if !defined(_WIN32) && !defined(_WIN64)
            PosixTerminationSignalMaskGuard posix_signal_mask_guard;
#           endif

#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_PRINT_INFO("Cleaning up application for exit code: ", exit_code);
#           endif

            try {
                m_manager.shutdown(exit_code);
            } catch (const std::exception& e) {
                cleanup_failed = true;
                log_fatal_exception("Shutdown error: ", e);
            } catch (...) {
                cleanup_failed = true;
                log_fatal_message("Shutdown error: unknown exception");
            }

            try {
                ServiceLocator::get_instance().clear_all();
            } catch (const std::exception& e) {
                cleanup_failed = true;
                log_fatal_exception("Service cleanup error: ", e);
            } catch (...) {
                cleanup_failed = true;
                log_fatal_message("Service cleanup error: unknown exception");
            }

#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_SHUTDOWN();
#           endif

            if (cleanup_failed && exit_code == 0) {
                return fatal_exit_code();
            }
            return exit_code;
        }

        int handle_fatal_exception(const std::exception& e) {
            log_fatal_exception("Unhandled exception: ", e);
            force_stop(fatal_exit_code());
            return fatal_exit_code();
        }

        int handle_unknown_fatal_exception() {
            log_fatal_message("Unhandled unknown exception");
            force_stop(fatal_exit_code());
            return fatal_exit_code();
        }

        void force_stop(int exit_code) {
            m_requested_exit_code.store(exit_code);
            m_stopping.store(true);
        }

        static int fatal_exit_code() {
            return -1;
        }

        static void log_fatal_exception(const char* prefix, const std::exception& e) {
#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_PRINT_FATAL(prefix, e.what());
#           else
            (void)prefix;
            (void)e;
#           endif
        }

        static void log_fatal_message(const char* message) {
#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_PRINT_FATAL(message);
#           else
            (void)message;
#           endif
        }

        static std::atomic<ConsoleApplicationRunner*>& current_runner() {
            static std::atomic<ConsoleApplicationRunner*> value{nullptr};
            return value;
        }

        static void signal_handler(int exit_code) {
            pending_signal_code() = static_cast<std::sig_atomic_t>(exit_code);
            signal_stop_requested() = 1;
            PosixSignalWakeService::notify_signal_handler();
        }

        static void reset_signal_state() {
            pending_signal_code() = 0;
            signal_stop_requested() = 0;
        }

        static volatile std::sig_atomic_t& pending_signal_code() {
            static volatile std::sig_atomic_t value = 0;
            return value;
        }

        static volatile std::sig_atomic_t& signal_stop_requested() {
            static volatile std::sig_atomic_t value = 0;
            return value;
        }

#       if defined(_WIN32) || defined(_WIN64)
        static BOOL WINAPI console_handler(DWORD win_event) {
            switch (win_event) {
            case CTRL_C_EVENT:
            case CTRL_BREAK_EVENT:
                request_current_stop(SIGINT);
                return TRUE;

            case CTRL_CLOSE_EVENT:
            case CTRL_LOGOFF_EVENT:
            case CTRL_SHUTDOWN_EVENT:
                request_current_forced_stop(
                    SIGTERM,
                    forced_shutdown_timeout());
                return TRUE;

            default:
                request_current_forced_stop(
                    SIGTERM,
                    forced_shutdown_timeout());
                return TRUE;
            }
        }

        static std::chrono::milliseconds forced_shutdown_timeout() {
            return std::chrono::milliseconds(CONSOLIX_FORCED_SHUTDOWN_TIMEOUT_MS);
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
#       endif
    }; // ConsoleApplicationRunner

} // namespace consolix

#endif // _CONSOLIX_CONSOLE_APPLICATION_RUNNER_HPP_INCLUDED
