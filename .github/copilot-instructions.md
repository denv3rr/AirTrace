# Copilot Instructions for AirTrace

Use this repository for deterministic sensor and mode-switch integration testing only.

## Hard Constraints

- Do not generate weapon targeting logic.
- Do not generate weapon employment logic.
- Do not generate operational intercept algorithms.

## Standards Authority

- `AGENTS.md` is mandatory.
- Use only valid standards sources listed in `sources_menu.md` (especially Section Q for compliance baselines).
- Keep `docs/requirements.md`, `docs/verification_plan.md`, and `docs/traceability.md` updated for behavior changes.

## Quality Gates

- Run deterministic test suites before merge:
  - Windows: `.\scripts\test.ps1`
  - Linux/macOS: `./scripts/test.sh`

## Safety and Failsafe Rules

- Fail closed on missing/invalid data.
- Preserve deterministic execution characteristics.
- Treat all external input as untrusted and validate explicitly.
