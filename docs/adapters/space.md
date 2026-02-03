# Adapter: Space

## Purpose
Define platform-specific adapter expectations for space platforms and the
UI extension fields required for official space adapters.

## Supported Surfaces
- tui
- remote_operator
- c2

## Adapter-Specific UI Extensions (Initial)
Ranges are initial defaults and must be validated for each program.

| Field ID | Description | Units | Range | Error Behavior | Surfaces |
| --- | --- | --- | --- | --- | --- |
| space.attitude_error_deg | Attitude error estimate. | deg | [0, 10] | Invalid -> deny. | remote_operator,c2 |
| space.star_tracker_lock | Star tracker lock status. | bool | true/false | False -> degrade or deny. | all |
| space.reaction_wheel_saturation | Reaction wheel saturation. | 0-1 | [0, 1] | Invalid -> deny. | remote_operator,c2 |
| space.eclipse_status | Eclipse flag. | bool | true/false | Invalid -> deny. | c2 |
| space.sun_vector_valid | Sun vector validity. | bool | true/false | False -> degrade or deny. | all |

## Environmental Constraints (MIL-STD-810H)
- Radiation, thermal cycling, and vacuum constraints must be documented.
- Orbit-specific limits must be listed in program notes.

## Safety Considerations
- Invalid attitude data requires safe-state entry.
- UI must show recovery guidance for loss of star tracker lock.

## Security Considerations
- Adapter identity must be signed and allowlisted.
- All external data is untrusted and validated.

## Verification
- V-110: capability and UI mapping inspection.
- V-111: adapter failure triggers safe state.
- V-112: environmental and HMI guidance documented.

## Traceability
- REQ-ADP-001..005
- REQ-INT-024
