# AirTrace Remediation Plan

## Purpose
Align the core/app with the mandatory standards, close sensor fallback gaps, and bring tests, UI, and docs to a verifiable state without duplicating work.

## Modularity and Safety Priority (Continuous)
- Every module must remain independently usable and testable with explicit interfaces and declared dependencies.
- Safety and fail-closed behavior remain first-order requirements for all modules and adapters.

## Next Session Priority (2026-02-13)
- Enforce network-aid deny-by-default and override auditing end-to-end (REQ-SEC-005/006/007).
- Add explicit plugin activation audit records for authorization/signature decisions (REQ-SEC-004, REQ-SEC-002/003).
- Complete full UI/TUI audit outside menu/test-focused flows with explicit fail-closed messaging (REQ-INT-003, REQ-INT-020).
- Begin front-view sensor display scaffolding (EO/IR + 2D/3D proximity views) with deterministic spoofed input support.
- Continue usability/visual mode validation implementation for scenario/test runs (REQ-INT-021/022/023).
- Continue modular interface and packaging boundary enforcement for core/tools/UI/adapters.

## Phase 0: Modularization and Interface Contracts (Gate Before Code)
Goal: Ensure each module is independently usable, with explicit, versioned interfaces and no hidden dependencies.

Scope and Design Updates
- Define module contracts for core, tools, UI, and platform adapters (inputs/outputs, units, error behavior).
- Define dependency rules and allowed import directions; prohibit implicit cross-module I/O or logging.
- Define packaging boundaries (build targets per module) and optional adapter inclusion.
- Define interface versioning, compatibility policy, and ABI/stability expectations.
- Define third-party adapter enablement rules limited to safety, security, and compliance gates.
- Define adapter registry and allowlist requirements, including signed manifests and rejection reason codes.
- Define adapter registry implementation path (config enforcement, allowlist lookup, signature validation, UI surface mapping).
- Define adapter UI mapping flow for baseline + extension data points (REQ-INT-024).

Requirements and Verification Gate (Before Code)
- Reconfirm REQ-SYS-002 and REQ-SYS-015 coverage and update traceability to modular interfaces.
- Add verification cases for module independence (build/use module in isolation).
- Confirm REQ-CFG-010, REQ-INT-024, REQ-MOD-004, and REQ-SEC-012 verification mappings for adapter selection, UI mapping, and registry enforcement.

Exit Criteria
- Module contracts documented in architecture and interface docs.
- Build graph reflects the module boundaries with independent targets.
- Third-party adapter acceptance criteria documented.
- Adapter registry and contract versioning documented.
- Adapter registry implementation plan and UI mapping plan documented with owners and integration points.

## Phase 1: Requirements, Safety, and Traceability (Gate Before Code)
- Define explicit fallback requirements for each sensor class (when used, how degraded data is labeled, safe-state rules).
- Update `docs/requirements.md`, `docs/verification_plan.md`, and `docs/traceability.md` with new REQ/V IDs.
- Update `docs/hazard_log.md` for fallback-specific hazards and mitigations.
- Add a data flow diagram and control mappings to `docs/security_threat_model.md`.
- Add/refresh `docs/waivers.md` if any coding-standard exceptions are required.
- Reconcile defaults between `docs/config_schema.md` and `include/core/sim_config.h`.
- Define dataset tiering and role-based authorization policy, including override workflows.
- Define operational vs simulation provenance tagging and mixed-input rejection rules.

## Phase 1.X: Mode Eligibility Fail-Closed + Policy/Provenance Authorization Integration (Gate Before Code)
Goal: Ensure mode eligibility evaluation fails closed by default and is explicitly bound to policy/provenance authorization, with deterministic denial reasons and safe-state behavior.

Scope and Design Updates
- Define an eligibility evaluation pipeline that treats missing or invalid health, freshness, policy, provenance, or dataset state as deny-by-default.
- Extend authorization decision flow and provenance enforcement in the security threat model, including data flow steps, decision points, and audit requirements.
- Update operator-facing behavior and denial messaging in operational concepts and UI standards.
- Extend schema documentation for policy/provenance authorization inputs and reason codes in the config schema.

Requirements Updates (Before Code)
- Add REQ-SYS-019, REQ-FUNC-022/023, REQ-SAFE-011, REQ-SEC-011, REQ-CFG-009, REQ-INT-018/019/020.
- Update verification cases V-095 through V-100 and traceability links.
- Add hazards HZ-032 through HZ-034 with mitigations and verification.

