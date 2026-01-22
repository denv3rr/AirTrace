# AirTrace Requirements

All requirements use "shall" language per MIL-STD-961E. Each requirement must be testable and traceable.

## System Requirements (SYS)
- REQ-SYS-001: The system shall provide a deterministic simulation mode with a user-specified seed and fixed timestep.
- REQ-SYS-002: The system shall separate core tracking logic from UI/IO layers with no direct IO in core.
- REQ-SYS-003: The system shall support multiple tracking modes selectable via a controlled state machine.
- REQ-SYS-004: The system shall support multiple sensor types with explicit capability declarations.
- REQ-SYS-005: The system shall fail closed and enter a defined safe state on invalid inputs.
- REQ-SYS-006: The system shall record build metadata and configuration identifiers for each run.
- REQ-SYS-007: The system shall define platform profiles (air, ground, maritime, space, handheld, fixed_site, subsea) that inherit from a common base and override capabilities.
- REQ-SYS-008: The system shall maintain a navigation fallback ladder that selects the highest-permitted source based on health, confidence, and policy.
- REQ-SYS-009: The system shall support dataset tiers to limit onboard data volume by platform profile and role.
- REQ-SYS-010: The system shall support multi-modal switching with fused modes that combine primary and aiding sensors while preserving source lineage.
- REQ-SYS-011: The system shall support concurrent execution of compatible sensor pipelines without interrupting ongoing modes.
- REQ-SYS-012: Build, run, and test scripts shall validate build type inputs and fail closed on invalid values.
- REQ-SYS-013: The system shall support platform profile inheritance with deterministic merge rules for permitted sensors and policy defaults.

## Functional Requirements (FUNC)
- REQ-FUNC-001: The core shall ingest sensor measurements using a common state and measurement contract.
- REQ-FUNC-002: The mode manager shall select modes using health status, confidence, and hysteresis windows.
- REQ-FUNC-003: The system shall reject unknown configuration keys and schema mismatches.
- REQ-FUNC-004: The system shall provide deterministic replay of simulations using logged seeds and configs.
- REQ-FUNC-005: The system shall expose projection outputs for XY, XZ, and YZ planes.
- REQ-FUNC-006: The system shall enforce environmental bounds for position, velocity, and acceleration.
- REQ-FUNC-007: The system shall provide a safe "hold" mode when all sensors are degraded.
- REQ-FUNC-008: Celestial navigation shall only activate when higher-priority sources are unavailable or disallowed.
- REQ-FUNC-009: The system shall accept celestial datasets as versioned artifacts with integrity verification.
- REQ-FUNC-010: The system shall select the minimal dataset tier required for the active platform profile unless explicitly overridden by policy.
- REQ-FUNC-011: The system shall provide fallback sensor modules for vision, lidar, magnetometer, barometer, and celestial sources with explicit measurement types and validity flags.
- REQ-FUNC-012: The system shall compute source eligibility using health, data freshness, confidence, and policy authorization.
- REQ-FUNC-013: The system shall provide fused navigation modes for GNSS+INS, vision/lidar-aided inertial, radar/terrain-aided inertial, and magnetometer+baro stabilization when permitted.
- REQ-FUNC-014: The system shall publish active mode contributors with weights or confidence for operator visibility and audit.
- REQ-FUNC-015: The fallback ladder ordering shall be configurable per platform profile and mission policy.
- REQ-FUNC-016: The system shall schedule concurrent sensor pipelines with deterministic arbitration and bounded resource usage.
- REQ-FUNC-017: The system shall preserve per-sensor scan continuity when auxiliary snapshot sensors run concurrently.
- REQ-FUNC-018: The system shall evaluate historical sensor context (recent health, confidence, and disagreement trends) when selecting modes.
- REQ-FUNC-019: The system shall enforce strict consistency checks for fused modes using cross-sensor residual thresholds.
- REQ-FUNC-020: The system shall apply lockout rules for sensors that repeatedly violate freshness or confidence thresholds.
- REQ-FUNC-021: The system shall require time-aligned measurements within a configured window for cross-sensor residual checks.

## Performance Requirements (PERF)
- REQ-PERF-001: The core update loop shall process a single state update in deterministic time for a fixed input.
- REQ-PERF-002: The system shall allow configuration of sensor update rates in Hertz.

