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

## Threats (STRIDE)
- Spoofing: fake sensor data or plugin identity.
- Tampering: modified config or logs.
- Repudiation: lack of audit trail for mode changes.
- Information disclosure: logs leaking sensitive data.
- Denial of service: malformed configs or excessive input rates.
- Elevation of privilege: untrusted plugin execution.

## Mitigations and Controls
- Input validation and schema enforcement (REQ-SEC-001, REQ-CFG-001).
- Signed and versioned plugins with allowlists (REQ-SEC-003).
- Explicit authorization gates for plugins/devices (REQ-SEC-002).
- Structured audit logging for mode/config changes (REQ-SEC-004).
- Least privilege for device access and isolated plugin execution.

## Required Security Artifacts
- STIG baseline for target OS.
- SBOM and dependency provenance for each release.
- Security test plan mapping to REQ-SEC requirements.
