# Adapter: Ground

## Purpose
Define platform-specific adapter expectations for ground platforms and the
UI extension fields required for official ground adapters.

## Supported Surfaces
- tui
- remote_operator
- c2

## Adapter-Specific UI Extensions (Initial)
Ranges are initial defaults and must be validated for each program.

| Field ID | Description | Units | Range | Error Behavior | Surfaces |
| --- | --- | --- | --- | --- | --- |
| ground.speed_mps | Ground speed. | m/s | [0, 60] | Invalid -> deny + safe-state. | all |
| ground.terrain_grade_deg | Terrain grade. | deg | [-45, 45] | Invalid -> deny. | remote_operator,c2 |
| ground.slip_ratio | Estimated wheel slip ratio. | 0-1 | [0, 1] | Invalid -> deny. | remote_operator,c2 |
| ground.odometry_status | Odometry health state. | enum | ok,degraded,failed | Unknown -> deny. | all |
| ground.stability_control | Stability control state. | enum | normal,limited,disabled | Unknown -> deny. | remote_operator,c2 |

## Environmental Constraints (MIL-STD-810H)
- Temperature, dust, mud, and vibration limits must be documented.
- Terrain-specific constraints must be listed in program-specific notes.

## Safety Considerations
- Invalid odometry or stability control data requires safe-state entry.
- UI must surface recovery guidance for degraded traction.

## Security Considerations
- Adapter identity must be signed and allowlisted.
- Vehicle bus access must be least privilege.

## Verification
- V-110: capability and UI mapping inspection.
- V-111: adapter failure triggers safe state.
- V-112: environmental and HMI guidance documented.

## Traceability
- REQ-ADP-001..005
- REQ-INT-024
