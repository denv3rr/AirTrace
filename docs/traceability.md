# Traceability Matrix

Trace links are updated at each review gate. "TBD" indicates incomplete design/code linkage.

| Requirement ID | Design Doc | Code Reference | Verification ID |
| --- | --- | --- | --- |
| REQ-SYS-001 | docs/operational_concepts.md | src/core/sim_config.cpp | V-001 |
| REQ-SYS-002 | docs/architecture.md | src/core/* | V-002 |
| REQ-SYS-003 | docs/operational_concepts.md | src/core/mode_manager.cpp | V-003 |
| REQ-SYS-004 | docs/architecture.md | src/core/sensors.cpp | V-004 |
| REQ-SYS-005 | docs/hazard_log.md | src/core/* | V-005 |
| REQ-SYS-006 | docs/security_threat_model.md | TBD | V-006 |
| REQ-SYS-007 | docs/operational_concepts.md | TBD | V-032 |
| REQ-SYS-008 | docs/navigation_fallbacks.md | src/core/mode_manager.cpp | V-033 |
| REQ-SYS-009 | docs/operational_concepts.md | src/core/sim_config.cpp | V-041 |
| REQ-SYS-010 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-049 |
| REQ-SYS-011 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-056 |
| REQ-SYS-012 | docs/architecture.md | scripts/build.ps1; scripts/build.sh; scripts/run.ps1; scripts/run.sh; scripts/test.ps1; scripts/test.sh | V-067 |
| REQ-SYS-013 | docs/platform_profile_inheritance.md | src/core/sim_config.cpp | V-073 |
| REQ-FUNC-001 | docs/architecture.md | src/core/sensors.cpp | V-007 |
| REQ-FUNC-002 | docs/operational_concepts.md | src/core/mode_manager.cpp | V-008 |
| REQ-FUNC-003 | docs/config_schema.md | src/core/sim_config.cpp | V-009 |
| REQ-FUNC-004 | docs/operational_concepts.md | TBD | V-010 |
| REQ-FUNC-005 | docs/architecture.md | src/core/state.cpp | V-011 |
| REQ-FUNC-006 | docs/operational_concepts.md | src/core/motion_models.cpp | V-012 |
| REQ-FUNC-007 | docs/operational_concepts.md | src/core/mode_manager.cpp | V-013 |
| REQ-FUNC-008 | docs/navigation_fallbacks.md | src/core/mode_manager.cpp | V-034 |
| REQ-FUNC-009 | docs/security_threat_model.md | src/core/hash.cpp | V-035 |
| REQ-FUNC-010 | docs/navigation_fallbacks.md | src/core/sim_config.cpp | V-042 |
| REQ-FUNC-011 | docs/navigation_fallbacks.md | src/core/sensors.cpp | V-046 |
| REQ-FUNC-012 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-050 |
| REQ-FUNC-013 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-051 |
| REQ-FUNC-014 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-052 |
| REQ-FUNC-015 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-053 |
| REQ-FUNC-016 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-057 |
| REQ-FUNC-017 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-058 |
| REQ-FUNC-018 | docs/edge_case_catalog.md | TBD | V-060 |
| REQ-FUNC-019 | docs/edge_case_catalog.md | TBD | V-061 |
| REQ-FUNC-020 | docs/edge_case_catalog.md | TBD | V-063 |
| REQ-FUNC-021 | docs/edge_case_catalog.md | TBD | V-065 |
| REQ-PERF-001 | docs/architecture.md | TBD | V-014 |
| REQ-PERF-002 | docs/config_schema.md | src/core/sensors.cpp | V-015 |
| REQ-SAFE-001 | docs/hazard_log.md | TBD | V-016 |
| REQ-SAFE-002 | docs/hazard_log.md | TBD | V-017 |
| REQ-SAFE-003 | docs/hazard_log.md | src/core/mode_manager.cpp | V-018 |
| REQ-SAFE-004 | docs/hazard_log.md | src/core/motion_models.cpp | V-019 |
| REQ-SAFE-005 | docs/hazard_log.md | TBD | V-036 |
| REQ-SAFE-006 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-054 |
| REQ-SAFE-007 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-059 |
| REQ-SAFE-008 | docs/edge_case_catalog.md | TBD | V-062 |
| REQ-SAFE-009 | docs/edge_case_catalog.md | TBD | V-064 |
| REQ-SEC-001 | docs/security_threat_model.md | src/core/sim_config.cpp | V-020 |
| REQ-SEC-002 | docs/security_threat_model.md | TBD | V-021 |
| REQ-SEC-003 | docs/security_threat_model.md | TBD | V-022 |
| REQ-SEC-004 | docs/security_threat_model.md | TBD | V-023 |
| REQ-SEC-005 | docs/security_threat_model.md | TBD | V-037 |
| REQ-SEC-006 | docs/security_threat_model.md | TBD | V-038 |
| REQ-SEC-007 | docs/security_threat_model.md | TBD | V-043 |
| REQ-SEC-008 | docs/multi_modal_switching_design.md | include/core/multi_modal_types.h | V-055 |
| REQ-INT-001 | docs/architecture.md | include/core/*.h | V-024 |
| REQ-INT-002 | docs/ui_standards.md | src/ui/*.cpp | V-025 |
| REQ-INT-003 | docs/ui_standards.md | src/ui/*.cpp | V-026 |
| REQ-INT-004 | docs/operational_concepts.md | TBD | V-039 |
| REQ-INT-005 | docs/ui_standards.md | src/ui/simulation.cpp | V-045 |
| REQ-INT-006 | docs/ui_standards.md | src/ui/menu.cpp; src/ui/menu_selection.cpp | V-066 |
| REQ-INT-007 | docs/ui_standards.md | src/ui/menu.cpp; src/ui/alerts.cpp | V-069 |
| REQ-INT-008 | docs/ui_standards.md | src/ui/input_harness.cpp; src/ui/tui.cpp; src/ui/inputValidation.cpp | V-070 |
| REQ-INT-009 | docs/ui_standards.md | src/ui/tui.cpp | V-075 |
| REQ-CFG-001 | docs/config_schema.md | src/core/sim_config.cpp | V-027 |
| REQ-CFG-002 | docs/config_schema.md | src/core/sim_config.cpp | V-028 |
| REQ-CFG-003 | docs/config_schema.md | docs/config_schema.md | V-029 |
| REQ-CFG-004 | docs/config_schema.md | src/core/sim_config.cpp | V-040 |
| REQ-CFG-005 | docs/config_schema.md | src/core/sim_config.cpp | V-044 |
| REQ-CFG-006 | docs/config_schema.md | src/core/sim_config.cpp | V-047 |
| REQ-CFG-007 | docs/config_schema.md | src/core/sim_config.cpp | V-074 |
| REQ-VER-001 | docs/verification_plan.md | docs/verification_plan.md | V-030 |
| REQ-VER-002 | docs/verification_plan.md | tests/core_sanity.cpp | V-031 |
| REQ-VER-003 | docs/verification_plan.md | scripts/test.ps1; scripts/test.sh | V-068 |
| REQ-VER-004 | docs/verification_plan.md | src/ui/input_harness.cpp | V-071 |
| REQ-VER-005 | docs/verification_plan.md | scripts/test.ps1; scripts/test.sh | V-072 |
| REQ-VER-006 | docs/verification_plan.md | tests/integration_flows.cpp | V-076 |
| REQ-DOC-001 | docs/requirements.md | README.md | V-048 |
