# Multi-Modal Switching Design

## Purpose
Define the multi-modal switching architecture, fused mode catalog, eligibility
gates, safety behaviors, and verification approach for the navigation fallback
ladder. This design satisfies REQ-SYS-010, REQ-FUNC-012..015, REQ-SAFE-006, and
REQ-SEC-008.

## Scope
- Core mode manager selection logic.
- Sensor health, freshness, confidence, and policy gating.
- Fused mode behavior and downgrade rules.
- Operator visibility of contributing sources.
- Deterministic behavior and testing.

Out of scope:
- External plugin interfaces.
- Real sensor drivers (simulation models only).

## Fused Mode Catalog
Primary modes:
- gps
- vision
- lidar
- radar
- thermal
- magnetometer
- baro
- celestial
- dead_reckoning
- imu
- hold

Fused modes (composite sources):
- gps_ins: GNSS + IMU aiding.
- vio: vision + IMU aiding.
- lio: lidar + IMU aiding.
- radar_inertial: radar + IMU aiding.
- mag_baro: magnetometer + baro stabilization.

Notes:
- Fused modes are explicit TrackingMode values, not just reason strings.
- A fused mode publishes contributor list and confidence/weights.

## Eligibility and Gating Rules
Inputs per sensor:
- health (boolean).
- data_age_seconds (time since last valid measurement).
- confidence (0-1).
- policy_allowed (boolean).
 - recent trend flags (health and disagreement history).

Global gates:
- dataset availability for celestial modes (catalog + ephemeris + hashes).
- role/policy gates for network-aid or restricted sources.
- min_healthy_count and min_dwell_steps.

Eligibility algorithm (deterministic):
1) Reject sensors failing policy_allowed.
2) Reject sensors with data_age_seconds > fusion.max_data_age_seconds.
3) Reject sensors with confidence below per-sensor threshold (configurable).
4) Reject sensors with persistent negative trend (configured history window).
5) If mode is fused, require all mandatory contributors to be eligible.
6) If disagreement exceeds fusion.disagreement_threshold, mark mode unsafe.

## Disagreement and Safe-State Behavior
Disagreement detection:
- Compare measurements from contributors over a window.
- If delta exceeds configured thresholds, mark conflict.
- Conflict triggers downgrade to a safer non-fused mode, or hold if none eligible.
- Persistent conflicts trigger temporary lockout for the offending source.
- Residual checks require time-aligned measurements within max_residual_age_seconds.
- Residual misalignment triggers explicit denial and safe fallback behavior.

## Lockout and Recovery
- Sensors exceeding stale or low-confidence thresholds enter lockout for
  mode.lockout_steps.
- Locked contributors block modes that require them.
- Lockouts expire deterministically and require fresh healthy samples to re-enter.

Safe-state enforcement:
- If no eligible modes, enter hold immediately.
- If policy or dataset inputs are invalid, deny fused/primary and hold.

## Concurrent Mode Execution
The system supports concurrent sensor pipelines when sensors are compatible and
resource budgets permit. Concurrency is used to avoid interrupting long scans
while allowing snapshots (e.g., IR or lidar) to run in parallel.

Concurrency rules:
- Each sensor pipeline has a budget (time, rate, and max outstanding work).
- Primary tracking pipeline must not be interrupted by auxiliary snapshots.
- Snapshot results are tagged and may contribute as auxiliary inputs only.
- Arbitration is deterministic and respects the ladder order and safety budgets.
- If concurrency risks safe-state constraints, auxiliary pipelines are paused.

Examples:
- During a flyover, IR snapshot and lidar snapshot run concurrently with a
  radar scan; radar scan continuity is preserved.
- GNSS+INS continues while vision snapshots are collected for VIO eligibility.

## Mode Ordering and Ladder Configuration
The ladder order is configurable per platform profile and policy:
- mode.ladder_order controls prioritized evaluation.
- Profiles provide defaults; mission policy can constrain order (never widen).

Example default ladder (high to low):
- gps_ins
- gps
- vio
- lio
- radar_inertial
- vision
- lidar
- radar
- thermal
- mag_baro
- magnetometer
- baro
- celestial
- dead_reckoning
- imu
- hold

## Transition and Handoff Rules
- Maintain current mode if it remains eligible and within dwell.
- When switching to fused mode, require a state alignment check.
- Alignment check: delta between current state and fused estimate below thresholds.
- On failed alignment, continue current mode or fall back to next eligible.
- Dwell count resets on mode switch.

## Scheduler and Arbitration (Planned)
- Scheduler evaluates eligible pipelines each tick in fixed order.
- Primary mode gets reserved budget; auxiliary pipelines are scheduled round-robin.
- If data freshness fails, the pipeline is skipped without affecting others.
- Starvation prevention: auxiliary pipelines have bounded minimum service but
  never at the expense of primary safety constraints.

## Determinism
- All randomness uses seeded RNG from SimConfig.
- Eligibility uses fixed thresholds and deterministic ordering.
- Tests must compare bit-for-bit outputs where applicable.

## Data Structures (Planned)
- ModeDefinition: name, required_sensors, optional_sensors, policy_tags.
- SourceStatus: health, data_age, confidence, policy_allowed.
- ModeDecision: selected_mode, contributors, confidence, reason.
- DisagreementReport: metrics per contributor pair.

## UI/TUI Reporting
Operator UI must display:
- Active mode.
- Contributing sensors and confidence/weights.
- Policy denial reasons for excluded sources.
- Downgrade reasons when conflicts occur.
- Concurrency status when auxiliary pipelines are active.

## Edge Cases
- Single contributor in fused mode missing: fused mode ineligible.
- All sensors healthy but policy denies: hold with explicit policy error.
- Celestial allowed but datasets missing: hold with dataset error.
- Conflicting data with high confidence: downgrade, log, and hold if needed.
- Stale data: ineligible even if health is true.
- Auxiliary snapshot bursts under heavy load: pause auxiliary, preserve primary.

## Verification Mapping
- V-049: Confirm fused mode catalog and lineage documentation.
- V-050: Validate eligibility rejection for stale/unauthorized sources.
- V-051: Validate fused modes only activate with required contributors.
- V-052: UI demonstrates contributors and confidence.
- V-053: Profile ladder ordering enforced.
- V-054: Disagreement detection downgrades or holds.
- V-055: Unauthorized fusion contribution rejected and logged.
