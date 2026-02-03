# Adapter: Air

## Purpose
Define platform-specific adapter expectations for air platforms and the
UI extension fields required for official air adapters.

## Supported Surfaces
- tui
- cockpit
- remote_operator
- c2

## Adapter-Specific UI Extensions (Initial)
Ranges are initial defaults and must be validated for each program.

| Field ID | Description | Units | Range | Error Behavior | Surfaces |
| --- | --- | --- | --- | --- | --- |
| air.altitude_agl_m | Altitude above ground level. | meters | [0, 30000] | Invalid -> deny + safe-state. | all |
| air.airspeed_mps | Airspeed. | m/s | [0, 350] | Invalid -> deny + safe-state. | all |
| air.climb_rate_mps | Vertical speed. | m/s | [-100, 100] | Invalid -> deny + safe-state. | all |
| air.flight_phase | Phase of flight. | enum | taxi,takeoff,climb,cruise,descent,landing | Unknown -> deny. | cockpit,remote_operator,c2 |
| air.g_load | Normal acceleration. | g | [0, 9] | Invalid -> deny. | cockpit,remote_operator |

## Environmental Constraints (MIL-STD-810H)
- Temperature, vibration, altitude, and shock limits must be documented.
- Airframe-specific constraints must be listed in program-specific notes.

## Safety Considerations
- Loss of air data or invalid phase requires safe-state entry.
- UI must display clear recovery guidance on invalid air data.

## Security Considerations
- Adapter identity must be signed and allowlisted.
- Any air data source must be explicitly authorized.

## Verification
- V-110: capability and UI mapping inspection.
- V-111: adapter failure triggers safe state.
- V-112: environmental and HMI guidance documented.

## Traceability
- REQ-ADP-001..005
- REQ-INT-024
