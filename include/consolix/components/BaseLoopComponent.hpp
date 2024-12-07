#pragma once
#ifndef _CONSOLIX_BASE_LOOP_COMPONENT_HPP_INCLUDED
#define _CONSOLIX_BASE_LOOP_COMPONENT_HPP_INCLUDED

/// \file BaseLoopComponent.hpp
/// \brief Provides a base class for implementing looping application components.
/// \ingroup Components

#include <thread>
#include <atomic>
#include <chrono>

namespace consolix {

    /// \class BaseLoopComponent
    /// \brief Abstract base class for application components with looping functionality.
    ///
    /// This class provides a framework for implementing components that require a
    /// continuous execution cycle. Derived classes must define:
    /// - `on_once()` for one-time initialization logic.
    /// - `on_loop()` for the recurring loop logic.
    /// - `on_shutdown(signal)` for handling cleanup when the application shuts down.
    class BaseLoopComponent : public IAppComponent, public IShutdownable {
    public:
        /// \brief Constructs a `BaseLoopComponent`.
        BaseLoopComponent() = default;

        /// \brief Destroys the `BaseLoopComponent`.
        virtual ~BaseLoopComponent() = default;

        /// \brief Called once during component initialization.
        ///
        /// Derived classes should implement this method to perform setup operations
        /// that need to happen before the main loop starts.
        /// \return `true` if initialization succeeds, `false` otherwise.
        virtual bool on_once() = 0;

        /// \brief Called repeatedly during the application's main loop.
        ///
        /// Derived classes should implement this method to define the recurring
        /// functionality of the component.
        virtual void on_loop() = 0;

        /// \brief Called when the application shuts down.
        ///
        /// Derived classes should implement this method to release resources or perform
        /// cleanup operations before termination.
        /// \param signal The signal that triggered the shutdown.
        virtual void on_shutdown(int signal) = 0;

    protected:

        /// \brief Initializes the component.
        ///
        /// Calls `on_once()` to perform one-time setup and marks the component
        /// as initialized if successful.
        /// \return `true` if initialization succeeds, `false` otherwise.
        bool initialize() override final {
            m_is_init = on_once();
            return m_is_init;
        }

        /// \brief Checks if the component has been initialized.
        /// \return `true` if the component is initialized, `false` otherwise.
        bool is_initialized() const override final {
            return m_is_init;
        }

        /// \brief Executes the loop logic for the component.
        ///
        /// Calls `on_loop()` to run the recurring functionality of the component.
        void process() override final {
            on_loop();
        }

        /// \brief Shuts down the component.
        ///
        /// Calls `on_shutdown(signal)` to clean up resources or terminate processes.
        /// \param signal The signal that triggered the shutdown.
        void shutdown(int signal) override final {
            on_shutdown(signal);
        }

    private:
        std::atomic<bool> m_is_init{false}; ///< Tracks whether the component is initialized.
    }; // BaseLoopComponent

}; // namespace consolix

#endif // _CONSOLIX_BASE_LOOP_COMPONENT_HPP_INCLUDED
