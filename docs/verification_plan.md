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
| V-021 | REQ-SEC-002 | TEST | Load config with plugin metadata and `plugin.authorization_granted=false`. | Config load fails closed with `plugin_not_authorized` reason. |
| V-022 | REQ-SEC-003 | TEST | Load config with plugin allowlist/signature mismatch and then a matching signed plugin. | Mismatch fails closed with `plugin_signature_invalid`; matching signed allowlisted plugin passes. |
| V-023 | REQ-SEC-004 | TEST | Trigger mode/config changes and capture audit records. | Records include event type, mode/config identifiers, role/actor, and timestamp. |
| V-024 | REQ-INT-001 | INSPECTION | Review public headers. | Units/ranges documented. |
| V-025 | REQ-INT-002 | DEMO | Operate UI with sensor changes. | Mode/health/status visible. |
| V-026 | REQ-INT-003 | DEMO | Trigger invalid input or file I/O failure in UI flows. | Explicit error and recovery shown. |
| V-027 | REQ-CFG-001 | TEST | Load versioned config. | Schema version enforced. |
| V-028 | REQ-CFG-002 | TEST | Use invalid ranges across baseline and extended fields. | Validation fails with explicit errors for all invalid fields. |
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
| V-049 | REQ-SYS-010 | ANALYSIS | Review fused mode catalog and source lineage definitions. | Fused modes and lineage are documented. |
| V-050 | REQ-FUNC-012 | TEST | Inject stale or unauthorized sensor inputs. | Eligibility rejects stale/unauthorized sources. |
| V-051 | REQ-FUNC-013 | TEST | Simulate each fused mode with required sensors present. | Mode activates only when required sources are healthy. |
| V-052 | REQ-FUNC-014 | DEMO | Inspect operator UI during fused mode. | Contributors and confidence are visible. |
| V-053 | REQ-FUNC-015 | TEST | Switch platform profiles with custom ladder ordering. | Ordering follows platform and policy. |
| V-054 | REQ-SAFE-006 | TEST | Inject conflicting sensor measurements beyond thresholds. | System degrades to safer mode or hold. |
| V-055 | REQ-SEC-008 | TEST | Attempt to fuse an unauthorized source. | Source is rejected and logged. |
| V-056 | REQ-SYS-011 | TEST | Run concurrent pipelines (e.g., IR snapshot + lidar scan + GNSS+INS). | No interruptions or dropped primary scans; outputs deterministic. |
| V-057 | REQ-FUNC-016 | TEST | Stress concurrent scheduling with bounded resources. | Arbitration remains deterministic and within limits. |
| V-058 | REQ-FUNC-017 | TEST | Trigger auxiliary snapshot sensors during active scan. | Scan continuity preserved and auxiliary results tagged. |
| V-059 | REQ-SAFE-007 | TEST | Induce load spikes across concurrent pipelines. | Safe-state constraints remain satisfied. |
| V-060 | REQ-FUNC-018 | TEST | Replay historical sensor trends with jitter and drift. | Mode selection reflects trend-aware gating. |
| V-061 | REQ-FUNC-019 | TEST | Inject cross-sensor residuals beyond thresholds. | Fused modes are rejected or downgraded. |
| V-062 | REQ-SAFE-008 | TEST | Simulate stale data and timing jitter. | System enters hold or safe degraded modes. |
| V-063 | REQ-FUNC-020 | TEST | Exceed stale/low-confidence counts. | Sensor is locked out for configured steps. |
| V-064 | REQ-SAFE-009 | TEST | Attempt mode re-entry with locked contributor. | Mode remains blocked until lockout expires. |
| V-065 | REQ-FUNC-021 | TEST | Provide residuals with mismatched timestamps. | Residual checks are skipped outside the window. |
| V-066 | REQ-INT-006 | TEST | Run UI menu selection mapping tests for scenario/test entries and invalid selections. | Valid indices map to actions; invalid selections trigger InputError. |
| V-067 | REQ-SYS-012 | TEST | Run build/run/test scripts with invalid BUILD_TYPE. | Scripts exit with an error before generating or building. |
| V-068 | REQ-VER-003 | TEST | Run test script with new test targets present. | All registered test targets are built before ctest runs. |
| V-069 | REQ-INT-007 | TEST | Run UI warning banner mapping tests for denial reasons. | Denial reasons map to deterministic recovery guidance. |
| V-070 | REQ-INT-008 | TEST | Run harness-driven menu flow with scripted inputs. | UI follows the same flow without interactive input. |
| V-071 | REQ-VER-004 | TEST | Attempt harness activation without build-time flag or runtime enablement. | Harness remains inactive and input is rejected. |
| V-072 | REQ-VER-005 | TEST | Run test scripts and inspect summary label output. | Output uses "Total Test time" and omits "(real)". |
| V-073 | REQ-SYS-013 | TEST | Load config with profile_parent and child profile, then verify merged permitted sensors. | Merge order is deterministic and includes parent + child defaults. |
| V-074 | REQ-CFG-007 | TEST | Provide invalid platform.profile_parent or child module identifiers. | Config validation fails with explicit errors. |
| V-075 | REQ-INT-009 | TEST | Run menu flows with harness enabled and no TTY. | UI menus operate and exit without input errors. |
| V-076 | REQ-VER-006 | TEST | Run integration tests for dropout ladder selection and dataset gating. | Modes follow ladder and dataset gating prevents celestial activation when unavailable. |
| V-077 | REQ-INT-010 | TEST | Build the production AirTrace target without AIRTRACE_TEST_HARNESS defined. | Build succeeds and harness interface remains disabled. |
| V-078 | REQ-SYS-014 | TEST | Attempt to invoke simulation/test modes without authorization. | Operational runs reject simulation/test modes and report denial. |
| V-079 | REQ-SYS-015 | INSPECTION | Audit core for I/O, wall-clock, and non-seeded randomness. | No core file I/O, wall-clock calls, or unseeded RNG usage. |
| V-080 | REQ-SYS-016 | TEST | Start app with and without an audit sink configured. | Logging health is reported; missing sink forces denial or safe state. |
| V-081 | REQ-SAFE-010 | TEST | Simulate audit log write failure during safety event. | System enters safe state and reports denial. |
| V-082 | REQ-SEC-009 | TEST | Attempt audit log tamper or retention violation. | Integrity checks detect tamper; retention policy enforced or explicitly denied. |
| V-083 | REQ-INT-011 | DEMO | Run operational flow and observe status banner. | Banner shows profile, source, auth, contributors, and denial status. |
| V-084 | REQ-INT-012 | TEST | Attempt destructive action without confirmation or role. | Action denied unless explicit confirmation and policy allow it. |
| V-085 | REQ-INT-013 | TEST | Trigger operator abort during a run. | Run exits safely with recovery guidance. |
| V-086 | REQ-INT-014 | INSPECTION | Review non-menu error/empty states. | Denial reasons and recovery guidance are present. |
| V-087 | REQ-INT-015 | INSPECTION | Review prompts and outputs for measurement units. | All operator-facing measurements include units and frames. |
| V-088 | REQ-SYS-017 | TEST | Sample sensor measurements without explicit provenance override and inject mixed-provenance inputs. | Deterministic default provenance is applied and provenance tags persist on outputs. |
| V-089 | REQ-SYS-018 | TEST | Attempt operational run with mixed/unknown provenance sources under `unknown_action=deny` and `unknown_action=hold`. | Deny mode rejects unknown provenance; hold mode remains in safe-state hold with explicit hold reason. |
| V-090 | REQ-SEC-010 | TEST | Trigger provenance accept/reject decisions. | Audit records include reason, sources, run ID, config version, config ID, build ID, and seed. |
| V-091 | REQ-CFG-008 | TEST | Provide invalid or mixed provenance policy settings. | Config validation fails with explicit errors. |
| V-092 | REQ-INT-016 | DEMO | Operate UI during provenance denial and unknown provenance conditions. | UI shows provenance status with recovery guidance. |
| V-094 | REQ-INT-017 | TEST | Force fallback/lockout conditions with disqualified sources. | UI shows reason codes, disqualified sources, and recovery guidance deterministically. |
| V-093 | REQ-SYS-015 | TEST | Run core purity scan for disallowed file I/O, wall-clock, or non-seeded RNG symbols. | Scan fails on violations and passes with allowlisted interfaces. |
| V-095 | REQ-SYS-019 | TEST | Inject missing eligibility inputs (health/freshness/policy/provenance/dataset). | Eligibility denies by default and safe state is entered. |
| V-096 | REQ-FUNC-022 | TEST | Exercise each eligibility denial path. | Reason codes are deterministic and stable. |
| V-097 | REQ-FUNC-023; REQ-SEC-011 | TEST | Simulate missing or unverifiable policy provenance. | Authorization is denied and an audit entry is recorded. |
| V-098 | REQ-SAFE-011 | TEST | Disable eligibility evaluation or authorization inputs. | Hold or safe degraded mode is entered. |
| V-099 | REQ-CFG-009 | TEST | Provide invalid or unknown policy/provenance authorization fields. | Schema validation fails with explicit errors. |
| V-100 | REQ-INT-018; REQ-INT-019; REQ-INT-020 | TEST | Inject deterministic sensor health and ladder states with denial reasons. | UI/TUI displays per-sensor health, ladder status, and recovery guidance with stable mappings. |
| V-101 | REQ-INT-021 | TEST | Run scenario/test flow with auto-visual mode cycling enabled. | Modes cycle deterministically and align to evaluated tracking modes. |
| V-102 | REQ-INT-022 | DEMO | Enable multi-mode visualization in a scenario run. | Concurrent mode views show contributors, confidence, and denial reasons. |
| V-103 | REQ-INT-023 | INSPECTION | Review UI interface contract versioning and compatibility checks. | Interface contract is versioned and changes are backward-compatible. |
| V-104 | REQ-INT-024 | DEMO | Render platform-specific UI extensions on at least one official adapter surface. | Adapter-defined fields appear with units and error behavior. |
| V-105 | REQ-MOD-001; REQ-MOD-002 | TEST | Build module targets (`airtrace_core`, `airtrace_adapters_contract`, `airtrace_tools`, `airtrace_ui`, `airtrace_ui_harness`) and build adapter SDK target (`airtrace_adapters_sdk`) independently in the test pipeline. | Modules build and operate independently without missing dependencies; core/tools/UI operate even when adapter SDK is optional. |
| V-106 | REQ-MOD-003 | TEST | Run automated dependency-boundary gate (`cmake/check_module_dependencies.cmake`) during test execution. | Undeclared cross-module include dependencies fail verification with file/line evidence. |
| V-107 | REQ-MOD-004 | TEST | Attempt to load an adapter with an incompatible interface version. | Activation fails closed with explicit reason code. |
| V-108 | REQ-MOD-005 | TEST | Register a third-party adapter that passes contract, safety, and security gates. | Adapter activates without additional restrictions. |
| V-109 | REQ-ADP-001 | INSPECTION | Review official adapter list and build targets. | Official adapters exist for each required platform profile. |
| V-110 | REQ-ADP-002; REQ-ADP-003 | INSPECTION | Review adapter capability declarations and UI mappings. | Capabilities/UI extensions are declared with units and mapped to surfaces. |
| V-111 | REQ-ADP-004; REQ-SAFE-012 | TEST | Inject adapter health/data failure during a run. | Safe-state entry with explicit denial and audit log. |
| V-112 | REQ-ADP-005 | INSPECTION | Review adapter design notes for environmental and human factors guidance. | Notes cover MIL-STD-810H and MIL-STD-1472H/2525 as applicable. |
| V-113 | REQ-SEC-012; REQ-CFG-010; REQ-ADP-006 | TEST | Attempt to load unsigned or unknown adapter; load a signed allowlisted adapter. | Unknown/unsigned adapters denied; allowlisted adapters load when configured and manifest hash matches allowlist. |
| V-114 | REQ-SYS-020 | INSPECTION | Verify config and audit logging I/O are owned by tools; core has no file or wall-clock I/O. | Core I/O scan passes; tools provide config/audit interfaces. |
| V-115 | REQ-ADP-007 | INSPECTION | Review adapter SDK skeleton headers and build templates. | SDK skeleton exists and is versioned. |
| V-116 | REQ-CFG-011 | TEST | Load config with adapter registry paths missing or invalid, then with valid overrides. | Missing/invalid paths fail closed; valid overrides load allowlisted manifest. |
| V-117 | REQ-CFG-012 | TEST | Load config with invalid adapter semantic versions and invalid allowlist freshness values. | Validation fails with explicit key-level errors before execution. |
| V-118 | REQ-SEC-013 | TEST | Validate adapter selection with stale, future-dated, and current allowlist approvals. | Stale/future approvals are denied with stable reason codes; current approvals pass. |
| V-119 | REQ-SYS-021 | TEST | Invoke tracker loop with speed <= 0. | Tracker logs explicit denial and transitions to inactive safe state without divide-by-zero. |
| V-120 | REQ-FUNC-024; REQ-SYS-015 | TEST | Generate target sequences with repeated and differing seeds. | Identical seeds produce identical sequences; different seeds produce deterministic but distinct sequences. |
| V-121 | REQ-FUNC-025 | TEST | Exercise heat-signature pathing with low/high/out-of-range/non-finite heat inputs. | Movement response remains monotonic for valid inputs and clamps safely for invalid/out-of-range values. |
| V-122 | REQ-INT-025 | TEST | From a single UI session, run platform workbench for selected profile and all profiles. | Profile selection/cycling is operator-invoked, deterministic, and available without restarting UI. |
| V-123 | REQ-INT-026 | TEST | Execute platform suites with valid and invalid profiles. | Sensor/adapters/mode-output checks produce explicit pass/fail with stable reason codes. |
| V-124 | REQ-INT-027 | TEST | Export machine-readable external I/O envelope after platform suite execution. | Envelope contains required versioned fields and deterministic run metadata. |
| V-125 | REQ-INT-028 | TEST | Run front-view workbench in single-mode and cycle-all paths with spoofed deterministic input. | Front-view mode output is deterministic for a fixed seed and exposes explicit mode/frame/latency status. |
| V-126 | REQ-INT-029 | TEST | Run front-view workbench with disabled or invalid spoof source settings. | Rendering is denied fail-closed with stable denial reason and recovery guidance. |
| V-127 | REQ-CFG-013 | TEST | Load valid and invalid `front_view.*` schema values. | Invalid front-view config keys/ranges fail closed with explicit key-level errors; valid config loads successfully. |
| V-128 | REQ-INT-030 | TEST | Export front-view records during deterministic cycle runs with multi-view enabled. | Each frame record includes deterministic timestamp, frame age, source/stream identifiers, latency breakdown, confidence, provenance, and authorization fields. |
| V-129 | REQ-INT-031 | TEST | Run front-view suite with stabilization/gimbal enabled and invalid stream/stabilization/gimbal combinations. | Valid runs expose stabilization/gimbal metadata; invalid combinations are denied fail-closed with stable reason codes. |
| V-130 | REQ-CFG-014 | TEST | Load invalid and valid `front_view.frame.*`, `front_view.multi_view.*`, `front_view.stabilization.*`, and `front_view.gimbal.*` settings. | Invalid ranges/combinations are rejected with explicit key-level errors; valid settings load and execute deterministically. |
| V-131 | REQ-INT-032 | TEST | Serialize and parse canonical external I/O envelopes across `ie_json_v1` and `ie_kv_v1`, list codec registry descriptors, and convert between both formats by enum and format name. | Round-trip preserves canonical fields deterministically and format discovery reports the supported canonical/alias set. |
| V-132 | REQ-INT-033 | TEST | Parse malformed payloads (invalid JSON, missing required keys, duplicate keys, invalid numeric/bool values). | Conversion/parser fails closed with explicit errors and no partial acceptance. |
| V-133 | REQ-CM-001 | INSPECTION | Review branch naming and PR evidence checklist requirements in workflow docs and PR template/checklist artifacts. | Protected-branch merge path requires standardized branch naming and REQ/V/HZ + deterministic-test evidence before approval. |
| V-134 | REQ-CFG-015 | TEST | Load valid and invalid `policy.role_preset.<role>.*` config entries including unknown roles, missing active-role preset, invalid surface, and invalid display-family lists. | Loader accepts valid role presets and fails closed with explicit key-level issues for invalid role preset data. |
| V-135 | REQ-INT-034 | TEST | Initialize UI context and platform profile transitions with role presets that override surface/front-view defaults. | Runtime status and defaults reflect the active-role preset deterministically on startup and profile changes. |
