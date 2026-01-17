# Operational Concepts

## Mission Modes
- gps: Primary mode using GPS measurements.
- thermal: Alternate mode using thermal sensor.
- radar: Alternate mode using radar range/bearing.
- dead_reckoning: Fallback mode using drift-based estimation.
- inertial: IMU-only mode for short-term stabilization.
- hold: Safe state with no tracking updates.

## Mode Transition Rules
- Mode changes are controlled by the mode manager using health, confidence, and hysteresis.
- A minimum dwell time shall be enforced to prevent oscillation.
- On loss of all sensors, the system shall enter hold mode.

## Operator Responsibilities
- Validate configuration files before execution.
- Monitor mode status, sensor health, and safety indicators.
- Acknowledge errors and follow recovery prompts.

## Multi-Device Operation
- Each device exposes capabilities and supported sensors.
- Device time sync is required to align measurements.
- Device health is monitored and aggregated to determine global mode.

## Environmental Constraints
- Position, velocity, acceleration, and turn rate are bounded by config.
- Out-of-range conditions trigger safe-state behavior or rejection.

## Data Handling
- Config inputs are untrusted; schema validation is mandatory.
- Simulation outputs include seed, config version, and build metadata.
