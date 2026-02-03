# Adapter Architecture and Official Platforms

## Purpose
Define the official platform adapter expectations, interface contracts, and
safety/security boundaries. Adapters are modular, independently deployable
extensions that provide platform-specific I/O bindings and UI data points
without compromising core determinism or safety behavior.

## Official Adapter Set (Initial)
The following platform profiles require official adapters:
- air
- ground
- maritime
- space
- handheld
- fixed_site
- subsea

Official adapter design notes live in `docs/adapters/`.

Each adapter may provide one or more UI surfaces:
- cockpit UI
- remote operator UI
- C2 display
- TUI (ASCII)

## Adapter Contract (Versioned)
Inputs:
- Configuration and policy bundle (validated, versioned).
- Provenance and authorization context.
- Sensor capability declarations (allowlisted).

Outputs:
- Normalized measurements using the core measurement contract.
- Adapter health and capability state.
- UI extension fields (platform-specific data points).

Contract rules:
- Interfaces are versioned and fail closed on mismatch.
- Adapters must declare all capabilities and UI extensions up front.
- Unknown capabilities or fields are rejected with explicit reason codes.
- Third-party adapters are accepted when they pass contract, safety, and security gates.

## Adapter Registry and Allowlist
- Adapters are registered via a controlled registry that enforces allowlists.
- Each adapter provides a signed manifest with:
  - adapter.id
  - adapter.version
  - adapter.contract_version
  - ui.contract_version
  - supported core/tools/ui version ranges
  - declared capabilities and UI extensions
- Registry rejects adapters that are unsigned, not allowlisted, or out of
  compatibility range. Rejections are logged with explicit reason codes.
- Allowlist default location: `adapters/allowlist.json` (override via config).
- See docs/adapter_contract.md for manifest and allowlist schemas.

## Contract Versioning
- `adapter.contract_version` governs the measurement and capability contract.
- `ui.contract_version` governs the baseline UI data-point contract.
- Backward-compatible changes increment MINOR; breaking changes increment MAJOR.
- Adapters must declare the exact contract versions they implement.
- Core/UI must deny activation if versions are incompatible.
- See docs/adapter_contract.md for validation steps and reason codes.

## Build Targets (Current + Planned)
Current:
- core: `airtrace_core`
- tools: `airtrace_tools`

Planned:
- ui: `airtrace_ui`
- adapters: `airtrace_adapter_<platform>`

Each adapter produces an artifact with:
- signed manifest
- versioned contract metadata
- SBOM entry and provenance metadata

## UI Data Points
Baseline (must always be present across all surfaces):
- Mode/source, contributors, confidence.
- Per-sensor health/availability/freshness/confidence.
- Fallback ladder state with disqualification reasons.
- Lockout status and denial reasons with recovery guidance.
- Authorization and provenance status.

Adapter-specific UI extensions:
- Must declare units, ranges, and error behavior.
- Must be mapped to each supported UI surface.
- Must follow human factors and safety guidance.
- See `docs/adapters/ui_data_points.md` for the baseline contract.

## Safety and Fail-Closed Behavior
- Adapter failures or invalid data must trigger safe-state entry.
- Missing or invalid adapter fields must deny activation.
- Adapter health is monitored and surfaced in UI status and audit logs.

## Security and Integrity
- Adapters are signed, allowlisted, and validated before activation.
- Unauthorized or incompatible adapters are denied and logged.
- No dynamic loading or network access without explicit authorization.

## Environmental and Human Factors
- Each official adapter must document environmental constraints (MIL-STD-810H).
- UI extensions must follow MIL-STD-1472H and applicable symbology standards
  (e.g., MIL-STD-2525 where required).

## Configuration (Defined, Enforcement Planned)
Config keys are defined in `docs/config_schema.md`:
- adapter.id (string)
- adapter.version (string)
- adapter.manifest_path (string)
- adapter.allowlist_path (string)
- ui.surface (string)

Parser enforcement, registry validation, and allowlist checks are implemented in the tools layer.

## Traceability
- REQ-MOD-001..004
- REQ-ADP-001..005
- REQ-SEC-012
- REQ-SAFE-012
- REQ-INT-024