Exit Criteria (Gate Before Code)
- Requirements, verification plan, traceability matrix, hazard log, and threat model updated.
- Denial reason taxonomy defined and referenced by UI requirements.

## Phase 1.Y: Provenance Tagging + Authorization Auditing + Policy Enforcement (Gate Before Code)
Goal: Implement provenance tagging for all inputs/outputs, auditable authorization decisions, and deny-by-default enforcement for network-aid and plugins.

Scope and Design Updates
- Define provenance tags at the measurement and fused output levels, including propagation rules.
- Define audit record schema for authorization decisions (policy, provenance, network-aid, plugin activation).
- Define deny-by-default behavior for network-aid and plugin/device authorization in operational mode.

Requirements and Verification Gate (Before Code)
- Confirm REQ-SYS-017/018, REQ-SEC-010/011, REQ-SEC-005/006/007, REQ-SEC-002/003, REQ-INT-016 coverage.
- Ensure verification mappings V-088/V-089/V-090/V-037/V-038/V-043/V-021/V-022/V-092 are updated with concrete test inputs.

Implementation Scope
- Add provenance tags to sensor measurements and propagate through fusion outputs.
- Enforce provenance policy in eligibility gating and safe-state transitions.
- Log authorization accept/reject decisions with required metadata fields.
- Enforce network-aid deny-by-default with override auditing.
- Enforce plugin/device authorization and signing checks.

Exit Criteria (Gate Before Code)
- Traceability updated with concrete code references.
- Security threat model and operational concepts updated for provenance and authorization flows.

### Dataset System Plan (Separate Track)
- Define dataset packaging format, versioning, and hash/signature scheme.
- Define tier sizes and update cadence per platform profile.
- Define offline update workflow and audit logging for dataset installs.
- Add validation tooling to verify dataset integrity at startup.
- Enforce dataset integrity in the tools layer and deny-by-default in core gating when validation fails.
- Add tests for missing/corrupt datasets and downgrade attacks.

## Traceability Closure Plan (Gate Before Release)
Goal: Eliminate all traceability "TBD" entries and ensure every requirement maps to code and verification.

Scope
- Enumerate all `TBD` entries in `docs/traceability.md` and assign owners.
- For each REQ with no implementation, either implement or explicitly mark "not implemented" with rationale.
- Update `docs/verification_plan.md` with concrete test inputs and expected outcomes for newly implemented items.

Exit Criteria
- No `TBD` entries remain in `docs/traceability.md`.
- Verification evidence exists for each mapped requirement.

## Phase 2: Sensor Fallback Modules (Core)
- Define a platform hierarchy: a shared parent module with child modules for air, ground, maritime, space, and handheld.
- Implement fallback modules per sensor (GPS, thermal, radar, IMU, dead-reckoning, vision, lidar, baro, magnetometer, celestial).
- Use deterministic math functions from `TrackerComponentLibrary` where appropriate.
- Expose degraded measurements explicitly (flags/notes) and keep fail-closed behavior for invalid data.
- Integrate fallback decisions with `ModeManager` and sensor health.
- Add a navigation fallback ladder that prefers permissible methods and enables celestial navigation only when higher-priority sources are unavailable.
- Define network-aid authorization rules per platform and mission profile (no broadcast unless permitted).

### Multi-Modal Switching Workstream (Gate Before Code)
Goal: Implement fused modes and switching logic aligned to the fallback ladder with traceability, safety, and policy compliance.

Requirements and Traceability
- Add explicit requirements for fused modes, eligibility gating, and operator visibility (REQ-SYS-010, REQ-FUNC-012..015, REQ-SAFE-006, REQ-SEC-008).
- Update verification cases and traceability links before implementation.
- Extend hazard log for fused-mode discontinuities and sensor disagreement.

Design and Architecture
- Define a fused-mode catalog: GNSS+INS, VIO/LIO, radar/terrain-aided, magnetometer+baro.
- Specify eligibility inputs: health, data freshness, confidence, policy, dataset availability.
- Define transition guard conditions and state handoff rules (alignment checks, reset behavior).
- Add a deterministic fusion strategy with explicit weighting and confidence outputs.
- Identify the minimal interface changes for mode, health, and contributor reporting.

