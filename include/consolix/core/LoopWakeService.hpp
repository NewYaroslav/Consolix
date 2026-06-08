#pragma once
#ifndef _CONSOLIX_LOOP_WAKE_SERVICE_HPP_INCLUDED
#define _CONSOLIX_LOOP_WAKE_SERVICE_HPP_INCLUDED

/// \file LoopWakeService.hpp
/// \brief Shared wake channel for Consolix polling-loop wait components.
/// \ingroup Core

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>

namespace consolix {

    /// \class LoopWakeService
    /// \brief Broadcasts wake requests to loop components that wait between passes.
    ///
    /// The service uses a generation counter so a wake request can be observed by
    /// multiple waiters and by waiters that start just after the request was made.
    class LoopWakeService {
    public:
        /// \brief Monotonic wake generation type.
        typedef std::uint64_t Generation;

        /// \brief Returns the current wake generation.
        /// \return Current generation value.
        Generation generation() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_generation;
        }

        /// \brief Wakes all current and next-pass waiters.
        void wake_all() {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                ++m_generation;
            }
            m_condition.notify_all();
        }

        /// \brief Waits until the wake generation changes or timeout expires.
        /// \param observed_generation Last generation observed by the caller.
        /// \param timeout Maximum wait duration.
        /// \return `true` if a wake generation change was observed.
        bool wait_for_change(
                Generation& observed_generation,
                std::chrono::milliseconds timeout) {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_generation != observed_generation) {
                observed_generation = m_generation;
                return true;
            }

            if (timeout <= std::chrono::milliseconds(0)) {
                return false;
            }

            const bool changed = m_condition.wait_for(
                lock,
                timeout,
                [this, observed_generation]() {
                    return m_generation != observed_generation;
                });

            if (changed) {
                observed_generation = m_generation;
            }
            return changed;
        }

    private:
        mutable std::mutex      m_mutex;
        std::condition_variable m_condition;
        Generation              m_generation{0};
    }; // LoopWakeService

} // namespace consolix

#endif // _CONSOLIX_LOOP_WAKE_SERVICE_HPP_INCLUDED
