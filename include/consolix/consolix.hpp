#pragma once
#ifndef _CONSOLIX_HPP_INCLUDED
#define _CONSOLIX_HPP_INCLUDED

/// \file consolix.hpp
/// \brief Single include header for Consolix framework.
///
/// This header aggregates all entry-point headers for Consolix, allowing
/// developers to include the entire framework with a single directive.

#include "config_macros.hpp"    ///< Global configuration macros for Consolix.
#include "core.hpp"             ///< Core functionalities of Consolix.
#include "components.hpp"       ///< Aggregated components of Consolix.
#include "utils.hpp"            ///< Utility modules and helpers.

/// \namespace consolix
/// \brief Main namespace for the Consolix framework.
///
/// The `consolix` namespace contains all classes, functions, and utilities provided by the Consolix framework.
/// It is the primary namespace used for building structured and flexible console applications.
namespace consolix {};

#endif // _CONSOLIX_HPP_INCLUDED
