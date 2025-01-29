#pragma once
#ifndef _CONSOLIX_COMPONENTS_HPP_INCLUDED
#define _CONSOLIX_COMPONENTS_HPP_INCLUDED

/// \file components.hpp
/// \brief Entry point for including all Consolix components.
/// \ingroup Components
///
/// This header serves as a unified inclusion point for the frequently used components
/// of the Consolix library, simplifying integration and ensuring consistent access
/// to core functionalities.
///
/// \defgroup Components Components
/// \brief Core components of the Consolix framework.
///
/// The Components group includes modules that provide functionality such as logging,
/// command-line argument handling, configuration management, logo rendering, and
/// structured execution loops.
///
/// ### Main Components:
/// - **LoggerComponent**: Configures and manages logging functionality.
/// - **LogoComponent**: Displays the application logo in the console.
/// - **CliComponent**: Handles command-line arguments using the cxxopts library.
/// - **ConfigComponent**: Loads and manages configuration from JSON files.
/// - **BaseLoopComponent**: A base class for loop-based components.
/// - **LoopComponent**: A component with customizable execution loops.
///
/// ### Key Features:
/// - Easy integration of logging and logo rendering.
/// - Convenient handling of command-line arguments.
/// - Lightweight JSON configuration management, including support for comments.
/// - Support for custom execution loops using either a base class or the LoopComponent.
///
/// ### Included Headers:
/// - `components/LoggerComponent.hpp`
/// - `components/LogoComponent.hpp`
/// - `components/CliComponent.hpp`
/// - `components/ConfigComponent.hpp`
/// - `components/BaseLoopComponent.hpp`
/// - `components/LoopComponent.hpp`
///
/// ### Example Usage:
///
/// ```cpp
/// #define CONSOLIX_USE_LOGIT 1
/// #include <consolix/components.hpp>
///
/// int main(int argc, char* argv[]) {
///     // Add logging functionality
///     consolix::add<consolix::LoggerComponent>();
///
///     // Display the application logo
///     consolix::add<consolix::LogoComponent>(consolix::TextColor::DarkYellow);
///
///     // Handle command-line arguments
///     consolix::add<consolix::CliComponent>("MyApp", "Program description", [](auto& options) {
///         options.add_options()("debug", "Enable debug mode");
///     }, argc, argv);
///
///     // Load configuration from a JSON file
///     consolix::add<consolix::ConfigComponent<MyConfig>>("config.json");
///
///     // Execute the main loop
///     consolix::run([]() {
///         CONSOLIX_STREAM() << "Executing main loop...";
///     });
/// }
/// ```

#include "config_macros.hpp"                ///< Global configuration macros for Consolix.

// Core interfaces needed for all components
#include "interfaces.hpp"                   ///< Core interfaces for component interaction.

// Service management utilities required by LoggerComponent, CliComponent, ConfigComponent
#include "core/ServiceLocator.hpp"          ///< Service locator for managing shared resources.
#include "core/service_utils.hpp"           ///< Utility functions for working with services.

// Utilities required for encoding transformations in LoggerComponent and CliComponent
#include "utils/encoding_utils.hpp"         ///< Tools for character encoding transformations.

// Core components of the Consolix framework
#include "components/TitleComponent.hpp"
#include "components/LoggerComponent.hpp"   ///< Component for managing logging.
#include "components/LogoComponent.hpp"     ///< Component for rendering logos in the console.
#include "components/BaseLoopComponent.hpp" ///< Base class for implementing loop-based components.
#include "components/CliComponent.hpp"      ///< Component for handling command-line arguments.

// Required by ConfigComponent
#include "utils/path_utils.hpp"             ///< Utilities for working with file and directory paths.
#include "utils/json_utils.hpp"             ///< Utilities for processing and manipulating JSON strings.

// JSON configuration management
#include "components/ConfigComponent.hpp"   ///< Component for loading and managing configuration data.

#endif // _CONSOLIX_COMPONENTS_HPP_INCLUDED
