# Verification Plan

Each requirement maps to at least one verification case. Methods: TEST, ANALYSIS, INSPECTION, DEMO.

| Verification ID | Requirement ID | Method | Description | Acceptance Criteria |
| --- | --- | --- | --- | --- |
| V-001 | REQ-SYS-001 | TEST | Run two simulations with same seed and config. | Outputs match bit-for-bit. |
| V-002 | REQ-SYS-002 | INSPECTION | Review core modules for direct IO use. | No direct IO in core. |
| V-003 | REQ-SYS-003 | TEST | Exercise mode transitions using scripted health states. | Only valid transitions occur with guards. |
| V-004 | REQ-SYS-004 | INSPECTION | Review sensor interfaces for capability flags. | Capabilities explicitly documented. |
| V-005 | REQ-SYS-005 | TEST | Inject invalid config and missing data. | System enters safe state. |
| V-006 | REQ-SYS-006 | INSPECTION | Verify build metadata logging. | Logs include build ID and config ID. |
| V-007 | REQ-FUNC-001 | TEST | Feed sensor samples into unified measurement. | Correct fusion input and schema adherence. |
| V-008 | REQ-FUNC-002 | TEST | Simulate fluctuating sensor health. | Hysteresis prevents mode thrash. |
| V-009 | REQ-FUNC-003 | TEST | Provide config with unknown key. | Parsing fails with explicit error. |
| V-010 | REQ-FUNC-004 | TEST | Replay logged run with seed/config. | Outputs match original. |
| V-011 | REQ-FUNC-005 | TEST | Project a known state. | XY/XZ/YZ values correct. |
| V-012 | REQ-FUNC-006 | TEST | Provide out-of-bounds inputs. | Outputs clamped or rejected per spec. |
| V-013 | REQ-FUNC-007 | TEST | Disable all sensors. | Mode is "hold" with safe-state status. |
| V-014 | REQ-PERF-001 | ANALYSIS | Measure update runtime under fixed load. | Meets documented budget. |
| V-015 | REQ-PERF-002 | TEST | Set sensor rates in config. | Sample times honor rate setting. |
| V-016 | REQ-SAFE-001 | TEST | Force dropout and invalid config. | Safe-state transitions observed. |
| V-017 | REQ-SAFE-002 | INSPECTION | Review safety logging. | Safety events recorded with mode/time. |
| V-018 | REQ-SAFE-003 | TEST | Oscillate sensor health. | Minimum dwell time enforced. |
| V-019 | REQ-SAFE-004 | TEST | Exceed turn/speed/accel limits. | Limits enforced. |
| V-020 | REQ-SEC-001 | TEST | Fuzz config inputs. | Input validation blocks malformed data. |
| V-021 | REQ-SEC-002 | INSPECTION | Review plugin loading paths. | Explicit authorization enforced. |
| V-022 | REQ-SEC-003 | TEST | Load unsigned plugin. | Rejected with audit log. |
| V-023 | REQ-SEC-004 | INSPECTION | Verify audit log entries. | Mode/config changes recorded. |
| V-024 | REQ-INT-001 | INSPECTION | Review public headers. | Units/ranges documented. |
| V-025 | REQ-INT-002 | DEMO | Operate UI with sensor changes. | Mode/health/status visible. |
| V-026 | REQ-INT-003 | DEMO | Trigger invalid input. | Explicit error and recovery shown. |
| V-027 | REQ-CFG-001 | TEST | Load versioned config. | Schema version enforced. |
| V-028 | REQ-CFG-002 | TEST | Use invalid ranges. | Validation fails with errors. |
| V-029 | REQ-CFG-003 | INSPECTION | Review config schema docs. | Defaults documented and justified. |
| V-030 | REQ-VER-001 | INSPECTION | Cross-check requirements vs plan. | Every REQ has V mapping. |
| V-031 | REQ-VER-002 | TEST | Run unit tests. | Boundary coverage present. |
| V-032 | REQ-SYS-007 | INSPECTION | Review platform profile definitions. | Base profile with inherited overrides present. |
| V-033 | REQ-SYS-008 | TEST | Force sensor outages and policy constraints. | Highest-permitted source selected. |
| V-034 | REQ-FUNC-008 | TEST | Simulate GPS-denied with permitted alternatives. | Celestial nav activates only when required. |
| V-035 | REQ-FUNC-009 | TEST | Load dataset with invalid hash/version. | Rejected with safe-state entry. |
| V-036 | REQ-SAFE-005 | TEST | Remove policy/dataset config. | Hold mode entered. |
| V-037 | REQ-SEC-005 | TEST | Configure deny-by-default network aid. | Network aids blocked. |
| V-038 | REQ-SEC-006 | TEST | Attempt override without credentials. | Override denied and logged. |
| V-039 | REQ-INT-004 | DEMO | UI shows source/policy status. | Visible and clear to operator. |
| V-040 | REQ-CFG-004 | TEST | Provide policy/platform keys in config. | Parsed and validated correctly. |
| V-041 | REQ-SYS-009 | INSPECTION | Review dataset tier definitions. | Tier constraints documented per profile. |
| V-042 | REQ-FUNC-010 | TEST | Activate profile with limited tier. | Minimal tier selected unless overridden. |
| V-043 | REQ-SEC-007 | TEST | Attempt override with insufficient role. | Override denied and logged. |
| V-044 | REQ-CFG-005 | TEST | Provide role/policy configuration. | Parsed and validated correctly. |
| V-045 | REQ-INT-005 | DEMO | Run test mode twice with same seed. | UI outputs match for deterministic inputs. |
| V-046 | REQ-FUNC-011 | TEST | Exercise vision/lidar/magnetometer/baro/celestial sensors with deterministic RNG. | Measurements carry expected types and validity flags. |
| V-047 | REQ-CFG-006 | TEST | Provide new sensor keys in config. | Parsed and validated correctly. |
| V-048 | REQ-DOC-001 | INSPECTION | Review README for submodule instructions and de-duplicated build/run guidance. | Quick start includes clone/pull with submodules and duplication is limited to the quick-start summary. |
