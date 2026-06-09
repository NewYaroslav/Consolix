#pragma once
#ifndef _CONSOLIX_POSIX_SIGNAL_WAKE_SERVICE_HPP_INCLUDED
#define _CONSOLIX_POSIX_SIGNAL_WAKE_SERVICE_HPP_INCLUDED

/// \file PosixSignalWakeService.hpp
/// \brief Optional self-pipe wake bridge for POSIX termination signals.
/// \ingroup Core

#include "LoopWakeService.hpp"
#include "ServiceLocator.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>

#if !defined(_WIN32) && !defined(_WIN64)
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace consolix {

    /// \class PosixSignalWakeService
    /// \brief Wakes polling-loop waiters when the runner observes POSIX signals.
    ///
    /// The regular POSIX signal handler remains async-signal-safe: it records the
    /// signal in the runner and asks this service to write one byte to a self-pipe.
    /// A watcher thread then wakes `LoopWakeService` from ordinary C++ code.
    ///
    /// This service is opt-in and POSIX-only. On Windows it compiles as a no-op.
    class PosixSignalWakeService {
    public:
        /// \brief Constructs an inactive wake service.
        PosixSignalWakeService() = default;

        /// \brief Stops the watcher thread and closes the self-pipe if active.
        ~PosixSignalWakeService() {
            stop();
        }

        PosixSignalWakeService(const PosixSignalWakeService&) = delete;
        PosixSignalWakeService& operator=(const PosixSignalWakeService&) = delete;

        /// \brief Returns whether this platform supports the POSIX wake bridge.
        /// \return `true` on POSIX platforms, `false` on Windows.
        bool is_supported() const {
#if defined(_WIN32) || defined(_WIN64)
            return false;
#else
            return true;
#endif
        }

        /// \brief Starts the self-pipe watcher.
        /// \return `true` when the service is active, `false` on unsupported platforms.
        /// \throws std::runtime_error if POSIX pipe/thread setup fails.
        /// \throws std::logic_error if another service instance is already active.
        bool start() {
#if defined(_WIN32) || defined(_WIN64)
            return false;
#else
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_running) {
                return true;
            }

            acquire_active_service();
            try {
                setup_loop_wake_service();
                create_pipe();
                set_close_on_exec(m_pipe_fds[0]);
                set_close_on_exec(m_pipe_fds[1]);
                set_nonblocking(m_pipe_fds[1]);

                m_stop_requested.store(false);
                signal_wake_fd() = static_cast<std::sig_atomic_t>(m_pipe_fds[1]);
                m_watcher = std::thread(&PosixSignalWakeService::watch_loop, this);
                m_running = true;
            } catch (...) {
                signal_wake_fd() = static_cast<std::sig_atomic_t>(-1);
                close_pipe();
                release_active_service();
                throw;
            }
            return true;
#endif
        }

        /// \brief Stops the watcher thread if it is active.
        void stop() {
#if !defined(_WIN32) && !defined(_WIN64)
            std::thread watcher;

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (!m_running) {
                    return;
                }

                m_stop_requested.store(true);
                clear_signal_wake_fd_if_active();
                write_wake_token(m_pipe_fds[1]);

                if (m_watcher.joinable()) {
                    watcher.swap(m_watcher);
                }
                m_running = false;
            }

            if (watcher.joinable()) {
                watcher.join();
            }

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                close_pipe();
                m_loop_wake_service.reset();
                m_stop_requested.store(false);
                release_active_service();
            }
#endif
        }

        /// \brief Checks whether the service is currently active.
        /// \return `true` if the self-pipe watcher is running.
        bool is_running() const {
#if defined(_WIN32) || defined(_WIN64)
            return false;
#else
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_running;
#endif
        }

        /// \brief Async-signal-safe hook called by the runner signal handler.
        ///
        /// This method must not use C++ runtime services, locks, allocation, or
        /// logging. It only writes a byte to the active self-pipe, if one exists.
        static void notify_signal_handler() {
#if !defined(_WIN32) && !defined(_WIN64)
            const int fd = static_cast<int>(signal_wake_fd());
            write_wake_token(fd);
#endif
        }

    private:
