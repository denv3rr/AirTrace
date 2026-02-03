# Adapter: Maritime

## Purpose
Define platform-specific adapter expectations for maritime platforms and the
UI extension fields required for official maritime adapters.

## Supported Surfaces
- tui
- remote_operator
- c2

## Adapter-Specific UI Extensions (Initial)
Ranges are initial defaults and must be validated for each program.

| Field ID | Description | Units | Range | Error Behavior | Surfaces |
| --- | --- | --- | --- | --- | --- |
| maritime.heading_deg | True heading. | deg | [-180, 180] | Invalid -> deny. | all |
| maritime.roll_deg | Roll angle. | deg | [-60, 60] | Invalid -> deny. | remote_operator,c2 |
| maritime.pitch_deg | Pitch angle. | deg | [-30, 30] | Invalid -> deny. | remote_operator,c2 |
| maritime.sea_state | Sea state index. | enum | 0-9 | Unknown -> deny. | c2 |
| maritime.gps_time_sync | GNSS time sync status. | bool | true/false | False -> deny in operational runs. | all |

## Environmental Constraints (MIL-STD-810H)
- Salt fog, humidity, and shock constraints must be documented.
- Sea-state-specific operating limits must be listed in program notes.

## Safety Considerations
- Invalid heading or GNSS time sync requires safe-state entry.
- UI must show recovery guidance for time sync loss.

## Security Considerations
- Adapter identity must be signed and allowlisted.
- External network-aid usage is deny-by-default.

## Verification
- V-110: capability and UI mapping inspection.
- V-111: adapter failure triggers safe state.
- V-112: environmental and HMI guidance documented.

## Traceability
- REQ-ADP-001..005
- REQ-INT-024
