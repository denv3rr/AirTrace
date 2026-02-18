# Traceability Matrix

Trace links are updated at each review gate. "TBD" indicates incomplete design/code linkage.

| Requirement ID | Design Doc | Code Reference | Verification ID |
| --- | --- | --- | --- |
| REQ-SYS-001 | docs/operational_concepts.md | src/tools/sim_config_loader.cpp | V-001 |
| REQ-SYS-002 | docs/architecture.md | src/core/* | V-002 |
| REQ-SYS-003 | docs/operational_concepts.md | src/core/mode_manager.cpp | V-003 |
| REQ-SYS-004 | docs/architecture.md | src/core/sensors.cpp | V-004 |
| REQ-SYS-005 | docs/hazard_log.md | src/core/* | V-005 |
| REQ-SYS-006 | docs/security_threat_model.md | src/tools/audit_log.cpp; src/ui/main.cpp | V-006 |
| REQ-SYS-007 | docs/operational_concepts.md | src/tools/sim_config_loader.cpp; include/core/sim_config.h | V-032 |
| REQ-SYS-008 | docs/navigation_fallbacks.md | src/core/mode_manager.cpp | V-033 |
| REQ-SYS-009 | docs/operational_concepts.md | src/tools/sim_config_loader.cpp | V-041 |
| REQ-SYS-010 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-049 |
| REQ-SYS-011 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-056 |
| REQ-SYS-012 | docs/architecture.md | scripts/build.ps1; scripts/build.sh; scripts/run.ps1; scripts/run.sh; scripts/test.ps1; scripts/test.sh | V-067 |
| REQ-SYS-013 | docs/platform_profile_inheritance.md | src/tools/sim_config_loader.cpp | V-073 |
| REQ-SYS-014 | docs/operational_concepts.md | src/ui/menu.cpp; src/ui/simulation.cpp | V-078 |
| REQ-SYS-015 | docs/architecture.md | src/core/*; src/core/simulation_utils.cpp; include/core/simulation_utils.h | V-079; V-093; V-120 |
| REQ-SYS-016 | docs/security_threat_model.md | src/tools/audit_log.cpp; src/ui/main.cpp | V-080 |
| REQ-SYS-017 | docs/operational_concepts.md | src/core/sensors.cpp; include/core/sensors.h; include/core/provenance.h | V-088 |
| REQ-SYS-018 | docs/operational_concepts.md | src/core/mode_manager.cpp | V-089 |
| REQ-SYS-019 | docs/multi_modal_switching_design.md | src/core/mode_manager.cpp | V-095 |
| REQ-SYS-020 | docs/architecture.md; docs/module_contracts.md | src/tools/sim_config_loader.cpp; src/tools/audit_log.cpp | V-114 |
| REQ-SYS-021 | docs/operational_concepts.md | src/core/Tracker.cpp | V-119 |
| REQ-SYS-022 | docs/operational_concepts.md; docs/agents_research/mission_scope_audit_2026-02-18.md | TBD (planned: src/tools/sim_config_loader.cpp; src/ui/main.cpp) | V-140 |
| REQ-FUNC-001 | docs/architecture.md | src/core/sensors.cpp | V-007 |
| REQ-FUNC-002 | docs/operational_concepts.md | src/core/mode_manager.cpp | V-008 |
| REQ-FUNC-003 | docs/config_schema.md | src/tools/sim_config_loader.cpp | V-009 |
| REQ-FUNC-004 | docs/operational_concepts.md | src/core/simulation_utils.cpp; src/tools/sim_config_loader.cpp; src/ui/simulation.cpp | V-010 |
| REQ-FUNC-005 | docs/architecture.md | src/core/state.cpp | V-011 |
| REQ-FUNC-006 | docs/operational_concepts.md | src/core/motion_models.cpp | V-012 |
| REQ-FUNC-007 | docs/operational_concepts.md | src/core/mode_manager.cpp | V-013 |
| REQ-FUNC-008 | docs/navigation_fallbacks.md | src/core/mode_manager.cpp | V-034 |
| REQ-FUNC-009 | docs/security_threat_model.md | src/core/hash.cpp | V-035 |
| REQ-FUNC-010 | docs/navigation_fallbacks.md | src/tools/sim_config_loader.cpp | V-042 |
| REQ-FUNC-011 | docs/navigation_fallbacks.md | src/core/sensors.cpp | V-046 |
| REQ-FUNC-012 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-050 |
| REQ-FUNC-013 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-051 |
| REQ-FUNC-014 | docs/multi_modal_switching_design.md | src/core/mode_manager.cpp; include/core/multi_modal_types.h | V-052 |
| REQ-FUNC-015 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-053 |
| REQ-FUNC-016 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-057 |
| REQ-FUNC-017 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-058 |
| REQ-FUNC-018 | docs/edge_case_catalog.md | src/core/mode_manager.cpp | V-060 |
| REQ-FUNC-019 | docs/edge_case_catalog.md | src/core/mode_manager.cpp | V-061 |
| REQ-FUNC-020 | docs/edge_case_catalog.md | src/core/mode_manager.cpp | V-063 |
| REQ-FUNC-021 | docs/edge_case_catalog.md | src/core/mode_manager.cpp | V-065 |
| REQ-FUNC-022 | docs/multi_modal_switching_design.md | src/core/mode_manager.cpp | V-096 |
| REQ-FUNC-023 | docs/multi_modal_switching_design.md | src/core/mode_manager.cpp (policy only) | V-097 |
| REQ-FUNC-024 | docs/operational_concepts.md | src/core/simulation_utils.cpp; include/core/simulation_utils.h | V-120 |
| REQ-FUNC-025 | docs/operational_concepts.md | src/core/HeatSignature.cpp | V-121 |
| REQ-PERF-001 | docs/architecture.md | TBD (not implemented) | V-014 |
| REQ-PERF-002 | docs/config_schema.md | src/core/sensors.cpp | V-015 |
| REQ-SAFE-001 | docs/hazard_log.md | src/core/mode_manager.cpp; src/tools/sim_config_loader.cpp | V-016 |
| REQ-SAFE-002 | docs/hazard_log.md | src/tools/audit_log.cpp | V-017 |
| REQ-SAFE-003 | docs/hazard_log.md | src/core/mode_manager.cpp | V-018 |
| REQ-SAFE-004 | docs/hazard_log.md | src/core/motion_models.cpp | V-019 |
| REQ-SAFE-005 | docs/hazard_log.md | src/core/mode_manager.cpp | V-036 |
| REQ-SAFE-006 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-054 |
| REQ-SAFE-007 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-059 |
| REQ-SAFE-008 | docs/edge_case_catalog.md | src/core/mode_manager.cpp | V-062 |
| REQ-SAFE-009 | docs/edge_case_catalog.md | src/core/mode_manager.cpp | V-064 |
| REQ-SAFE-010 | docs/hazard_log.md | src/tools/audit_log.cpp | V-081 |
| REQ-SAFE-011 | docs/hazard_log.md | src/core/mode_manager.cpp (authorization path only) | V-098 |
| REQ-SAFE-012 | docs/adapter_architecture.md | src/core/adapter_registry.cpp; src/tools/adapter_registry_loader.cpp | V-111 |
| REQ-SEC-001 | docs/security_threat_model.md | src/tools/sim_config_loader.cpp | V-020 |
| REQ-SEC-002 | docs/security_threat_model.md | src/core/plugin_auth.cpp; include/core/plugin_auth.h; src/tools/sim_config_loader.cpp; include/core/sim_config.h | V-021 |
| REQ-SEC-003 | docs/security_threat_model.md | src/core/plugin_auth.cpp; include/core/plugin_auth.h; src/tools/sim_config_loader.cpp; include/core/sim_config.h | V-022 |
| REQ-SEC-004 | docs/security_threat_model.md | src/tools/audit_log.cpp | V-023 |
| REQ-SEC-005 | docs/security_threat_model.md | TBD (planned: src/core/network_aid_policy.cpp; include/core/network_aid_policy.h) | V-037 |
| REQ-SEC-006 | docs/security_threat_model.md | TBD (planned: src/core/network_aid_policy.cpp; include/core/network_aid_policy.h) | V-038 |
| REQ-SEC-007 | docs/security_threat_model.md | TBD (planned: src/core/network_aid_policy.cpp; include/core/network_aid_policy.h) | V-043 |
| REQ-SEC-008 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-055 |
| REQ-SEC-009 | docs/security_threat_model.md | src/tools/audit_log.cpp | V-082 |
| REQ-SEC-010 | docs/security_threat_model.md | src/tools/audit_log.cpp; include/tools/audit_log.h; src/ui/simulation.cpp | V-090 |
| REQ-SEC-011 | docs/security_threat_model.md | src/tools/sim_config_loader.cpp; src/core/mode_manager.cpp; src/ui/simulation.cpp | V-097 |
| REQ-SEC-012 | docs/adapter_architecture.md; docs/security_threat_model.md | src/core/adapter_registry.cpp; src/tools/adapter_registry_loader.cpp; adapters/allowlist.json | V-113 |
| REQ-SEC-013 | docs/security_threat_model.md; docs/adapter_contract.md | src/tools/adapter_registry_loader.cpp | V-118 |
| REQ-SEC-014 | docs/security_threat_model.md; docs/module_contracts.md | src/tools/federation_bridge.cpp; src/tools/audit_log.cpp; tests/federation_bridge.cpp | V-139 |
| REQ-INT-001 | docs/architecture.md | include/core/*.h | V-024 |
| REQ-INT-002 | docs/ui_standards.md | src/ui/*.cpp | V-025 |
| REQ-INT-003 | docs/ui_standards.md | src/ui/*.cpp | V-026 |
| REQ-INT-004 | docs/operational_concepts.md | src/ui/simulation.cpp; src/ui/menu.cpp | V-039 |
| REQ-INT-005 | docs/ui_standards.md | src/ui/simulation.cpp | V-045 |
| REQ-INT-006 | docs/ui_standards.md | src/ui/menu.cpp; src/ui/menu_selection.cpp | V-066 |
| REQ-INT-007 | docs/ui_standards.md | src/ui/menu.cpp; src/ui/alerts.cpp | V-069 |
| REQ-INT-008 | docs/ui_standards.md | src/ui/input_harness.cpp; src/ui/tui.cpp; src/ui/inputValidation.cpp | V-070 |
| REQ-INT-009 | docs/ui_standards.md | src/ui/tui.cpp | V-075 |
| REQ-INT-010 | docs/ui_standards.md | include/ui/input_harness.h; src/ui/input_harness.cpp | V-077 |
| REQ-INT-011 | docs/ui_standards.md | src/ui/simulation.cpp; src/ui/scenario.cpp | V-083 |
| REQ-INT-012 | docs/ui_standards.md | src/ui/simulation.cpp | V-084 |
| REQ-INT-013 | docs/ui_standards.md | src/ui/simulation.cpp; src/ui/scenario.cpp | V-085 |
| REQ-INT-014 | docs/ui_standards.md | src/ui/simulation.cpp; src/ui/menu.cpp | V-086 |
| REQ-INT-015 | docs/ui_standards.md | src/ui/simulation.cpp | V-087 |
| REQ-INT-016 | docs/ui_standards.md | src/ui/simulation.cpp; include/ui/simulation.h; src/ui/alerts.cpp | V-092 |
| REQ-INT-017 | docs/ui_standards.md | src/ui/simulation.cpp; src/ui/menu.cpp; src/ui/alerts.cpp | V-094 |
| REQ-INT-018 | docs/ui_standards.md | src/ui/simulation.cpp; include/ui/simulation.h | V-100 |
| REQ-INT-019 | docs/ui_standards.md | src/ui/simulation.cpp; include/ui/simulation.h | V-100 |
| REQ-INT-020 | docs/ui_standards.md | src/ui/simulation.cpp; src/ui/alerts.cpp | V-100 |
| REQ-INT-021 | docs/ui_standards.md | src/ui/front_view.cpp; src/ui/simulation.cpp; include/ui/front_view.h | V-101 |
| REQ-INT-022 | docs/ui_standards.md | src/ui/front_view.cpp; src/ui/simulation.cpp; include/ui/simulation.h | V-102 |
| REQ-INT-023 | docs/ui_standards.md | include/core/external_io_envelope.h; src/ui/simulation.cpp | V-103 |
| REQ-INT-024 | docs/adapter_architecture.md; docs/adapters/ui_data_points.md | src/ui/adapter_ui_mapping.cpp; include/ui/adapter_ui_mapping.h; src/ui/simulation.cpp | V-104 |
| REQ-INT-025 | docs/ui_standards.md; docs/operational_concepts.md | src/ui/menu.cpp; src/ui/menu_selection.cpp; include/ui/menu_selection.h; src/ui/simulation.cpp | V-122 |
| REQ-INT-026 | docs/ui_standards.md; docs/operational_concepts.md | src/ui/simulation.cpp; include/ui/simulation.h | V-123 |
| REQ-INT-027 | docs/adapters/ui_data_points.md; docs/module_contracts.md | include/core/external_io_envelope.h; src/ui/simulation.cpp; include/ui/simulation.h | V-124 |
| REQ-INT-028 | docs/front_view_display_architecture.md; docs/ui_standards.md | src/ui/menu.cpp; src/ui/front_view.cpp; include/ui/front_view.h; src/ui/simulation.cpp; include/ui/simulation.h | V-125 |
| REQ-INT-029 | docs/front_view_display_architecture.md; docs/security_threat_model.md | src/ui/front_view.cpp; include/ui/front_view.h; src/ui/alerts.cpp; src/ui/menu.cpp; src/ui/simulation.cpp | V-126 |
| REQ-INT-030 | docs/front_view_display_architecture.md; docs/adapters/ui_data_points.md | include/ui/front_view.h; src/ui/front_view.cpp; include/core/external_io_envelope.h; src/ui/simulation.cpp; include/ui/simulation.h | V-128 |
| REQ-INT-031 | docs/front_view_display_architecture.md; docs/operational_concepts.md; docs/security_threat_model.md | src/ui/front_view.cpp; src/tools/sim_config_loader.cpp; include/core/sim_config.h; src/ui/alerts.cpp | V-129 |
| REQ-INT-032 | docs/module_contracts.md; docs/operational_concepts.md | include/tools/io_packager.h; src/tools/io_packager.cpp; examples/io_packager.cpp | V-131 |
| REQ-INT-033 | docs/security_threat_model.md; docs/module_contracts.md | include/tools/io_packager.h; src/tools/io_packager.cpp; tests/core_sanity.cpp | V-132 |
| REQ-INT-034 | docs/ui_standards.md; docs/operational_concepts.md | src/ui/simulation.cpp; include/core/sim_config.h; src/tools/sim_config_loader.cpp | V-135 |
| REQ-INT-035 | docs/module_contracts.md; docs/agents_research/co_simulation_federation_architecture_2026-02-18.md | include/tools/federation_bridge.h; src/tools/federation_bridge.cpp; tests/federation_bridge.cpp | V-136 |
| REQ-INT-036 | docs/module_contracts.md; docs/agents_research/co_simulation_federation_architecture_2026-02-18.md | include/tools/federation_bridge.h; src/tools/federation_bridge.cpp; tests/federation_bridge.cpp | V-137 |
| REQ-INT-037 | docs/module_contracts.md; docs/agents_research/co_simulation_federation_architecture_2026-02-18.md | include/tools/federation_bridge.h; src/tools/federation_bridge.cpp; tests/federation_bridge.cpp | V-138 |
| REQ-INT-038 | docs/module_contracts.md; docs/security_threat_model.md; docs/agents_research/co_simulation_federation_architecture_2026-02-18.md | include/tools/federation_bridge.h; src/tools/federation_bridge.cpp; tests/federation_bridge.cpp | V-139 |
| REQ-INT-039 | docs/ui_standards.md; docs/operational_concepts.md; docs/agents_research/mission_scope_audit_2026-02-18.md | TBD (planned: src/ui/alerts.cpp; src/ui/simulation.cpp; src/ui/menu.cpp) | V-141 |
| REQ-MOD-001 | docs/architecture.md; docs/module_contracts.md; docs/adapter_architecture.md | CMakeLists.txt; adapters/sdk/CMakeLists.txt; scripts/test.ps1; scripts/test.sh | V-105 |
| REQ-MOD-002 | docs/architecture.md; docs/module_contracts.md; docs/adapter_architecture.md | CMakeLists.txt; scripts/test.ps1; scripts/test.sh | V-105 |
| REQ-MOD-003 | docs/architecture.md; docs/module_contracts.md | CMakeLists.txt; cmake/check_module_dependencies.cmake | V-106 |
| REQ-MOD-004 | docs/adapter_architecture.md; docs/adapter_contract.md | src/core/adapter_registry.cpp; src/tools/adapter_registry_loader.cpp | V-107 |
| REQ-MOD-005 | docs/adapter_architecture.md; docs/adapter_contract.md | src/core/adapter_registry.cpp; src/tools/adapter_registry_loader.cpp; adapters/sdk/include/adapter_sdk.h; adapters/sdk/CMakeLists.txt | V-108 |
| REQ-ADP-001 | docs/adapter_architecture.md | adapters/official/air/manifest.json; adapters/official/ground/manifest.json; adapters/official/maritime/manifest.json; adapters/official/space/manifest.json; adapters/official/handheld/manifest.json; adapters/official/fixed_site/manifest.json; adapters/official/subsea/manifest.json | V-109 |
| REQ-ADP-002 | docs/adapter_architecture.md; docs/adapter_contract.md | adapters/official/air/manifest.json; adapters/official/ground/manifest.json; adapters/official/maritime/manifest.json; adapters/official/space/manifest.json; adapters/official/handheld/manifest.json; adapters/official/fixed_site/manifest.json; adapters/official/subsea/manifest.json; src/core/adapter_registry.cpp; src/tools/adapter_registry_loader.cpp | V-110 |
| REQ-ADP-003 | docs/adapter_architecture.md | src/ui/adapter_ui_mapping.cpp; include/ui/adapter_ui_mapping.h; src/ui/simulation.cpp | V-110 |
| REQ-ADP-004 | docs/adapter_architecture.md | src/core/adapter_registry.cpp; src/tools/adapter_registry_loader.cpp | V-111 |
| REQ-ADP-005 | docs/adapter_architecture.md | docs/adapters/air.md; docs/adapters/ground.md; docs/adapters/maritime.md; docs/adapters/space.md; docs/adapters/handheld.md; docs/adapters/fixed_site.md; docs/adapters/subsea.md | V-112 |
| REQ-ADP-006 | docs/adapter_architecture.md | src/core/adapter_registry.cpp; src/tools/adapter_registry_loader.cpp; adapters/allowlist.json | V-113 |
| REQ-ADP-007 | docs/adapter_contract.md | adapters/sdk/adapter_sdk.h; adapters/sdk/adapter_sdk.cpp; adapters/sdk/CMakeLists.txt | V-115 |
| REQ-CFG-001 | docs/config_schema.md | src/tools/sim_config_loader.cpp | V-027 |
| REQ-CFG-002 | docs/config_schema.md | src/tools/sim_config_loader.cpp | V-028 |
| REQ-CFG-003 | docs/config_schema.md | docs/config_schema.md | V-029 |
| REQ-CFG-004 | docs/config_schema.md | src/tools/sim_config_loader.cpp | V-040 |
| REQ-CFG-005 | docs/config_schema.md | src/tools/sim_config_loader.cpp | V-044 |
| REQ-CFG-006 | docs/config_schema.md | src/tools/sim_config_loader.cpp | V-047 |
| REQ-CFG-007 | docs/config_schema.md | src/tools/sim_config_loader.cpp | V-074 |
| REQ-CFG-008 | docs/config_schema.md | src/tools/sim_config_loader.cpp | V-091 |
| REQ-CFG-009 | docs/config_schema.md | src/tools/sim_config_loader.cpp; include/core/sim_config.h | V-099 |
| REQ-CFG-010 | docs/config_schema.md; docs/adapter_architecture.md | src/tools/sim_config_loader.cpp; include/core/sim_config.h; src/tools/adapter_registry_loader.cpp | V-113 |
| REQ-CFG-011 | docs/config_schema.md; docs/adapter_contract.md | src/tools/sim_config_loader.cpp; include/core/sim_config.h; src/tools/adapter_registry_loader.cpp | V-116 |
| REQ-CFG-012 | docs/config_schema.md; docs/adapter_contract.md | src/tools/sim_config_loader.cpp; include/core/sim_config.h | V-117 |
| REQ-CFG-013 | docs/config_schema.md; docs/front_view_display_architecture.md | src/tools/sim_config_loader.cpp; include/core/sim_config.h | V-127 |
| REQ-CFG-014 | docs/config_schema.md; docs/front_view_display_architecture.md | src/tools/sim_config_loader.cpp; include/core/sim_config.h; src/ui/front_view.cpp | V-130 |
| REQ-CFG-015 | docs/config_schema.md; docs/ui_standards.md | src/tools/sim_config_loader.cpp; include/core/sim_config.h; tests/core_sanity.cpp | V-134 |
| REQ-CM-001 | docs/git_process.md; docs/plan.md; AGENTS.md | docs/git_process.md; AGENTS.md | V-133 |
| REQ-CM-002 | docs/git_process.md | .gitignore; docs/git_process.md | V-144 |
| REQ-VER-001 | docs/verification_plan.md | docs/verification_plan.md | V-030 |
| REQ-VER-002 | docs/verification_plan.md | tests/core_sanity.cpp | V-031 |
| REQ-VER-003 | docs/verification_plan.md | scripts/test.ps1; scripts/test.sh | V-068 |
| REQ-VER-004 | docs/verification_plan.md | src/ui/input_harness.cpp | V-071 |
| REQ-VER-005 | docs/verification_plan.md | scripts/test.ps1; scripts/test.sh | V-072 |
| REQ-VER-006 | docs/verification_plan.md | tests/integration_flows.cpp | V-076 |
| REQ-VER-007 | docs/verification_plan.md; docs/agents_research/mission_scope_audit_2026-02-18.md | TBD (planned: tests/integration_flows.cpp; tests/ui_status.cpp; scripts/test.ps1; scripts/test.sh) | V-142 |
| REQ-DOC-001 | docs/requirements.md | README.md | V-048 |
| REQ-DOC-002 | docs/agents_research/mission_scope_audit_2026-02-18.md | TBD (planned: release evidence bundle process) | V-143 |

