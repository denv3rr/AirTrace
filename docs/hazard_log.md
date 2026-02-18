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
| HZ-019 | Unauthorized plugin, device driver, or sensor source contaminates fused solution. | Critical | Remote | Plugin/device authorization gates, signed allowlists, and source allowlists for fusion. | V-021, V-022, V-055 |
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
| HZ-036 | Front-view display renders stale, invalid, or unauthorized imagery and misleads operator decisions. | Critical | Occasional | Front-view frame contract validation, spoof/config validation, fail-closed denial banners, and explicit latency/drop telemetry. | V-125, V-126, V-127 |
| HZ-037 | Multi-view front-view streams become unsynchronized or unstabilized, causing incorrect operator interpretation of target geometry. | Critical | Occasional | Enforce deterministic frame timestamp/age contract, stabilization/gimbal validation, stream-ID constraints, and fail-closed denial on invalid stream/stabilization/gimbal combinations. | V-128, V-129, V-130 |
| HZ-038 | Federation bridge accepts untrusted/expired federate key material or missing attestation, enabling unsafe cross-domain data propagation without accountable denial evidence. | Critical | Occasional | Enforce key-validity windows, endpoint key allowlists, endpoint attestation requirements, deterministic fail-closed denial, and auditable publish/deny events with route/endpoint context. | V-139 |

## Requirement and Control Crosswalk
| Hazard ID | Requirement Links | Security/Interface Control Links |
| --- | --- | --- |
| HZ-001 | REQ-SYS-005; REQ-CFG-001; REQ-CFG-002; REQ-SEC-001 | Config Parser (TB-1) |
| HZ-002 | REQ-SAFE-001; REQ-SAFE-003; REQ-FUNC-002 | Sensor Adapters + Mode Manager |
| HZ-003 | REQ-FUNC-012; REQ-SAFE-006 | Sensor Adapters + Provenance Gate |
| HZ-004 | REQ-SAFE-003; REQ-FUNC-002 | Mode Manager guard conditions |
| HZ-005 | REQ-FUNC-006; REQ-FUNC-025; REQ-SAFE-004 | Config Parser bounds validation |
| HZ-006 | REQ-SYS-006; REQ-SAFE-002; REQ-SEC-004 | Audit Log Sink |
| HZ-007 | REQ-INT-002; REQ-INT-003; REQ-INT-020; REQ-INT-025; REQ-INT-026; REQ-INT-027 | UI/TUI Input + UI standards |
| HZ-008 | REQ-FUNC-012; REQ-SAFE-008 | Sensor freshness gating |
| HZ-009 | REQ-SEC-005; REQ-SEC-006; REQ-SEC-007 | Network Aid Gate (TB-2) |
| HZ-010 | REQ-SYS-008; REQ-FUNC-015; REQ-SAFE-011 | Mode eligibility pipeline |
| HZ-011 | REQ-FUNC-009; REQ-SAFE-005 | Dataset Validator |
| HZ-012 | REQ-SYS-007; REQ-CFG-004 | Config Parser + profile policy |
| HZ-013 | REQ-SEC-007; REQ-SEC-006 | Authorization Bundle + Network Aid Gate |
| HZ-014 | REQ-SYS-009; REQ-FUNC-010; REQ-CFG-005 | Dataset tier policy |
| HZ-015 | REQ-FUNC-011; REQ-FUNC-019 | Sensor Adapters + residual checks |
| HZ-016 | REQ-FUNC-011; REQ-SAFE-008 | Sensor Adapters + safe degraded mode |
| HZ-017 | REQ-SAFE-006; REQ-FUNC-019 | Residual conflict detection |
| HZ-018 | REQ-SAFE-003; REQ-FUNC-013 | Mode transition guards |
| HZ-019 | REQ-SEC-002; REQ-SEC-003; REQ-SEC-008; REQ-SYS-019 | Plugin authorization gate + provenance/authorization gates |
| HZ-020 | REQ-SAFE-007; REQ-FUNC-016 | Scheduler resource controls |
| HZ-021 | REQ-FUNC-017; REQ-SAFE-007 | Scheduler snapshot arbitration |
| HZ-022 | REQ-SAFE-008; REQ-FUNC-012 | Freshness and jitter gating |
| HZ-023 | REQ-FUNC-019; REQ-SAFE-006 | Residual threshold controls |
| HZ-024 | REQ-FUNC-020; REQ-SAFE-009; REQ-INT-017 | Lockout pipeline + UI guidance |
| HZ-025 | REQ-SEC-008; REQ-FUNC-019; REQ-SYS-008 | Source authorization + fallback |
| HZ-026 | REQ-SAFE-008; REQ-FUNC-009 | Dataset and fallback constraints |
| HZ-027 | REQ-FUNC-012; REQ-SAFE-008 | Sensor confidence/freshness gating |
| HZ-028 | REQ-SYS-013; REQ-CFG-007 | Profile inheritance validator |
| HZ-029 | REQ-SYS-016; REQ-SAFE-010; REQ-SEC-009 | Audit Log Sink health checks |
| HZ-030 | REQ-SYS-001; REQ-SYS-015; REQ-FUNC-024 | Deterministic RNG + fixed timestep |
| HZ-031 | REQ-SYS-014; REQ-SYS-018 | Provenance Gate |
| HZ-032 | REQ-SYS-019; REQ-FUNC-023; REQ-SAFE-011 | Eligibility + authorization gating |
| HZ-033 | REQ-SEC-011; REQ-CFG-009; REQ-SAFE-011 | Authorization Bundle controls |
| HZ-034 | REQ-INT-017; REQ-INT-018; REQ-INT-019; REQ-INT-020 | UI/TUI denial messaging |
| HZ-035 | REQ-SEC-012; REQ-SEC-013; REQ-SAFE-012; REQ-INT-024; REQ-INT-027; REQ-MOD-004 | Adapter Registry + Allowlist + UI surface controls |
| HZ-036 | REQ-INT-028; REQ-INT-029; REQ-CFG-013 | Front-view display contract + spoof/cycle validation controls |
| HZ-037 | REQ-INT-030; REQ-INT-031; REQ-CFG-014 | Front-view multi-view timing/stabilization contract + fail-closed validation controls |
| HZ-038 | REQ-INT-038; REQ-SEC-014; REQ-SAFE-011 | Federation bridge trust-policy + key-lifecycle + auditable fail-closed controls |
