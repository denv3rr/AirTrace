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
| HZ-007 | UI ambiguity causes operator error. | Marginal | Occasional | Clear mode/health/safety indicators. | V-025, V-026 |
| HZ-008 | Sensor fusion uses stale data. | Marginal | Occasional | Timestamp validation and timeouts. | V-007 |
| HZ-009 | Unauthorized network-aid usage reveals position. | Critical | Occasional | Deny-by-default policy with credentialed overrides and logging. | V-037, V-038 |
| HZ-010 | Incorrect fallback ladder selects unsafe source. | Critical | Occasional | Health/confidence gating, policy checks, and verification tests. | V-033, V-034 |
| HZ-011 | Celestial dataset corruption produces wrong fix. | Marginal | Remote | Versioned datasets with integrity verification and safe-state on failure. | V-035, V-036 |
| HZ-012 | Platform profile mismatch enables unsupported sensor use. | Marginal | Occasional | Profile capability validation and explicit operator prompts. | V-032, V-040 |
| HZ-013 | Role misuse enables unsafe overrides. | Critical | Occasional | Role-based access, credentialed overrides, audit logging. | V-043, V-044 |
| HZ-014 | Oversized datasets exceed platform storage limits. | Marginal | Occasional | Dataset tiering and profile-based constraints. | V-041, V-042 |
| HZ-015 | Magnetometer interference yields incorrect heading. | Marginal | Occasional | Speed gating, bias modeling, and cross-check with inertial sources. | V-046 |
| HZ-016 | Barometric drift yields incorrect altitude. | Marginal | Occasional | Drift modeling and range checks with safe-state fallback. | V-046 |
