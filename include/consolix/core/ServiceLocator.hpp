#pragma once
#ifndef _CONSOLIX_SERVICE_LOCATOR_HPP_INCLUDED
#define _CONSOLIX_SERVICE_LOCATOR_HPP_INCLUDED

/// \file ServiceLocator.hpp
/// \brief Provides a universal service locator for managing shared resources.
/// \ingroup Core

#if CONSOLIX_USE_LOGIT == 1
#include <log-it/LogIt.hpp>
#endif

#include <unordered_map>
#include <functional>
#include <memory>
#include <typeindex>
#include <stdexcept>
#include <shared_mutex>
#include <mutex>

namespace consolix {

    /// \class ServiceLocator
    /// \brief A universal service locator for managing shared resources.
    ///
    /// The `ServiceLocator` class implements a singleton pattern to provide
    /// a centralized way of registering and accessing shared resources or services.
    class ServiceLocator {
    public:

        /// \brief Retrieves the singleton instance of the `ServiceLocator`.
        /// \return Reference to the `ServiceLocator` instance.
        static ServiceLocator& get_instance() {
            static ServiceLocator* instance = new ServiceLocator();
            return *instance;
        }

        /// \brief Registers a resource or service.
        /// \tparam T The type of the resource.
        /// \param creator A function to create the resource (optional).
        /// \throws `std::runtime_error` if the resource is already registered.
        template <typename T>
        void register_service(std::function<std::shared_ptr<T>()> creator) {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            const auto type = std::type_index(typeid(T));
            if (m_services.find(type) != m_services.end()) {
#               if CONSOLIX_USE_LOGIT == 1
                LOGIT_PRINT_ERROR("Service already registered: ", std::string(typeid(T).name()));
#               endif
                throw std::runtime_error("Service already registered: " + std::string(typeid(T).name()));
            }
            lock.unlock();
            auto ptr = creator();
            lock.lock();
            m_services[type] = std::move(ptr);
        }

        /// \brief Registers a resource with default construction.
        /// \tparam T The type of the resource.
        /// \throws `std::runtime_error` if the resource is already registered.
        template <typename T>
        void register_service() {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            const auto type = std::type_index(typeid(T));
            if (m_services.find(type) != m_services.end()) {
#               if CONSOLIX_USE_LOGIT == 1
                LOGIT_PRINT_ERROR("Service already registered: ", std::string(typeid(T).name()));
#               endif
                throw std::runtime_error("Service already registered: " + std::string(typeid(T).name()));
            }
            m_services[type] = std::make_shared<T>();
        }

        /// \brief Retrieves a resource from the locator.
        /// \tparam T The type of the resource.
        /// \return Reference to the resource.
        /// \throws `std::runtime_error` if the resource is not registered.
        template <typename T>
        T& get_service() {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            const auto type = std::type_index(typeid(T));
            auto it = m_services.find(type);
            if (it == m_services.end()) {
#               if CONSOLIX_USE_LOGIT == 1
                LOGIT_PRINT_ERROR("Service not registered: ", std::string(typeid(T).name()));
#               endif
                throw std::runtime_error("Service not registered: " + std::string(typeid(T).name()));
            }
            return *std::static_pointer_cast<T>(it->second);
        }

        /// \brief Checks if a resource is registered.
        /// \tparam T The type of the resource.
        /// \return `true` if the resource is registered, `false` otherwise.
        template <typename T>
        bool has_service() {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            const auto type_id = std::type_index(typeid(T));
            return m_services.find(type_id) != m_services.end();
        }

        /// \brief Clears all registered resources.
        void clear_all() {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            m_services.clear();
        }

    private:
        std::unordered_map<
            std::type_index,
            std::shared_ptr<void>> m_services; ///< Registered services.
        std::shared_mutex          m_mutex;    ///< Mutex for thread-safe access.

        ServiceLocator() = default;
        ~ServiceLocator() = default;

        // Delete copy and move constructors and assignment operators.
        ServiceLocator(const ServiceLocator&) = delete;
        ServiceLocator& operator=(const ServiceLocator&) = delete;
        ServiceLocator(ServiceLocator&&) = delete;
        ServiceLocator& operator=(ServiceLocator&&) = delete;
    }; // ServiceLocator

} // namespace consolix

#endif // _CONSOLIX_SERVICE_LOCATOR_HPP_INCLUDED
