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

Consolix is a `header-only` C++ library for building console applications around reusable components, a service locator, and a small set of utility modules.

## Highlights

- `header-only` delivery with an official `Consolix::Consolix` CMake `INTERFACE` target
- aggregate-first public entry headers:
  - `#include <consolix/consolix.hpp>`
  - `#include <consolix/core.hpp>`
  - `#include <consolix/components.hpp>`
  - `#include <consolix/utils.hpp>`
- intended standalone utility leaves:
  - `#include <consolix/utils/json_utils.hpp>`
  - `#include <consolix/utils/path_utils.hpp>`
  - `#include <consolix/utils/enums.hpp>`
  - `#include <consolix/utils/types.hpp>`
- compatibility with `C++11`, `C++14`, and `C++17`
- optional integration with:
  - [LogIt](https://github.com/NewYaroslav/log-it-cpp)
  - [cxxopts](https://github.com/jarro2783/cxxopts)
  - [nlohmann/json](https://github.com/nlohmann/json)

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

API documentation: https://newyaroslav.github.io/Consolix/
