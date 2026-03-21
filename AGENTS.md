# Repository Agent Notes

Use the repository guidance in this order:

- `docs/header-implementation-guidelines.md`
- `agents/header-implementation-guidelines.md`

Additional policy:

* For reusable `.hpp` / `.ipp` / `.tpp` ownership and include-structure policy, prefer:
  * developer doc: `docs/header-implementation-guidelines.md`
  * agent playbook: `agents/header-implementation-guidelines.md`
* Preserve existing repository branding ASCII/ANSI art.
  * This includes the banner in `README.md` and `README-RU.md`, documentation source such as `docs/mainpage.dox`, and code-owned logo art such as `LogoComponent`.
  * Do not remove, replace, normalize, re-encode, simplify, or restyle these art blocks unless the user explicitly asks to change the logo itself.
  * This rule protects existing branded art only; it is not permission to add new ASCII/ANSI art where none existed before.
  * Do not edit generated documentation copies in `docs/html` or `docs/latex` manually; keep source-of-truth changes in README, `.dox`, or code.
* Temporary build/test artifacts
  * By default, store ephemeral agent-created build, test, verify, install-consumer, and scratch artifacts only in `tmp/agent-work/`.
  * Prefer task-specific subdirectories such as `tmp/agent-work/build-cxx17`, `tmp/agent-work/verify-json`, or `tmp/agent-work/install-consumer-cxx11`.
  * Reuse or clean subdirectories inside `tmp/agent-work/` instead of creating `build_*`, `verify_*`, `install-*`, or similar scratch directories in the repository root.
  * Do not create temporary `.bat`, `.txt`, `.log`, `.md`, or similar scratch files in the repository root unless they are intended project files.
  * This rule applies to temporary agent-created artifacts, not to every build directory in the repository.
  * Do not interpret this rule as "all builds must always be temporary".
  * If a build directory is an intentional development build for the project, it may live in a normal project location instead of `tmp/agent-work/`.
  * Existing or intended development build directories such as `build-mingw` are not violations of this rule.
  * Do not move, delete, or redefine established development build directories unless the user explicitly asks for that change.
