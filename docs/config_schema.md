# Configuration Schema

Schema version: 1.0

## Rules
- All keys are required unless marked optional.
- Unknown keys are errors.
- Units are mandatory and indicated below.
- Validation rejects out-of-range values and inter-field violations.

## Global
- config.version (string): required; default "1.0".

## Platform
- platform.profile (string): optional; default "base".
  - Allowed: base, air, ground, maritime, space, handheld, fixed_site, subsea.
- platform.permitted_sensors (list): optional; default profile-based list.
  - Format: comma-separated sensor names (example: gps,imu,radar).

## Policy
- policy.network_aid.mode (string): optional; default "deny".
  - Allowed: deny, allow, test_only.
- policy.network_aid.override_required (bool): optional; default true.
- policy.network_aid.override_auth (string): optional; default "credential".
  - Allowed: credential, key, token.
- policy.network_aid.override_timeout_seconds (seconds): optional; default 0 (no cached override).

## Roles
- policy.roles (list): optional; default ["operator"].
  - Suggested: operator, supervisor, mission_owner, test, research, internal.
- policy.role_permissions (map): optional; default empty.
  - Keys: role names.
  - Values: allowed overrides (network_aid, dataset_tier, mode_override).
  - Format: policy.role_permissions.<role>=comma-separated list.
- policy.active_role (string): optional; default "operator".

## Dataset Tiers
- dataset.celestial.tier (string): optional; default "minimal".
  - Allowed: minimal, standard, extended.
- dataset.celestial.max_size_mb (number): optional; default 0 (no explicit limit).

## Celestial Datasets
- dataset.celestial.catalog_path (string): optional; default empty.
- dataset.celestial.ephemeris_path (string): optional; default empty.
- dataset.celestial.catalog_hash (string): optional; default empty.
- dataset.celestial.ephemeris_hash (string): optional; default empty.
- Hash format: SHA-256 hex string.

## State
- state.position.x (meters): default 0.0; range [-1e6, 1e6]
- state.position.y (meters): default 0.0; range [-1e6, 1e6]
- state.position.z (meters): default 100.0; range [-1e6, 1e6]
- state.velocity.x (m/s): default 15.0; range [-1e4, 1e4]
- state.velocity.y (m/s): default 10.0; range [-1e4, 1e4]
- state.velocity.z (m/s): default 0.0; range [-1e4, 1e4]
- state.acceleration.x (m/s^2): default 0.2; range [-1e3, 1e3]
- state.acceleration.y (m/s^2): default -0.1; range [-1e3, 1e3]
- state.acceleration.z (m/s^2): default 0.0; range [-1e3, 1e3]
- state.time (seconds): default 0.0; range [0, 1e6]

## Simulation
- sim.dt (seconds): default 0.2; range (0, 10]
- sim.steps (count): default 20; range [1, 1e7]
- sim.seed (uint32): default 42; range [0, 4294967295]

## Bounds
- bounds.min.x (meters): default -1000.0; range [-1e6, 1e6]
- bounds.min.y (meters): default -1000.0; range [-1e6, 1e6]
- bounds.min.z (meters): default 0.0; range [-1e6, 1e6]
- bounds.max.x (meters): default 1000.0; range [-1e6, 1e6]
- bounds.max.y (meters): default 1000.0; range [-1e6, 1e6]
- bounds.max.z (meters): default 1000.0; range [-1e6, 1e6]
- bounds.max_speed (m/s): default 250.0; range (0, 1e5]
- bounds.max_accel (m/s^2): default 20.0; range (0, 1e4]
- bounds.max_turn_rate_deg (deg/s): default 12.0; range (0, 360]

Inter-field constraints:
- bounds.min.* <= bounds.max.*

## Maneuvers
- maneuver.random_accel_std (m/s^2): default 3.0; range [0, 1e3]
- maneuver.probability (0-1): default 0.35; range [0, 1]

