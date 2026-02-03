# Adapter: Subsea

## Purpose
Define platform-specific adapter expectations for subsea platforms and the
UI extension fields required for official subsea adapters.

## Supported Surfaces
- tui
- remote_operator
- c2

## Adapter-Specific UI Extensions (Initial)
Ranges are initial defaults and must be validated for each program.

| Field ID | Description | Units | Range | Error Behavior | Surfaces |
| --- | --- | --- | --- | --- | --- |
| subsea.depth_m | Depth below surface. | meters | [0, 11000] | Invalid -> deny. | all |
| subsea.pressure_kpa | Ambient pressure. | kPa | [0, 120000] | Invalid -> deny. | all |
| subsea.dvl_valid | DVL validity flag. | bool | true/false | False -> degrade or deny. | all |
| subsea.salinity_psu | Salinity estimate. | PSU | [0, 50] | Invalid -> deny. | c2 |
| subsea.acoustic_link | Acoustic link status. | enum | ok,degraded,lost | Unknown -> deny. | remote_operator,c2 |

## Environmental Constraints (MIL-STD-810H)
- Pressure, corrosion, and biofouling constraints must be documented.
- Depth-dependent limits must be listed in program notes.

## Safety Considerations
- Invalid pressure or depth data requires safe-state entry.
- UI must show recovery guidance for acoustic link loss.

## Security Considerations
- Adapter identity must be signed and allowlisted.
- External acoustic inputs are untrusted and validated.

## Verification
- V-110: capability and UI mapping inspection.
- V-111: adapter failure triggers safe state.
- V-112: environmental and HMI guidance documented.

## Traceability
- REQ-ADP-001..005
- REQ-INT-024