#if !defined(_WIN32) && !defined(_WIN64)
        mutable std::mutex          m_mutex;
        std::thread                 m_watcher;
        std::weak_ptr<LoopWakeService> m_loop_wake_service;
        std::atomic<bool>           m_stop_requested{false};
        int                         m_pipe_fds[2]{-1, -1};
        bool                        m_running{false};

        void setup_loop_wake_service() {
            auto service = ServiceLocator::get_instance().find_service<LoopWakeService>();
            if (!service) {
                ServiceLocator::get_instance().register_service<LoopWakeService>();
                service = ServiceLocator::get_instance().find_service<LoopWakeService>();
            }
            m_loop_wake_service = service;
        }

        void create_pipe() {
            if (::pipe(m_pipe_fds) != 0) {
                throw std::runtime_error("PosixSignalWakeService pipe() failed");
            }
        }

        void watch_loop() {
            while (true) {
                unsigned char buffer[64];
                const ssize_t bytes = ::read(m_pipe_fds[0], buffer, sizeof(buffer));

                if (bytes > 0) {
                    wake_loop_waiters();
                    if (m_stop_requested.load()) {
                        return;
                    }
                    continue;
                }

                if (bytes == 0) {
                    return;
                }

                if (errno == EINTR) {
                    continue;
                }
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }
                return;
            }
        }

        void wake_loop_waiters() {
            std::shared_ptr<LoopWakeService> service;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                service = m_loop_wake_service.lock();
            }
            if (service) {
                service->wake_all();
            }
        }

        void close_pipe() {
            close_fd(m_pipe_fds[0]);
            close_fd(m_pipe_fds[1]);
        }

        void acquire_active_service() {
            std::lock_guard<std::mutex> lock(active_service_mutex());
            if (active_service() != nullptr && active_service() != this) {
                throw std::logic_error("PosixSignalWakeService is already active");
            }
            active_service() = this;
        }

        void release_active_service() {
            std::lock_guard<std::mutex> lock(active_service_mutex());
            if (active_service() == this) {
                active_service() = nullptr;
            }
        }

        void clear_signal_wake_fd_if_active() {
            std::lock_guard<std::mutex> lock(active_service_mutex());
            if (active_service() == this) {
                signal_wake_fd() = static_cast<std::sig_atomic_t>(-1);
            }
        }

        static void close_fd(int& fd) {
            if (fd >= 0) {
                ::close(fd);
                fd = -1;
            }
        }

        static void set_close_on_exec(int fd) {
            const int flags = ::fcntl(fd, F_GETFD, 0);
            if (flags < 0 || ::fcntl(fd, F_SETFD, flags | FD_CLOEXEC) != 0) {
                throw std::runtime_error("PosixSignalWakeService fcntl(FD_CLOEXEC) failed");
            }
        }

        static void set_nonblocking(int fd) {
            const int flags = ::fcntl(fd, F_GETFL, 0);
            if (flags < 0 || ::fcntl(fd, F_SETFL, flags | O_NONBLOCK) != 0) {
                throw std::runtime_error("PosixSignalWakeService fcntl(O_NONBLOCK) failed");
            }
        }

        static void write_wake_token(int fd) {
            if (fd < 0) {
                return;
            }

            const unsigned char token = 1;
            const ssize_t result = ::write(fd, &token, sizeof(token));
            (void)result;
        }

        static volatile std::sig_atomic_t& signal_wake_fd() {
            static volatile std::sig_atomic_t fd = -1;
            return fd;
        }

        static std::mutex& active_service_mutex() {
            static std::mutex mutex;
            return mutex;
        }

        static PosixSignalWakeService*& active_service() {
            static PosixSignalWakeService* service = nullptr;
            return service;
        }
#endif
    }; // PosixSignalWakeService

} // namespace consolix

#endif // _CONSOLIX_POSIX_SIGNAL_WAKE_SERVICE_HPP_INCLUDED
