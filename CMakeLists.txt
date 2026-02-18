cmake_minimum_required(VERSION 3.10)

# Project name
project(AirTrace VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
option(AIRTRACE_BUILD_ADAPTER_SDK "Build adapter SDK module target" ON)

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
        src/core/mode_scheduler.cpp
        src/core/logging.cpp
        src/core/hash.cpp
        src/core/plugin_auth.cpp
)

target_include_directories(airtrace_core PUBLIC include)
set_target_properties(airtrace_core PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
)
target_compile_definitions(airtrace_core PUBLIC AIRTRACE_CORE_CONTRACT_VERSION="${PROJECT_VERSION}")

# Adapter contract module (optional extension support; core remains adapter-agnostic).
add_library(airtrace_adapters_contract
        src/core/adapter_registry.cpp
)
target_include_directories(airtrace_adapters_contract PUBLIC include)
target_link_libraries(airtrace_adapters_contract PUBLIC airtrace_core)
set_target_properties(airtrace_adapters_contract PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
)
target_compile_definitions(airtrace_adapters_contract PUBLIC AIRTRACE_ADAPTER_CONTRACT_VERSION="${PROJECT_VERSION}")

# Tools library
add_library(airtrace_tools
        src/tools/sim_config_loader.cpp
        src/tools/audit_log.cpp
        src/tools/io_packager.cpp
        src/tools/adapter_registry_loader.cpp
)
target_include_directories(airtrace_tools PUBLIC include)
target_link_libraries(airtrace_tools PUBLIC airtrace_core airtrace_adapters_contract)
set_target_properties(airtrace_tools PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
)
target_compile_definitions(airtrace_tools PUBLIC AIRTRACE_TOOLS_CONTRACT_VERSION="${PROJECT_VERSION}")

# UI module
add_library(airtrace_ui
        src/ui/menu.cpp
        src/ui/menu_selection.cpp
        src/ui/alerts.cpp
        src/ui/tui.cpp
        src/ui/adapter_ui_mapping.cpp
        src/ui/front_view.cpp
        src/ui/simulation.cpp
        src/ui/inputValidation.cpp
        src/ui/scenario.cpp
)
target_include_directories(airtrace_ui PUBLIC include)
target_link_libraries(airtrace_ui PUBLIC airtrace_core airtrace_tools)
set_target_properties(airtrace_ui PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
)
target_compile_definitions(airtrace_ui PUBLIC AIRTRACE_UI_CONTRACT_VERSION="${PROJECT_VERSION}")

# UI harness variant for deterministic non-interactive tests.
add_library(airtrace_ui_harness
        src/ui/menu.cpp
        src/ui/menu_selection.cpp
        src/ui/alerts.cpp
        src/ui/tui.cpp
        src/ui/adapter_ui_mapping.cpp
        src/ui/front_view.cpp
        src/ui/simulation.cpp
        src/ui/inputValidation.cpp
        src/ui/scenario.cpp
        src/ui/input_harness.cpp
)
target_include_directories(airtrace_ui_harness PUBLIC include)
target_link_libraries(airtrace_ui_harness PUBLIC airtrace_core airtrace_tools)
set_target_properties(airtrace_ui_harness PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
)
target_compile_definitions(airtrace_ui_harness PUBLIC AIRTRACE_UI_CONTRACT_VERSION="${PROJECT_VERSION}" AIRTRACE_TEST_HARNESS=1)

# CLI application
add_executable(AirTrace
        src/ui/main.cpp
)

target_include_directories(AirTrace PRIVATE include)
target_link_libraries(AirTrace PRIVATE airtrace_ui)
if(NOT DEFINED AIRTRACE_BUILD_ID)
    set(AIRTRACE_BUILD_ID "${PROJECT_NAME}-${PROJECT_VERSION}")
    if(CMAKE_BUILD_TYPE)
        set(AIRTRACE_BUILD_ID "${AIRTRACE_BUILD_ID}-${CMAKE_BUILD_TYPE}")
    endif()
endif()
target_compile_definitions(AirTrace PRIVATE AIRTRACE_BUILD_ID="${AIRTRACE_BUILD_ID}")

if(AIRTRACE_BUILD_ADAPTER_SDK)
    add_subdirectory(adapters/sdk)
endif()

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
target_link_libraries(AirTraceSimExample PRIVATE airtrace_tools)

add_executable(AirTraceIoPackager
        examples/io_packager.cpp
)
target_include_directories(AirTraceIoPackager PRIVATE include)
target_link_libraries(AirTraceIoPackager PRIVATE airtrace_tools)

enable_testing()
add_executable(AirTraceCoreTests
        tests/core_sanity.cpp
)
target_include_directories(AirTraceCoreTests PRIVATE include)
target_link_libraries(AirTraceCoreTests PRIVATE airtrace_core airtrace_tools)
add_test(NAME AirTraceCoreTests COMMAND AirTraceCoreTests)

add_executable(AirTraceEdgeCaseTests
        tests/edge_case_integration.cpp
)
target_include_directories(AirTraceEdgeCaseTests PRIVATE include)
target_link_libraries(AirTraceEdgeCaseTests PRIVATE airtrace_core)
add_test(NAME AirTraceEdgeCaseTests COMMAND AirTraceEdgeCaseTests)

add_executable(AirTraceUiTests
        tests/ui_ui_tests_main.cpp
        tests/ui_menu_selection.cpp
        tests/ui_alerts.cpp
        tests/ui_status.cpp
        tests/front_view_display.cpp
)
target_include_directories(AirTraceUiTests PRIVATE include)
target_link_libraries(AirTraceUiTests PRIVATE airtrace_ui)
add_test(NAME AirTraceUiTests COMMAND AirTraceUiTests)

add_executable(AirTraceHarnessRunner
        tests/ui_harness_runner.cpp
)
target_include_directories(AirTraceHarnessRunner PRIVATE include)
target_link_libraries(AirTraceHarnessRunner PRIVATE airtrace_ui_harness)

add_executable(AirTraceHarnessTests
        tests/ui_harness_tests.cpp
        src/ui/input_harness.cpp
)
target_include_directories(AirTraceHarnessTests PRIVATE include)
target_compile_definitions(AirTraceHarnessTests PRIVATE AIRTRACE_TEST_HARNESS=1)
add_test(NAME AirTraceHarnessTests COMMAND AirTraceHarnessTests)

add_executable(AirTraceIntegrationTests
        tests/integration_flows.cpp
)
target_include_directories(AirTraceIntegrationTests PRIVATE include)
target_link_libraries(AirTraceIntegrationTests PRIVATE airtrace_ui_harness)
add_test(NAME AirTraceIntegrationTests COMMAND AirTraceIntegrationTests)

add_test(
        NAME AirTraceModuleDependencyCheck
        COMMAND ${CMAKE_COMMAND}
        -DROOT_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_SOURCE_DIR}/cmake/check_module_dependencies.cmake
)
