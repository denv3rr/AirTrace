cmake_minimum_required(VERSION 3.10)

# Project name
project(AirTrace VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Core library
add_library(airtrace_core
        src/core/Object.cpp
        src/core/Tracker.cpp
        src/core/PathCalculator.cpp
        src/core/PredictionAlgorithm.cpp
        src/core/KalmanFilter.cpp
        src/core/HeatSignature.cpp
        src/core/DeadReckoning.cpp
        src/core/GPSAlgorithm.cpp
        src/core/simulation_utils.cpp
        src/core/state.cpp
        src/core/motion_models.cpp
        src/core/sensors.cpp
        src/core/mode_manager.cpp
        src/core/sim_config.cpp
        src/core/logging.cpp
)

target_include_directories(airtrace_core PUBLIC include)

# CLI application
add_executable(AirTrace
        src/ui/main.cpp
        src/ui/menu.cpp
        src/ui/tui.cpp
        src/ui/simulation.cpp
        src/ui/inputValidation.cpp
        src/ui/scenario.cpp
)

target_include_directories(AirTrace PRIVATE include)
target_link_libraries(AirTrace PRIVATE airtrace_core)

# Examples
add_executable(AirTraceExample
        examples/cli_demo.cpp
)
target_include_directories(AirTraceExample PRIVATE include)
target_link_libraries(AirTraceExample PRIVATE airtrace_core)

add_executable(AirTraceSimExample
        examples/sim_demo.cpp
)
target_include_directories(AirTraceSimExample PRIVATE include)
target_link_libraries(AirTraceSimExample PRIVATE airtrace_core)

enable_testing()
add_executable(AirTraceCoreTests
        tests/core_sanity.cpp
)
target_include_directories(AirTraceCoreTests PRIVATE include)
target_link_libraries(AirTraceCoreTests PRIVATE airtrace_core)
add_test(NAME AirTraceCoreTests COMMAND AirTraceCoreTests)
