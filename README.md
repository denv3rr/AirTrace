<div>
  <div>

  ![Platforms](https://img.shields.io/badge/platforms-windows%20%7C%20linux%20%7C%20macos-black)
  ![GitHub top language](https://img.shields.io/github/languages/top/denv3rr/AirTrace)
  ![C++ Standard](https://img.shields.io/badge/c%2B%2B-17+-blue)
  ![CMake Min Version Required](https://img.shields.io/badge/cmake_min_vers_req-3.10+-green)
  ![Build System](https://img.shields.io/badge/build-cmake%2Fninja-blue)
  ![GitHub Created At](https://img.shields.io/github/created-at/denv3rr/AirTrace)
  ![GitHub repo size](https://img.shields.io/github/repo-size/denv3rr/AirTrace)
  ![GitHub last commit (branch)](https://img.shields.io/github/last-commit/denv3rr/AirTrace/main)

  <br>
  
[Overview](#overview) | [Safety](#safety-and-fail-closed-behavior) | [Quick Start](#quick-start) | [Build and Run](#build-and-run) | [Config](#configuration-notes) | [Documentation](#documentation) | [Status](#status) | [seperet.com](https://seperet.com)
  
</div>

<div align="left">
<br>
<div>
  <a href="https://seperet.com">
    <img width="100" src="https://github.com/denv3rr/denv3rr/blob/main/IMG_4225.gif" />
  </a>
</div>

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white) ![Ninja](https://img.shields.io/badge/Ninja-%232C2C2C.svg?style=for-the-badge&logo=ninja&logoColor=white)

![PowerShell](https://img.shields.io/badge/PowerShell-%235391FE.svg?style=for-the-badge&logo=powershell&logoColor=white) ![Bash](https://img.shields.io/badge/Bash-%23000000.svg?style=for-the-badge&logo=gnu-bash&logoColor=white)

</div>

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
