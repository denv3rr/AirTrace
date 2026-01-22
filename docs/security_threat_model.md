# Security Threat Model

## Scope
- Core tracking library, configuration ingestion, UI/TUI, examples, and future plugin/device interfaces.

## Assets
- State vectors, sensor measurements, mode decisions, configuration files, simulation outputs.
- Build metadata, audit logs, and verification evidence.

## Trust Boundaries
- File system inputs (configs, logs).
- Operator inputs (TUI).
- Future plugin/device interfaces (explicit boundary).

## Entry Points
- Config parser and CLI arguments.
- UI menu selections and scenario inputs.
- Example binaries and scripts.
- Celestial dataset ingestion.
- Network-aid interfaces (when enabled).

## Data Flow Diagram (Text)
[Operator] -> [UI/TUI] -> [Core Mode Manager] -> [Tracker/State]
[Config File] -> [Config Parser] -> [Policy + Platform Profile] -> [Core Mode Manager]
[Sensors] -> [Sensor Adapters] -> [Measurement Contract] -> [Core Mode Manager]
[Celestial Dataset] -> [Dataset Validator] -> [Celestial Nav Module]
[Network Aid] -> [Authorization Gate] -> [Core Mode Manager]
[Core Mode Manager] -> [Audit Log Sink]

## Threats (STRIDE)
- Spoofing: fake sensor data or plugin identity.
- Tampering: modified config or logs.
- Repudiation: lack of audit trail for mode changes.
- Information disclosure: logs leaking sensitive data.
- Denial of service: malformed configs or excessive input rates.
- Elevation of privilege: untrusted plugin execution.
- Spoofing: unauthorized network-aid injection.
- Tampering: celestial dataset modification or downgrade.
- Spoofing: unauthorized sensor contribution into fused modes.
- Tampering: forced ladder reordering or fusion weight manipulation.

## Mitigations and Controls
- Input validation and schema enforcement (REQ-SEC-001, REQ-CFG-001).
- Signed and versioned plugins with allowlists (REQ-SEC-003).
- Explicit authorization gates for plugins/devices (REQ-SEC-002).
- Structured audit logging for mode/config changes (REQ-SEC-004).
- Least privilege for device access and isolated plugin execution.
- Deny-by-default network-aid policy with credentialed overrides (REQ-SEC-005, REQ-SEC-006).
- Dataset integrity checks for celestial nav inputs (REQ-FUNC-009).
- Role-based authorization for overrides and policy changes (REQ-SEC-007).
- Fusion source allowlists and authorization checks before contributing to fused modes (REQ-SEC-008).
- Ladder ordering and fusion parameters locked to signed configs with audit logging.

## Required Security Artifacts
- STIG baseline for target OS.
- SBOM and dependency provenance for each release.
- Security test plan mapping to REQ-SEC requirements.
