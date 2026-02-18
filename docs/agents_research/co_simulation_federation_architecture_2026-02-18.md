# Co-Simulation and Interoperability Architecture Research (2026-02-18)

## Problem Framing
Multi-device integration becomes unstable when each platform pair needs bespoke converters and timing logic. This increases branching complexity, onboarding time, and failure modes under field updates.

## Current Baseline in AirTrace
- Canonical external envelope with deterministic conversion (`ie_json_v1`, `ie_kv_v1`).
- Tools-layer conversion and fail-closed validation.
- Modular boundaries (`core`, `tools`, `ui`, `adapters`) with explicit contracts.

This is a strong base, but it still needs a scalable orchestration pattern for large heterogeneous fleets and simulation-to-field continuity.

## Architecture Options
1. Point-to-point adapters
- Pros: fast for one integration.
- Cons: O(N^2) interface growth, high drift risk, fragile validation consistency.
- Disposition: reject as long-term strategy.

2. Canonical envelope + codec registry (current and expanding)
- Pros: deterministic schema boundary, smaller integration surface, fail-closed parsing.
- Cons: orchestration/time-sync semantics still externalized.
- Disposition: keep as mandatory data-contract core.

3. Federation/co-simulation orchestrator (HLA/FMI/DCP style)
- Pros: formal coordination for multiple simulators/devices, time management abstractions, reusable integration harnesses.
- Cons: additional runtime complexity; strict profile hardening needed for operational deployment.
- Disposition: pursue for lab/integration/test orchestration first, then mission-tailored operational profile.

4. Hybrid (recommended)
- Runtime mission stack: canonical envelope + hardened adapter boundary.
- Integration/test stack: federation orchestrator with deterministic time authority.
- Shared governance: one schema registry, one requirements/verification trace path.

## Recommended Target Pattern
1. `Device/Platform Adapters`
- Normalize device-specific I/O to canonical envelope.
- Enforce signed/allowlisted adapter policy.

2. `Canonical Envelope Bus`
- Versioned schema with strict validation.
- Codec registry for approved transport formats only.

3. `Federation/Co-Sim Orchestrator (Lab + TRR usage)`
- Time synchronization authority.
- Scenario-level orchestration and replay.
- Deterministic scheduling and bounded latency policy.

4. `Operator UX Profiles`
- Role-profiled defaults (pilot, field operator, C2).
- One-primary-action surfaces, progressive disclosure, explicit denial/recovery guidance.

## Safety/Security Constraints for the Hybrid Pattern
- Fail closed on unknown message fields, unsupported codecs, or invalid timing metadata.
- Keep core deterministic and side-effect bounded.
- No dynamic plugin or network expansion without threat-model and control mapping updates.
- Maintain provenance tags end-to-end for simulation vs operational separation.

## Candidate Promotion Items (for SRR/PDR)
- Candidate REQ-INT-035: co-simulation orchestration shall preserve deterministic event ordering and explicit timing authority boundaries.
- Candidate REQ-CFG-016: federation profile configuration shall define time-sync mode, step budget, and safe fallback behavior.
- Candidate REQ-SEC-014: federation endpoints shall be allowlisted and authenticated with fail-closed rejection.
- Candidate V-136..V-138: deterministic replay, timing-fault injection, and unauthorized-endpoint denial tests.

## Immediate Backlog
- Prototype a tools-layer federation bridge that ingests canonical envelopes and emits deterministic event frames.
- Define a timing contract extension (event timestamp, logical tick, source latency budget).
- Add a compliance profile matrix: lab co-simulation profile vs operational hardened profile.
- Add operator burden metrics (action count, denial recovery time, error rate) per role profile.

## Primary Sources
- FMI standard docs: https://fmi-standard.org/docs/3.0.2/
- DCP standard overview: https://dcp-standard.org/about/
- DCP specification: https://dcp-standard.org/assets/DCP_Specification_v1.0.pdf
- OMG DDS portal: https://www.omg.org/omg-dds-portal/
- OMG DDS 1.4: https://www.omg.org/spec/DDS/1.4/
- IEEE 1516 HLA standard page: https://standards.ieee.org/ieee/1516/6687/
- SISO note on HLA 4 approval: https://www.sisostandards.org/news/695782/HLA-4-Approved-by-IEEE.htm
- FACE approach: https://www.opengroup.org/face/approach
- FACE technical standard 2.1: https://www.opengroup.org/face/tech-standard-2.1
- SOSA overview: https://www.opengroup.org/sosa
- MOSA legal basis: https://www.law.cornell.edu/uscode/text/10/4401
- MOSA acquisition reference: https://www.acquisition.gov/afars/modular-open-systems-approach-mosa-10-u.s.-code-%C2%A7-4401
