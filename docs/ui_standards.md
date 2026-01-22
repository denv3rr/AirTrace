# UI/TUI Standards and Audit Criteria

This document captures the UI/TUI standards used for AirTrace and the
audit checklist applied during updates. It is based on government and
university sources with unclassified guidance.

## Standards Applied
- MIL-STD-1472H (human factors for displays, controls, and warnings).
  https://quicksearch.dla.mil/qsDocDetails.aspx?ident_number=36092
- NASA-STD-3001 (human factors and habitability).
  https://standards.nasa.gov/standard/nasa/nasa-std-3001
- NIST SP 800-53 Rev 5 (access control, auditability, least privilege).
  https://csrc.nist.gov/pubs/sp/800/53/r5/upd1/final

## Core UI/TUI Principles (Audit Checklist)
1) Clear state and mode indicators at all times.
2) No color-only cues for status, errors, or warnings.
3) Explicit error prompts and recovery actions.
4) Deterministic inputs and repeatable outputs for test modes.
5) Least-privilege role gating for risky actions and overrides.
6) Safe defaults and fail-closed behavior on invalid inputs or menu selections.
7) Denial reasons are surfaced with clear recovery guidance.
8) Test harness inputs must be explicitly enabled and must not bypass authorization checks.

## Implementation Notes
- Menus must show platform profile, active source, and authorization
  status so operators can reason about the system state quickly.
- If a parent profile or child modules are configured, show them in the
  main status line for auditability.
- When fused modes are active, display contributing sensors and their
  confidence or weights.
- Inputs must be validated with bounds and handled safely on EOF or
  stream failure, including a logged exit when menu selection is unavailable.
- Denial warnings should be visible in the main menu status area.
- Harness-driven inputs must follow the same menu flow as operators.
- When the test harness is enabled, menu flows must be allowed even without a TTY.
- All overrides must be logged and shown in UI where relevant.

## Practical Notes
Guidance for defense-facing UI favors high-contrast text, stable layouts,
predictable navigation, and explicit acknowledgements for high-risk actions.
These patterns align with MIL-STD-1472H and are used as baseline criteria.
