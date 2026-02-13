# Security Threat Model

## Scope
- Core tracking library, configuration ingestion, UI/TUI, examples, and platform adapters (official and third-party).

## Assets
- State vectors, sensor measurements, mode decisions, configuration files, simulation outputs.
- Build metadata, audit logs, and verification evidence.
- Provenance tags and run identifiers.

## Trust Boundaries
- File system inputs (configs, logs).
- Operator inputs (TUI).
- Platform adapters and plugin/device interfaces (explicit boundary).

## Entry Points
- Config parser and CLI arguments.
- UI menu selections and scenario inputs.
- Example binaries and scripts.
- Celestial dataset ingestion.
- Network-aid interfaces (when enabled).
- Adapter activation and capability registration.
- Adapter manifest and allowlist files.

## Data Flow Diagram (Text)
Legend: E = External entity, P = Process, D = Data store, TB = Trust boundary.

TB-1 (External Inputs) crosses from operator/files into the app.
TB-2 (Future Network) crosses from network-aid sources into the app.

E1 Operator
E2 Sensors
E3 Network Aid (future)

D1 Config Files
D2 Celestial Dataset
D3 Simulation/Run Logs

P1 UI/TUI
P2 Config Parser + Policy Gate
P3 Sensor Adapters
P4 Core Mode Manager
P5 Tracker/State
P6 Dataset Validator
P7 Audit Log Sink
P8 Provenance Gate

Flows:
E1 -> P1 (menu selections, scenario inputs) [TB-1]
D1 -> P2 (configs, policy, profiles) [TB-1]
E2 -> P3 (measurements) [TB-1]
D2 -> P6 (dataset files + hashes) [TB-1]
P1 -> P4 (mode requests, operator actions)
P2 -> P4 (policy + profile decisions)
P3 -> P8 (normalized measurements + provenance)
P8 -> P4 (authorized measurements)
P6 -> P4 (dataset validation status)
E3 -> P4 (network aids) [TB-2]
P4 -> P5 (selected mode + state updates)
P4 -> P7 (audit events) -> D3 (logs)

## Interface Controls Map
| Interface | Inputs | Trust Boundary | Controls | Requirements | Hazard Links | Verification |
| --- | --- | --- | --- | --- | --- | --- |
| UI/TUI Input | Menu selections, scenario values | TB-1 | Input validation, fail-closed on unavailable input, explicit recovery prompts | REQ-SEC-001, REQ-INT-003, REQ-INT-006 | HZ-007, HZ-034 | V-026, V-066, V-069, V-100 |
| External I/O Envelope Export | Platform, mode, sensor, adapter, policy status fields | TB-1 | Versioned schema, deterministic metadata, explicit reason codes, fail-closed profile validation before export | REQ-INT-025, REQ-INT-026, REQ-INT-027 | HZ-007, HZ-035 | V-122, V-123, V-124 |
| Front-View Display Pipeline | Front-view mode selection, spoofed frame generation, render telemetry | TB-1 | Contract validation for spoof source and mode/cycle inputs, fail-closed denial on invalid frame source, deterministic status export | REQ-INT-028, REQ-INT-029, REQ-CFG-013 | HZ-036 | V-125, V-126, V-127 |
| Config Parser | Config files, policy, profiles | TB-1 | Schema validation, unknown key rejection, version checks | REQ-CFG-001, REQ-FUNC-003, REQ-CFG-012 | HZ-001, HZ-028 | V-009, V-027, V-028, V-117 |
| Sensor Adapters | Sensor measurements | TB-1 | Measurement contract, health/confidence checks, freshness gating | REQ-FUNC-001, REQ-FUNC-012 | HZ-002, HZ-003, HZ-022 | V-007, V-050, V-062 |
| Dataset Validator | Celestial dataset + hashes | TB-1 | Hash verification, versioned artifacts, deny on mismatch | REQ-FUNC-009, REQ-SAFE-005 | HZ-011, HZ-026 | V-035, V-036 |
| Authorization Bundle | policy.authorization.* inputs | TB-1 | Versioned authorization bundle, provenance verification, deny-by-default on unverifiable inputs | REQ-SEC-011, REQ-FUNC-023 | HZ-032, HZ-033 | V-097, V-099 |
| Plugin Authorization Gate | plugin.id/version/signature/allowlist and authorization flags | TB-1 | Deny-by-default plugin activation, explicit authorization required, SHA-256 signature and allowlist version match validation | REQ-SEC-002, REQ-SEC-003 | HZ-019 | V-021, V-022 |
| Network Aid Gate | Network-aid inputs | TB-2 | Deny-by-default, role-based authorization, override logging | REQ-SEC-005, REQ-SEC-006, REQ-SEC-007 | HZ-009, HZ-013 | V-037, V-038, V-043 |
| Audit Log Sink | Mode/config changes | Internal | Structured audit logging, integrity/retention controls, build/config identifiers | REQ-SEC-004, REQ-SEC-009, REQ-SYS-006 | HZ-006, HZ-029 | V-023, V-080, V-081, V-082 |
| Provenance Gate | Measurement provenance tags | TB-1 | Reject mixed/unknown provenance, log decisions | REQ-SYS-017, REQ-SYS-018, REQ-SEC-010 | HZ-031, HZ-032 | V-088, V-089, V-090 |
| Adapter Registry | Adapter identity, capabilities, UI extensions | TB-1 | Signed/allowlisted adapters, versioned contracts, approval-date freshness checks, deny on mismatch | REQ-SEC-012, REQ-SEC-013, REQ-MOD-004, REQ-ADP-002 | HZ-035 | V-107, V-111, V-113, V-118 |

