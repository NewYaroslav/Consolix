#pragma once
#ifndef _CONSOLIX_UTILS_HPP_INCLUDED
#define _CONSOLIX_UTILS_HPP_INCLUDED

/// \file utils.hpp
/// \brief Entry point for including utility headers for Consolix.
/// \ingroup Utilities
///
/// This header serves as a unified inclusion point for various utility modules within the Consolix framework.
/// By including this file, all common utilities, enumerations, and helper functions become available.
///
/// \defgroup Utilities Utilities
/// \brief Common utility modules and helpers in Consolix.
///
/// The Utilities group includes a variety of modules that provide helper functions, type definitions,
/// and features to simplify application development with Consolix. These utilities cover
/// JSON manipulation, path handling, enumeration definitions, encoding transformations, and more.
///
/// ### Included Modules:
/// - **Enumerations (`utils/enums.hpp`)**: Centralized definitions of shared constants and flags.
/// - **Types (`utils/types.hpp`)**: Common type aliases and structures.
/// - **Color Manipulation (`utils/ColorManipulator.hpp`)**: Helpers for stream-based text color styling.
/// - **Path Utilities (`utils/path_utils.hpp`)**: Functions for handling file and directory paths.
/// - **JSON Utilities (`utils/json_utils.hpp`)**: Helpers for processing and manipulating JSON strings.
/// - **Encoding Utilities (`utils/encoding_utils.hpp`)**: Tools for working with character encoding transformations.
///
/// ### Example Usage:
///
/// ```cpp
/// #include <consolix/utils.hpp>
///
/// int main() {
///     // Example of using ColorManipulator for styled console output
///     CONSOLIX_STREAM() << consolix::color(consolix::TextColor::Green) << "Hello, Consolix!" << std::endl;
///
///     // Example of working with paths
///     std::string exe_dir = consolix::resolve_executable_path();
///     CONSOLIX_STREAM() << "Executable Directory: " << exe_dir << std::endl;
///
///     // Example of stripping comments from a JSON string
///     std::string raw_json = R"({
///         "key": "value", // This is a comment
///         /* Another comment */
///         "key2": "value2"
///     })";
///     std::string clean_json = consolix::strip_json_comments(raw_json);
///     CONSOLIX_STREAM() << "Clean JSON: " << clean_json << std::endl;
///
///     // Example of encoding transformations
///     std::string utf8_string = "Привет, мир!";
///     std::string ansi_string = consolix::utf8_to_ansi(utf8_string);
///     CONSOLIX_STREAM() << "ANSI Encoded: " << ansi_string << std::endl;
///
///     return 0;
/// }
/// ```

#include "config_macros.hpp"          ///< Global configuration macros for Consolix.
#include "core/platform_includes.hpp" ///< Platform-specific includes and definitions.
#include "utils/enums.hpp"            ///< Enumerations for shared constants.
#include "utils/types.hpp"            ///< Common type definitions and aliases.
#include "utils/ColorManipulator.hpp" ///< Stream-based text color manipulation utilities.
#include "utils/path_utils.hpp"       ///< File and directory path utilities.
#include "utils/json_utils.hpp"       ///< Utilities for working with JSON strings.
#include "utils/encoding_utils.hpp"   ///< Tools for character encoding transformations.

#endif // _CONSOLIX_UTILS_HPP_INCLUDED
