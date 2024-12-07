#pragma once
#ifndef _CONSOLIX_CORE_HPP_INCLUDED
#define _CONSOLIX_CORE_HPP_INCLUDED

/// \file core.hpp
/// \brief Entry point for including all core headers of the Consolix framework.
/// \ingroup Core
///
/// This file acts as a central inclusion point for the Consolix framework,
/// aggregating core modules, utilities, and components. Including this header
/// provides access to all essential framework functionalities.
///
/// \defgroup Core Core
/// \brief The core functionality of the Consolix framework.
///
/// The Core group includes fundamental classes and utilities for managing the application lifecycle,
/// global services, and component interaction.
///
/// ### Main Components:
/// - **ServiceLocator**: A mechanism for registering and accessing global services.
/// - **AppComponentManager**: A manager for handling application components.
/// - **ConsoleApplication**: A singleton for managing the console application. It includes `AppComponentManager`.
/// - **Utilities**: Functions to simplify working with applications and services.
///
/// ### Key Features:
/// - Management of service registration and access.
/// - Lifecycle support for components (initialization, execution, shutdown).
/// - Utilities for quick application setup.
///
/// ### Header Files:
/// - `core/ServiceLocator.hpp`
/// - `core/service_utils.hpp`
/// - `core/AppComponentManager.hpp`
/// - `core/ConsoleApplication.hpp`
/// - `core/application_utils.hpp`
///
/// ### Example Usage:
/// ```cpp
/// #define CONSOLIX_USE_LOGIT 1
/// #include <consolix/core.hpp>
///
/// int main(int argc, char* argv[]) {
///     // Initialize the logger. This must be the first component.
///     consolix::add<consolix::LoggerComponent>();
///
///     // Add a logo component to display the application logo at startup.
///     consolix::add<consolix::LogoComponent>(consolix::TextColor::DarkYellow);
///
///     // Register a service.
///     consolix::register_service<std::string>([]() {
///         return std::make_shared<std::string>("Hello, Consolix!");
///     });
///
///     // Check and retrieve the service.
///     if (consolix::has_service<std::string>()) {
///         auto& message = consolix::get_service<std::string>();
///         std::cout << message << std::endl;
///     }
///
///     // Initialize application components and start the main loop.
///     consolix::run([]() {
///         CONSOLIX_STREAM() <<
///             consolix::color(consolix::TextColor::Green) << "Running in loop";
///     });
/// }
/// ```

#include "config_macros.hpp"            ///< Global configuration macros for Consolix.
#include "interfaces.hpp"               ///< Interfaces defining the base structure of components.
#include "utils.hpp"                    ///< General utilities for the framework.
#include "components.hpp"               ///< Predefined application components.
#include "core/ServiceLocator.hpp"      ///< Singleton for managing globally accessible services.
#include "core/service_utils.hpp"       ///< Helper functions for working with services.
#include "core/AppComponentManager.hpp" ///< Manager for application components and their lifecycle.
#include "core/ConsoleApplication.hpp"  ///< Singleton managing the console application's lifecycle.
#include "core/application_utils.hpp"   ///< Helper functions for application setup and execution.

#endif // _CONSOLIX_CORE_HPP_INCLUDED
