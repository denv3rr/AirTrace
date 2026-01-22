# Navigation Fallbacks and Platform Profiles

This document explains the fallback ladder, platform profile model, and
dataset tiering, with sources from government and university references.

## Why a Fallback Ladder Exists
Modern navigation systems are designed to keep providing position and
time even when primary sources (like GNSS) are degraded or denied.
Industry practice uses multi-sensor fusion and policy gating so the
system can move to the best available source while remaining safe and
authorized.

The ladder in this project is ordered by:
1) Accuracy and stability under typical conditions.
2) Availability and survivability in degraded environments.
3) Policy permissions and mission constraints.

## Platform Profiles (Inheritance Model)
All platforms inherit from a base profile that defines shared data
contracts and common sensors. Each child profile can enable or disable
capabilities and set policy defaults.

Profiles:
- base
- air
- ground
- maritime
- space
- handheld
- fixed_site
- subsea

## Typical Sensor Suites (Non-Exhaustive)
These are common, unclassified sensor combinations observed in current
and prototype systems. The actual configuration is controlled by the
platform profile and policy.

Air:
- GNSS, IMU, baro, magnetometer, radar altimeter, EO/IR, optional lidar.

Ground:
- GNSS, IMU, wheel odometry, EO/IR, lidar, optional radar.

Maritime:
- GNSS, IMU, radar, compass, baro, optional AIS and EO/IR.

Space:
- Star tracker, IMU, GNSS (when in view), sun sensor.

Handheld:
- GNSS, IMU, magnetometer, baro, optional vision.

Fixed_site:
- GNSS, timing reference, optional inertial stability.

Subsea:
- IMU, depth/pressure, DVL/sonar, optional surface GNSS when available.

## Current Implementation Coverage
The current codebase models these fallback sensors:
- GNSS, IMU, radar, thermal, dead reckoning, vision, lidar, magnetometer, baro, celestial.

Other sensors listed above are industry references only and are not yet
implemented in the core.

## Navigation Fallback Ladder (Rationale)
This ladder is intended for the core mode manager. Actual usage is gated
by policy and platform profile.

Multi-modal switching prefers fused modes when primary and aiding sensors
are healthy and authorized. If fused sources disagree beyond configured
thresholds, the system downgrades to a safer mode or hold.

1) GNSS (multi-constellation when available).
   - Primary source when permitted and healthy.

2) GNSS + INS (tight or loose coupling).
   - Standard practice to stabilize GNSS and bridge short outages.
   - IMU drift grows with time, so GNSS aiding is preferred.

3) Vision- or lidar-aided inertial (VIO/LIO).
   - Provides local relative navigation when GNSS is denied.
   - Useful in structured environments with sufficient features.

4) Radar- or terrain-aided inertial (when supported).
   - Useful in poor visibility or for longer-range terrain matching.

5) Magnetometer + baro + coarse map constraints (platform dependent).
   - Low-precision, best for coarse stabilization.

6) Celestial navigation (star/sun/planet based).
   - High-precision in clear conditions, but requires data and compute.
   - Must only activate when higher-priority sources are unavailable or
     disallowed by policy.

7) Dead reckoning only.
   - Short-term fallback with rapid drift; safe-state triggers apply.

## Celestial Navigation Data and Tiers
Celestial navigation depends on validated catalogs and ephemerides.
Dataset tiers allow storage to scale by platform profile:

- Minimal: low-density catalog + narrow ephemeris window.
- Standard: medium catalog + standard ephemeris.
- Extended: dense catalog + long-range ephemeris for high-capability
  platforms.

Tier selection is policy-controlled and defaults to minimal for handheld
and personnel devices unless explicitly overridden.

## Authorization and Network Aids
- Deny-by-default is the baseline.
- Overrides require role-based authorization and credentials/keys.
- All overrides are logged with timestamp and role context.

Suggested roles (least privilege):
- operator
- supervisor
- mission_owner
- test
- research
- internal

## Sources (Government and University)
- NIST SP 800-53 Rev 5 (security controls, least privilege):
  https://csrc.nist.gov/pubs/sp/800/53/r5/upd1/final
- NIST SP 800-162 (RBAC guidance):
  https://csrc.nist.gov/pubs/sp/800/162/upd2/final
- USNO Astronomical Applications (celestial references):
  https://aa.usno.navy.mil/
- NASA NAIF SPICE (ephemerides and kernels):
  https://naif.jpl.nasa.gov/naif/
- JPL ephemerides data repository:
  https://ssd.jpl.nasa.gov/ftp/eph/planets/
- MIT Kalman Filter primer (state estimation):
  https://www.mit.edu/~jwilson/kalman.pdf
- VINS-Mono (visual-inertial state estimation, university research):
  https://arxiv.org/abs/1708.03852
- LOAM (lidar odometry and mapping, university research):
  https://arxiv.org/abs/1406.5714
