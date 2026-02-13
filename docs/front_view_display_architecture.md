# Front-View Sensor Display Architecture (Draft)

## Purpose
Define a professional, cross-platform front-view display subsystem that can:
- ingest real or spoofed sensor feeds through a stable interface;
- render operator-usable grayscale/thermal/false-color outputs with deterministic behavior;
- support platform-style display cycling and 2D/3D proximity views from the same session;
- remain compatible with military and search-and-rescue style operations.

This document is intentionally context-agnostic: tracker, ISR-style platform testing, and SAR workflows are all supported through one interface contract and mode policy.

## Implementation Status (Current)
- Config schema scaffolding is active under `front_view.*` and validated by tools loader.
- UI front-view processing scaffold is implemented in `src/ui/front_view.cpp` with contract in `include/ui/front_view.h`.
- Operator entry path is implemented via `Front-View Display Workbench` in `src/ui/menu.cpp`.
- External envelope now carries front-view telemetry via `include/core/external_io_envelope.h` and `src/ui/simulation.cpp`.
- Deterministic tests are present in `tests/front_view_display.cpp`, `tests/ui_status.cpp`, and harness integration flow tests.
- Stage-1 contract hardening is implemented for deterministic frame timestamp, frame age, latency-stage breakdown, stream identity, and stabilization/gimbal metadata with fail-closed validation paths.

## Designated Workstreams
- Team A (`core/tools`): config and safety/security gate enforcement for `front_view.*` keys and fail-closed behavior.
- Team B (`ui/runtime`): rendering/cycle execution pipeline, denial messaging, and operator workflow quality.
- Team C (`integration/test`): deterministic spoof fixtures, envelope conformance, and latency/drop verification evidence.
- Team D (`docs/compliance`): REQ/V traceability, hazard and threat-model maintenance, and review-gate evidence packaging.

## Standards and Guidance Baseline
- MIL-STD-1472 (Human Engineering), active as of 15-SEP-2020 in ASSIST Quick Search.
- MIL-STD-2525 (Joint Military Symbology), active revision dated 02-MAR-2025 in ASSIST Quick Search.
- MISB/GWG motion imagery interoperability guidance and conformance process (DoD/IC/NSG/NATO usage scope).
- IMO MSC.192(79) radar performance and presentation requirements for navigation-centric display behavior and detection metrics.
- Existing project baselines in `docs/ui_standards.md`, `docs/security_threat_model.md`, and `docs/operational_concepts.md`.

## Operational Modes and User Experience
### Operator Display Goals
- Persistent state awareness: active sensor, mode, confidence, and denial reason always visible.
- No color-only signaling; each visual cue has text + symbol support.
- Fast mode comprehension for stressed operators (SAR or tactical conditions).
- Deterministic visual mode cycling for test and validation runs.

### Display Families
- `front_view.eo_gray`: visible-light or derived grayscale.
- `front_view.ir_white_hot`: thermal white-hot mapping.
- `front_view.ir_black_hot`: thermal black-hot mapping.
- `front_view.ir_false_color`: perceptually controlled false-color mapping.
- `front_view.fusion_overlay`: EO + IR overlays with confidence/eligibility annotation.
- `front_view.proximity_2d`: top-down relative geometry and distance bands.
- `front_view.proximity_3d`: stream-from-data 3D geometry/track view.

### Cycling Model
- Manual cycling (default operational posture).
- Optional deterministic auto-cycle for scenario/test validation.
- Cycle order is config-driven and profile-aware, not hardcoded.
- Each transition logs source mode, destination mode, reason, and timestamp.

## Interface and Data Contract
### Ingress Contract (Sensor-Agnostic)
Define a normalized frame packet contract (tools/UI boundary):
- `frame_id`, `sensor_id`, `sensor_type`, `timestamp_utc`, `sequence`.
- `provenance`, `authorization_state`, `health_state`, `confidence`.
- `pixel_format` (for raster) or `point_format` (for 3D stream).
- `intrinsics/extrinsics` where applicable.
- `units` and valid ranges for all geometric fields.

