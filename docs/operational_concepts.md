# Operational Concepts

## Mission Modes
- gps: Primary mode using GPS measurements.
- thermal: Thermal sensor mode.
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

## Provenance and Gating
- Every measurement and operator input is tagged with provenance: operational, simulation, test, or unknown.
- Operational runs accept only operational provenance unless explicitly authorized by policy.
- Mixed or unknown provenance triggers denial, audit logging, and safe-state transition.
- Provenance decisions are recorded with reason codes, sources, and run identifiers.

## Mode Transition Rules
- Mode changes are controlled by the mode manager using health, confidence, and hysteresis.
- A minimum dwell time shall be enforced to prevent oscillation.
- On loss of all sensors, the system shall enter hold mode.
- The navigation ladder prefers higher-permitted sources before lower-permitted ones.
- Celestial navigation is only permitted when higher-priority sources are unavailable or disallowed.

## Multi-Modal Switching and Fusion
- Fused modes combine a primary source with one or more aiding sources (GNSS+INS, VIO/LIO, radar/terrain, magnetometer+baro).
- Eligibility uses health, data freshness, confidence, and policy gates.
- Mode transitions require state alignment checks to avoid discontinuities.
- Operator status displays the active mode and contributing sources with confidence.
- Disagreement beyond thresholds triggers downgrade to a safer mode or hold.

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
- Celestial datasets are treated as controlled inputs and must pass integrity checks.
- Audit logs include mode/config changes with timestamps and integrity protections.