## Threats (STRIDE)
- Spoofing: fake sensor data or plugin identity.
- Tampering: modified config or logs.
- Repudiation: lack of audit trail for mode changes.
- Repudiation: missing provenance acceptance/rejection records.
- Information disclosure: logs leaking sensitive data.
- Denial of service: malformed configs or excessive input rates.
- Elevation of privilege: untrusted plugin execution.
- Spoofing: unauthorized network-aid injection.
- Tampering: celestial dataset modification or downgrade.
- Spoofing: unauthorized sensor contribution into fused modes.
- Tampering: forced ladder reordering or fusion weight manipulation.
- Spoofing: forged front-view frame sources or unsupported rendering mode injection.

## Mitigations and Controls
- Input validation and schema enforcement (REQ-SEC-001, REQ-CFG-001).
- Signed and versioned plugins with allowlists; activation denied when allowlist identity/version/signature do not match (REQ-SEC-003).
- Explicit authorization gates for plugins/devices with deny-by-default enforcement when authorization is missing or not granted (REQ-SEC-002).
- Signed/allowlisted adapter validation with versioned contracts (REQ-SEC-012).
- Allowlist hash validation for adapter manifests; deny on hash mismatch.
- Allowlist approval-date freshness validation with configurable max-age; reject stale or future approvals (REQ-SEC-013, REQ-CFG-012).
- Public-key signature validation is tracked as a waiver until PKI integration is available (docs/waivers.md).
- Structured audit logging for mode/config changes (REQ-SEC-004).
- Audit log integrity and retention protections (REQ-SEC-009).
- Least privilege for device access and isolated plugin execution.
- Deny-by-default network-aid policy with credentialed overrides (REQ-SEC-005, REQ-SEC-006).
- Dataset integrity checks for celestial nav inputs (REQ-FUNC-009).
- Role-based authorization for overrides and policy changes (REQ-SEC-007).
- Versioned authorization bundles with provenance checks; deny-by-default on unverifiable inputs (REQ-SEC-011).
- Fusion source allowlists and authorization checks before contributing to fused modes (REQ-SEC-008).
- Ladder ordering and fusion parameters locked to signed configs with audit logging.
- Test harness input paths are compile-time gated and require explicit runtime enablement.
- External I/O envelope exports are schema-versioned and only produced after profile suite validation.
- Front-view display rendering requires valid `front_view.*` config inputs and fails closed on invalid/unsupported mode, source, or spoof parameters.
- Front-view output includes explicit latency/drop telemetry and denial reason codes for operator recovery.

## Required Security Artifacts
- STIG baseline for target OS.
- SBOM and dependency provenance for each release.
- Security test plan mapping to REQ-SEC requirements.

## External UI Library Security Review (Gate)
Any third-party cockpit/C2/visual display library requires a formal
security review before integration.

Minimum review criteria:
- License compatibility and usage restrictions documented.
- SBOM entry with version pin, source URL, and integrity hash.
- Provenance verified (signed release or reproducible source build).
- No network access or dynamic loading at runtime.
- No telemetry/analytics, hidden network calls, or data exfil paths.
- Deterministic rendering behavior with stable assets.
- Explicit input validation for any UI bindings.
- Static analysis and dependency vulnerability scan results recorded.

Approval evidence must be captured in the security test plan and linked
to REQ-SEC-001/004/009 and REQ-SYS-015.