Configuration and Policy
- Extend config schema with ladder ordering per profile and mission policy overrides.
- Add thresholds for disagreement detection and fusion downgrade.
- Define policy gating for each aiding source (deny-by-default).

Implementation Plan
- Update `ModeManager` to support composite modes and multiple contributors.
- Implement sensor contributor registry with per-source confidence and freshness tracking.
- Add fusion arbitration logic and state alignment checks.
- Ensure fail-closed behavior for missing datasets, invalid policy, or unknown sources.

UI and Operator Feedback
- Display active mode plus contributing sensors and confidence/weights.
- Add explicit prompts for policy-denied sources and downgrade triggers.
- Ensure safe-state messaging is visible and non-ambiguous.

Verification and Test Readiness
- Unit tests for fused mode eligibility and downgrade rules.
- Integration tests for ladder switching under mixed sensor health.
- Deterministic simulation fixtures for each fused mode with golden outputs.
- TRR: document test data, expected results, and acceptance thresholds.

## Phase 3: Verification Coverage
- Unit tests for each fallback path, sensor dropout, false positives, and range limits.
- Mode transition tests covering hysteresis, dwell times, and hold mode.
- Config validation tests for schema/version/range and unknown keys.
- Deterministic simulation fixtures (seeded runs) with golden outputs.

## Phase 4: UI/TUI Improvements
- Add explicit mode, sensor health, and safety-state indicators (no color-only cues).
- Provide clear error prompts and recovery guidance on invalid input.
- Add deterministic test inputs (seed selection) to UI simulation flows.
- Ensure UI logging does not write outside approved locations.
- Add explicit operator abort controls and confirmation prompts for destructive actions.
- Add units and reference frames to all operator prompts and outputs.

### UI/TUI Safety Gap Closure (Per-Sensor Health, Ladder Status, Denials)
Goal: Ensure operators can see per-sensor health, fallback ladder state, and explicit denial reasons with recovery guidance.

Requirements and Verification Gate (Before Code)
- Add UI requirements for per-sensor health detail, fallback ladder state, and denial reason taxonomy with recovery steps.
- Add verification cases for deterministic display content and mapping from internal reason codes to operator text.

Design and Data Contract
- Define a UI status contract for per-sensor health fields (health, freshness, confidence, lockout, last_update_age_seconds).
- Define a fallback ladder status payload (ordered sources, eligibility, disqualifier reasons, and active selection).
- Define denial reason taxonomy with stable IDs and recovery actions for safe-state entry, policy denial, and data invalidation.

Implementation Scope
- Add UI/TUI panels and banners for per-sensor health list and ladder status.
- Surface disqualified sources with explicit reasons (health, authorization, freshness, lockout).
- Ensure all messages avoid color-only cues and include recovery guidance.

Verification and TRR Artifacts
- Add deterministic UI/TUI tests that validate mapping of reason codes to display text.
- Add integration tests for ladder selection transitions showing accurate per-sensor status and denial reasons.
- Update traceability for new requirements and verification IDs before merge.

### Usability and Visual Mode Validation (Scenario/Test)
Goal: Ensure scenario and test runs can rapidly validate mode behavior via deterministic visual mode cycling and multi-mode views.

Requirements and Verification Gate (Before Code)
- Add REQ-INT-021/022/023 with verification cases V-101/V-102/V-103.
- Confirm UI contract stability requirements are reflected in UI standards and traceability.

Design and Data Contract
- Define a visual mode cycle mapping between evaluated tracking modes and UI panels.
- Define multi-mode view layout and per-mode status fields (contributors, confidence, denial).
- Define UI interface contract versioning and compatibility checks.

Implementation Scope
- Add a visual mode cycle controller for scenario/test runs with deterministic sequencing.
- Support optional multi-mode views for concurrent mode evaluation.
- Add interface contract validation to prevent breaking UI updates.

Verification and TRR Artifacts
- Add automated test to validate deterministic visual cycling behavior.
- Add demo/inspection for multi-mode view and UI contract versioning.
- Re-run UI/TUI audit checklist against sources in `sources_menu.md` section O.

## Phase 5: README + Operator Docs
- Rewrite README to be concise, readable, and jargon-light with explicit explanations.
- Add operator-focused sections: modes, safe-state behavior, and configuration basics.
- Confirm all docs link to current artifacts and examples.

