# Adapter: Handheld

## Purpose
Define platform-specific adapter expectations for handheld platforms and the
UI extension fields required for official handheld adapters.

## Supported Surfaces
- tui
- remote_operator

## Adapter-Specific UI Extensions (Initial)
Ranges are initial defaults and must be validated for each program.

| Field ID | Description | Units | Range | Error Behavior | Surfaces |
| --- | --- | --- | --- | --- | --- |
| handheld.device_heading_deg | Device heading. | deg | [-180, 180] | Invalid -> deny. | all |
| handheld.device_pitch_deg | Device pitch. | deg | [-90, 90] | Invalid -> deny. | all |
| handheld.device_roll_deg | Device roll. | deg | [-180, 180] | Invalid -> deny. | all |
| handheld.location_accuracy_m | Estimated location accuracy. | meters | [0, 1000] | Invalid -> deny. | all |
| handheld.battery_pct | Battery level. | percent | [0, 100] | Invalid -> deny. | all |

## Environmental Constraints (MIL-STD-810H)
- Temperature, shock, and ingress protection constraints must be documented.
- Battery-dependent performance constraints must be listed in program notes.

## Safety Considerations
- Invalid orientation or accuracy data requires safe-state entry.
- UI must show recovery guidance for low battery.

## Security Considerations
- Adapter identity must be signed and allowlisted.
- Storage and input sources must be least privilege.

## Verification
- V-110: capability and UI mapping inspection.
- V-111: adapter failure triggers safe state.
- V-112: environmental and HMI guidance documented.

## Traceability
- REQ-ADP-001..005
- REQ-INT-024
