# AirTrace

[Overview](#overview) | [Safety](#safety-and-fail-closed-behavior) | [Quick Start](#quick-start) | [Build and Run](#build-and-run) | [Config](#configuration-notes) | [Documentation](#documentation) | [Status](#status) | [seperet.com](https://seperet.com)

## Overview
AirTrace is a deterministic tracking and navigation tool with a TUI for
operational workflows. Simulation and test modes are used strictly for
verification and training and are explicitly gated. Supported platforms:
Windows, Linux, and macOS. Build tools: C++17, CMake 3.10+, Ninja optional.

Core behavior:
- Deterministic simulation with seeded RNG and fixed timesteps.
- Mode management with a fallback ladder and safe-state behavior.
- Sensor modeling for GNSS, IMU, radar, thermal, vision, lidar, magnetometer, baro, and celestial.
- Platform profiles with controlled sensor permissions and dataset tiers.
- Audit logging for mode/config changes with integrity chaining, health status reporting, and run/config identifiers.

## Safety and Fail-Closed Behavior
- Invalid configs or missing datasets force hold mode.
- Mode transitions enforce guard conditions and dwell times.
- Degraded or conflicting inputs downgrade to safer sources.
- Audit logging is required for operational runs; failure to initialize audit logging forces a safe exit.

## Quick Start
- Clone with submodules: `git clone --recurse-submodules https://github.com/denv3rr/AirTrace`
- Build: `./scripts/build.sh` (macOS/Linux) or `.\scripts\build.ps1` (Windows)
- Run TUI: `./build/AirTrace` (Windows: `.\build\AirTrace.exe`)

## Build and Run
Clone and update:
- `git clone --recurse-submodules https://github.com/denv3rr/AirTrace`
- `git submodule update --init --recursive`
- Pull updates: `git pull --recurse-submodules`

Build:
- `./scripts/build.sh` (macOS/Linux)
- `.\scripts\build.ps1` (Windows)

Run:
- Main TUI: `./build/AirTrace` (Windows: `.\build\AirTrace.exe`)
- CLI demo: `./build/AirTraceExample` (Windows: `.\build\AirTraceExample.exe`)
- 3D sim demo: `./build/AirTraceSimExample configs/sim_default.cfg`

Tests:
- `./scripts/test.sh` (macOS/Linux)
- `.\scripts\test.ps1` (Windows)

Test harness (non-interactive, test-only):
- Build runner: `.\build\AirTraceHarnessRunner.exe` (Windows) or `./build/AirTraceHarnessRunner` (macOS/Linux)
- Enable: `AIRTRACE_TEST_HARNESS=1`
- Command file: `AIRTRACE_HARNESS_COMMANDS=configs/harness_commands.txt`
- Example commands: `docs/harness_commands.example.txt`
- Audit logs: `audit_log.jsonl` (operational) and `audit_log_test.jsonl` (test harness) are generated locally and gitignored.

TUI controls:
- Up/Down to move, Space/Enter to select, Esc to go back/exit.
- During active runs, type `x` then Enter to abort safely.

## Configuration Notes
- Configs are schema-validated and versioned; unknown keys are errors.
- Policy defaults to deny network aids unless explicitly allowed.
- Dataset tiers constrain celestial data size by platform profile.
- Mode selection and scheduling use `mode.*`, `fusion.*`, and `scheduler.*` settings.
- Lockout behavior uses `mode.max_stale_count`, `mode.max_low_confidence_count`, and `mode.lockout_steps`.
- Residual checks use `fusion.disagreement_threshold`, `fusion.max_disagreement_count`, and `fusion.max_residual_age_seconds`.
- Trend evaluation windows use `mode.history_window`.
- Role permissions gate test modes, history viewing, and destructive actions.
- Provenance policy keys configure allowed inputs and mixed-input handling (see `docs/config_schema.md`).

## Documentation
- Architecture: `docs/architecture.md`
- Requirements: `docs/requirements.md`
- Verification plan: `docs/verification_plan.md`
- Traceability: `docs/traceability.md`
- Hazard log: `docs/hazard_log.md`
- Security threat model: `docs/security_threat_model.md`
- Config schema: `docs/config_schema.md`
- Operational concepts: `docs/operational_concepts.md`
- Navigation fallbacks: `docs/navigation_fallbacks.md`
- Multi-modal switching design: `docs/multi_modal_switching_design.md`
- Multi-modal switching implementation plan: `docs/multi_modal_switching_implementation_plan.md`
- Edge case catalog: `docs/edge_case_catalog.md`
- UI/TUI standards: `docs/ui_standards.md`
- Test harness: `docs/test_harness.md`
- Harness command example: `docs/harness_commands.example.txt`
- Sources: `sources_menu.md`
