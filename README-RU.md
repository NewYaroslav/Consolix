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

Она закрывает типовые задачи, которые постоянно встречаются в утилитах и консольных сервисах: запуск и завершение приложения, реакцию на termination signals, сборку приложения из компонентов, общие сервисы, вывод логотипа и title, парсинг аргументов, загрузку конфигурации и набор прикладных helper'ов. Идея в том, чтобы быстрее собирать, расширять и поддерживать консольные приложения без лишней инфраструктуры.

## Возможности

- помогают собирать консольное приложение из переиспользуемых компонентов и shared services, а не держать всё в одном `main.cpp`
- дают lifecycle для приложения, execution loops и controlled shutdown, включая обработку запросов на остановку от termination signals
- закрывают прикладные задачи вроде console title, ASCII logo и логирования без повторения boilerplate-кода
- дают готовые компоненты для CLI parsing и JSON-конфигурации
- включают переиспользуемые helper'ы для путей, цветов, кодировок и других типовых консольных задач
- остаются лёгкими в подключении: `header-only`, совместимы с `C++11`/`14`/`17`, а внешние интеграции подключаются только по необходимости

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

## Зависимости

Репозиторий хранит несколько внешних библиотек как submodules в `libs/`.

- [LogIt](https://github.com/NewYaroslav/log-it-cpp) используется для `LoggerComponent` и LogIt-based пути логирования при `CONSOLIX_USE_LOGIT=1`
- [time-shield-cpp](https://github.com/NewYaroslav/time-shield-cpp) — vendored dependency, которая используется вместе с интеграцией `LogIt`; обычно пользователь Consolix напрямую с ней не работает
- [cxxopts](https://github.com/jarro2783/cxxopts) включает `CliComponent` и связанные CLI aliases при `CONSOLIX_USE_CXXOPTS=1`
- [nlohmann/json](https://github.com/nlohmann/json) включает `ConfigComponent` и JSON-based загрузку конфигурации при `CONSOLIX_USE_JSON=1`

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

Consolix поставляется как `header-only` библиотека и экспортирует официальный `INTERFACE` target `Consolix::Consolix` для нормального CMake-подключения.

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
