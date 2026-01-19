# Operational Concepts

## Mission Modes
- gps: Primary mode using GPS measurements.
- thermal: Alternate mode using thermal sensor.
- radar: Alternate mode using radar range/bearing.
- vision: Alternate mode using EO/IR vision position fixes.
- lidar: Alternate mode using lidar range/bearing.
- magnetometer: Heading stabilization using magnetic compass inputs.
- baro: Altitude stabilization using barometric inputs.
- dead_reckoning: Fallback mode using drift-based estimation.
- inertial: IMU-only mode for short-term stabilization.
- celestial: Celestial fix using onboard star/planet references when permitted and required.
- hold: Safe state with no tracking updates.

## Mode Transition Rules
- Mode changes are controlled by the mode manager using health, confidence, and hysteresis.
- A minimum dwell time shall be enforced to prevent oscillation.
- On loss of all sensors, the system shall enter hold mode.
- The navigation ladder prefers higher-permitted sources before lower-permitted ones.
- Celestial navigation is only permitted when higher-priority sources are unavailable or disallowed.

## Platform Profiles
- Base profile defines common sensors, policy defaults, and data contracts.
- Child profiles extend the base: air, ground, maritime, space, handheld, fixed_site, subsea.
- Profiles declare capabilities (sensors available, permitted aids, and auth policy).
- See docs/navigation_fallbacks.md for ladder ordering and dataset tiers.

## Dataset Tiers
- Tiered celestial datasets limit storage based on platform profile.
- Handheld/personnel profiles default to minimal tiers unless explicitly overridden.
- High-capability platforms may use expanded tiers when authorized by policy.

## Authorization Roles
- Roles are defined in configuration and mapped to permitted overrides.
- Suggested roles: operator, supervisor, mission_owner, test, research, internal.
- Role policy is least-privilege; deny-by-default remains the baseline.

## Operator Responsibilities
- Validate configuration files before execution.
- Monitor mode status, sensor health, and safety indicators.
- Acknowledge errors and follow recovery prompts.
- Ensure policy authorization is set for any network-aid usage.
- UI/TUI behavior follows docs/ui_standards.md.

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
- Celestial datasets are treated as controlled inputs and must pass integrity checks.