## Sensors (rates in Hz; noise in meters, m/s, or radians as noted)
- sensor.gps.rate_hz: default 1.0; range (0, 1000]
- sensor.gps.noise_std (meters): default 2.0; range [0, 1e4]
- sensor.gps.dropout (0-1): default 0.1; range [0, 1]
- sensor.gps.false_positive (0-1): default 0.03; range [0, 1]
- sensor.gps.max_range (meters): default 5000.0; range (0, 1e7]

- sensor.thermal.rate_hz: default 5.0; range (0, 1000]
- sensor.thermal.noise_std (meters): default 5.0; range [0, 1e4]
- sensor.thermal.dropout (0-1): default 0.15; range [0, 1]
- sensor.thermal.false_positive (0-1): default 0.08; range [0, 1]
- sensor.thermal.max_range (meters): default 1200.0; range (0, 1e7]

- sensor.dead_reckoning.rate_hz: default 20.0; range (0, 1000]
- sensor.dead_reckoning.noise_std (meters): default 0.5; range [0, 1e4]
- sensor.dead_reckoning.dropout (0-1): default 0.0; range [0, 1]
- sensor.dead_reckoning.false_positive (0-1): default 0.0; range [0, 1]
- sensor.dead_reckoning.max_range (meters): default 1e6; range (0, 1e7]

- sensor.imu.rate_hz: default 50.0; range (0, 2000]
- sensor.imu.noise_std (m/s): default 0.2; range [0, 1e3]
- sensor.imu.dropout (0-1): default 0.05; range [0, 1]
- sensor.imu.false_positive (0-1): default 0.0; range [0, 1]
- sensor.imu.max_range (meters): default 1e6; range (0, 1e7]

- sensor.radar.rate_hz: default 2.0; range (0, 1000]
- sensor.radar.noise_std (meters): default 1.0; range [0, 1e4]
- sensor.radar.dropout (0-1): default 0.08; range [0, 1]
- sensor.radar.false_positive (0-1): default 0.02; range [0, 1]
- sensor.radar.max_range (meters): default 2000.0; range (0, 1e7]

- sensor.vision.rate_hz: default 10.0; range (0, 1000]
- sensor.vision.noise_std (meters): default 1.5; range [0, 1e4]
- sensor.vision.dropout (0-1): default 0.1; range [0, 1]
- sensor.vision.false_positive (0-1): default 0.03; range [0, 1]
- sensor.vision.max_range (meters): default 1500.0; range (0, 1e7]

- sensor.lidar.rate_hz: default 5.0; range (0, 1000]
- sensor.lidar.noise_std (meters): default 0.5; range [0, 1e4]
- sensor.lidar.dropout (0-1): default 0.05; range [0, 1]
- sensor.lidar.false_positive (0-1): default 0.02; range [0, 1]
- sensor.lidar.max_range (meters): default 1000.0; range (0, 1e7]

- sensor.magnetometer.rate_hz: default 20.0; range (0, 1000]
- sensor.magnetometer.noise_std (radians): default 0.05; range [0, 1e4]
- sensor.magnetometer.dropout (0-1): default 0.02; range [0, 1]
- sensor.magnetometer.false_positive (0-1): default 0.0; range [0, 1]
- sensor.magnetometer.max_range (meters): default 1e6; range (0, 1e7]

- sensor.baro.rate_hz: default 2.0; range (0, 1000]
- sensor.baro.noise_std (meters): default 0.3; range [0, 1e4]
- sensor.baro.dropout (0-1): default 0.02; range [0, 1]
- sensor.baro.false_positive (0-1): default 0.0; range [0, 1]
- sensor.baro.max_range (meters): default 20000.0; range (0, 1e7]

- sensor.celestial.rate_hz: default 0.2; range (0, 1000]
- sensor.celestial.noise_std (meters): default 0.8; range [0, 1e4]
- sensor.celestial.dropout (0-1): default 0.15; range [0, 1]
- sensor.celestial.false_positive (0-1): default 0.01; range [0, 1]
- sensor.celestial.max_range (meters): default 1e7; range (0, 1e7]
