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

**Consolix** is a `header-only` C++ library providing components and utilities for developing console applications.

## Features

- **Component-Based Architecture**: Easy-to-integrate modules like logging, configuration management, and command-line argument parsing.
- **Global Service Locator**: A convenient mechanism for dependency and service management.
- **Customizable Execution Loops**: Support for structured loops to execute user-defined tasks.
- **Enhanced Logging**: Integration with [LogIt](https://github.com/NewYaroslav/log-it-cpp).
- **JSON Support with Comments**: Load JSON configurations with comments using [nlohmann/json](https://github.com/nlohmann/json).
- **Cross-Platform**: Works on Windows and POSIX-compatible systems. *(POSIX is not fully tested)*.
- **C++17 Compatibility**: Requires a C++17-compliant compiler (GCC 7.1+, Clang 5.0+, MSVC 15.3+).

## Installation

**Consolix** is a `header-only` library. To start using it, simply add it to your project:

1. Download or clone the repository:
   ```bash
   git clone https://github.com/yourusername/Consolix.git
   ```
   
2. Add the path to the include files in your project configuration:
   ```
   Consolix/include
   ```

3. Include the header in your code:
   ```cpp
   #include <consolix/consolix.hpp>
   ```

4. Dependencies

Consolix supports integration with external libraries via macros that can be enabled or disabled as needed:

- **LogIt** (enabled with `CONSOLIX_USE_LOGIT 1`):
    - Used for logging.
    - [LogIt GitHub Repository](https://github.com/NewYaroslav/log-it-cpp.git).
    - Depends on [time-shield](https://github.com/NewYaroslav/time-shield-cpp.git).
    - Both are *header-only* libraries, so adding their headers to your project is sufficient.
- **cxxopts** (enabled with `CONSOLIX_USE_CXXOPTS 1`):
    - Used for command-line argument parsing.
    - [cxxopts GitHub Repository](https://github.com/jarro2783/cxxopts.git).
    - A *header-only* library.
- **nlohmann/json** (enabled with `CONSOLIX_USE_JSON 1`):
    - Used for JSON parsing, including support for comments.
    - [nlohmann/json GitHub Repository](https://github.com/nlohmann/json.git).
    - A *header-only* library.

To specify which dependencies to use, define the respective macros before including Consolix:

```cpp
// Disable LogIt and cxxopts, but enable nlohmann/json
#define CONSOLIX_USE_LOGIT   0
#define CONSOLIX_USE_CXXOPTS 0
#define CONSOLIX_USE_JSON    1

#include <consolix/consolix.hpp>
```

To include dependencies, add their header paths to your project:

```
log-it-cpp/include
time-shield-cpp/include
cxxopts/include
nlohmann-json/include
```

## Example Usage

```cpp
#include <consolix/components.hpp>

int main(int argc, char* argv[]) {
    // Add logging functionality
    consolix::add<consolix::LoggerComponent>();

    // Display an ASCII logo in the console
    consolix::add<consolix::LogoComponent>(consolix::TextColor::DarkYellow);

    // Handle command-line arguments
    consolix::add<consolix::CliComponent>("MyApp", "Program description", [](auto& options) {
        options.add_options()("debug", "Enable debug mode");
    }, argc, argv);

    // Load configuration from a JSON file
    consolix::add<consolix::ConfigComponent<MyConfig>>("config.json");

    // Execute the main loop
    consolix::run([]() {
        CONSOLIX_STREAM() << "Executing main loop...";
    });

    return 0;
}
```

## Modules

### Core Modules
- **ServiceLocator**: Manage global services.
- **ConsoleApplication**: Framework for managing console applications.

### Components
- **LoggerComponent**: Centralized logging.
- **CliComponent**: Simplified command-line argument handling.
- **ConfigComponent**: Work with JSON configurations, including comments.
- **LogoComponent**: Render ASCII logos.
- **LoopComponent**: Support for user-defined execution loops.

### Utilities
- **path_utils.hpp**: Work with file and directory paths.
- **json_utils.hpp**: Remove comments and handle JSON data.
- **ColorManipulator.hpp**: Style console text output.

## Documentation

The complete documentation is available [here](#). It includes installation instructions, module descriptions, usage examples, and more.

## License

Consolix is distributed under the MIT License. See the LICENSE file for details.