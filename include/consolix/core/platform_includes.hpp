#pragma once
#ifndef _CONSOLIX_PLATFORM_INCLUDES_HPP_INCLUDED
#define _CONSOLIX_PLATFORM_INCLUDES_HPP_INCLUDED

/// \file platform_includes.hpp
/// \brief Contains platform-specific API includes and definitions.
/// \ingroup Core

/// \cond DOXYGEN_IGNORE
#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <direct.h>
#include <minwindef.h>
#include <winbase.h>
#else
#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#endif
/// \endcond

#endif // _CONSOLIX_PLATFORM_INCLUDES_HPP_INCLUDED
