# Agents Research Backlog

## Queue

1. Federation Bridge Spike (v3 trust policy + key lifecycle controls implemented in tools)
- Goal: map canonical envelope events into a deterministic federation timeline.
- Owner: tools/integration research agent.
- Acceptance: prototype design note + deterministic replay evidence outline.
- Status: route identity, endpoint fan-out, source/time-authority controls, endpoint key allowlists, key-validity window checks, and required attestation-tag policy paths are implemented with test coverage in `tests/core_sanity.cpp`; next increment is cryptographic attestation/signature verification and key rotation provenance attestation evidence.

2. Timing Contract Extension
- Goal: define required timing metadata for cross-platform synchronization and stale-data rejection.
- Owner: core/tools research agent.
- Acceptance: proposed schema extension and fail-closed rules.

3. Operational vs Lab Profile Split
- Goal: define what federation capabilities are allowed in lab only vs operational runtime.
- Owner: security/compliance research agent.
- Acceptance: control matrix mapped to RMF and STIG concerns.

4. Role UX Load Study
- Goal: measure step count, denial frequency, and recovery latency per role profile.
- Owner: UI/human-factors research agent.
- Acceptance: metric template and candidate thresholds for REQ/V promotion.

5. Interface Evolution Policy
- Goal: define backward compatibility process for new codecs and platform fields.
- Owner: module-contract research agent.
- Acceptance: proposed versioning lifecycle and deprecation policy language.
