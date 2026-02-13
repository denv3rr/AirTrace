# Adapter UI Data-Point Contract

This document defines the baseline UI data points and the rules for
adapter-specific extensions. All UI surfaces must render the baseline
contract consistently. Adapter extensions must follow the same validation,
units, and recovery guidance rules.

## Baseline UI Contract (All Surfaces)

| Field ID | Description | Units | Range | Error Behavior |
| --- | --- | --- | --- | --- |
| ui.contract_version | UI contract version. | semver | non-empty | Missing -> deny in operational runs. |
| adapter.id | Active adapter identifier. | none | allowlisted | Unknown -> deny. |
| adapter.extension_version | Adapter extension contract version. | semver | non-empty | Missing -> deny when adapter is active. |
| platform.profile | Active platform profile. | none | enumerated | Unknown value is an error and must deny. |
| platform.parent_profile | Parent profile if configured. | none | enumerated | Missing is allowed; invalid is an error. |
| platform.child_modules | Child module identifiers. | none | list | Invalid identifiers are errors. |
| mode.active | Active mode name. | none | enumerated | Unknown value is an error. |
| mode.contributors | Contributing sources for the active mode. | none | list | Empty is allowed only in hold mode. |
| mode.confidence | Mode confidence or weight summary. | 0-1 | [0, 1] | Out of range is an error. |
| mode.decision_reason | Decision reason code. | none | stable ID | Unknown value is an error. |
| mode.denial_reason | Denial reason code. | none | stable ID | Empty only when no denial. |
| fallback.ladder_status | Ordered ladder with eligibility/disqualification. | none | structured text | Missing is an error for operational runs. |
| fallback.disqualified | Disqualified sources with reason codes. | none | list | Empty allowed when none disqualified. |
| lockout.status | Lockout list with remaining steps and reasons. | none | list | Empty allowed when none locked. |
| sensors.summary | Per-sensor health/age/confidence summary. | mixed | per field | Invalid values are errors. |
| auth.status | Authorization status summary. | none | stable text | Unknown is an error in operational runs. |
| provenance.status | Provenance policy status summary. | none | stable text | Unknown is an error in operational runs. |
| logging.status | Audit log status. | none | stable text | Unknown is an error in operational runs. |
| run.seed | Seed used for deterministic runs. | uint32 | [0, 4294967295] | Missing is an error in simulation/test. |
| run.deterministic | Determinism flag. | bool | true/false | False is an error in simulation/test. |

## External I/O Envelope Contract

The UI exports a machine-readable envelope for cross-system integration.
The envelope schema is versioned and backward-compatible.

| Field ID | Description | Units | Range | Error Behavior |
| --- | --- | --- | --- | --- |
| schema_version | Envelope schema version identifier. | semver | non-empty | Missing -> reject payload. |
| interface_id | Stable contract family identifier. | none | non-empty | Unknown -> reject payload. |
| metadata.platform_profile | Active platform profile under test. | none | enumerated | Unknown -> reject payload. |
| metadata.adapter_id | Active adapter identifier. | none | allowlisted or empty | Unknown -> reject payload. |
| metadata.ui_surface | Active UI surface. | none | enumerated | Unknown -> reject payload. |
| metadata.seed | Deterministic run seed. | uint32 | [0, 4294967295] | Missing -> reject payload. |
| metadata.deterministic | Determinism marker. | bool | true/false | False -> reject simulation evidence payloads. |
| mode.active | Active mode/source. | none | enumerated | Unknown -> reject payload. |
| mode.contributors[] | Active contributors list. | none | list | Unknown source IDs -> reject payload. |
| sensors[] | Per-sensor health/freshness/confidence records. | mixed | per field | Missing sensor id -> reject payload. |
| front_view.* | Front-view display mode/state/frame/latency/drop/provenance/auth telemetry. | mixed | per field | Invalid or unknown front-view fields -> reject payload. |
| status.* | Authorization, provenance, logging, adapter status and reasons. | none | stable IDs/text | Unknown mandatory status -> reject payload. |

## Adapter Extension Rules
- Each adapter must declare extension fields with units, ranges, and
  explicit error behavior.
- Extensions must be mapped to all supported UI surfaces or explicitly
  marked as not applicable with a reason.
- Unknown extension fields are errors and must fail closed.
- Extension values are validated by the adapter before passing to UI.

## Surfaces
Supported surfaces:
- tui
- cockpit
- remote_operator
- c2