## Phase 6: Repo Hygiene
- Clean `.gitignore` for logs, local configs, build outputs, and generated files.
- Ensure sensitive outputs (simulation logs, test logs) are not committed.
- Add a linkable section index to `sources_menu.md` for faster navigation.

## Perpetual Plan (Rolling)

Purpose: Maintain a continuously updated, priority-ordered backlog with ownership, prerequisite gates, and verification mapping. Every phase closure must result in a new plan increment.

Rules
- No code work without the corresponding requirements and verification plan updates.
- Each backlog item must include REQ IDs, V IDs, and hazard references if applicable.
- Each completed phase must spawn a new "next plan" entry for the subsequent phase or gap.

Rolling Backlog (Initial)
- Close REQ-INT-010 duplication and restore unique requirement numbering.
- Fill traceability `TBD` code links where implementation exists; mark as not implemented where missing.
- Complete UI/TUI safety visibility (per-sensor health, ladder status, denial reasons).
- Integrate policy/provenance authorization into mode eligibility (deny-by-default).
- Implement provenance tagging and mixed/unknown rejection in core and UI (REQ-SYS-017/018, REQ-INT-016).
- Implement authorization bundle parsing and enforcement (REQ-CFG-009, REQ-SEC-011).
- Implement plugin authorization/signing gates (REQ-SEC-002/003).
- Enforce network-aid deny-by-default and credentialed overrides (REQ-SEC-005/006/007).
- Implement deterministic replay logging and restore (REQ-FUNC-004).
- Define and measure deterministic performance budget for fixed inputs (REQ-PERF-001).
- Implement adapter registry + allowlist enforcement with contract version checks and audit logging (REQ-MOD-004, REQ-SEC-012, REQ-ADP-006).
- Enforce adapter selection and UI surface config keys in tools parsing (REQ-CFG-010).
- Implement adapter UI mapping for baseline + extensions and surface rendering (REQ-INT-024, REQ-ADP-003).
- Add official adapter skeletons with signed manifests and UI extension mappings per platform (REQ-ADP-001/002/005).
- Implement public-key signature verification for adapter manifests and retire WVR-001.

## Perpetual Plan Addendum (Gap Closure Subplans)
### Gap Closure A: REQ-SEC-010 Audit Fields and Run Context
Objective: Ensure audit records for provenance accept/reject include reason codes, source identifiers, run ID, config version, config ID, build ID, and seed.
Prereqs: Confirm audit record schema in `docs/security_threat_model.md` and audit sink contract.
Implementation Targets: `src/tools/audit_log.cpp`, `include/tools/audit_log.h`, `src/ui/simulation.cpp`.
Verification: V-090 with explicit field presence and value checks.
Traceability Update: Replace REQ-SEC-010 TBD with concrete code references.

### Gap Closure B: REQ-SEC-002/003 Plugin Authorization and Signing
Objective: Enforce explicit authorization and signed plugin validation with deny-by-default behavior.
Prereqs: Define plugin allowlist, signing manifest format, and versioned ABI check requirements.
Implementation Targets: `src/core/plugin_auth.cpp`, `include/core/plugin_auth.h`, plugin loader entrypoint.
Verification: V-021 plugin authorization denial test, V-022 signature mismatch rejection and signed allowlisted activation test.
Traceability Update: Replace REQ-SEC-002/003 TBD entries with code references.

### Gap Closure C: REQ-SEC-005/006/007 Network-Aid Deny-by-Default
Objective: Enforce deny-by-default network-aid usage with credentialed overrides and role checks.
Prereqs: Document override credentials and approval workflow in `docs/security_threat_model.md`.
Implementation Targets: `src/core/network_aid_policy.cpp`, `include/core/network_aid_policy.h`, enforcement call sites.
Verification: V-037 deny-by-default, V-038 override without credentials denied and logged, V-043 override with insufficient role denied and logged.
Traceability Update: Replace REQ-SEC-005/006/007 TBD entries with code references.

### Gap Closure D: REQ-FUNC-004 Deterministic Replay
Objective: Provide deterministic replay using logged seeds/configs and stable output comparison.
Prereqs: Define replay format and output equivalence rules in `docs/operational_concepts.md`.
Implementation Targets: `src/core/replay.cpp`, `include/core/replay.h`, UI or CLI entrypoint.
Verification: V-010 replay matches original output deterministically.
Traceability Update: Replace REQ-FUNC-004 TBD with code references.

