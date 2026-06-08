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

It helps with the recurring problems that show up in real console tools: organizing startup and shutdown, reacting to termination signals, wiring shared services, showing a logo or title, parsing arguments, loading configuration, and keeping the main loop readable. The goal is to make console apps easier to assemble, extend, and maintain without introducing a heavy runtime framework.

## Capabilities

- structure console applications around reusable components and a shared service locator instead of a monolithic `main.cpp`
- manage application lifecycle, execution loops, and graceful shutdown flows, including termination-signal-driven stop requests
- add console title handling, ASCII logo output, and logging without repeating boilerplate in every tool
- parse command-line arguments and load JSON-based configuration through dedicated components
- reuse common helpers for paths, colors, encodings, and system-oriented console tasks
- stay lightweight with `header-only` delivery, `C++11`/`14`/`17` support, and opt-in integrations only when a project needs them

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

## Dependencies

The repository vendors several external libraries as submodules under `external/`.

- [LogIt](https://github.com/NewYaroslav/log-it-cpp) powers `LoggerComponent` and the LogIt-backed logging path when `CONSOLIX_USE_LOGIT=1`
- [time-shield-cpp](https://github.com/NewYaroslav/time-shield-cpp) is a vendored dependency used by the LogIt integration; most Consolix users do not interact with it directly through the Consolix API
- [cxxopts](https://github.com/jarro2783/cxxopts) enables `CliComponent` and related CLI aliases when `CONSOLIX_USE_CXXOPTS=1`
- [nlohmann/json](https://github.com/nlohmann/json) enables `ConfigComponent` and JSON-backed configuration loading when `CONSOLIX_USE_JSON=1`

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

Consolix is published as a `header-only` library and exports the official `Consolix::Consolix` `INTERFACE` target for normal CMake consumption.

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

When `main()` must return the lifecycle result instead of letting
`ConsoleApplication::run()` call `std::exit`, use the exit-code runner:

```cpp
int main() {
    consolix::add<MyComponent>();
    return consolix::run_for_exit_code();
}
```

Components can request a specific code with `consolix::stop(code)`;
`consolix::request_stop(code)` is available as the explicit form.
For applications that already own an `AppComponentManager`, use
`consolix::ConsoleApplicationRunner runner(manager);` and return
`runner.run_for_exit_code()`.

A `ConsoleApplicationRunner` instance is single-use. Create a new manager and a
new runner for another lifecycle. While `run_for_exit_code()` is active, global
`consolix::stop()` and `consolix::request_stop()` target the active runner;
otherwise they target the singleton `ConsoleApplication` facade.

`ConsoleApplication::run()` remains the process-owning compatibility facade:
internally it uses the same runner and exits with the returned code. On Windows,
Ctrl+C/Ctrl+Break request cooperative shutdown; close/logoff/shutdown events
request shutdown on the runner thread and wait for a bounded cleanup window.

### Shutdown Ordering and Final Hooks

`AppComponentManager` shuts down components in reverse registration order. After
all component shutdown callbacks finish, the runner clears `ServiceLocator` and
then shuts down LogIt. This means services and logging are still available while
component `shutdown()` callbacks run.

If an application needs a final cleanup hook after ordinary components stop, but
before `ServiceLocator::clear_all()`, register that hook before the components
that use it:

```cpp
consolix::add<consolix::LoopComponent>(
    []() { return true; },
    []() {},
    [](int exit_code) {
        // Final cleanup while services and LogIt are still alive.
    });

consolix::add<RealComponent>();
```

Because shutdown is LIFO, `RealComponent` shuts down first and the hook runs
after it. Prefer this component-level pattern over runner-level shutdown hooks
so cleanup stays in the same lifecycle model as the rest of the application.

### Main Loop and CPU Usage

Consolix runs components in a polling loop and does not sleep between
iterations by default. If all components return immediately, the process may
use excessive CPU. For latency-sensitive applications this is intentional:
the framework does not hide an implicit delay inside the runner.

For ordinary services and tools, add a throttle component near the end of the
component list:

```cpp
auto throttle = consolix::add<consolix::LoopThrottleComponent>(
    std::chrono::milliseconds(1));

// Another thread can end the wait early when it posts new work.
throttle->wake();
```

`LoopThrottleComponent` waits at most for the configured delay on every loop
pass. `wake()` interrupts the current wait, and wake requests made before the
wait begins are consumed by the next pass. When the application is run through
`ConsoleApplicationRunner`, `consolix::stop()` and `consolix::request_stop()`
also wake throttle waits through the shared `LoopWakeService`, so shutdown does
not have to wait for a long throttle delay. If you drive `AppComponentManager`
directly without the runner, keep the delay short or call `wake()` from the
same control path that requests stop.

On POSIX platforms, signal handlers remain async-signal-safe: they only store a
pending signal flag. They do not wake C++ condition variables directly, so a
long blocking `process()` call can delay SIGINT/SIGTERM handling until control
returns to the runner loop.

## Diagnostic Streams

Consolix provides two multi-target log macros that route messages through
LogIt backends. The console backend is configured so that `ERROR` and `FATAL`
levels go to `std::cerr`, while `TRACE` through `WARN` go to `std::cout`.

| Macro | Console backend | Regular backends | Extra backends | Color |
|---|---|---|---|---|
| `CONSOLIX_LOG_STREAM(level)` | yes (level-based routing) | broadcast | - | via `<< color(...)` |
| `CONSOLIX_LOG_STREAM_NO_BROADCAST(level)` | yes (level-based routing) | file fallback only | - | via `<< color(...)` |
| `CONSOLIX_LOG_STREAM_EX(level, ...)` | yes (level-based routing) | file fallback only | inline indices | via `<< color(...)` |

The `level` argument is mandatory (`logit::LogLevel::LOG_LVL_INFO`,
`LOG_LVL_ERROR`, etc.). Color is applied explicitly by the caller through
`<< consolix::color(consolix::TextColor::Red)`.

**Note on `_EX` variants.** The standard console backend (`CONSOLIX_LOGIT_CONSOLE_INDEX`)
is already included by both `CONSOLIX_LOG_STREAM` and `CONSOLIX_LOG_STREAM_EX`.
`CONSOLIX_LOG_STREAM` broadcasts to regular non-single backends, including the
standard file logger and user-added regular backends. `CONSOLIX_LOG_STREAM_EX`
does not broadcast; it writes to the file fallback and to the listed backend
indices.

**Inline targets.** To target selected LogIt backends beyond the standard
console and file fallback, use the `_EX` variant:

```cpp
CONSOLIX_LOG_STREAM_EX(
        logit::LogLevel::LOG_LVL_ERROR,
        CONSOLIX_LOGIT_UNIQUE_FILE_INDEX)
    << "Hello with an extra single-mode backend" << std::endl;
```

See `examples/example_stderr_diagnostics.cpp` for a runnable example.

## Documentation

Additional repository guidance:

- developer guidelines: `docs/header-implementation-guidelines.md`
- agent playbook: `guides/header-implementation-guidelines.md`
- lifecycle example: `examples/example_shutdown_and_resources.cpp`
- exit-code runner example: `examples/example_exit_code_runner.cpp`
- loop throttle example: `examples/example_loop_throttle_component.cpp`
- diagnostic streams: `examples/example_stderr_diagnostics.cpp`

API documentation: https://newyaroslav.github.io/Consolix/
