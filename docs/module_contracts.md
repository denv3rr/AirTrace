# Module Contracts

## Purpose
Define explicit, versioned contracts for core, tools, UI, and adapters.
Modules are independently buildable and must be usable in isolation.

## Dependency Rules
- core has no dependency on tools, UI, or adapters.
- adapter contract module depends on core and provides adapter manifest/allowlist contract validation.
- tools depends on core + adapter contract module and provides all external I/O and policy enforcement.
- UI depends on tools and core; UI performs no direct policy or I/O beyond UI.
- adapters depend on core contracts only; adapters do not depend on UI internals.
- Automated dependency-boundary checks fail verification on undeclared module include edges.

## Versioning
- Each module publishes a semantic contract version.
- Backward-compatible changes increment MINOR; breaking changes increment MAJOR.
- Modules must fail closed on incompatible contract versions.

## Core Contract
Core responsibilities:
- Deterministic algorithms and mode logic.
- No direct I/O, wall-clock, or non-seeded randomness.
- Pure data structures and predictable state transitions.

Inputs:
- Validated configuration parameters.
- Normalized measurements with provenance tags.

Outputs:
- Mode decisions, contributor lists, confidence values.
- Deterministic state updates and projection outputs.

## Tools Contract
Tools responsibilities:
- Configuration ingestion and schema validation.
- Policy enforcement and authorization decisioning.
- Dataset integrity validation and audit logging sinks.
- Adapter registry I/O (manifest + allowlist) and signature/hash checks.
- Adapter runtime context negotiation (core/tools/ui contract versions) and allowlist approval freshness checks.
- External I/O envelope packaging and conversion across approved formats (`ie_json_v1`, `ie_kv_v1`) with deterministic numeric fidelity, explicit codec discovery, and fail-closed error handling.
- Deterministic federation-bridge event framing from canonical envelopes to logical ticks/timestamps with route identity (`federate_id`, `route_key`, `route_sequence`), bounded-latency validation, and fail-closed time-authority rejection paths.

Inputs:
- Config files and policy bundles.
- Dataset artifacts and manifest metadata.

Outputs:
- Validated configuration objects.
- Authorization status and eligibility gates.
- Audit log status and run identifiers.
- Versioned external payloads and explicit conversion failure reasons for malformed or schema-loss inputs.

## UI Contract
UI responsibilities:
- Operator presentation and input handling.
- Rendering of baseline UI contract and adapter extensions.
- Front-view display rendering for EO/IR/proximity families with deterministic cycle control.
- No direct policy enforcement or external I/O beyond UI controls.

Inputs:
- Mode decisions, sensor health, authorization/provenance state.
- Adapter UI extension fields (validated).

Outputs:
- Operator prompts, status banners, and denial guidance.
- Versioned external I/O envelope records for interoperable platform test exchange.
- Front-view telemetry fields (mode/state/frame/source/stream/timestamp/frame-age/latency-breakdown/stabilization/gimbal/drop/provenance/auth) in the external I/O envelope.

## Adapter Contract
Adapter responsibilities:
- Platform-specific I/O bindings and measurement normalization.
- Capability declarations and UI extensions.
- Fail-closed behavior on invalid data.

Inputs:
- Configuration and policy bundle.
- Adapter manifest and allowlist authorization.

Outputs:
- Normalized measurements and health status.
- Adapter UI extension fields and health indicators.

## Determinism and Safety
- All modules must preserve deterministic behavior.
- Safety and fail-closed behavior apply across module boundaries.
- Missing or invalid contract data denies activation and logs explicit reasons.