### Spoof/Test Ingress
Support deterministic synthetic inputs without changing production paths:
- `spoof.frame_pattern` (fixed, sweep, gradient, hotspot, noise-injection).
- `spoof.motion_profile` (linear, orbit, jitter, dropout).
- `spoof.seed`, `spoof.rate_hz`, `spoof.latency_ms`.
- Fail closed when spoof schema or units are invalid.

### Egress Contract
Extend operator/UI output envelope for front-view artifacts:
- active display mode, frame source, render pipeline ID/version.
- render latency metrics (acquire/process/render).
- dropped frame counts and reason codes.
- 2D/3D view state and proximity summary.

## Rendering Pipeline (Reference)
### Common Stage Graph
1. Acquire/Decode
2. Radiometric/Format normalization
3. Contrast/tone mapping
4. Optional denoise/edge enhancement
5. Palette mapping or geometric projection
6. Overlay composition (symbology, text, confidence, alerts)
7. Present + export metrics

All stages should have explicit time budgets and deterministic fallback behavior.

### Render Profile Classes (Initial Targets)
- `profile.handheld_low_power`:
  - Target update rate: 15-30 FPS.
  - End-to-end latency target: <= 120 ms.
  - Preferred algorithms: fast normalization + lightweight contrast + minimal overlays.
- `profile.vehicle_standard`:
  - Target update rate: 30 FPS.
  - End-to-end latency target: <= 90 ms.
  - Preferred algorithms: CLAHE/tone-map with optional bilateral pass.
- `profile.airborne_high_dynamics`:
  - Target update rate: 30-60 FPS.
  - End-to-end latency target: <= 70 ms.
  - Preferred algorithms: GPU-accelerated mapping and overlay composition.
- `profile.fixed_site_analysis`:
  - Target update rate: 20-30 FPS.
  - End-to-end latency target: <= 150 ms.
  - Preferred algorithms: richer overlays and optional heavier denoise.

Targets above are working engineering budgets for implementation planning, not final acceptance limits.

### EO/Visible Path
- Convert BGR/RGB to grayscale when required using stable color conversion.
- Optional local contrast enhancement (CLAHE) with capped clip limits.
- Edge-preserving denoise option (bilateral-style filter) for high-noise sensors.

### IR/Thermal Path
- First-class support for nonuniformity correction (NUC) stage inputs.
- Dynamic range compression / tone mapping for operator readability.
- Palette sets must be standardized and explicitly labeled (white-hot, black-hot, false-color variant).
- Safety note: never hide absolute-value readouts behind color-only representation.

### 2D/3D Proximity Path
- 2D proximity: range rings, bearing, track confidence, uncertainty bands.
- 3D stream: point cloud or mesh-like representation from incoming data.
- Downsampling options required for frame stability (voxel or grid decimation).

## Algorithm Portfolio (Initial)
Algorithms are selected by profile and latency budget; no single algorithm is forced globally.

### Contrast and Visibility
- Global histogram equalization (baseline).
- CLAHE for local contrast with explicit clip/tiles config.
- Tone mapping operators for high dynamic range scenes.

### Denoise and Edge Preservation
- Gaussian smoothing for fast low-cost cleanup.
- Bilateral filtering for edge-preserving denoise in clutter.
- Edge extraction overlays (Canny-style) as optional aids.

### Detection/Segmentation Assists
- Thresholding modes (fixed, Otsu-style auto threshold).
- Temporal differencing and motion cues for target emphasis.
- Confidence-weighted overlays from upstream tracking/fusion.

### 2D/3D Representation
- Occupancy-grid-like 2D accumulation for proximity context.
- Point-cloud decimation and centroid/voxel simplification for 3D responsiveness.
- Optional optical flow for camera-relative motion cues.

