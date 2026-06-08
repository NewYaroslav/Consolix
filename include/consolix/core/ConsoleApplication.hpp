#pragma once
#ifndef _CONSOLIX_CONSOLE_APPLICATION_HPP_INCLUDED
#define _CONSOLIX_CONSOLE_APPLICATION_HPP_INCLUDED

/// \file ConsoleApplication.hpp
/// \brief Singleton facade for the Consolix application lifecycle.
/// \ingroup Core

#include "ConsoleApplicationRunner.hpp"

#include <chrono>
#include <cstdlib>
#include <exception>
#include <memory>
#include <thread>
#include <utility>

namespace consolix {

    /// \class ConsoleApplication
    /// \brief Singleton facade over `ConsoleApplicationRunner`.
    ///
    /// The singleton preserves the compact `consolix::add()` / `consolix::run()`
    /// API. The actual lifecycle, signal handling, shutdown, service cleanup, and
    /// logger shutdown are owned by `ConsoleApplicationRunner`.
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
        void init() {
            initialize_or_exit();
        }

        /// \brief Initializes the application with a custom action.
        /// \tparam InitAction A callable type for the custom initialization action.
        /// \param init_action The custom action to perform during initialization.
        template <typename InitAction>
        void init(InitAction init_action) {
            initialize_or_exit();
            try {
                init_action();
                exit_if_stopping();
            } catch (const std::exception& e) {
                exit_after_fatal_exception(e);
            } catch (...) {
                exit_after_unknown_fatal_exception();
            }
        }

        /// \brief Runs the application with the registered components.
        ///
        /// This legacy entry point preserves process-owning behavior by exiting
        /// with the code produced by the runner.
        void run() {
            std::exit(run_for_exit_code());
        }

        /// \brief Runs the application with a custom loop action.
        /// \tparam IterationAction A callable type executed within the main loop.
        /// \param iteration_action The custom action to perform in each loop iteration.
        template <typename IterationAction>
        void run(IterationAction iteration_action) {
            std::exit(run_for_exit_code(iteration_action));
        }

        /// \brief Runs the application and returns an exit code without calling std::exit.
        /// \return The requested exit code, a signal code, or a non-zero fatal error code.
        int run_for_exit_code() {
            return m_runner.run_for_exit_code();
        }

        /// \brief Runs the application with a custom loop action and returns an exit code.
        /// \tparam IterationAction A callable type executed within the main loop.
        /// \param iteration_action The custom action to perform in each loop iteration.
        /// \return The requested exit code, a signal code, or a non-zero fatal error code.
        template <typename IterationAction>
        int run_for_exit_code(IterationAction iteration_action) {
            return m_runner.run_for_exit_code(iteration_action);
        }

        /// \brief Stops the application's main loop with exit code 0.
        void stop() {
            request_stop(0);
        }

        /// \brief Stops the application's main loop with an explicit exit code.
        /// \param exit_code Exit code to pass to shutdown and return from `run_for_exit_code`.
        void stop(int exit_code) {
            request_stop(exit_code);
        }

        /// \brief Requests stop with an explicit exit code.
        /// \param exit_code Exit code to pass to shutdown and return from `run_for_exit_code`.
        void request_stop(int exit_code) {
            m_runner.request_stop(exit_code);
        }

        /// \brief Legacy shutdown entry point.
        ///
        /// If the runner is active, shutdown is deferred to the runner thread.
        /// If the runner is inactive, this method performs the legacy process-owning
        /// behavior by running cleanup through the runner and exiting.
        void shutdown(int exit_code) {
            request_stop(exit_code);
            if (!m_runner.is_running()) {
                std::exit(m_runner.run_for_exit_code());
            }
        }

    private:
        AppComponentManager      m_manager;
        ConsoleApplicationRunner m_runner;

        void initialize_or_exit() {
            try {
                while (!m_runner.is_stop_requested() && !m_manager.initialize()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                exit_if_stopping();
            } catch (const std::exception& e) {
                exit_after_fatal_exception(e);
            } catch (...) {
                exit_after_unknown_fatal_exception();
            }
        }

        void exit_if_stopping() {
            if (m_runner.is_stop_requested()) {
                std::exit(m_runner.run_for_exit_code());
            }
        }

        void exit_after_fatal_exception(const std::exception& e) {
#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_PRINT_FATAL("Unhandled exception: ", e.what());
#           else
            (void)e;
#           endif
            m_runner.request_stop(-1);
            std::exit(m_runner.run_for_exit_code());
        }

        void exit_after_unknown_fatal_exception() {
#           if CONSOLIX_USE_LOGIT == 1
            LOGIT_PRINT_FATAL("Unhandled unknown exception");
#           endif
            m_runner.request_stop(-1);
            std::exit(m_runner.run_for_exit_code());
        }

        ConsoleApplication() :
            m_runner(m_manager) {
        }
        ~ConsoleApplication() = default;

        ConsoleApplication(const ConsoleApplication&) = delete;
        ConsoleApplication& operator=(const ConsoleApplication&) = delete;
        ConsoleApplication(ConsoleApplication&&) = delete;
        ConsoleApplication& operator=(ConsoleApplication&&) = delete;
    }; // ConsoleApplication

}; // namespace consolix

#endif // _CONSOLIX_CONSOLE_APPLICATION_HPP_INCLUDED
