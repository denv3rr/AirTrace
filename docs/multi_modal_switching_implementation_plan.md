# Multi-Modal Switching Implementation Plan

## Purpose
Define the concrete implementation steps, interfaces, and test scaffolding to
deliver multi-modal switching with fused modes and concurrent pipelines while
preserving determinism, safety, and policy gating.

## Scope and Constraints
- No new behavior without requirement and verification updates.
- Deterministic scheduling and seeded RNG only.
- Fail-closed on invalid policy, missing datasets, or unknown sources.
- No implicit I/O in core libraries.

## Target Modules and Responsibilities
Core:
- Mode manager: eligibility evaluation, ladder selection, dwell rules.
- Sensor registry: per-sensor health, freshness, confidence, policy allowed.
- Fusion controller: contributor selection, disagreement checks, confidence output.
- Scheduler: concurrent pipeline arbitration and budget enforcement.

UI/TUI:
- Display active mode, contributors, confidence/weights.
- Display concurrency status and policy denials.

Config:
- Parse ladder order, fusion thresholds, and scheduler budgets.

## Data Structures (Planned)
- ModeDefinition
  - name
  - required_sensors
  - optional_sensors
  - policy_tags
  - type (primary/fused/aux_snapshot)
- SourceStatus
  - name
  - health
  - data_age_seconds
  - confidence
  - policy_allowed
  - last_update_tick
- ModeDecision
  - selected_mode
  - contributors
  - confidence
  - reason
  - downgrade_reason
- PipelineBudget
  - reserved_ms
  - max_outstanding
  - min_service_interval

## Config Schema Extensions (Implementation)
- mode.ladder_order
- mode.min_healthy_count
- mode.min_dwell_steps
- fusion.max_data_age_seconds
- fusion.disagreement_threshold
- fusion.source_weights
- scheduler.primary_budget_ms
- scheduler.aux_budget_ms
- scheduler.max_aux_pipelines
- scheduler.aux_min_service_interval
- scheduler.allow_snapshot_overlap

## Implementation Steps
1) Config Parsing and Validation
   - Extend `SimConfig` for fusion and scheduler fields.
   - Validate ranges and inter-field constraints.
   - Reject unknown keys (fail closed).

2) Sensor Registry and Status Tracking
   - Maintain per-sensor freshness and confidence.
   - Mark policy_allowed based on role and platform profile.

3) Mode Definitions and Ladder Ordering
   - Define explicit modes for gps_ins, vio, lio, radar_inertial, mag_baro.
   - Build ladder from config or profile defaults.

4) Eligibility Evaluation
   - Combine health, freshness, confidence, and policy gates.
   - For fused modes, require all mandatory contributors.

5) Disagreement Detection
   - Compute residuals between contributors.
   - If thresholds exceeded, mark conflict and downgrade.

6) Concurrent Scheduling
   - Primary pipeline reserved budget.
   - Auxiliary snapshots scheduled in deterministic round-robin.
   - Preserve scan continuity by deferring snapshots under pressure.

7) Mode Transition and Alignment
   - Alignment checks on fused mode entry.
   - Dwell and guard conditions enforced.

8) UI/TUI Reporting
   - Expose contributors, confidence, and concurrency status.
   - Provide explicit downgrade and policy denial prompts.

## Testing Plan (Scaffold)
Unit tests:
- Eligibility gating for health, freshness, confidence, policy.
- Fused mode activation only with required contributors.
- Disagreement downgrade behavior.
- Scheduler determinism and budget enforcement.

Integration tests:
- Concurrent pipelines: IR snapshot + lidar snapshot + GNSS+INS.
- Load spikes causing auxiliary pausing without primary interruption.
- Policy deny on auxiliary sources.

Determinism tests:
- Repeatable outputs with same seed and config.

## Acceptance Criteria
- All REQ-SYS-010..011 and REQ-FUNC-012..017 pass with V-049..V-059.
- Concurrency does not interrupt primary scans or violate safety constraints.
- Operator UI displays active mode, contributors, and concurrency status.