### Algorithm Selection Matrix (Practical)
| Stage | Option | Strength | Tradeoff | Best Use |
| --- | --- | --- | --- | --- |
| Contrast | Linear stretch | Fastest | Weak local detail recovery | Low-power handheld baseline |
| Contrast | CLAHE | Strong local detail in haze/low contrast | Can amplify noise if untuned | EO/IR operator views |
| Denoise | Gaussian | Very low compute cost | Blurs edges | Fast preview path |
| Denoise | Bilateral | Preserves edges | Higher compute | Tactical detail retention |
| Segmentation | Fixed threshold | Deterministic/simple | Sensitive to scene changes | Controlled lab tests |
| Segmentation | Otsu auto threshold | Adaptive global threshold | Unstable in multimodal scenes | Fast auto-assist |
| Motion cue | Frame differencing | Cheap temporal highlight | Prone to false alarms | Basic motion cue |
| Motion cue | Farneback optical flow | Dense motion field | Heavier compute | Air/vehicle dynamic scenes |
| 3D decimation | Uniform sampling | Fastest | Detail loss | High-rate streams |
| 3D decimation | Voxel/grid | Better geometric stability | Slightly heavier | Operator proximity views |

## Multithreading and Performance Model
### Thread Roles
- `ingest_thread`: capture/decode and contract validation.
- `process_thread_pool`: sensor-specific preprocess + enhancement.
- `compose_thread`: overlay + symbology composition.
- `present_thread`: final display swap + metrics emission.

### Scheduling Rules
- Lock-free queue boundaries where practical.
- Explicit frame deadlines; stale frames dropped with reason code.
- Primary tracking outputs must not block on optional 3D rendering.
- Deterministic test mode: fixed worker count and fixed scheduling policy.

### Acceleration Options
- CPU baseline path must always exist.
- Optional GPU path for tone mapping, palette mapping, and 3D projection.
- GPU path must preserve deterministic behavior in validation mode.

## Human Factors and Operator Workflow
### Required UI Behaviors
- Always-on banner: sensor, mode, confidence, auth/provenance state.
- Explicit textual interpretation for palette and gain mode.
- Consistent keybinds and controls across cockpit/remote/C2/TUI surfaces.
- One-action reset to safe default display mode.

### Alerting Rules
- Alert tiers: advisory, caution, warning.
- Every alert carries cause + action guidance.
- No ambiguous abbreviations in primary operator labels.

### SAR-Specific Emphasis
- Fast hotspot detect/readability in low-contrast environments.
- Distance-to-target and bearing shown in primary panel.
- Track-loss handling with immediate recovery cues.

## Safety and Security Constraints
- Deny rendering activation if source authorization/provenance is invalid.
- Deny plugin-based display extensions without allowlist/signature validation.
- Log all mode transitions, sensor switches, and algorithm profile changes.
- Fail closed on schema mismatch, unknown keys, or unsupported sensor format.

## Verification Strategy (Proposed)
### Functional
- Deterministic replay of spoofed EO/IR streams (bit-for-bit where practical).
- Mode cycling correctness across all configured display families.
- 2D/3D toggle behavior preserves state and does not break primary tracking view.

### Performance
- Frame latency budget checks (P50/P95/P99).
- Stress tests with mixed EO + IR + point-cloud style inputs.
- Frame-drop cause classification and budget compliance.

### Human Factors
- Readability and alert-comprehension checks with standardized scripts.
- Error recovery path timing and correctness.
- Color-independence verification for critical cues.

### Safety/Security
- Invalid provenance/authorization inputs force denial.
- Unsupported pixel formats and malformed packets fail closed.
- Audit evidence contains mode/source/reason fields.

### Usability Validation
- Display mode recognition test: operator identifies active mode/palette in constrained time.
- Recovery test: operator resolves forced sensor dropout using on-screen guidance only.
- Cross-surface consistency test: cockpit/remote/C2/TUI show identical reason semantics.

