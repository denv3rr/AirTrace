# Platform Profile Inheritance

This design note defines how platform profiles inherit defaults and how
child hardware modules are declared. It supports deterministic behavior,
clear auditability, and fail-closed validation.

## Goals
- Provide a deterministic inheritance model for permitted sensors.
- Allow explicit declaration of child hardware modules.
- Keep production behavior unchanged unless configured.

## Inputs
- platform.profile
- platform.profile_parent (optional)
- platform.permitted_sensors (optional)
- platform.child_modules (optional)

## Merge Rules
1) If platform.permitted_sensors is set, it is authoritative and no
   inheritance is applied.
2) If platform.profile_parent is set and platform.permitted_sensors is
   empty, start with the parent profile defaults.
3) Append the child profile defaults in order, skipping duplicates.
4) If platform.profile_parent equals platform.profile, validation fails.

## Child Modules
- platform.child_modules is a comma-separated list of identifiers.
- Identifiers use lowercase letters, numbers, underscore, and hyphen.
- Unknown or malformed identifiers are rejected during validation.

## Safety and Fail-Closed Behavior
- Invalid profile names or self-parenting fail validation.
- Invalid child module identifiers fail validation.
- If validation fails, the system remains in safe-state and refuses
  to proceed with normal operation.

## Traceability
- Requirements: REQ-SYS-013, REQ-CFG-007
- Verification: V-073, V-074
