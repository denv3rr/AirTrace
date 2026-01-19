<div>
  <div>

  ### **AirTrace**
  
  ![GitHub top language](https://img.shields.io/github/languages/top/denv3rr/AirTrace)
  ![C++ Standard](https://img.shields.io/badge/c%2B%2B-17+-blue)
  ![CMake Min Version Required](https://img.shields.io/badge/cmake_min_vers_req-3.10+-green)
  ![Build System](https://img.shields.io/badge/build-cmake%2Fninja-blue)
  ![Platform](https://img.shields.io/badge/platform-windows%20%7C%20linux%20%7C%20macos-lightgrey)
  ![GitHub Created At](https://img.shields.io/github/created-at/denv3rr/AirTrace)
  ![GitHub repo size](https://img.shields.io/github/repo-size/denv3rr/AirTrace)
  ![GitHub last commit (branch)](https://img.shields.io/github/last-commit/denv3rr/AirTrace/main)

  <br><br>
[About](#a-motion-and-sensor-core-cli) | [Run and Debug](#run-and-debug) | [Documentation](#documentation) | [Links](#links) | [seperet.com](https://seperet.com)
    <br></br>
  <a href="https://seperet.com">
    <img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif">
  </a>      
</div>

<div align="left">
<br></br>
<div>
  <a href="https://seperet.com">
    <img width="100" src="https://github.com/denv3rr/denv3rr/blob/main/IMG_4225.gif" />
  </a>
</div>

### Motion and Sensor Core

automates:

  - multi‑sensor input
  - mode switching
  - sensor failure/error handling

Includes a core library for motion models and sensor behavior, and a command line interface for running tests/scenarios.

Focuses:

  - State estimation, sensor modeling, and robust fallback behavior
  under degraded inputs.
  - Fallback sensor models for GNSS, IMU, radar, thermal, vision,
  lidar, magnetometer, baro, celestial, and dead reckoning.

Navigation fallback ladder and platform profiles are documented in
`docs/navigation_fallbacks.md` with government and university sources.

The fallback ladder prefers GNSS and tightly coupled inertial updates,
then vision/lidar or radar/terrain aids, and only enables celestial
navigation when higher-priority sources are unavailable or disallowed.

<br></br>

<br></br>
  
**Built with:**

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white)
<br></br>

<!--
**Features:**
<details><summary>Expand</summary>

  - **Kalman Filtering:** Optimal state estimation, ensuring accurate position tracking even in noisy conditions.
   
  - **Heat Signature Detection:** Integrates thermal data to enhance tracking capabilities, enabling the identification of objects based on their heat emissions.
   
  - **Predictive Algorithms:** Employ quick prediction techniques to anticipate movements and improve response times in any potential scenarios.
    
  - **Dead Reckoning Failsafe:** Maintains tracking integrity when GPS or thermal inputs are compromised, ensuring continuous operation through smart estimations of position.
    
  - **Real-Time and Test Modes:** Easily switch between real-time tracking and a comprehensive test mode for simulations and performance evaluations, facilitating robust development and debugging.

</details>
-->

</div>

<br></br>
<br></br>
<br></br>

<img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif">

### **Run and Debug**

<details><summary>Expand</summary>

<br></br>

<div align="left">

**Quick start (cross-platform):**

- Clone with submodules: `git clone --recurse-submodules https://github.com/denv3rr/AirTrace`
- `cd AirTrace`
- If already cloned: `git submodule update --init --recursive`
- Build: `./scripts/build.sh` (macOS/Linux) or `.\scripts\build.ps1` (Windows)
- Run main TUI: `./build/AirTrace` (Windows: `.\build\AirTrace.exe`)
- Run sim demo: `./build/AirTraceSimExample configs/sim_default.cfg`
- Tests: `./scripts/test.sh` (macOS/Linux) or `.\scripts\test.ps1` (Windows)

**Build commands menu:**

- Scripts (recommended): `./scripts/build.sh` or `.\scripts\build.ps1`
- Manual configure: `cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++`
- Manual build: `cmake --build build --target AirTrace AirTraceExample AirTraceSimExample`
- Manual test: `cmake --build build --target AirTraceTests` (if available)

**TUI controls:**

- Use Up/Down to move, Space or Enter to select, Esc to go back/exit.

**Run targets:**

| Purpose | Command |
| --- | --- |
| Run main TUI | `./build/AirTrace` (Windows: `.\build\AirTrace.exe`) |
| Run CLI demo | `./build/AirTraceExample` (Windows: `.\build\AirTraceExample.exe`) |
| Run 3D sim demo | `./build/AirTraceSimExample` (Windows: `.\build\AirTraceSimExample.exe`) |

**Flags (sim demo):**

| Flag | Description | Example |
| --- | --- | --- |
| `CONFIG_PATH` (arg 1) | Path to the sim config file | `./build/AirTraceSimExample configs/sim_default.cfg` |

**Clone, pull, and submodules:**

- Clone (with submodules): `git clone --recurse-submodules https://github.com/denv3rr/AirTrace`
- `cd AirTrace`
- Pull updates (including submodules):
  - `git pull --recurse-submodules`
  - `git submodule update --recursive`
- `.zip` (no submodules): https://github.com/denv3rr/AirTrace/archive/refs/heads/main.zip
</div>
</details>
<br></br>
<br></br>
<br></br>

<img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif">

### **Documentation**

<details>
<summary>Expand</summary>
<br></br>

<div align="left">

- Core architecture: [`docs/architecture.md`](docs/architecture.md)
- Data sources: [`docs/data_sources.md`](docs/data_sources.md)
- Requirements: [`docs/requirements.md`](docs/requirements.md)
- Verification plan: [`docs/verification_plan.md`](docs/verification_plan.md)
- Traceability: [`docs/traceability.md`](docs/traceability.md)
- Hazard log: [`docs/hazard_log.md`](docs/hazard_log.md)
- Security threat model: [`docs/security_threat_model.md`](docs/security_threat_model.md)
- Config schema: [`docs/config_schema.md`](docs/config_schema.md)
- Operational concepts: [`docs/operational_concepts.md`](docs/operational_concepts.md)
- Navigation fallbacks: [`docs/navigation_fallbacks.md`](docs/navigation_fallbacks.md)
- Standards and sources: [`docs/navigation_fallbacks.md`](docs/navigation_fallbacks.md)
- UI/TUI standards: [`docs/ui_standards.md`](docs/ui_standards.md)

</div>
</details>

<br></br>
<br></br>
<br></br>

<img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif">

</details>
<br></br>
<br></br>
<br></br>
<img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif">

### **Links**

<a href="/AirTrace/sources_menu.md">**Sources Menu**</a>

</div>
</details>

<br></br>
<div align="center">
  <a href="https://seperet.com">
    <img width="300" src="https://github.com/denv3rr/denv3rr/blob/main/IMG_4225.gif" />
  </a>
</div>
<br></br>
<br></br>
