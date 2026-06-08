#pragma once
#ifndef _CONSOLIX_LOOP_THROTTLE_COMPONENT_HPP_INCLUDED
#define _CONSOLIX_LOOP_THROTTLE_COMPONENT_HPP_INCLUDED

/// \file LoopThrottleComponent.hpp
/// \brief Defines a wakeable component for throttling polling application loops.
/// \ingroup Components

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

namespace consolix {

    /// \class LoopThrottleComponent
    /// \brief Adds an explicit wakeable wait step to the Consolix polling loop.
    ///
    /// Consolix processes registered components in a loop. If every component returns
    /// immediately, the application can become a busy loop. Register this component,
    /// preferably after active work components, to pause each pass for a bounded time.
    ///
    /// Another thread can call `wake()` to end the wait early when new work arrives.
    class LoopThrottleComponent : public IAppComponent {
    public:
        /// \brief Constructs a throttle component with a short default delay.
        LoopThrottleComponent() = default;

        /// \brief Constructs a throttle component with a custom delay.
        /// \param delay Maximum time to wait during each process pass.
        explicit LoopThrottleComponent(std::chrono::milliseconds delay) :
            m_delay(delay) {
        }

        /// \brief Virtual destructor.
        virtual ~LoopThrottleComponent() override {
            wake();
        }

        /// \brief Wakes the component if it is currently waiting.
        ///
        /// A wake request made before `process()` starts is remembered and consumed by
        /// the next process pass, so producer threads can signal work without racing
        /// the exact wait window.
        void wake() {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_wake_requested = true;
            }
            m_condition.notify_all();
        }

        /// \brief Changes the throttle delay and wakes any active wait.
        /// \param delay New maximum wait duration per process pass.
        void set_delay(std::chrono::milliseconds delay) {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_delay = delay;
                m_wake_requested = true;
            }
            m_condition.notify_all();
        }

        /// \brief Returns the currently configured throttle delay.
        /// \return Maximum wait duration per process pass.
        std::chrono::milliseconds delay() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_delay;
        }

    protected:
        bool initialize() override {
            m_is_initialized.store(true);
            return true;
        }

        bool is_initialized() const override {
            return m_is_initialized.load();
        }

        void process() override {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_wake_requested) {
                m_wake_requested = false;
                return;
            }

            const std::chrono::milliseconds current_delay = m_delay;
            if (current_delay <= std::chrono::milliseconds(0)) {
                return;
            }

            m_condition.wait_for(
                lock,
                current_delay,
                [this]() {
                    return m_wake_requested;
                });
            m_wake_requested = false;
        }

    private:
        mutable std::mutex          m_mutex;
        std::condition_variable     m_condition;
        std::chrono::milliseconds   m_delay{std::chrono::milliseconds(1)};
        bool                        m_wake_requested{false};
        std::atomic<bool>           m_is_initialized{false};
    }; // LoopThrottleComponent

} // namespace consolix

#endif // _CONSOLIX_LOOP_THROTTLE_COMPONENT_HPP_INCLUDED
