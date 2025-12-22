# Examples

This folder is reserved for end-to-end usage examples and reproducible demos.

Planned examples:
- Scenario mode walkthrough with default parameters.
- Test mode tracking profiles (prediction, Kalman, heat signature, GPS).

## CLI demo

Build with CMake:

- `cmake -S . -B build`
- `cmake --build build --target AirTraceExample`
- `cmake --build build --target AirTraceSimExample`

Run:

- `./build/AirTraceExample` (or `./build/Debug/AirTraceExample` on multi-config generators)
- `./build/AirTraceSimExample` (or `./build/Debug/AirTraceSimExample` on multi-config generators)
- `./build/AirTraceSimExample configs/sim_default.cfg`
