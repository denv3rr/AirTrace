# Operational Concepts

## Mission Modes
- gps: Primary mode using GPS measurements.
- thermal: Thermal sensor mode.
  Thermal path adjustment is bounded and monotonic with validated heat intensity to avoid abrupt unsafe motion responses.
- radar: Radar range/bearing mode.
- vision: EO/IR vision position fixes.
- lidar: Lidar range/bearing mode.
- magnetometer: Heading stabilization using magnetic compass inputs.
- baro: Altitude stabilization using barometric inputs.
- dead_reckoning: Fallback mode using drift-based estimation.
- inertial: IMU-only mode for short-term stabilization.
- celestial: Celestial fix using onboard star/planet references when permitted and required.
- hold: Safe state with no tracking updates.

## Operational vs Simulation Modes
- Operational tool runs are the primary mission posture.
- Simulation and test modes are explicitly identified, gated, and prohibited in operational runs unless authorized by policy.
- Mixed provenance (sim + operational) is rejected and triggers a safe-state denial.

## Operator Displays and UI Surfaces
- The system supports multiple operator surfaces: cockpit UI, remote operator visuals, C2 screens, and TUI.
- All surfaces must present consistent semantics for mode, source, contributors, and denial reason codes.
- Displays expose per-sensor health/availability/freshness/confidence and lockout state.
- Fallback selection reasons and disqualified sources are shown with recovery guidance (REQ-INT-017).
- Scenario and test runs auto-cycle through visual modes aligned to evaluated tracking modes for rapid validation.
- Multi-mode visualization may present multiple eligible modes concurrently when configured.
- Platform workbench provides operator-invoked profile selection and optional profile cycling from a single UI session.
- Front-view workbench provides EO/IR/proximity display families with deterministic spoof-input support when physical sensors are unavailable.
- Front-view runs support both single-mode rendering and cycle-all rendering from one session.

## Modularity and Interface Contracts
- Core, tools, UI, and platform adapters are modular and independently deployable.
- Interfaces are versioned and explicit; integration uses allowlisted capabilities and deny-by-default policies.
- Safety and fail-closed behavior apply consistently across all modules and adapters.
- Third-party platform adapters are supported when they pass contract, safety, and security gates; no additional restrictions apply.
- Adapter selection uses manifest and allowlist validation; hash mismatches or missing manifests deny activation.
- Adapter allowlist approvals are freshness-gated; stale or future-dated approvals are denied.

## Provenance and Gating
- Every measurement and operator input is tagged with provenance: operational, simulation, test, or unknown.
- Operational runs accept only operational provenance unless explicitly authorized by policy.
- Mixed or unknown provenance triggers denial, audit logging, and safe-state transition.
- Provenance decisions are recorded with reason codes, sources, and run identifiers.

## Mode Transition Rules
- Mode changes are controlled by the mode manager using health, confidence, and hysteresis.
- A minimum dwell time shall be enforced to prevent oscillation.
- On loss of all sensors, the system shall enter hold mode.
- Invalid operator speed divisors (<= 0) are denied and force tracker inactivity.
- The navigation ladder prefers higher-permitted sources before lower-permitted ones.
- Celestial navigation is only permitted when higher-priority sources are unavailable or disallowed.
 - Fallback selection decisions provide explicit reason codes for disqualification (policy, health, freshness, lockout).

## Multi-Modal Switching and Fusion
- Fused modes combine a primary source with one or more aiding sources (GNSS+INS, VIO/LIO, radar/terrain, magnetometer+baro).
- Eligibility uses health, data freshness, confidence, and policy gates.
- Mode transitions require state alignment checks to avoid discontinuities.
- Operator status displays the active mode and contributing sources with confidence.
- Disagreement beyond thresholds triggers downgrade to a safer mode or hold.

## UI Data Model (Operator-Facing)
- Active mode/source, contributors, and confidence.
- Per-sensor status: available, healthy, freshness age, confidence, last error, has measurement.
- Fallback ladder status with ordered candidates and disqualification reasons.
- Lockout state with remaining steps and reason code.
- Provenance status and policy authorization state.
- Visual mode cycle status, including the current visual mode and its mapping to evaluated tracking modes.
- Front-view status: active display family, view state (front/2d/3d), frame ID, sensor type, latency (ms), dropped-frame count, and drop reason.
- External I/O envelope status with version, platform profile, adapter status, and deterministic metadata fields.

## Concurrent Pipeline Operation
- Compatible sensor pipelines may run concurrently when budgets permit.
- Primary tracking pipelines are protected from interruption by auxiliary snapshots.
- Snapshot results are tagged and contribute as auxiliary inputs only.
- If resource pressure risks safety constraints, auxiliary pipelines are paused.

## Historical Context and Edge-Case Handling
- Mode decisions consider recent health, confidence, and disagreement trends.
- Timing jitter and stale data trigger safe degraded modes or hold.
- Cross-sensor residual checks gate fused mode eligibility.
- See docs/edge_case_catalog.md for scenario coverage.

## Platform Profiles
- Base profile defines common sensors, policy defaults, and data contracts.
- Child profiles extend the base: air, ground, maritime, space, handheld, fixed_site, subsea.
- Profiles declare capabilities (sensors available, permitted aids, and auth policy).
- Profiles may inherit from a parent profile with deterministic merge rules for permitted sensors.
- Child hardware modules are listed explicitly to document platform composition.
- See docs/navigation_fallbacks.md for ladder ordering and dataset tiers.

## External Display Library Governance
- Open-source display assets may be evaluated, but integration requires a security review.
- No network access or dynamic loading; assets must be vendored, scanned, and listed in SBOM.

## Dataset Tiers
- Tiered celestial datasets limit storage based on platform profile.
- Handheld/personnel profiles default to minimal tiers unless explicitly overridden.
- High-capability platforms may use expanded tiers when authorized by policy.

## Authorization Roles
- Roles are defined in configuration and mapped to permitted overrides.
- Suggested roles: operator, supervisor, mission_owner, test, research, internal.
- Role policy is least-privilege; deny-by-default remains the baseline.

## Operator Responsibilities
- Validate configuration files before execution.
- Monitor mode status, sensor health, and safety indicators.
- Acknowledge errors and follow recovery prompts.
- Ensure policy authorization is set for any network-aid usage.
- Verify audit logging health before and during operational runs.
- UI/TUI behavior follows docs/ui_standards.md.

## Multi-Device Operation
- Each device exposes capabilities and supported sensors.
- Device time sync is required to align measurements.
- Device health is monitored and aggregated to determine global mode.

## Environmental Constraints
- Position, velocity, acceleration, and turn rate are bounded by config.
- Out-of-range conditions trigger safe-state behavior or rejection.

## Data Handling
- Config inputs are untrusted; schema validation is mandatory.
- Simulation outputs include seed, config version, and build metadata.
- Target generation utilities are seeded and deterministic for repeatable replay evidence.
- Celestial datasets are treated as controlled inputs and must pass integrity checks.
- Audit logs include mode/config changes with timestamps and integrity protections.
