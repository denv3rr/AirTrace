# Hazard Log (MIL-STD-882E)

Severity: Catastrophic, Critical, Marginal, Negligible. Likelihood: Frequent, Probable, Occasional, Remote, Improbable.

| Hazard ID | Description | Severity | Likelihood | Mitigation | Verification |
| --- | --- | --- | --- | --- | --- |
| HZ-001 | Invalid configuration leads to undefined behavior. | Critical | Occasional | Strict schema validation, fail-closed on error. | V-005, V-028 |
| HZ-002 | Sensor dropout causes unstable tracking. | Critical | Probable | Safe-state hold mode, health history, hysteresis. | V-016, V-018 |
| HZ-003 | False positive sensor data drives incorrect mode. | Marginal | Occasional | Gating and confidence thresholds. | V-008 |
| HZ-004 | Mode thrashing during intermittent sensor health. | Marginal | Probable | Minimum dwell time, guard conditions. | V-018 |
| HZ-005 | Out-of-range motion yields non-physical state. | Critical | Occasional | Enforce bounds, clamp or reject. | V-012, V-019 |
| HZ-006 | Missing audit data prevents post-event analysis. | Marginal | Occasional | Structured logging with build/config IDs. | V-006, V-017 |
| HZ-007 | UI ambiguity causes operator error. | Marginal | Occasional | Clear mode/health/safety indicators. | V-025, V-026, V-069, V-083, V-092 |
| HZ-008 | Sensor fusion uses stale data. | Marginal | Occasional | Timestamp validation and timeouts. | V-007 |
| HZ-009 | Unauthorized network-aid usage reveals position. | Critical | Occasional | Deny-by-default policy with credentialed overrides and logging. | V-037, V-038 |
| HZ-010 | Incorrect fallback ladder selects unsafe source. | Critical | Occasional | Health/confidence gating, policy checks, and verification tests. | V-033, V-034, V-039, V-094 |
| HZ-011 | Celestial dataset corruption produces wrong fix. | Marginal | Remote | Versioned datasets with integrity verification and safe-state on failure. | V-035, V-036 |
| HZ-012 | Platform profile mismatch enables unsupported sensor use. | Marginal | Occasional | Profile capability validation and explicit operator prompts. | V-032, V-040 |
| HZ-013 | Role misuse enables unsafe overrides. | Critical | Occasional | Role-based access, credentialed overrides, audit logging. | V-043, V-044 |
| HZ-014 | Oversized datasets exceed platform storage limits. | Marginal | Occasional | Dataset tiering and profile-based constraints. | V-041, V-042 |
| HZ-015 | Magnetometer interference yields incorrect heading. | Marginal | Occasional | Speed gating, bias modeling, and cross-check with inertial sources. | V-046 |
| HZ-016 | Barometric drift yields incorrect altitude. | Marginal | Occasional | Drift modeling and range checks with safe-state fallback. | V-046 |
| HZ-017 | Multi-sensor disagreement produces divergent state updates. | Critical | Occasional | Cross-check thresholds, reject outliers, degrade to safer mode. | V-054 |
| HZ-018 | Fused mode switches without proper alignment cause discontinuity. | Critical | Occasional | State handoff rules, alignment checks, and dwell constraints. | V-051 |
| HZ-019 | Unauthorized sensor source contaminates fused solution. | Critical | Remote | Authorization gates and source allowlists for fusion. | V-055 |
| HZ-020 | Concurrent pipelines starve timing for safety-critical updates. | Critical | Occasional | Deterministic scheduler with budgets and safe-state fallback. | V-059 |
| HZ-021 | Snapshot sensor interrupts long-scan pipeline causing stale output. | Marginal | Occasional | Preserve scan continuity and tag auxiliary snapshots. | V-058 |
| HZ-022 | Timing jitter or stale data leads to unsafe mode selection. | Critical | Occasional | Freshness thresholds, jitter detection, safe-state downgrade. | V-062 |
| HZ-023 | Cross-sensor residual conflicts drive fused divergence. | Critical | Occasional | Residual checks and forced downgrade. | V-061 |
| HZ-024 | Sensor lockout misapplied causes prolonged loss of capability. | Marginal | Remote | Lockout bounds, recovery hysteresis, and operator prompt. | V-063, V-064, V-069, V-094 |
| HZ-025 | GNSS jamming or spoofing yields misleading position. | Critical | Occasional | Source authorization, cross-sensor disagreement checks, fallback ladder. | V-033, V-054 |
| HZ-026 | Space weather degrades GNSS accuracy or availability. | Marginal | Occasional | Dataset/space-weather awareness and safe-state fallback. | V-062, V-036 |
| HZ-027 | High-power RF interference saturates receivers. | Critical | Occasional | Signal health gating, confidence thresholds, degrade to safe sources. | V-033, V-054 |
| HZ-028 | Misconfigured profile inheritance enables unsupported sensors. | Critical | Occasional | Deterministic merge rules, validation, and explicit permitted sensors. | V-073, V-074 |
| HZ-029 | Audit logging unavailable or blocked causes loss of safety/audit events. | Marginal | Occasional | Bounded non-blocking logging, logging health status, fail closed on safety-event logging failure. | V-080, V-081 |
| HZ-030 | Nondeterminism in core or simulation causes divergent outputs and unsafe decisions. | Critical | Occasional | Seeded RNG only, fixed timestep, deterministic scheduler, audit for nondeterminism. | V-079 |
| HZ-031 | Simulation inputs contaminate operational runs. | Critical | Occasional | Explicit sim/test gating, provenance tagging, reject mixed provenance. | V-078 |
| HZ-032 | Eligibility evaluation fails open due to missing policy/provenance inputs, enabling unsafe mode activation. | Critical | Occasional | Deny-by-default eligibility, explicit safe-state entry, audit logging. | V-095, V-097, V-098 |
| HZ-033 | Stale or unverifiable authorization policy yields incorrect eligibility decisions. | Critical | Occasional | Policy provenance verification, version gating, deny-by-default. | V-097, V-099 |
| HZ-034 | Operator cannot interpret eligibility denial causes, leading to unsafe manual override attempts. | Critical | Occasional | Explicit reason codes and recovery guidance in UI/TUI. | V-100 |
| HZ-035 | Adapter malfunction or incompatible interface yields unsafe data or UI ambiguity. | Critical | Occasional | Signed allowlisted adapters, versioned contracts, safe-state on adapter failure, explicit UI validation. | V-111, V-113 |
