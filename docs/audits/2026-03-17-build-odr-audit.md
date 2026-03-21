# Consolix Audit: ODR, Build, and Header Usage

Date: 2026-03-17
Environment: Windows, MinGW GCC 15.2.0, CMake 4.2.3
Scope: `include/consolix`, root `CMakeLists.txt`, `examples/`, `tests/`

## Contract Used For This Audit

- Primary usage model is through aggregated entry points:
  `consolix.hpp`, `core.hpp`, `components.hpp`, `utils.hpp`.
- Direct standalone-header checks were limited to headers that are explicitly used that way in docs or tests:
  `utils/json_utils.hpp`, `utils/path_utils.hpp`, `utils/enums.hpp`, `utils/types.hpp`.
- `libs/*` were treated as external dependencies and were not audited for their own internal defects.

## What Was Reproduced

- Root `CMakeLists.txt` configures and builds the example executables successfully on Windows/MinGW with the current default `C++17` setup.
- The main entry points compile with `C++17` when optional features are disabled.
- The main entry points do not compile as `C++11` or `C++14`.
- `components.hpp` does not compile as a standalone advertised entry point in the current Windows/MinGW setup.
- Some `C++17` feature-macro combinations fail even though the library is documented as macro-configurable.
- Representative two-translation-unit ODR checks fail for public headers that define non-`inline` free functions in headers.
- Several headers documented or used as standalone are not actually standalone on Windows.

## Confirmed Findings

### 1. ODR failures in the current header-only layout

Representative failures were reproduced with multi-TU link checks.

- `include/consolix/utils/path_utils.hpp`
  defines non-`inline` free functions in a public header:
  `get_exec_path`, `get_exec_dir`, `get_file_name`, `resolve_exec_path`, `create_directories`.
- `include/consolix/utils/system_utils.hpp`
  defines multiple non-`inline` free functions in a public header.
- `include/consolix/utils/encoding_utils.hpp`
  defines Windows conversion helpers as non-`inline` free functions in a public header.
- `include/consolix/core/application_utils.hpp`
  defines `consolix::init()` as a non-`inline` free function in a public header.
- `include/consolix/utils/json_utils.hpp`
  defines `strip_json_comments()` as a non-`inline` free function in a public header.

Impact:
Including these headers through normal aggregated entry points in multiple translation units breaks the `header-only` ODR contract.

### 2. Current codebase is `C++17`, not `C++11/14/17`

`C++11` and `C++14` entry-point checks fail for structural reasons, not for isolated warning-level issues.

- `include/consolix/utils/path_utils.hpp`
  uses `std::filesystem`.
- `include/consolix/core/ServiceLocator.hpp`
  uses `std::shared_mutex`.
- `include/consolix/utils/encoding_utils.hpp`
  relies on `std::string::data()` mutability that is only valid in `C++17` for the current code path.

Impact:
The current implementation and current root CMake configuration match a `C++17` baseline. Supporting `C++11/14` would require real code changes, not only documentation cleanup.

### 3. Intended standalone headers are inconsistent

The audit treated only documented/tested direct-use headers as standalone candidates. Several of them currently fail that contract.

- `docs/path_utils.dox` documents direct inclusion of `consolix/utils/path_utils.hpp`, but on Windows that header is not self-sufficient because it depends on platform types/functions supplied elsewhere.
- `tests/test_strip_json_comments.cpp`
  directly includes `consolix/utils/path_utils.hpp`, reinforcing that standalone usage is intended.
- `docs/common_types.dox`
  documents direct inclusion of `consolix/utils/enums.hpp`, but on Windows that header exposes `WORD` and console color macros without directly including the required Windows declarations.
- `docs/path_utils.dox`
  uses `resolve_executable_path`, but the implementation exposes `resolve_exec_path`.

Impact:
The public contract for “directly includable utility header” is currently unclear and partly broken.

### 4. `components.hpp` is currently a broken public entry point

`components.hpp` is documented and presented as a public entry point, but it does not currently compile on its own in the audited environment.

Confirmed causes include:

- `include/consolix/components/TitleComponent.hpp`
  uses `get_exec_path()` and `get_file_name()` without receiving them through the include chain used by `components.hpp`.
- `include/consolix/components/LoggerComponent/MultiStream.hpp`
  uses `std::cout` without directly including `<iostream>`.
- `include/consolix/components.hpp`
  includes component headers before making `TextColor` and related color helpers available to all users that need them.

Impact:
One of the advertised module-level entry points is not currently safe to consume directly, even under `C++17`.

### 5. Macro configurability is partially broken

The repository builds examples with all optional features enabled, but some supported macro combinations fail at compile time.

- `core.hpp` and `consolix.hpp` fail in the `JSON only` scenario because `ConfigComponent` unconditionally references `CliOptions` and `CliArguments` even when `CONSOLIX_USE_CXXOPTS == 0`.
- `core.hpp` and `consolix.hpp` fail in the `LOGIT only` scenario because `LoggerComponent` unconditionally references `CliOptions` and `CliArguments` even when `CONSOLIX_USE_CXXOPTS == 0`.
- `components.hpp` fails across all tested macro combinations, so the examples-only build hides this issue.

Impact:
The documented macro-based dependency toggling is less reliable than the current public API suggests.

### 6. POSIX branch issues exist in code, even without Linux runtime validation

- `include/consolix/core/ConsoleApplication.hpp`
  POSIX signal handling calls `cleanup(exit_code)` from a static function even though `cleanup` is a non-static member.

Impact:
The Windows build passing does not validate the non-Windows lifecycle path.

### 7. Build-layer does not currently model the library itself

- Root `CMakeLists.txt`
  defines `project(ConsolixExamples ...)`.
- It builds only example executables.
- It hard-codes `CMAKE_CXX_STANDARD 17`.
- It does not export a library target for the header-only package.
- It does not define any optional static-library target.

Impact:
The current build setup is an examples-only build, not a packaging/build contract for the library.

### 8. Additional code-quality and behavioral risks

- `include/consolix/components/ConfigComponent.hpp`
  `reload()` calls `load_config()`, which registers the same service type again and can throw on repeated reload.
- `include/consolix/core/ServiceLocator.hpp`
  `register_service(std::function<...>)` unlocks before object creation and does not re-check uniqueness after re-locking, which creates a race window.
- `docs/setup.dox`
  documents dependency macros as defaulting to `1`, but `include/consolix/config_macros.hpp` defaults them to `0`.

## Overall Assessment

- The repository behaves like a `C++17` header-only library with working example builds on Windows/MinGW.
- The examples build passes because it uses the fully enabled path and does not exercise `components.hpp` as an isolated entry point or macro-reduced configurations.
- In its current form it is not ODR-safe as a general header-only library.
- The documented standalone-header surface is broader than what the implementation actually supports.
- One advertised module entry point (`components.hpp`) and some advertised macro combinations are currently broken.
- The repository does not currently provide a real package-level build description for the library itself.

## Recommended Follow-Up Order

1. Fix ODR-breaking free-function definitions in public headers.
2. Repair the public entry-point surface:
   make `components.hpp` compile as documented and make macro-reduced configurations honest.
3. Decide and document the actual standard baseline:
   keep `C++17`, or invest in restoring `C++11/14`.
4. Define the intended standalone-header surface and make docs/tests match it.
5. Add a real CMake package target for the library.
6. Repair POSIX-only code paths and add at least compile-level coverage for them.
