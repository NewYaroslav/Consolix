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

Consolix — это `header-only` библиотека C++ для консольных приложений с компонентной моделью, service locator и набором утилит.

## Основное

- поставляется как `header-only`
- в CMake экспортируется как `Consolix::Consolix` через `INTERFACE` target
- основные публичные точки входа:
  - `#include <consolix/consolix.hpp>`
  - `#include <consolix/core.hpp>`
  - `#include <consolix/components.hpp>`
  - `#include <consolix/utils.hpp>`
- intended standalone utility headers:
  - `#include <consolix/utils/json_utils.hpp>`
  - `#include <consolix/utils/path_utils.hpp>`
  - `#include <consolix/utils/enums.hpp>`
  - `#include <consolix/utils/types.hpp>`
- поддерживаются стандарты `C++11`, `C++14`, `C++17`
- опциональные зависимости:
  - [LogIt](https://github.com/NewYaroslav/log-it-cpp)
  - [cxxopts](https://github.com/jarro2783/cxxopts)
  - [nlohmann/json](https://github.com/nlohmann/json)

## Модель подключений

Consolix спроектирован в aggregate-first стиле. Для обычного использования предпочитайте агрегирующие заголовки:

```cpp
#include <consolix/consolix.hpp>
```

или модульные entry points:

```cpp
#include <consolix/core.hpp>
#include <consolix/components.hpp>
#include <consolix/utils.hpp>
```

Прямое подключение leaf headers считается поддерживаемым только для следующих утилит:

```cpp
#include <consolix/utils/json_utils.hpp>
#include <consolix/utils/path_utils.hpp>
#include <consolix/utils/enums.hpp>
#include <consolix/utils/types.hpp>
```

Остальные внутренние leaf headers следует считать деталями реализации aggregate entry points, если документация явно не говорит обратное.

## Макросы возможностей

Все опциональные зависимости по умолчанию выключены:

```cpp
#define CONSOLIX_USE_LOGIT   0
#define CONSOLIX_USE_CXXOPTS 0
#define CONSOLIX_USE_JSON    0

#include <consolix/consolix.hpp>
```

Включайте только те подсистемы, которые действительно нужны вашему приложению.

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

Допустимые значения `CONSOLIX_CXX_STANDARD`: `11`, `14`, `17`.

Для GNU toolchains в режимах `C++11` и `C++14` target автоматически добавляет `stdc++fs`, если нужен experimental filesystem.

## Короткий пример

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
                ("c,config", "Path to configuration file", cxxopts::value<std::string>())
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

## Документация

- developer guidelines: `docs/header-implementation-guidelines.md`
- agent playbook: `agents/header-implementation-guidelines.md`
- API docs: https://newyaroslav.github.io/Consolix/
