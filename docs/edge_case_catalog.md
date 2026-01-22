# Edge Case Catalog (Platform and Airframe History)

This catalog captures known edge cases observed across air, ground, maritime,
space, handheld, and fixed-site platforms. Each scenario informs strict mode
decision logic, safety gates, and verification cases.

## Sensor Dropout and Degradation
- GNSS denial or intermittent jamming causing stale fixes.
- IMU bias drift under high vibration or thermal gradients.
- IMU axis swap or inversion after incorrect maintenance or cabling.
- Radar multipath and sidelobe interference near terrain or structures.
- Thermal saturation during high-contrast scenes or plume occlusion.
- Vision failure under low light, glare, or high motion blur.
- Lidar dropout in rain, fog, or low-reflectivity surfaces.
- Magnetometer anomalies near power systems or ferrous structures.
- Barometric drift during rapid altitude changes or temperature swings.
- Celestial obstruction from clouds, sun glare, or spacecraft attitude limits.
- Sudden sensor reboot that resets confidence and timestamp history.
- Sensor health flapping at the minimum dwell threshold.
- Partial sensor data availability (position without velocity or heading).

## Timing and Synchronization
- Sensor update jitter causing misalignment between contributors.
- Latency spikes from overloaded compute or IO contention.
- Out-of-order measurement arrival in concurrent pipelines.
- Clock drift between devices in multi-sensor setups.
- Zero or negative dt from time source reset.
- Burst updates that violate expected rate limits.
- PPS loss or holdover drift after GNSS time source failure.

## Data Integrity and Policy
- Dataset gaps or corrupted celestial catalogs.
- Unauthorized network aids or policy override attempts.
- Configuration mismatch between platform profile and deployed sensors.
- Unknown or untrusted sensor sources in fusion.
- Conflicting role permissions vs active role settings.
- Policy allows source but dataset hashes are missing or mismatched.
- Regional datum changes or geoid model mismatches.
- Stale ephemeris feeds during extended offline operation.
- Conflicting time standards between dataset metadata and runtime clocks.
- Leap second or leap smear inconsistencies between timing sources.
- Policy updates during active sessions without re-authorization handshake.

## Cross-Sensor Disagreement
- IMU vs GNSS divergence beyond thresholds.
- Vision vs radar disagreement under occlusion.
- Lidar vs radar range conflict from multipath reflections.
- Magnetometer vs IMU heading divergence under interference.
- Fused contributors disagree only after alignment window expires.
- Disagreement spikes during a mode handoff.

## Environmental and Platform Constraints
- High dynamic maneuvers exceeding configured bounds.
- Platform-specific constraints (subsea pressure, space attitude, handheld motion).
- Power or thermal limits reducing sensor availability.
- Rapid weather change causing simultaneous sensor degradation.
- Thermal throttling reduces compute budgets mid-run.
- Electrostatic discharge causing transient sensor resets.
- Interference from high-power transmitters saturating RF front ends.
- Co-sited friendly emitters desense RF front ends or trigger blanking intervals.
- Vehicle power bus load dumps or brownouts causing rolling sensor resets.
- GNSS regional outages or spoofing events impacting PNT availability.
- Ionospheric scintillation causing rapid GNSS phase errors.

## Operator and UI Edge Cases
- Ambiguous status display during rapid transitions.
- Unclear denial reason for policy or dataset rejection.
- Insufficient recovery prompts after safe-state entry.
- Menu input stream unavailable or non-interactive terminals.
- Selection inputs outside available menu bounds.
- Operator exits test mode during a scenario handoff.
- Denial banner persists after recovery, masking current status.
- Multiple concurrent denial reasons overwrite each other.
- Recovery guidance conflicts with current mode/state.
- Harness command stream missing or truncated mid-flow.
- Harness command prompt string mismatch causes safe-state exit.

## Configuration and Schema
- Unknown keys mixed with valid config sections.
- Version bump with missing default values.
- Units mismatch between config and UI prompts.
- Profile inheritance chains that skip required defaults.
- Duplicate keys with conflicting values across included config files.

## Simulation and Mode Control
- Scenario mode with zero timeout windows.
- Mode selection with all contributors at minimum confidence.
- Lockout expires while a contributor remains stale.
- Fused mode selected while one contributor reports partial measurement payloads.
- Deterministic seed mismatch between recorded log and replay config.

## Build and Tooling
- Invalid or injected BUILD_TYPE values (e.g., '$BuildType').
- Build directory path points to a non-writable location.
- Generator mismatch between cached build and script generator selection.
- Mixed toolchain versions within a single build directory.

## External Data and Operational Context
- Regional spectrum allocations interfere with sensor RF bands.
- Civil/military GNSS policy switches during operations.
- Target-rich environments producing sensor overload or saturation.
- Electromagnetic interference from nearby radars or jammers.
- Urban canyon multipath causing correlated errors across sensors.
- Space weather impacts on GNSS availability and accuracy.
- Official GPS service interruptions and advisories.
- GNSS anomaly reporting workflows differ by region.
- Conflict-zone GNSS interference with cross-border spillover into civil corridors.
- Constellation-specific interference causing divergent multi-GNSS solutions.
- Emergency notices issued faster than local ops can ingest and apply.
- National spectrum authority emergency reallocations causing sudden band conflicts.

## External Incident Research Backlog (Sources Needed)
- Regional GNSS outages, jamming notices, and interference advisories.
- Spectrum management bulletins affecting radar or comm bands.
- Environmental anomaly reports that impact sensor availability (ionospheric storms, solar events).
- Public safety or defense incident summaries tied to navigation failures.
- Civil aviation or maritime safety advisories tied to regional GNSS interference.
- Conflict-zone interference summaries from international safety bodies.
- Eastern Europe/Black Sea GNSS interference advisories (aviation/maritime).
- Ukraine/Russia border-region interference reports with operational impacts.

## External Sources (Initial)
- USCG NAVCEN GPS Service Interruptions: https://www.navcen.uscg.gov/gps-service-interruptions
- USCG NAVCEN GPS Problem Report: https://www.navcen.uscg.gov/contact/gps-problem-report
- USCG NAVCEN Space Weather Information: https://www.navcen.uscg.gov/gps-space-weather-information-and-ephemeris-data
- NOAA SWPC Space Weather: https://www.swpc.noaa.gov/
- GPS.gov (US PNT program): https://gps.gov/
- ESA GSSC (EU GNSS status/services): https://gssc.esa.int/
- UK GNSS disruption impact report: https://www.gov.uk/government/publications/report-the-economic-impact-on-the-uk-of-a-disruption-to-gnss/the-economic-impact-on-the-uk-of-a-disruption-to-gnss-executive-summary
- GPSJAM OSINT interference map: https://gpsjam.org/
- ITU interference reporting (FMD): https://www.itu.int/en/ITU-R/terrestrial/fmd/Pages/default.aspx
- European GNSS Service Centre: https://www.gsc-europa.eu/
