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