## Safety Requirements (SAFE)
- REQ-SAFE-001: The system shall define safe-state behavior for sensor dropout, invalid configs, and mode failures.
- REQ-SAFE-002: The system shall log safety-relevant events with timestamps and mode context.
- REQ-SAFE-003: The system shall prevent mode thrashing using guard conditions and minimum dwell times.
- REQ-SAFE-004: The system shall enforce limits on turn rate, speed, and acceleration per config.
- REQ-SAFE-005: The system shall enter hold mode when authorization policy or required datasets are missing or invalid.
- REQ-SAFE-006: The system shall detect multi-sensor disagreement beyond configured thresholds and degrade to a safer mode or hold.
- REQ-SAFE-007: The system shall prevent concurrency-induced resource starvation or timing jitter from violating safe-state constraints.
- REQ-SAFE-008: The system shall enter hold or safe degraded modes when timing jitter or stale data exceeds configured limits.
- REQ-SAFE-009: The system shall prevent re-entry into a mode when contributors remain in lockout.

## Security Requirements (SEC)
- REQ-SEC-001: All external inputs shall be validated and sanitized before use.
- REQ-SEC-002: The system shall require explicit authorization to load plugins or device drivers.
- REQ-SEC-003: Plugins shall be versioned, signed, and validated before activation.
- REQ-SEC-004: The system shall provide an audit log of configuration and mode changes.
- REQ-SEC-005: Network-aid usage shall be deny-by-default unless explicitly permitted by configuration.
- REQ-SEC-006: Overrides of deny-by-default policies shall require credential or key-based confirmation and be logged.
- REQ-SEC-007: Authorization policies shall enforce role-based access with least-privilege defaults.
- REQ-SEC-008: Multi-sensor fusion shall reject untrusted or unauthorized sources from contributing to fused modes.

## Interface Requirements (INT)
- REQ-INT-001: All public interfaces shall declare units, valid ranges, and error behavior.
- REQ-INT-002: The UI shall display current mode, sensor health, and safety status.
- REQ-INT-003: The UI shall provide explicit error prompts and recovery guidance.
- REQ-INT-004: The UI shall display the active navigation source and its authorization status.
- REQ-INT-005: The UI shall use deterministic inputs for test and simulation modes using the configured seed.
- REQ-INT-006: The UI shall fail closed on invalid or unavailable menu selections, log the error, and exit.
- REQ-INT-007: The UI shall display denial warnings with recovery guidance when safe-state or input failures occur.
- REQ-INT-008: The test harness shall provide a non-interactive input source for UI flows without altering production input behavior.
- REQ-INT-009: When the test harness is enabled, UI menu flows shall be permitted without a TTY while preserving production input checks.

## Configuration Requirements (CFG)
- REQ-CFG-001: Configuration files shall be versioned and schema-validated.
- REQ-CFG-002: Configuration validation shall enforce inter-field constraints (min <= max, dt > 0).
- REQ-CFG-003: Default values shall be documented and justified in the config schema.
- REQ-CFG-004: Configuration shall define platform profile, permitted sensors, and network-aid policy.
- REQ-CFG-005: Configuration shall define roles, override rules, and dataset tier constraints.
- REQ-CFG-006: Configuration shall define parameters for vision, lidar, magnetometer, barometer, and celestial sensors.
- REQ-CFG-007: Configuration shall define platform.profile_parent and platform.child_modules with validation and deterministic defaults.

## Verification Requirements (VER)
- REQ-VER-001: Every requirement shall be mapped to at least one verification method (test, analysis, inspection).
- REQ-VER-002: Core algorithms shall have unit tests covering boundary conditions.
- REQ-VER-003: Test scripts shall build all registered test targets before running them.
- REQ-VER-004: Test harness inputs shall be authenticated by build-time gating and runtime enablement before use.
- REQ-VER-005: Test scripts shall normalize test summary labels to avoid redundant qualifiers (e.g., omit "real").
- REQ-VER-006: Integration tests shall cover dropout ladder selection, dataset gating, and UI menu flows.

## Documentation Requirements (DOC)
- REQ-DOC-001: The README shall include submodule-aware clone/pull instructions and non-duplicative build/run guidance, with a quick-start summary that may repeat the core steps.
