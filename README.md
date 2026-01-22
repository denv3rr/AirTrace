# AirTrace

## Overview
AirTrace is a deterministic tracking and navigation simulation core with a TUI for
running scenarios and evaluating sensor fallback behavior. It emphasizes safe-state
handling, explicit policy gating, and traceable requirements.

Key capabilities:
- Deterministic simulation with seeded RNG and fixed timesteps.
- Mode management with fallback ladder logic and safe-state behavior.
- Sensor modeling for GNSS, IMU, radar, thermal, vision, lidar, magnetometer, baro, and celestial.
- Platform profiles with policy-controlled sensor permissions and dataset tiers.

## Safety and Fail-Closed Behavior
- Invalid configs or missing datasets force hold mode.
- Mode transitions enforce guard conditions and dwell times.
- Degraded or conflicting inputs downgrade to safer sources.

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

TUI controls:
- Up/Down to move, Space/Enter to select, Esc to go back/exit.

## Configuration Notes
- Configs are schema-validated and versioned; unknown keys are errors.
- Policy defaults to deny network aids unless explicitly allowed.
- Dataset tiers constrain celestial data size by platform profile.
- Mode selection and scheduling use `mode.*`, `fusion.*`, and `scheduler.*` settings.
- Historical lockout behavior uses `mode.max_stale_count`, `mode.max_low_confidence_count`, and `mode.lockout_steps`.
- Cross-sensor residual checks use `fusion.disagreement_threshold` and `fusion.max_disagreement_count`.
- Trend evaluation windows use `mode.history_window`.
- Residual alignment windows use `fusion.max_residual_age_seconds`.

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
- Sources: `sources_menu.md`

## Status
The platform is under active remediation to meet the full standards baseline.
See `docs/plan.md` for sequencing, review gates, and multi-modal switching work.
