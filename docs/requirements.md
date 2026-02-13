# AirTrace Requirements

All requirements use "shall" language per MIL-STD-961E. Each requirement must be testable and traceable.
Modularity and safety remain continuous priorities. The system is expected to support independently usable modules,
with explicit interfaces and no hidden cross-module dependencies or side effects.

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
- REQ-SYS-014: The system shall prioritize operational tool modes; simulation/test modes shall be explicitly identified, gated, and prohibited in operational runs unless authorized.
- REQ-SYS-015: The core shall be free of direct file I/O, wall-clock time access, and non-seeded randomness; all external effects shall route through controlled interfaces.
- REQ-SYS-016: The system shall bind a controlled audit log sink at startup and report logging health status.
- REQ-SYS-017: The system shall tag every measurement and input with provenance (operational, simulation, test, unknown) and propagate provenance through fusion outputs.
- REQ-SYS-018: Operational runs shall reject mixed or unknown provenance inputs by default and enter a defined safe state with explicit denial.
- REQ-SYS-019: The system shall evaluate mode eligibility via a deterministic, fail-closed pipeline that denies activation if any required eligibility input is missing or invalid (health, freshness, policy, provenance, dataset availability).
- REQ-SYS-020: The tools layer shall own configuration and audit logging I/O; the core shall not perform file I/O or wall-clock operations.
- REQ-SYS-021: Core tracking loops shall reject non-positive operator speed divisors, log an explicit denial, and enter an inactive safe state.

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
- REQ-FUNC-022: The system shall compute and expose per-source eligibility decisions with explicit, stable reason codes for allow/deny outcomes.
- REQ-FUNC-023: The system shall require policy and provenance authorization decisions prior to mode activation and shall deny if authorization cannot be confirmed.
- REQ-FUNC-024: Deterministic target generation utilities shall use explicit seeded RNG inputs and produce repeatable sequences for identical seeds.
- REQ-FUNC-025: Heat-signature path adjustment shall be monotonic with validated heat input and clamped to configured-safe response bounds.

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
- REQ-SAFE-010: The system shall enter a safe state or deny operations when audit logging is unavailable or fails to record safety-relevant events.
- REQ-SAFE-011: The system shall enter hold or a safe degraded mode when eligibility evaluation or authorization decisioning is unavailable, inconsistent, or stale.
- REQ-SAFE-012: Adapter health or data failures shall trigger safe-state entry and deny further mode activation until recovery.

## Security Requirements (SEC)
- REQ-SEC-001: All external inputs shall be validated and sanitized before use.
- REQ-SEC-002: The system shall require explicit authorization to load plugins or device drivers; when plugin loading is configured, `plugin.authorization_required=true` and `plugin.authorization_granted=true` are mandatory or activation is denied.
- REQ-SEC-003: Plugins shall be versioned, signed, and validated before activation, including identifier/version allowlist match and SHA-256 signature match checks.
- REQ-SEC-004: The system shall provide an audit log of configuration and mode changes.
- REQ-SEC-005: Network-aid usage shall be deny-by-default unless explicitly permitted by configuration.
- REQ-SEC-006: Overrides of deny-by-default policies shall require credential or key-based confirmation and be logged.
- REQ-SEC-007: Authorization policies shall enforce role-based access with least-privilege defaults.
- REQ-SEC-008: Multi-sensor fusion shall reject untrusted or unauthorized sources from contributing to fused modes.
- REQ-SEC-009: Audit logs shall be protected for integrity and retention per policy.
- REQ-SEC-010: Provenance decisions (accept/reject) shall be recorded with reason codes, source identifiers, run ID, config version, config ID, build ID, and seed.
- REQ-SEC-011: Authorization decisions shall be versioned, auditable, and deny-by-default when policy provenance cannot be verified.
- REQ-SEC-012: Adapters shall be signed, allowlisted, and validated before activation; failures shall be denied and logged.
- REQ-SEC-013: Adapter allowlist approvals shall include a valid non-future date and shall be rejected when older than the configured freshness window.

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
- REQ-INT-010: When the test harness is disabled, the UI shall provide a no-op harness interface and production builds shall succeed with the harness inactive.
- REQ-INT-011: The UI shall display a persistent status banner during active operations showing profile, source, authorization, contributors, and denial status.
- REQ-INT-012: Destructive actions shall require explicit confirmation and honor role/override policies.
- REQ-INT-013: Active runs shall provide an operator abort control with explicit recovery messaging.
- REQ-INT-014: Non-menu error and empty-state messages shall include explicit denial reasons and recovery guidance.
- REQ-INT-015: Operator prompts and outputs shall include explicit units and reference frames for all measurements.
- REQ-INT-016: The UI shall display current provenance status and explicit recovery guidance when provenance is denied or unknown.
- REQ-INT-017: The UI shall display the active fallback selection reason, including disqualified sources (health/authorization/freshness) and lockout state with reason codes and recovery guidance.
- REQ-INT-018: The UI/TUI shall display a per-sensor health list that includes health state, freshness age, confidence, lockout state, and last update time with explicit units.
- REQ-INT-019: The UI/TUI shall display the fallback ladder status with ordered sources, eligibility state, active selection, and disqualifier reason codes for each source.
- REQ-INT-020: The UI/TUI shall display explicit denial reasons with recovery guidance for safe-state entry, policy denial, invalid data, and audit/logging failures.
- REQ-INT-021: Scenario and test runs shall auto-cycle through visual modes aligned to the active or evaluated tracking modes, with deterministic ordering and timing.
- REQ-INT-022: The UI shall support simultaneous visualization of multiple eligible modes when configured, including per-mode contributors, confidence, and denial reasons.
- REQ-INT-023: Operator-facing UI interfaces shall be versioned and backward-compatible; updates shall preserve the interface contract and rendering semantics.
- REQ-INT-024: Platform-specific UI surfaces shall render adapter-defined data points in addition to the baseline UI contract, with explicit units, ranges, and error behavior.
- REQ-INT-025: The UI shall provide an operator-invoked platform workbench that allows selection of any supported platform profile from a single surface and execution of deterministic profile validation suites; profile cycling is optional per run.
- REQ-INT-026: Platform workbench suites shall validate profile sensor sets, adapter contract status, and mode-output visibility with explicit pass/fail reason codes.
- REQ-INT-027: The system shall emit a versioned, machine-readable external I/O envelope with platform, mode, sensor, adapter, authorization, provenance, and determinism fields for cross-system integration.
- REQ-INT-028: The UI shall provide a front-view display workbench with deterministic EO/IR/proximity display families and operator-selectable single-mode or cycle-all execution from one session.
- REQ-INT-029: Front-view rendering shall fail closed on invalid, unauthorized, or unavailable frame sources and shall provide explicit denial reasons and recovery guidance.

