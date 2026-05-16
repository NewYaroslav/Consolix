#pragma once
#ifndef _CONSOLIX_EVENT_HUB_COMPONENT_HPP_INCLUDED
#define _CONSOLIX_EVENT_HUB_COMPONENT_HPP_INCLUDED

/// \file EventHubComponent.hpp
/// \brief Defines optional event-hub-cpp processing component.
/// \ingroup Components

#include "../config_macros.hpp"

#if CONSOLIX_USE_EVENT_HUB == 1

#include <cstddef>

#include <event_hub.hpp>

namespace consolix {

    /// \class EventHubComponent
    /// \brief Processes event-hub-cpp sources from the Consolix component loop.
    ///
    /// The component is intentionally passive. It does not own a thread and only
    /// calls `EventBus::process()` and `TaskManager::process()` when Consolix
    /// invokes the component's `process()` method.
    class EventHubComponent :
        public IAppComponent,
        public IShutdownable {
    public:
        /// \brief Default maximum number of tasks processed per loop pass.
        static constexpr std::size_t default_max_tasks_per_pass = 128;

        /// \brief Constructs an empty component.
        EventHubComponent() = default;

        /// \brief Constructs component with optional non-owning sources.
        /// \param event_bus Event bus to process, or nullptr.
        /// \param task_manager Task manager to process, or nullptr.
        /// \param max_tasks_per_pass Maximum ready tasks processed per pass.
        EventHubComponent(
                event_hub::EventBus* event_bus,
                event_hub::TaskManager* task_manager = nullptr,
                std::size_t max_tasks_per_pass = default_max_tasks_per_pass) :
            m_event_bus(event_bus),
            m_task_manager(task_manager),
            m_max_tasks_per_pass(max_tasks_per_pass) {
        }

        /// \brief Sets event bus processed by this component.
        /// \param event_bus Event bus to process, or nullptr.
        void set_event_bus(event_hub::EventBus* event_bus) {
            m_event_bus = event_bus;
        }

        /// \brief Sets task manager processed by this component.
        /// \param task_manager Task manager to process, or nullptr.
        void set_task_manager(event_hub::TaskManager* task_manager) {
            m_task_manager = task_manager;
        }

        /// \brief Sets maximum ready tasks processed per loop pass.
        /// \param max_tasks_per_pass Maximum ready tasks processed per pass.
        void set_max_tasks_per_pass(std::size_t max_tasks_per_pass) {
            m_max_tasks_per_pass = max_tasks_per_pass;
        }

        /// \brief Returns total work units processed by the last pass.
        /// \return Number of processed events and tasks.
        std::size_t last_work_count() const {
            return m_last_work_count;
        }

    protected:
        bool initialize() override {
            m_is_init = true;
            return true;
        }

        bool is_initialized() const override {
            return m_is_init;
        }

        void process() override {
            std::size_t work_count = 0;

            if (m_event_bus) {
                work_count += m_event_bus->process();
            }
            if (m_task_manager) {
                work_count += m_task_manager->process(m_max_tasks_per_pass);
            }

            m_last_work_count = work_count;
        }

        void shutdown(int /*signal*/) override {
            if (m_event_bus) {
                m_event_bus->clear_pending();
            }
            if (m_task_manager) {
                m_task_manager->cancel_all();
            }
        }

    private:
        event_hub::EventBus*     m_event_bus{nullptr};       ///< Non-owning event bus pointer.
        event_hub::TaskManager*  m_task_manager{nullptr};    ///< Non-owning task manager pointer.
        std::size_t              m_max_tasks_per_pass{default_max_tasks_per_pass};
        std::size_t              m_last_work_count{0};
        bool                     m_is_init{false};
    };

} // namespace consolix

#endif // CONSOLIX_USE_EVENT_HUB == 1

#endif // _CONSOLIX_EVENT_HUB_COMPONENT_HPP_INCLUDED
