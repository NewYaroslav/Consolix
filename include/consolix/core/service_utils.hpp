#pragma once
#ifndef _CONSOLIX_SERVICE_UTILS_HPP_INCLUDED
#define _CONSOLIX_SERVICE_UTILS_HPP_INCLUDED

/// \file service_utils.hpp
/// \brief Utility functions for working with the `ServiceLocator`.
/// \ingroup Core
/// ### Example usage of the ServiceLocator.
/// ```cpp
/// consolix::register_service<CliOptions>([]() {
///     return std::make_shared<CliOptions>("AppName", "Description");
/// });
///
/// auto& options = consolix::get_service<CliOptions>();
/// options.add_options()("debug", "Enable debugging", cxxopts::value<bool>()->default_value("false"));
///
/// consolix::register_service<std::string>([]() {
///     return std::make_shared<std::string>("Hello, World!");
/// });
///
/// if (consolix::has_service<std::string>()) {
///     auto& message = consolix::get_service<std::string>();
///     std::cout << message << std::endl;
/// }
/// ```

namespace consolix {

    /// \brief Registers a resource or service globally.
    /// Registers a resource or service with the `ServiceLocator` using the provided creator function.
    /// \tparam T The type of the resource.
    /// \param creator A function to create the resource (optional).
    template <typename T>
    inline void register_service(std::function<std::shared_ptr<T>()> creator) {
        ServiceLocator::get_instance().register_service<T>(std::move(creator));
    }

    /// \brief Registers a resource with default construction globally.
    /// Registers a resource or service using default construction.
    /// \tparam T The type of the resource.
    template <typename T>
    inline void register_service() {
        ServiceLocator::get_instance().register_service<T>();
    }

    /// \brief Retrieves a resource globally.
    /// Retrieves a reference to a globally registered resource from the `ServiceLocator`.
    /// \tparam T The type of the resource.
    /// \return Reference to the resource.
    template <typename T>
    inline T& get_service() {
        return ServiceLocator::get_instance().get_service<T>();
    }

    /// \brief Checks if a resource is registered globally.
    /// \tparam T The type of the resource.
    /// \return `true` if the resource is registered, `false` otherwise.
    template <typename T>
    inline bool has_service() {
        return ServiceLocator::get_instance().has_service<T>();
    }

    /// \brief Clears all registered resources globally.
    /// Clears all resources and services registered in the `ServiceLocator`
    inline void clear_all() {
        ServiceLocator::get_instance().clear_all();
    }

}; // namespace consolix

#endif // _CONSOLIX_SERVICE_UTILS_HPP_INCLUDED
