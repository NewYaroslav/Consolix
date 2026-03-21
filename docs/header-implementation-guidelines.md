# Header / Implementation Ownership Guidelines

This document defines a reusable policy for structuring `.hpp`, `.ipp`, and `.tpp` files in C++ projects.

The guidance is intentionally generic so it can be copied into other repositories with minimal changes.

## Canonical Language

English is the canonical version of this document.

Russian localization:
- `docs/header-implementation-guidelines-RU.md`

## Purpose

Use this guidance when deciding:

- what belongs in `.hpp`, `.tpp`, and `.ipp`
- whether a subsystem should expose a single aggregate entry header
- where shared STL and domain includes should live
- when forward declarations help and when they only add noise
- how to keep implementation-only helpers local to the real implementation boundary

## File Roles

### `.hpp`

Use `.hpp` for:

- public declarations
- public type aliases and configuration types
- ownership of the public include contract
- inclusion of template-visible implementation when needed

A header should make it clear whether it is:

- a standalone leaf header
- or part of an aggregate-first subsystem

### `.tpp`

Use `.tpp` for template-visible implementation.

Rules:

- template definitions must be visible wherever the template may be instantiated
- place such code either directly in `.hpp` or in a `.tpp` that is always included from `.hpp`
- do not hide consumer-instantiated templates behind implementation-only include paths

### `.ipp`

Use `.ipp` for non-template implementation that should stay close to the declaration.

Common uses:

- shared implementation between header-only and compiled-library modes
- inline-capable helper bodies
- out-of-class member definitions that conceptually belong with the header

## Ownership Model

### Always-included implementation

Use always-included implementation only when the code is naturally header-owned:

- templates
- tiny leaf helpers
- small inline methods
- logic that is intentionally consumer-visible in every translation unit

This model is a good fit when a file is expected to behave like a true standalone leaf header.

### Ownership-controlled implementation

Use explicit ownership when implementation should not be distributed to every consumer TU:

- large non-template methods
- code that should be emitted once by a compiled ownership translation unit
- implementation shared between header-only and static-library modes

In this model:

- the public header declares the interface
- the implementation file remains nearby
- the compiled ownership TU is responsible for single-TU emission in non-header-only builds

## Aggregate Entry Header vs Leaf Header

Choose one primary include strategy per subsystem.

### Aggregate-first subsystem

Use an aggregate entry header when:

- the subsystem is usually consumed as a whole
- several leaf headers naturally depend on a shared include context
- centralizing common dependencies improves structure and readability

In this model:

- normal consumers should include the aggregate entry header
- shared STL and domain includes may be centralized there
- leaf headers do not need to be artificially optimized into standalone public entrypoints

### Standalone-leaf-first subsystem

Use standalone leaf headers when:

- individual headers are expected to be consumed independently
- leaf types are reused across unrelated parts of the codebase
- a broad aggregate header would create unnecessary coupling

In this model:

- each leaf header should own the dependencies needed for its public contract
- always-included implementation is more acceptable when the leaf is intentionally self-contained

## Dependency Placement

### Shared STL and domain includes

Centralize shared includes only when the subsystem intentionally uses a canonical aggregate entry header.

That approach is appropriate when:

- multiple leaf headers depend on the same small set of STL headers
- the aggregate header is already the intended public entrypoint
- centralization reduces duplication without obscuring ownership

Do not centralize dependencies merely to save a few include lines if the subsystem is designed around standalone leaf headers.

### Implementation-only helpers

Build-mode helpers, implementation macros, and similar infrastructure should stay at the implementation boundary.

Preferred placement:

- in the `.ipp` that directly uses them
- or in the direct owning `.hpp` if that is the narrowest correct boundary

Avoid spreading such helpers through broad aggregate headers unless every valid usage path truly depends on that aggregate.

## Forward Declarations

Use forward declarations when they provide a real structural benefit:

- reducing unavoidable coupling
- breaking cycles cleanly
- keeping stable leaf interfaces lightweight

Avoid them when they mostly:

- make dependencies less explicit
- increase include-order sensitivity
- add visual noise without meaningful architectural benefit

If a subsystem already has a canonical aggregate entry header, prefer a clear and flat dependency structure over mechanically maximizing forward declarations.

## Private / Vendor Boundaries

Application code should not include private implementation files from dependencies in order to bypass public API boundaries.

If a needed capability is not available through public headers:

- extend the dependency properly
- or keep the missing ownership local to the application

Do not patch around the boundary by including vendor `.ipp` or other private implementation details from application code.

## Test Contract Guidance

Tests should validate the include contract that the project actually intends to support.

For aggregate-first subsystems:

- add public-facing tests through the aggregate entry header
- do not accidentally lock the project into a stronger standalone-leaf contract unless that contract is intentional

For white-box tests:

- direct inclusion of leaf headers is acceptable
- such tests should declare their own prerequisites explicitly
- internal tests must not silently redefine the public include policy for the subsystem

## Practical Checklist

Before finalizing a new header layout, verify:

- template-visible code is available from headers
- `.ipp` ownership is explicit and consistent
- the subsystem clearly chooses aggregate-first or standalone-leaf-first behavior
- shared dependencies are centralized only when that matches the chosen model
- implementation-only helpers stay near the real implementation boundary
- forward declarations are used for structural reasons, not as reflex
- tests validate the intended include contract rather than an accidental one
