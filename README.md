# Consolix

```bash
   █████████                                       ████   ███             
  ███░░░░░███                                     ░░███  ░░░              
 ███     ░░░   ██████  ████████    █████   ██████  ░███  ████  █████ █████
░███          ███░░███░░███░░███  ███░░   ███░░███ ░███ ░░███ ░░███ ░░███ 
░███         ░███ ░███ ░███ ░███ ░░█████ ░███ ░███ ░███  ░███  ░░░█████░  
░░███     ███░███ ░███ ░███ ░███  ░░░░███░███ ░███ ░███  ░███   ███░░░███ 
 ░░█████████ ░░██████  ████ █████ ██████ ░░██████  █████ █████ █████ █████
  ░░░░░░░░░   ░░░░░░  ░░░░ ░░░░░ ░░░░░░   ░░░░░░  ░░░░░ ░░░░░ ░░░░░ ░░░░░ 
```

Consolix is a `header-only` C++ library for building structured console applications around reusable components, a service locator, and a focused set of utility modules.

## Overview

Consolix is designed for console applications that need more structure than a single `main.cpp`, but still want a lightweight, easy-to-integrate setup.

It provides a component-based application model, shared services, path and JSON helpers, and optional integrations for logging, CLI parsing, and configuration loading. The goal is to make console apps easier to assemble, extend, and maintain without introducing a heavy runtime framework.

## Capabilities

- `header-only` delivery with an official `Consolix::Consolix` CMake `INTERFACE` target
- component-based application architecture for logging, configuration, titles, logos, and execution loops
- aggregate-first public entry headers for normal consumption:
  - `#include <consolix/consolix.hpp>`
  - `#include <consolix/core.hpp>`
  - `#include <consolix/components.hpp>`
  - `#include <consolix/utils.hpp>`
- intended standalone utility leaves for focused direct includes:
  - `#include <consolix/utils/json_utils.hpp>`
  - `#include <consolix/utils/path_utils.hpp>`
  - `#include <consolix/utils/enums.hpp>`
  - `#include <consolix/utils/types.hpp>`
- compatibility with `C++11`, `C++14`, and `C++17`
- optional integration with:
  - [LogIt](https://github.com/NewYaroslav/log-it-cpp)
  - [cxxopts](https://github.com/jarro2783/cxxopts)
  - [nlohmann/json](https://github.com/nlohmann/json)

## Quick Start

For most projects, the fastest way to start is:

1. Add Consolix to your project and make `include/` available to the compiler.
2. Define only the feature macros you actually need.
3. Include one of the aggregate entry headers, usually:

```cpp
#include <consolix/consolix.hpp>
```

If you use CMake, the preferred consumer path is `Consolix::Consolix`. If you need more detail about include layout, feature flags, or build setup, the next sections cover those contracts explicitly.

## Include Model

Consolix is designed primarily around aggregate entry headers.

For normal consumer code, prefer:

```cpp
#include <consolix/consolix.hpp>
```

or one of the module-level entry headers:

```cpp
#include <consolix/core.hpp>
#include <consolix/components.hpp>
#include <consolix/utils.hpp>
```

The utility leaf headers below are also supported as direct includes:

```cpp
#include <consolix/utils/json_utils.hpp>
#include <consolix/utils/path_utils.hpp>
#include <consolix/utils/enums.hpp>
#include <consolix/utils/types.hpp>
```

Other internal leaf headers should be treated as implementation details of the aggregate entry points unless the documentation says otherwise.

## Feature Macros

All optional integrations are disabled by default.

```cpp
#define CONSOLIX_USE_LOGIT   0
#define CONSOLIX_USE_CXXOPTS 0
#define CONSOLIX_USE_JSON    0

#include <consolix/consolix.hpp>
```

Enable only the features you need before including Consolix headers.

## CMake

```cmake
cmake_minimum_required(VERSION 3.14)
project(MyApp LANGUAGES CXX)

set(CONSOLIX_CXX_STANDARD 17 CACHE STRING "")
set(CONSOLIX_USE_LOGIT ON CACHE BOOL "")
set(CONSOLIX_USE_CXXOPTS ON CACHE BOOL "")
set(CONSOLIX_USE_JSON ON CACHE BOOL "")

add_subdirectory(path/to/Consolix)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE Consolix::Consolix)
```

Supported values for `CONSOLIX_CXX_STANDARD` are `11`, `14`, and `17`.

On GNU toolchains, `Consolix::Consolix` links `stdc++fs` automatically for `C++11` and `C++14` builds that use experimental filesystem support.

## Example

```cpp
#define CONSOLIX_USE_LOGIT   1
#define CONSOLIX_USE_CXXOPTS 1
#define CONSOLIX_USE_JSON    1

#include <consolix/core.hpp>

struct AppConfig {
    std::string text;
    std::vector<std::string> items;
    int period;
    bool debug_mode;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(AppConfig, text, items, period, debug_mode)
};

int main(int argc, char* argv[]) {
    consolix::add<consolix::TitleComponent>("Consolix demo");
    consolix::add<consolix::LoggerComponent>();
    consolix::add<consolix::CliComponent>(
        "Consolix",
        "Example application",
        [](consolix::CliOptions& options) {
            options.add_options()
                ("c,config", "Path to the configuration file", cxxopts::value<std::string>())
                ("d,debug", "Enable debug mode", cxxopts::value<bool>()->default_value("false"));
        },
        argc,
        argv);
    consolix::add<consolix::ConfigComponent<AppConfig>>("config.json", "config");

    consolix::run([]() {
        CONSOLIX_STREAM() << "Running...";
    });
}
```

## Documentation

Additional repository guidance:

- developer guidelines: `docs/header-implementation-guidelines.md`
- agent playbook: `agents/header-implementation-guidelines.md`
- lifecycle example: `examples/example_shutdown_and_resources.cpp`

API documentation: https://newyaroslav.github.io/Consolix/
