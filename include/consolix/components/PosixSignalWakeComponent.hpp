#pragma once
#ifndef _CONSOLIX_POSIX_SIGNAL_WAKE_COMPONENT_HPP_INCLUDED
#define _CONSOLIX_POSIX_SIGNAL_WAKE_COMPONENT_HPP_INCLUDED

/// \file PosixSignalWakeComponent.hpp
/// \brief Opt-in component that wakes polling waits for POSIX termination signals.
/// \ingroup Components

#include <atomic>
#include <memory>

namespace consolix {

    /// \class PosixSignalWakeComponent
    /// \brief Starts `PosixSignalWakeService` for the application lifecycle.
    ///
    /// Register this component when POSIX SIGINT/SIGTERM should wake
    /// `LoopThrottleComponent` waits immediately. The component is a no-op on
    /// Windows, where console-control handling already uses the runner stop path.
    class PosixSignalWakeComponent final : public IAppComponent, public IShutdownable {
    public:
        /// \brief Stops the wake service if it is still active.
        ~PosixSignalWakeComponent() override {
            stop_service();
        }

    protected:
        /// \brief Starts or reuses the shared POSIX signal wake service.
        /// \return Always `true` once setup succeeds or when unsupported.
        bool initialize() override {
            if (m_initialized.load()) {
                return true;
            }

            auto service = ServiceLocator::get_instance().find_service<PosixSignalWakeService>();
            if (!service) {
                ServiceLocator::get_instance().register_service<PosixSignalWakeService>();
                service = ServiceLocator::get_instance().find_service<PosixSignalWakeService>();
            }

            if (service) {
                service->start();
                m_service = service;
            }

            m_initialized.store(true);
            return true;
        }

        /// \brief Reports whether the service has been initialized.
        /// \return `true` after successful initialization.
        bool is_initialized() const override {
            return m_initialized.load();
        }

        /// \brief Does no per-loop work.
        void process() override {
        }

        /// \brief Stops the POSIX wake service during application shutdown.
        /// \param signal Exit code or signal that initiated shutdown.
        void shutdown(int signal) override {
            (void)signal;
            stop_service();
        }

    private:
        void stop_service() {
            if (m_service) {
                m_service->stop();
            }
            m_initialized.store(false);
        }

        std::atomic<bool>                    m_initialized{false};
        std::shared_ptr<PosixSignalWakeService> m_service;
    }; // PosixSignalWakeComponent

} // namespace consolix

#endif // _CONSOLIX_POSIX_SIGNAL_WAKE_COMPONENT_HPP_INCLUDED
