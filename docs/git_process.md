# AirTrace Git Process

## Purpose
Define a consistent branch and pull-request workflow for multi-agent execution without branch conflicts.

## Branch Strategy
- `main` is protected and accepts only reviewed pull requests with deterministic green tests.
- Feature branches: `feature/REQ-XXX-short-name`.
- Bugfix branches: `bugfix/V-XXX-short-name`.
- Multi-agent convergence branches (short-lived only): `integration/capability-name`.
- Rebase each active branch on latest `main` before opening a PR and again before merge.

## Commit Standards
- Include requirement and verification IDs in commit subject or body (for example: `REQ-INT-032 V-131`).
- Keep commits scoped to one coherent requirement change set when possible.
- Do not merge documentation-only policy changes without matching traceability updates.

## Pull Request Evidence Checklist
- Linked `REQ-*` IDs.
- Linked `V-*` IDs.
- Hazard/security impact summary (`HZ-*` and threat-model impact when applicable).
- Deterministic test evidence (`scripts/test.ps1` or equivalent) with pass/fail summary.
- Changed artifact list (`docs/requirements.md`, `docs/verification_plan.md`, `docs/traceability.md`, and affected code paths).

## Conflict Avoidance for Multiple Agents
- One owner per branch per requirement increment.
- No direct pushes to another agent's branch without explicit handoff.
- If multiple agents touch one capability, converge via `integration/*`, then split to clean `feature/*` PRs.
- Prefer small-lived branches (target 1 to 5 days) to reduce rebase drift.

## Approval Practices
- Required reviewer checks:
  - standards alignment and fail-closed behavior
  - requirement/verification/traceability completeness
  - deterministic test pass evidence
- Merge approval is denied if PR checklist evidence is incomplete.