## Candidate Configuration Additions
- `front_view.enabled`
- `front_view.display_families`
- `front_view.auto_cycle.enabled`
- `front_view.auto_cycle.order`
- `front_view.auto_cycle.interval_ms`
- `front_view.render.latency_budget_ms`
- `front_view.proximity.max_range_m`
- `front_view.frame.max_age_ms`
- `front_view.frame.min_confidence`
- `front_view.multi_view.max_streams`
- `front_view.multi_view.stream_ids`
- `front_view.stabilization.enabled`
- `front_view.stabilization.mode`
- `front_view.gimbal.enabled`
- `front_view.gimbal.max_yaw_rate_deg_s`
- `front_view.gimbal.max_pitch_rate_deg_s`
- `front_view.spoof.*`
- `front_view.threading.*`

## Suggested Implementation Phases
1. Contract and scaffolding:
- Add normalized frame contract and spoof source interface.
- Add one baseline EO grayscale display + deterministic cycle framework.

2. IR and palette support:
- Add thermal modes (white-hot/black-hot/false-color) with audit fields.
- Add latency and frame-drop reason telemetry.

3. 2D/3D proximity:
- Add 2D proximity panel and 3D stream panel with independent budgets.
- Add decimation controls and profile-bound defaults.

4. Human factors hardening:
- Complete full-surface UI/TUI wording and alert audit.
- Validate color-independence and recovery-path quality.

5. Security/safety closure:
- Tie all display source activations to auth/provenance gates.
- Finalize verification evidence and traceability links.

## Additional Suggestions
- Keep the front-view module as a separate UI submodule with versioned interface to avoid coupling with core tracking logic.
- Define platform profile presets for rendering budgets (handheld vs fixed-site vs air/maritime) rather than per-device ad hoc tuning.
- Add a playback harness that can scrub frame-by-frame with synchronized metadata to speed incident review and test triage.
- Plan a conformance bundle for external consumers: sample envelopes, schema tests, and reference renders per mode.
- Add a "degraded vision" compatibility option set (text size, contrast mode, reduced clutter) for stressful SAR/night operations.
- Add a deterministic "operator training profile" that replays curated edge-case scenes with expected display outcomes.

## Source Notes (for this draft)
- MIL-STD-1472 listing (ASSIST Quick Search): https://quicksearch.dla.mil/qsDocDetails.aspx?ident_number=36092
- MIL-STD-2525 listing (ASSIST Quick Search): https://quicksearch.dla.mil/qsDocDetails.aspx?ident_number=14828
- MISB charter/mission (GWG): https://nsgreg.nga.mil/misb.jsp
- MISB FAQ and standards context (GWG): https://gwg.nga.mil/misb/faq.html
- MISB standards listing (NSG Registry): https://nsgreg.nga.mil/misb.jsp?col=published_standards
- IMO Radar performance standard MSC.192(79): https://rise.odessa.ua/texts/MSC192_79e.php3
- OpenCV color conversions: https://docs.opencv.org/4.x/de/d25/imgproc_color_conversions.html
- OpenCV CLAHE API: https://docs.opencv.org/4.x/d6/db6/classcv_1_1CLAHE.html
- OpenCV colormap API: https://docs.opencv.org/4.x/d3/d50/group__imgproc__colormap.html
- OpenCV Canny tutorial reference: https://docs.opencv.org/4.x/da/d22/tutorial_py_canny.html
- OpenCV thresholding/Otsu reference: https://docs.opencv.org/4.x/d7/d4d/tutorial_py_thresholding.html
- OpenCV bilateral filtering reference: https://docs.opencv.org/4.x/dd/d6a/tutorial_js_filtering.html
- OpenCV Farneback optical flow API reference: https://docs.opencv.org/4.x/dc/d6b/group__video__track.html
- Vulkan threading guidance: https://docs.vulkan.org/guide/latest/threading.html
