# Edge Case Catalog (Platform and Airframe History)

This catalog captures known edge cases observed across air, ground, maritime,
space, handheld, and fixed-site platforms. Each scenario informs strict mode
decision logic, safety gates, and verification cases.

## Sensor Dropout and Degradation
- GNSS denial or intermittent jamming causing stale fixes.
- IMU bias drift under high vibration or thermal gradients.
- Radar multipath and sidelobe interference near terrain or structures.
- Thermal saturation during high-contrast scenes or plume occlusion.
- Vision failure under low light, glare, or high motion blur.
- Lidar dropout in rain, fog, or low-reflectivity surfaces.
- Magnetometer anomalies near power systems or ferrous structures.
- Barometric drift during rapid altitude changes or temperature swings.
- Celestial obstruction from clouds, sun glare, or spacecraft attitude limits.

## Timing and Synchronization
- Sensor update jitter causing misalignment between contributors.
- Latency spikes from overloaded compute or IO contention.
- Out-of-order measurement arrival in concurrent pipelines.
- Clock drift between devices in multi-sensor setups.

## Data Integrity and Policy
- Dataset gaps or corrupted celestial catalogs.
- Unauthorized network aids or policy override attempts.
- Configuration mismatch between platform profile and deployed sensors.
- Unknown or untrusted sensor sources in fusion.

## Cross-Sensor Disagreement
- IMU vs GNSS divergence beyond thresholds.
- Vision vs radar disagreement under occlusion.
- Lidar vs radar range conflict from multipath reflections.
- Magnetometer vs IMU heading divergence under interference.

## Environmental and Platform Constraints
- High dynamic maneuvers exceeding configured bounds.
- Platform-specific constraints (subsea pressure, space attitude, handheld motion).
- Power or thermal limits reducing sensor availability.

## Operator and UI Edge Cases
- Ambiguous status display during rapid transitions.
- Unclear denial reason for policy or dataset rejection.
- Insufficient recovery prompts after safe-state entry.
