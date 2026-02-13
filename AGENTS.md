# AGENTS

All work must comply with the baseline below with no exceptions.
Note `sources_menu.md`and `docs/`.

## Compliance Baseline (Mandatory)
- MIL-STD-498 (systems/software development and documentation)
- MIL-STD-882E (system safety)
- MIL-STD-961E (requirements format and management)
- MIL-STD-1472H (human factors for UI/TUI)
- MIL-STD-810H (environmental considerations for fielded systems)
- NIST SP 800-53 + DoD RMF controls as applicable
- DISA STIG hardening for target platforms
- CERT C++ secure coding + JSF AV C++/MISRA C++ (document any deviations with formal waivers)

## Non-Negotiable Operating Rules
- No code change without a corresponding requirements update and verification plan update.
- All new behavior must be traceable to a requirement ID and a verification case ID.
- Fail closed, safe state by default. If data is missing or invalid, do not proceed silently.
- Determinism is required for simulations and tests (seeded RNG, fixed timesteps).
- No implicit I/O or logging in core libraries; all I/O is routed through a controlled interface.
- No network access or dynamic loading unless explicitly approved in security review.
- No exceptions, shortcuts, or untracked deviations from standards.

## Required Project Artifacts
Create and maintain these files under `docs/` before adding features:
- `docs/requirements.md` with unique, testable requirement IDs (REQ-XXX).
- `docs/verification_plan.md` mapping each REQ-XXX to tests (V-XXX).
- `docs/traceability.md` linking REQ-XXX -> design -> code -> V-XXX.
- `docs/hazard_log.md` with hazard IDs, severity, mitigations, verification.
- `docs/security_threat_model.md` with data flow diagrams and controls.
- `docs/config_schema.md` with versioned schema, units, ranges, defaults.
- `docs/operational_concepts.md` describing modes, device types, and constraints.

## Review Gates (Mandatory)
- SRR (System Requirements Review): requirements complete, testable, reviewed.
- PDR (Preliminary Design Review): architecture, interfaces, hazards, security.
- CDR (Critical Design Review): detailed design and code readiness.
- TRR (Test Readiness Review): verification plan, test data, expected results.
- PRR (Production Readiness Review): reproducible builds, packaging, SBOM.

## Coding Standards (C++ Core)
- Use modern C++17 with deterministic behavior and explicit ownership.
- Follow JSF AV C++ or MISRA C++ rules; document all exceptions in `docs/waivers.md`.
- No raw new/delete unless ownership is clearly documented and bounded.
- No unbounded recursion; all loops must have explicit termination conditions.
- Inputs must be validated with range and unit checks; reject out-of-range values.
- All math units must be explicit (meters, seconds, degrees) in names and docs.
- No `std::cout`/`std::cerr` in core; route logging via a structured logging API.
- Avoid global mutable state; if required, isolate and document.

## Safety Engineering (MIL-STD-882E)
- Maintain hazard analyses (FMEA/FTA) for all sensor and mode logic.
- Define safe-state behavior for sensor dropout, invalid configs, and mode failures.
- Provide explicit transition rules between modes with guard conditions and timeouts.
- Enforce limits (position bounds, velocity, acceleration, turn rate, energy).
- All safety mitigations must be verified with tests.

## Security Engineering (RMF/NIST 800-53)
- All external inputs are untrusted. Validate, sanitize, and log.
- Threat model required before any network, file ingestion, or plugin loading.
- Enforce least privilege and compartmentalization of plugins and device drivers.
- Require signed plugins, allowlists, and versioned ABI compatibility checks.
- Provide SBOM and dependency provenance for every release.

## Configuration Management
- Config files must be schema-validated and versioned; unknown keys are errors.
- Validation must check ranges and inter-field constraints (min <= max, etc).
- Default values must be documented and justified in `docs/config_schema.md`.
- All simulation runs must log config version, seed, and build identifiers.

## Agent Hygiene
- Agent scratch files and local notes must remain untracked and gitignored.
- Only the project-standard artifacts in `docs/` are committed.

## Local Notes (Untracked)
- Config now includes `fusion.min_confidence` and scheduler fields.
- UI status exposes contributors, confidence, and concurrency status.
- Edge case catalog added to `docs/edge_case_catalog.md`.
- Mode lockout config fields added under `mode.*`.
- Trend history window added via `mode.history_window`.
- Disagreement lockout count available via `fusion.max_disagreement_count`.
- Residual alignment window configured via `fusion.max_residual_age_seconds`.
- Front-view config namespace added under `front_view.*` with spoof/cycle/threading controls.
- Next session: add sliding per-sensor residual buffers for time-aligned checks.
- Next session: add edge-case fixtures for GNSS spoofing, IMU bias drift, magnetometer interference.
- Next session: add live TUI warning banners for denial reasons and recovery steps.

## Verification and Validation
- Unit tests for core algorithms and boundaries (100% for safety-critical modules).
- Integration tests for multi-sensor fusion and mode transitions.
- Simulation tests with seeded Monte Carlo for robustness and stress.
- Static analysis (clang-tidy/cppcheck), compiler warnings as errors.
- Optional: sanitizers in CI for memory and undefined behavior checks.

## UI/TUI Standards (MIL-STD-1472H)
- Avoid ambiguous terms, non-standard abbreviations, and color-only cues.
- Provide clear state, mode, and safety status indicators at all times.
- Input validation is mandatory; provide explicit error prompts and recovery paths.

## Plugin and Multi-Device Architecture Rules
- All devices must use a common abstraction layer with explicit capability flags.
- Plugin interface must be versioned, documented, and backward compatible.
- Prefer out-of-process plugins with strict IPC schemas and timeouts.
- Plugin failures must not crash or corrupt core tracking logic.

## Documentation and Traceability
- Every major module must have a design note in `docs/`.
- Every public interface must specify units, range, error behavior, and determinism.
- Every change must update `docs/traceability.md` and `docs/verification_plan.md`.

## Release and Fielding
- Build must be reproducible from a clean checkout.
- Artifacts must include SBOM, build metadata, and test evidence.
- Provide a deployment checklist aligned to RMF and STIG requirements.

## Current Gaps and Remediation Plan (Tracked)
- Platform profile inheritance and child hardware modules are implemented in config parsing and UI status; broader core/module integration remains pending.
- Fallback sensors are modeled in core, but the tracker app does not yet fuse or display sensor health/mode outputs beyond summary status.
- UI/TUI audit remains incomplete outside core menu/test flows; remaining screens need a full pass for validation paths and safe-state messaging.
- Tests now include integration coverage for dropout ladders, dataset gating, and UI menu flows; broader integration and scenario coverage remains limited.
- Front-view display workbench and spoofed display scaffolding are implemented; deeper sensor-specific rendering validation remains pending.
- README has been simplified; further cleanup for clarity and operator focus is still needed.
- Security threat model includes a data flow diagram; control mappings per interface require periodic review and expansion as interfaces evolve.

Remediation details and sequencing are maintained in docs/plan.md.
