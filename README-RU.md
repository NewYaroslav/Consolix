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

Репозиторий хранит несколько внешних библиотек как submodules в `external/`.

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

Если `main()` должен вернуть результат lifecycle, а не завершаться через
`std::exit` из `ConsoleApplication::run()`, используйте runner с exit code:

```cpp
int main() {
    consolix::add<MyComponent>();
    return consolix::run_for_exit_code();
}
```

Компонент может запросить конкретный код через `consolix::stop(code)`;
`consolix::request_stop(code)` остается доступен как более явная форма.
Если приложение уже владеет своим `AppComponentManager`, можно создать
`consolix::ConsoleApplicationRunner runner(manager);` и вернуть
`runner.run_for_exit_code()`.

`ConsoleApplication::run()` остается совместимым process-owning facade:
внутри он использует тот же runner и завершает процесс с полученным кодом.
На Windows Ctrl+C/Ctrl+Break запрашивают cooperative shutdown, а close/logoff/
shutdown events запрашивают shutdown на runner thread и ждут ограниченное окно
для cleanup.

### Порядок shutdown и финальные hooks

`AppComponentManager` завершает компоненты в обратном порядке регистрации. После
всех component shutdown callbacks runner очищает `ServiceLocator`, а затем
закрывает LogIt. Это значит, что services и logging еще доступны, пока
выполняются component `shutdown()` callbacks.

Если приложению нужен финальный cleanup hook после остановки обычных
компонентов, но до `ServiceLocator::clear_all()`, зарегистрируйте этот hook
раньше компонентов, которые им пользуются:

```cpp
consolix::add<consolix::LoopComponent>(
    []() { return true; },
    []() {},
    [](int exit_code) {
        // Финальный cleanup, пока services и LogIt еще живы.
    });

consolix::add<RealComponent>();
```

Так как shutdown идет в LIFO-порядке, `RealComponent` завершится первым, а hook
выполнится после него. Такой component-level паттерн лучше runner-level
shutdown hooks: cleanup остается в той же lifecycle-модели, что и остальное
приложение.

### Main loop and CPU usage

Consolix выполняет компоненты в polling loop и по умолчанию не делает sleep
между итерациями. Если все компоненты сразу возвращаются из `process()`, процесс
может тратить слишком много CPU. Для latency-sensitive приложений это
намеренно: framework не прячет implicit delay внутри runner.

Для обычных сервисов и утилит добавьте throttle component ближе к концу списка
компонентов:

```cpp
auto throttle = consolix::add<consolix::LoopThrottleComponent>(
    std::chrono::milliseconds(1));

// Другой поток может завершить ожидание раньше, когда добавил новую работу.
throttle->wake();
```

`LoopThrottleComponent` ждет не дольше настроенного delay на каждом проходе
loop. `wake()` прерывает текущее ожидание, а wake request, сделанный до начала
ожидания, будет обработан на следующем проходе.

## Documentation

- developer guidelines: `docs/header-implementation-guidelines.md`
- agent playbook: `guides/header-implementation-guidelines.md`
- lifecycle example: `examples/example_shutdown_and_resources.cpp`
- exit-code runner example: `examples/example_exit_code_runner.cpp`
- loop throttle example: `examples/example_loop_throttle_component.cpp`
- API docs: https://newyaroslav.github.io/Consolix/