### Gap Closure E: REQ-PERF-001 Deterministic Timing Budget
Objective: Define a deterministic timing budget and measure update runtime under fixed load.
Prereqs: Document timing budget in `docs/architecture.md` with platform constraints.
Implementation Targets: `tests/perf_timing.cpp` or `tests/core_sanity.cpp`.
Verification: V-014 measurement under fixed load meets documented budget.
Traceability Update: Replace REQ-PERF-001 TBD with code references.

### Gap Closure F: Usability and Visual Mode Validation Implementation
Objective: Implement auto-visual mode cycling and multi-mode visualization for scenario/test runs.
Prereqs: UI contract versioning and visual mode mapping defined in `docs/ui_standards.md`.
Implementation Targets: `src/ui/simulation.cpp`, `src/ui/scenario.cpp`, `include/ui/ui_contract.h`.
Verification: V-101 deterministic mode cycling, V-102 multi-mode view demo, V-103 UI contract inspection.
Traceability Update: Replace REQ-INT-021/022/023 TBD entries with code references.

### Gap Closure G: Adapter Registry + UI Mapping + Official Skeletons
Objective: Implement adapter registry enforcement, config selection, and UI mapping with official adapter skeletons.
Prereqs: Adapter contract and UI data-point baseline are finalized in `docs/adapter_contract.md` and `docs/adapters/ui_data_points.md`.
Implementation Targets: `src/core/adapter_registry.cpp`, `include/core/adapter_registry.h`, `src/ui/adapter_ui_mapping.cpp`, `include/ui/adapter_ui_mapping.h`, `adapters/official/*`.
Verification: V-104 UI extensions render, V-107 version mismatch denied, V-109/110 official adapter inspection, V-113 allowlist enforcement.
Traceability Update: Replace REQ-INT-024/REQ-ADP-001..006/REQ-MOD-004/REQ-SEC-012 TBD entries with code references.

### Gap Closure H: Front-View Sensor Display Pipeline and Cycling
Objective: Provide a professional, cross-platform front-view display subsystem that supports EO/IR rendering modes, deterministic cycling, and 2D/3D proximity views from normalized sensor data.
Prereqs: Finalize ingress/egress display contract fields and add configuration schema entries for display modes, cycling, and latency budgets.
Design Note: `docs/front_view_display_architecture.md`.
Implementation Targets: `src/ui/simulation.cpp`, `src/ui/scenario.cpp`, `include/ui/simulation.h`, `include/core/external_io_envelope.h`, new UI display pipeline module files.
Verification: Deterministic spoofed frame replay, mode-cycle correctness, latency/drop reason telemetry, UI/TUI fail-closed messaging checks.
Traceability Update: Add REQ/VER mappings for front-view display contracts, mode cycling, and proximity views once implementation starts.

## Perpetual Execution Framework (Autonomous)
Purpose: Allow continuous project advancement without waiting for low-value prompt-by-prompt direction while still honoring review gates and standards.

Cadence:
- Weekly engineering cycle: execute one highest-risk safety/security item, one modularization item, and one verification coverage item.
- Biweekly review package: publish SRR/PDR/CDR/TRR delta checklist with REQ/V/HZ changes and evidence links.
- Monthly hardening cycle: run static-analysis, dependency provenance review, and adapter allowlist/surface contract audits.

Default Prioritization Rules:
- Priority 1: any fail-open behavior, undefined behavior, or stale/unknown authorization path.
- Priority 2: missing traceability links or verification evidence for already implemented behavior.
- Priority 3: modular interface completion and adapter platform expansion.
- Priority 4: usability refinements after safety/security acceptance criteria are green.

Autonomous Decision Policy:
- If a change touches parser, scheduler, mode logic, or adapter registry: require simultaneous updates to `docs/requirements.md`, `docs/verification_plan.md`, and `docs/traceability.md`.
- If a new deny reason appears: update `docs/hazard_log.md`, `docs/security_threat_model.md`, and UI denial mapping in the same change set.
- If a behavior cannot be fully implemented in one increment: ship the fail-closed guard first, then feature-complete in the next scheduled cycle.

Rolling Work Queues:
- Queue A Safety/Security:
  - Keep REQ-SEC-002/003 plugin authorization/signing gate coverage green and add explicit audit events for plugin activation decisions.
  - Complete REQ-SEC-005/006/007 network-aid deny-by-default enforcement path.
  - Retire waiver for public-key manifest signatures after PKI validation path exists.
