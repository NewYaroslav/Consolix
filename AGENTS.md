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
