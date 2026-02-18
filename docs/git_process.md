# AirTrace Git Process

## Purpose
Define a consistent branch and pull-request workflow for multi-agent execution without branch conflicts.

## Branch Strategy
- `main` is protected and accepts only reviewed pull requests with deterministic green tests.
- Feature branches: `feature/REQ-XXX-short-name`.
- Bugfix branches: `bugfix/V-XXX-short-name`.
- Multi-agent convergence branches (short-lived only): `integration/capability-name`.
- Rebase each active branch on latest `main` before opening a PR and again before merge.

## Agent Branch and Commit Protocol
- Agents may create and commit to `feature/*`, `bugfix/*`, and `integration/*` branches without additional approval when work is traceable to active REQ/V scope.
- Agents must never commit directly to `main`.
- Each agent-owned branch must have a single declared owner in the PR body or handoff note.
- Before commit, agents must confirm:
  - requirement + verification IDs are updated
  - changed files are scoped to one coherent capability increment
  - deterministic tests were run and results recorded
- If a branch needs handoff, the current owner must add a short handoff note listing remaining steps and known risks.

## Commit Standards
- Include requirement and verification IDs in commit subject or body (for example: `REQ-INT-032 V-131`).
- Keep commits scoped to one coherent requirement change set when possible.
- Do not merge documentation-only policy changes without matching traceability updates.

## Agentic Research Publication Controls
- Agentic research is private-by-default in version control.
- Keep only control artifacts tracked under `docs/agents_research/`:
  - `README.md`
  - `backlog.md`
- Raw research notes, exploratory analyses, and archive bundles must remain ignored unless explicitly approved for publication.
- Before publishing any research note:
  - perform security/compliance review (CUI/export-control/legal-policy boundary check)
  - convert findings into formal `REQ-*`, `V-*`, `HZ-*`, and traceability entries
  - sanitize mission-sensitive details and keep only gate-appropriate rationale
- If a file is already tracked but violates this policy, untrack it with `git rm --cached <path>` and keep it local/ignored.

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