- Queue B Modularization:
  - Finish independent build targets for core/tools/UI/adapters (REQ-MOD-001/002).
  - Add dependency graph enforcement and CI gate for undeclared module edges (REQ-MOD-003).
  - Add out-of-process adapter execution scaffold with timeout/error isolation.
- Queue C Verification:
  - Add dedicated adapter registry test suite for reason-code reachability and surface gating.
  - Add deterministic replay harness for REQ-FUNC-004.
  - Add performance-budget fixture and evidence capture for REQ-PERF-001.
- Queue D UI/TUI:
  - Complete REQ-INT-021/022/023 visual mode cycling and multi-mode rendering.
  - Implement front-view EO/IR display scaffolding with deterministic spoofed-input harness support.
  - Add 2D/3D proximity toggle views with explicit units, confidence, and denial reason rendering.
  - Expand denial banners to include adapter approval freshness, plugin/network-aid denial context, and recovery guidance.
  - Complete full-screen UI/TUI audit outside menu/test paths.

Per-Change Exit Criteria:
- Source change merged with matching REQ, V, and traceability entries.
- Safety/security impacts logged with hazard/threat links.
- Tests pass in deterministic mode with seeded fixtures.
- Outstanding risk or deferment explicitly recorded in `docs/plan.md`.

## Notes
- No code changes may proceed until Phase 1 updates are merged.
- Keep all new additions ASCII unless the file already uses Unicode.
- Keep the external incident source list in `docs/edge_case_catalog.md` updated as part of each AGENTS pass.

## Current Status Update (Do Not Delete Incomplete Items)
- Menu selection validation, UI denial banners, and harness-based inputs are in place.
- Build/test/run scripts validate build type inputs and fail closed.
- Edge-case catalog expanded with conflict-zone interference entries.
- Platform profile inheritance and child hardware modules are implemented in config parsing, validation, and UI status display.
- Integration tests for dropout ladders, dataset gating, and UI menu flows are in place.
- UI/TUI validation paths now fail closed on invalid selections and report file I/O errors with recovery guidance.
- Audit logging sink, health reporting, and fail-closed gating are in place for operational/test flows.
- Status banners, abort controls, and destructive confirmations are enforced in simulation flows with audit trails.
- Provenance policy keys now parsed and validated in config schema (REQ-CFG-008).
- Requirements updated to remove duplicate REQ-INT-010 and add eligibility/authorization and UI visibility requirements.
- Single-UI platform workbench now supports selected-profile and all-profile suite execution with deterministic external I/O envelope export (REQ-INT-025/026/027).
- Authorization bundle inputs are parsed/validated and mode eligibility now denies when authorization is unavailable or denied.
- UI status banner now includes ladder status and per-sensor lockout details.
- Provenance tagging added to measurements with eligibility gating and UI provenance status display.
- Adapter architecture and official platform adapter design notes are documented under `docs/adapter_architecture.md` and `docs/adapters/`.
- Tools layer scaffolded with config loader and audit log ownership; adapter SDK skeleton created under `adapters/sdk/`.
- Adapter registry loader validates manifests/allowlists with hash checks; adapter config keys and UI extension summaries are enforced; official adapter manifests and allowlist are present under `adapters/`.
- Plugin authorization/signing gate is now enforced through `plugin.*` schema + core validator with fail-closed reason codes for unauthorized, non-allowlisted, or signature-invalid activation attempts.
- Front-view display architecture draft is now documented in `docs/front_view_display_architecture.md` with EO/IR mode families, deterministic cycling, spoofed-input strategy, and 2D/3D proximity rendering plan.
- Front-view display scaffolding is now implemented with a dedicated UI module, menu workbench flow, `front_view.*` config validation, and external I/O envelope telemetry fields.
- Front-view Stage-1 contract hardening is implemented with deterministic frame timestamp/age, latency-stage breakdown, stream identity, stabilization/gimbal metadata, and fail-closed validation for invalid frame/stream/stabilization/gimbal conditions.
- Remaining gaps still open: platform profile inheritance usage beyond config/UI in broader core workflows, full UI/TUI audit outside the menu/test flows, provenance tagging for sim vs operational inputs, and broader documentation simplification.
