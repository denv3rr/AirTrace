# AirTrace Remediation Plan

## Purpose
Align the core/app with the mandatory standards, close sensor fallback gaps, and bring tests, UI, and docs to a verifiable state without duplicating work.

## Phase 1: Requirements, Safety, and Traceability (Gate Before Code)
- Define explicit fallback requirements for each sensor class (when used, how degraded data is labeled, safe-state rules).
- Update `docs/requirements.md`, `docs/verification_plan.md`, and `docs/traceability.md` with new REQ/V IDs.
- Update `docs/hazard_log.md` for fallback-specific hazards and mitigations.
- Add a data flow diagram and control mappings to `docs/security_threat_model.md`.
- Add/refresh `docs/waivers.md` if any coding-standard exceptions are required.
- Reconcile defaults between `docs/config_schema.md` and `include/core/sim_config.h`.
- Define dataset tiering and role-based authorization policy, including override workflows.

### Dataset System Plan (Separate Track)
- Define dataset packaging format, versioning, and hash/signature scheme.
- Define tier sizes and update cadence per platform profile.
- Define offline update workflow and audit logging for dataset installs.
- Add validation tooling to verify dataset integrity at startup.
- Add tests for missing/corrupt datasets and downgrade attacks.

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

## Phase 5: README + Operator Docs
- Rewrite README to be concise, readable, and jargon-light with explicit explanations.
- Add operator-focused sections: modes, safe-state behavior, and configuration basics.
- Confirm all docs link to current artifacts and examples.

## Phase 6: Repo Hygiene
- Clean `.gitignore` for logs, local configs, build outputs, and generated files.
- Ensure sensitive outputs (simulation logs, test logs) are not committed.
- Add a linkable section index to `sources_menu.md` for faster navigation.

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
- Remaining gaps still open: platform profile inheritance usage beyond config/UI in broader core workflows, full UI/TUI audit outside the menu/test flows, and broader documentation simplification.
