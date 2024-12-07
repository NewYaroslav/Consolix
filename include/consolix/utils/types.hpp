#pragma once
#ifndef _CONSOLIX_TYPES_HPP_INCLUDED
#define _CONSOLIX_TYPES_HPP_INCLUDED

/// \file types.hpp
/// \brief Contains common type.

#if CONSOLIX_USE_CXXOPTS == 1
#include <cxxopts.hpp>
#endif

namespace consolix {

#   if CONSOLIX_USE_CXXOPTS == 1

    /// \typedef CliOptions
    /// \brief Alias for cxxopts::Options.
    using CliOptions   = cxxopts::Options;

    /// \typedef CliArguments
    /// \brief Alias for cxxopts::ParseResult.
    using CliArguments = cxxopts::ParseResult;

#   endif // CONSOLIX_USE_CXXOPTS == 1

}; // namespace consolix

#endif // _CONSOLIX_TYPES_HPP_INCLUDED
