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

Consolix — это `header-only` библиотека C++ для структурированных консольных приложений с компонентной моделью, service locator и набором прикладных утилит.

## Обзор

Consolix рассчитан на консольные приложения, которым уже тесно в одном `main.cpp`, но при этом не нужен тяжёлый runtime-framework.

Библиотека даёт компонентную сборку приложения, общие сервисы, утилиты для путей и JSON, а также опциональные интеграции для логирования, CLI-парсинга и конфигурации. Идея в том, чтобы быстрее собирать, расширять и поддерживать консольные приложения без лишней инфраструктуры.

## Возможности

- поставляется как `header-only`
- в CMake экспортируется как `Consolix::Consolix` через `INTERFACE` target
- компонентная архитектура для логирования, конфигурации, заголовка окна, логотипа и execution loop
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

## Быстрый старт

Для большинства проектов самый короткий путь такой:

1. Подключить Consolix к проекту и сделать `include/` доступным для компилятора.
2. Включить только те feature macros, которые действительно нужны.
3. Подключить агрегирующий entry header, обычно:

```cpp
#include <consolix/consolix.hpp>
```

Если используется CMake, предпочтительный consumer path — `Consolix::Consolix`. Более точные правила по include-модели, feature flags и build setup приведены ниже.

## Include Model

Consolix спроектирован в aggregate-first стиле.

Для обычного использования предпочитайте:

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

## Feature Macros

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

## Documentation

- developer guidelines: `docs/header-implementation-guidelines.md`
- agent playbook: `agents/header-implementation-guidelines.md`
- lifecycle example: `examples/example_shutdown_and_resources.cpp`
- API docs: https://newyaroslav.github.io/Consolix/
