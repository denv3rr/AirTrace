# AirTrace

Deterministic tracking and platform-interface validation tool with a fail-closed safety posture.

AirTrace is built for teams that need one operator surface to test platform behavior, adapter compatibility, mode decisions, and integration outputs across multiple profiles (air, ground, maritime, space, handheld, fixed-site, subsea).

## Who This Is For
- Engineers validating sensor/mode logic under deterministic conditions.
- Operators/test teams running repeatable platform workbench checks.
- Integrators consuming a stable machine-readable output envelope.

If you have never coded before: start with **Quick Start** below, then use the **Main Menu** in the app. You do not need to modify source code to run baseline checks.

## Project Stack
- Language: C++17
- Build system: CMake (3.10+)
- Runtime UI: terminal UI (TUI)
- Modules:
  - `airtrace_core`: deterministic algorithms and mode logic
  - `airtrace_tools`: config parsing, policy gates, audit logging, adapter registry loading
  - `AirTrace`: operator-facing UI app

## Core Capabilities
- Deterministic simulation and test execution (seeded RNG + fixed timestep).
- Mode and fallback ladder evaluation with explicit deny reasons.
- Platform workbench: single session validation for selected/all platform profiles.
- Front-view display workbench scaffolding:
  - EO/IR/proximity display families
  - deterministic spoofed input path for test environments without physical sensors
  - single-mode and cycle-all execution
  - front-view telemetry exported in the external I/O envelope
- Adapter manifest/allowlist validation.
- Versioned external I/O envelope for cross-system integration.

## Safety and Security Model
- Fail closed by default on invalid/missing data.
- External inputs are validated and schema-checked.
- Authorization/provenance/policy denials are surfaced with recovery guidance.
- Audit logging is required for operationally relevant actions.

## Quick Start
1. Clone with submodules:
   - `git clone --recurse-submodules https://github.com/denv3rr/AirTrace`
2. Build:
   - Windows: `.\scripts\build.ps1`
   - Linux/macOS: `./scripts/build.sh`
3. Run:
   - Windows: `.\build\AirTrace.exe`
   - Linux/macOS: `./build/AirTrace`
4. Run tests:
   - Windows: `.\scripts\test.ps1`
   - Linux/macOS: `./scripts/test.sh`

## Menu Guide (Operator View)
- `Scenario Mode`: run operational-style flow checks.
- `Test Mode`: run deterministic mode-specific tests.
- `Platform Workbench`: validate one/all platform profiles and adapter status from one UI.
- `Front-View Display Workbench`: validate EO/IR/proximity display modes with spoofed data and export front-view telemetry.

## Where To Go In The Repo
- Core logic: `src/core/`, headers in `include/core/`
- Tools/parsing/audit: `src/tools/`, headers in `include/tools/`
- UI/TUI flows: `src/ui/`, headers in `include/ui/`
- Tests: `tests/`
- Design and compliance docs: `docs/`

## Key Documentation
- Requirements: `docs/requirements.md`
- Verification plan: `docs/verification_plan.md`
- Traceability matrix: `docs/traceability.md`
- Hazard log: `docs/hazard_log.md`
- Security threat model: `docs/security_threat_model.md`
- Configuration schema: `docs/config_schema.md`
- Operational concepts: `docs/operational_concepts.md`
- UI standards: `docs/ui_standards.md`
- Front-view architecture: `docs/front_view_display_architecture.md`
- Plan and sequencing: `docs/plan.md`

## Configuration Notes
- Main config keys are documented in `docs/config_schema.md`.
- Front-view scaffolding keys use the `front_view.*` namespace.
- Unknown keys or invalid values are rejected.

## Test Harness (Non-Interactive)
- Enable harness:
  - `AIRTRACE_TEST_HARNESS=1`
  - `AIRTRACE_HARNESS_COMMANDS=configs/harness_commands.txt`
- Example command format: `docs/harness_commands.example.txt`

## External Sources and Standards
- Standards and references list: `sources_menu.md`
- Public project site: https://seperet.com
