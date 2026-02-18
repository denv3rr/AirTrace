# Architecture Overview

AirTrace is an operational tool with simulation and test harnesses used strictly for verification and training. The system is split into a reusable core library, a controlled tools layer, and a thin CLI/UI layer.

## Modules

- `core` (library):
  - Deterministic tracking algorithms, models, and shared utilities.
  - No file I/O, wall-clock access, or non-seeded randomness.
  - Exported via the `airtrace_core` CMake target.

- `tools` (controlled operations layer):
  - Config ingestion, audit logging, run provenance, and policy gating.
  - Orchestrates operational runs and isolates simulation/test tooling.
  - Owns all external I/O and environment access.

- `ui` (application):
  - Console menus, TUI input handling, and operational presentation.
  - Depends on `tools` for orchestration and `airtrace_core` for algorithms.

- `adapters` (official platform modules):
  - Platform-specific I/O bindings, data points, and UI extensions.
  - Built and deployed independently from core/tools/UI.
  - Use versioned interface contracts; no direct coupling to internal core types.
  - Third-party adapters are permitted when they pass contract, safety, and security gates.

- `examples`:
  - Small, deterministic demos that exercise core behaviors (test-only).

- `docs`:
  - Project conventions and data source notes.

- `configs`:
  - Configuration files for operational and test runs with explicit gating.
- `scripts`:
  - Build and test entry points with input validation and fail-closed behavior.

## Modularity and Independence (Priority)
- Each module must be independently usable and testable with explicit, stable interfaces.
- No hidden cross-module dependencies; all dependencies must be declared at build and interface levels.
- Core remains pure and deterministic (no I/O, wall-clock, or non-seeded randomness).
- Tools own all external I/O and policy enforcement; UI owns presentation only.
- Platform adapters are optional, modular extensions with versioned contracts and allowlisted capabilities.

## Build Targets (Current + Planned)
Current:
- `airtrace_core` (core library)
- `airtrace_adapters_contract` (adapter contract validation module)
- `airtrace_tools` (controlled I/O and policy)
- `airtrace_ui` (operator surfaces as a reusable library target)
- `airtrace_ui_harness` (deterministic harness-enabled UI module for test/integration flows)
- `airtrace_adapters_sdk` (adapter SDK skeleton target)
- `AirTrace` (operator TUI application)

Planned:
- `airtrace_adapter_<platform>` (official adapters; per platform profile)

## Boundaries

- Core code must not depend on UI or tools headers.
- Tools code can depend on core and own all I/O, timing, and audit sinks.
- UI code can depend on tools and core; data handling remains separate from rendering.
- Simulation/test flows must be explicitly gated and isolated from operational runs.

## Configuration and Profiles
- Config parsing and file I/O live in the tools layer and enforce profile inheritance rules.
- Core validation routines remain pure and deterministic.
- UI reads the resolved profile/parent/modules for operator visibility.
