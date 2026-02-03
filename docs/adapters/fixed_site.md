# Adapter: Fixed Site

## Purpose
Define platform-specific adapter expectations for fixed site platforms and the
UI extension fields required for official fixed site adapters.

## Supported Surfaces
- tui
- c2

## Adapter-Specific UI Extensions (Initial)
Ranges are initial defaults and must be validated for each program.

| Field ID | Description | Units | Range | Error Behavior | Surfaces |
| --- | --- | --- | --- | --- | --- |
| fixed_site.time_holdover_seconds | Timing holdover duration. | seconds | [0, 86400] | Invalid -> deny. | all |
| fixed_site.antenna_status | Antenna health. | enum | ok,degraded,failed | Unknown -> deny. | all |
| fixed_site.power_state | Power state. | enum | normal,backup,degraded | Unknown -> deny. | all |
| fixed_site.timing_reference | Timing reference health. | enum | ok,holdover,lost | Unknown -> deny. | c2 |
| fixed_site.clock_drift_ppb | Clock drift estimate. | ppb | [0, 100000] | Invalid -> deny. | c2 |

## Environmental Constraints (MIL-STD-810H)
- Temperature, power quality, and environmental ingress constraints must be documented.
- Facility-specific limits must be listed in program notes.

## Safety Considerations
- Loss of timing reference requires safe-state entry.
- UI must show recovery guidance for power degradation.

## Security Considerations
- Adapter identity must be signed and allowlisted.
- Physical security assumptions must be documented.

## Verification
- V-110: capability and UI mapping inspection.
- V-111: adapter failure triggers safe state.
- V-112: environmental and HMI guidance documented.

## Traceability
- REQ-ADP-001..005
- REQ-INT-024
