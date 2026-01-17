# Configuration Schema

Schema version: 1.0

## Rules
- All keys are required unless marked optional.
- Unknown keys are errors.
- Units are mandatory and indicated below.
- Validation rejects out-of-range values and inter-field violations.

## Global
- config.version (string): required; default "1.0".

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
- sim.dt (seconds): default 0.1; range (0, 10]
- sim.steps (count): default 100; range [1, 1e7]
- sim.seed (uint32): default 1; range [0, 4294967295]

## Bounds
- bounds.min.x (meters): default -10000.0; range [-1e6, 1e6]
- bounds.min.y (meters): default -10000.0; range [-1e6, 1e6]
- bounds.min.z (meters): default -10000.0; range [-1e6, 1e6]
- bounds.max.x (meters): default 10000.0; range [-1e6, 1e6]
- bounds.max.y (meters): default 10000.0; range [-1e6, 1e6]
- bounds.max.z (meters): default 10000.0; range [-1e6, 1e6]
- bounds.max_speed (m/s): default 1000.0; range (0, 1e5]
- bounds.max_accel (m/s^2): default 100.0; range (0, 1e4]
- bounds.max_turn_rate_deg (deg/s): default 30.0; range (0, 360]

Inter-field constraints:
- bounds.min.* <= bounds.max.*

## Maneuvers
- maneuver.random_accel_std (m/s^2): default 0.5; range [0, 1e3]
- maneuver.probability (0-1): default 0.1; range [0, 1]

## Sensors (all rates in Hz, noise in meters or m/s as appropriate)
- sensor.gps.rate_hz: default 1.0; range (0, 1000]
- sensor.gps.noise_std (meters): default 5.0; range [0, 1e4]
- sensor.gps.dropout (0-1): default 0.05; range [0, 1]
- sensor.gps.false_positive (0-1): default 0.01; range [0, 1]
- sensor.gps.max_range (meters): default 1e6; range (0, 1e7]

- sensor.thermal.rate_hz: default 2.0; range (0, 1000]
- sensor.thermal.noise_std (meters): default 2.0; range [0, 1e4]
- sensor.thermal.dropout (0-1): default 0.05; range [0, 1]
- sensor.thermal.false_positive (0-1): default 0.02; range [0, 1]
- sensor.thermal.max_range (meters): default 50000.0; range (0, 1e7]

- sensor.dead_reckoning.rate_hz: default 5.0; range (0, 1000]
- sensor.dead_reckoning.noise_std (meters): default 1.0; range [0, 1e4]
- sensor.dead_reckoning.dropout (0-1): default 0.01; range [0, 1]
- sensor.dead_reckoning.false_positive (0-1): default 0.0; range [0, 1]
- sensor.dead_reckoning.max_range (meters): default 1e6; range (0, 1e7]

- sensor.imu.rate_hz: default 10.0; range (0, 2000]
- sensor.imu.noise_std (m/s): default 0.2; range [0, 1e3]
- sensor.imu.dropout (0-1): default 0.01; range [0, 1]
- sensor.imu.false_positive (0-1): default 0.0; range [0, 1]
- sensor.imu.max_range (meters): default 1e6; range (0, 1e7]

- sensor.radar.rate_hz: default 2.0; range (0, 1000]
- sensor.radar.noise_std (meters): default 3.0; range [0, 1e4]
- sensor.radar.dropout (0-1): default 0.03; range [0, 1]
- sensor.radar.false_positive (0-1): default 0.01; range [0, 1]
- sensor.radar.max_range (meters): default 200000.0; range (0, 1e7]
