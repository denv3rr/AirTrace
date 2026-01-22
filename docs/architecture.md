# Architecture Overview

AirTrace is split into a reusable core library and a thin CLI/UI layer.

## Modules

- `core` (library):
  - Tracking algorithms, models, and shared utilities.
  - No user interaction or terminal I/O.
  - Exported via the `airtrace_core` CMake target.

- `ui` (application):
  - Console menus, TUI input handling, and simulation presentation.
  - Depends on `airtrace_core` for all algorithmic work.

- `examples`:
  - Small, deterministic demos that exercise core behaviors.

- `docs`:
  - Project conventions and data source notes.

- `configs`:
  - Simulation configuration files for motion, sensors, and bounds.
- `scripts`:
  - Build and test entry points with input validation and fail-closed behavior.

## Boundaries

- Core code must not depend on UI headers.
- UI code can depend on core and should keep data handling separate from rendering.

## Configuration and Profiles
- Config parsing lives in core and enforces profile inheritance rules.
- UI reads the resolved profile/parent/modules for operator visibility.
