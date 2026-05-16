#pragma once
#ifndef _CONSOLIX_MODULE_HUB_COMPONENT_HPP_INCLUDED
#define _CONSOLIX_MODULE_HUB_COMPONENT_HPP_INCLUDED

/// \file ModuleHubComponent.hpp
/// \brief Defines optional event-hub-cpp ModuleHub processing component.
/// \ingroup Components

#if CONSOLIX_USE_EVENT_HUB == 1

#include <cstddef>

#include <event_hub.hpp>

namespace consolix {

    /// \class ModuleHubComponent
    /// \brief Processes an event_hub::ModuleHub from the Consolix component loop.
    ///
    /// The component does not own the hub. The referenced ModuleHub must outlive
    /// this component and should not run its own active `run()` or `start()` loop
    /// while it is processed by Consolix.
    class ModuleHubComponent :
        public IAppComponent,
        public IShutdownable {
    public:
        /// \brief Constructs an empty component.
        ModuleHubComponent() = default;

        /// \brief Constructs component with a non-owning ModuleHub pointer.
        /// \param module_hub ModuleHub to process, or nullptr.
        explicit ModuleHubComponent(event_hub::ModuleHub* module_hub) :
            m_module_hub(module_hub) {
        }

        /// \brief Sets ModuleHub processed by this component.
        /// \param module_hub ModuleHub to process, or nullptr.
        void set_module_hub(event_hub::ModuleHub* module_hub) {
            m_module_hub = module_hub;
        }

        /// \brief Returns work units processed by the last pass.
        /// \return Number of processed events, tasks, and module hooks.
        std::size_t last_work_count() const {
            return m_last_work_count;
        }

    protected:
        bool initialize() override {
            if (m_module_hub) {
                m_module_hub->initialize();
            }
            m_is_init = true;
            return true;
        }

        bool is_initialized() const override {
            return m_is_init;
        }

        void process() override {
            if (!m_module_hub) {
                m_last_work_count = 0;
                return;
            }

            m_last_work_count = m_module_hub->process();
        }

        void shutdown(int /*signal*/) override {
            if (m_module_hub) {
                m_module_hub->request_stop();
                m_module_hub->shutdown();
            }
        }

    private:
        event_hub::ModuleHub* m_module_hub{nullptr}; ///< Non-owning ModuleHub pointer.
        std::size_t           m_last_work_count{0};
        bool                  m_is_init{false};
    };

} // namespace consolix

#endif // CONSOLIX_USE_EVENT_HUB == 1

#endif // _CONSOLIX_MODULE_HUB_COMPONENT_HPP_INCLUDED