## Modularity Requirements (MOD)
- REQ-MOD-001: The system shall provide independently buildable modules for core, tools, UI, and adapters with explicit, versioned interfaces.
- REQ-MOD-002: The core shall operate without adapters; adapters shall be optional extensions.
- REQ-MOD-003: Modules shall declare all dependencies and shall not access undeclared modules at runtime.
- REQ-MOD-004: Module interface version mismatches shall be detected and shall fail closed with explicit reason codes.
- REQ-MOD-005: The system shall support third-party platform adapters via the versioned adapter contract; integration shall only be constrained by safety, security, and compliance gates.

## Adapter Requirements (ADP)
- REQ-ADP-001: The system shall provide official adapters for the platform profiles: air, ground, maritime, space, handheld, fixed_site, and subsea.
- REQ-ADP-002: Each adapter shall declare its capabilities and UI extension fields with units, ranges, and error behavior; unknown capabilities or fields shall be rejected.
- REQ-ADP-003: Official adapters shall provide a platform UI mapping that includes the baseline UI contract and adapter-specific extensions for each supported surface.
- REQ-ADP-004: Adapter failures or invalid data shall trigger safe-state entry with explicit denial and audit logging.
- REQ-ADP-005: Each official adapter shall provide a design note documenting environmental constraints (MIL-STD-810H) and applicable human factors/symbology guidance (MIL-STD-1472H, MIL-STD-2525 where applicable).
- REQ-ADP-006: Third-party adapters shall be allowed when they pass contract validation, safety checks, and security authorization, without additional restrictions.
- REQ-ADP-007: The system shall provide an adapter SDK skeleton with versioned interface headers and build templates.

## Configuration Requirements (CFG)
- REQ-CFG-001: Configuration files shall be versioned and schema-validated.
- REQ-CFG-002: Configuration validation shall enforce inter-field constraints (min <= max, dt > 0).
- REQ-CFG-003: Default values shall be documented and justified in the config schema.
- REQ-CFG-004: Configuration shall define platform profile, permitted sensors, and network-aid policy.
- REQ-CFG-005: Configuration shall define roles, override rules, and dataset tier constraints.
- REQ-CFG-006: Configuration shall define parameters for vision, lidar, magnetometer, barometer, and celestial sensors.
- REQ-CFG-007: Configuration shall define platform.profile_parent and platform.child_modules with validation and deterministic defaults.
- REQ-CFG-008: Configuration shall define allowed provenance values per run mode and reject unknown or mixed-policy settings.
- REQ-CFG-009: Configuration shall define policy/provenance authorization inputs (version, source, allowed modes) and reject unknown or missing fields.
- REQ-CFG-010: Configuration shall specify adapter selection (id + version) and UI surface; unknown adapter identifiers or surfaces are errors.
- REQ-CFG-011: Configuration shall support adapter manifest and allowlist paths with deterministic defaults for official adapters; missing or invalid adapter registry paths shall fail closed.
- REQ-CFG-012: Configuration shall define adapter runtime compatibility context versions and allowlist freshness limits; invalid semantic versions or freshness limits are errors.
- REQ-CFG-013: Configuration shall define `front_view.*` display, cycle, spoof, latency, proximity, security, and threading controls with explicit units/ranges/defaults; invalid values shall fail closed.

## Verification Requirements (VER)
- REQ-VER-001: Every requirement shall be mapped to at least one verification method (test, analysis, inspection).
- REQ-VER-002: Core algorithms shall have unit tests covering boundary conditions.
- REQ-VER-003: Test scripts shall build all registered test targets before running them.
- REQ-VER-004: Test harness inputs shall be authenticated by build-time gating and runtime enablement before use.
- REQ-VER-005: Test scripts shall normalize test summary labels to avoid redundant qualifiers (e.g., omit "real").
- REQ-VER-006: Integration tests shall cover dropout ladder selection, dataset gating, and UI menu flows.

## Documentation Requirements (DOC)
- REQ-DOC-001: The README shall include submodule-aware clone/pull instructions and non-duplicative build/run guidance, with a quick-start summary that may repeat the core steps.
