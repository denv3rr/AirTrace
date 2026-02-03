# Adapter Contract and Registry

## Purpose
Define the adapter contract, manifest schema, and registry/allowlist rules.
Adapters are optional modules and must pass safety, security, and contract
validation before activation.

## Contract Versions
- adapter.contract_version: measurement and capability contract.
- ui.contract_version: baseline UI contract (see docs/adapters/ui_data_points.md).
- Versioning is semantic; incompatible versions must fail closed.

## Adapter Manifest (Signed)
The adapter manifest is a signed JSON document. Current tooling validates
the manifest using a SHA-256 hash recorded in the allowlist; public-key
signature support is planned (see `docs/waivers.md`).

Required fields:
- adapter.id
- adapter.version
- adapter.contract_version
- ui.contract_version
- core.compatibility.min
- core.compatibility.max
- tools.compatibility.min
- tools.compatibility.max
- ui.compatibility.min
- ui.compatibility.max
- capabilities (list)
- ui_extensions (list)

Capabilities entry:
- id
- description
- units (if applicable)
- range_min
- range_max
- error_behavior

UI extension entry:
- field_id
- type
- units
- range_min
- range_max
- error_behavior
- surfaces (list)

## Allowlist Entry
Allowlist entries are stored locally and signed.

Required fields:
- adapter.id
- adapter.version
- signature.hash
- signature.algorithm
- approved_by
- approval_date
- allowed_surfaces

## Validation Steps (Fail Closed)
1) Verify signature on adapter manifest.
2) Verify allowlist contains adapter.id and version.
3) Verify contract version compatibility with core/tools/ui.
4) Validate capabilities and UI extensions against schema rules.
5) Reject unknown fields or missing required fields.

## Reason Codes (Stable)
- adapter_signature_invalid
- adapter_not_allowlisted
- adapter_version_incompatible
- adapter_contract_mismatch
- adapter_schema_invalid
- adapter_capability_invalid
- adapter_ui_extension_invalid
- adapter_surface_not_allowed
- adapter_manifest_missing
- adapter_manifest_invalid
- adapter_allowlist_missing
- adapter_allowlist_invalid

## Security and Safety
- No dynamic loading or network access without explicit authorization.
- Adapter failures trigger safe-state entry and audit logging.
- Third-party adapters are permitted only after passing all gates.

## Traceability
- REQ-ADP-002..006
- REQ-SEC-012
- REQ-MOD-004
