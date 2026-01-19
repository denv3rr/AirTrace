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

## Performance Requirements (PERF)
- REQ-PERF-001: The core update loop shall process a single state update in deterministic time for a fixed input.
- REQ-PERF-002: The system shall allow configuration of sensor update rates in Hertz.

## Safety Requirements (SAFE)
- REQ-SAFE-001: The system shall define safe-state behavior for sensor dropout, invalid configs, and mode failures.
- REQ-SAFE-002: The system shall log safety-relevant events with timestamps and mode context.
- REQ-SAFE-003: The system shall prevent mode thrashing using guard conditions and minimum dwell times.
- REQ-SAFE-004: The system shall enforce limits on turn rate, speed, and acceleration per config.
- REQ-SAFE-005: The system shall enter hold mode when authorization policy or required datasets are missing or invalid.

## Security Requirements (SEC)
- REQ-SEC-001: All external inputs shall be validated and sanitized before use.
- REQ-SEC-002: The system shall require explicit authorization to load plugins or device drivers.
- REQ-SEC-003: Plugins shall be versioned, signed, and validated before activation.
- REQ-SEC-004: The system shall provide an audit log of configuration and mode changes.
- REQ-SEC-005: Network-aid usage shall be deny-by-default unless explicitly permitted by configuration.
- REQ-SEC-006: Overrides of deny-by-default policies shall require credential or key-based confirmation and be logged.
- REQ-SEC-007: Authorization policies shall enforce role-based access with least-privilege defaults.

## Interface Requirements (INT)
- REQ-INT-001: All public interfaces shall declare units, valid ranges, and error behavior.
- REQ-INT-002: The UI shall display current mode, sensor health, and safety status.
- REQ-INT-003: The UI shall provide explicit error prompts and recovery guidance.
- REQ-INT-004: The UI shall display the active navigation source and its authorization status.
- REQ-INT-005: The UI shall use deterministic inputs for test and simulation modes using the configured seed.

## Configuration Requirements (CFG)
- REQ-CFG-001: Configuration files shall be versioned and schema-validated.
- REQ-CFG-002: Configuration validation shall enforce inter-field constraints (min <= max, dt > 0).
- REQ-CFG-003: Default values shall be documented and justified in the config schema.
- REQ-CFG-004: Configuration shall define platform profile, permitted sensors, and network-aid policy.
- REQ-CFG-005: Configuration shall define roles, override rules, and dataset tier constraints.
- REQ-CFG-006: Configuration shall define parameters for vision, lidar, magnetometer, barometer, and celestial sensors.

## Verification Requirements (VER)
- REQ-VER-001: Every requirement shall be mapped to at least one verification method (test, analysis, inspection).
- REQ-VER-002: Core algorithms shall have unit tests covering boundary conditions.

## Documentation Requirements (DOC)
- REQ-DOC-001: The README shall include submodule-aware clone/pull instructions and non-duplicative build/run guidance, with a quick-start summary that may repeat the core steps.
