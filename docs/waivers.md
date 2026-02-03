# Waivers and Deviations

All waivers are time-boxed and must be resolved before production release.

## WVR-001: Adapter Manifest Signature Verification
- Standard/Requirement: REQ-SEC-012 (signed/allowlisted adapters).
- Deviation: Current adapter registry validation uses SHA-256 manifest hashes
  recorded in the allowlist instead of public-key signature verification.
- Rationale: Provide deterministic allowlist integrity checks until PKI
  integration and signing infrastructure are available.
- Mitigation: Allowlist is treated as a controlled artifact; manifest hash
  mismatches fail closed and are audited.
- Exit Criteria: Implement public-key signature verification and update
  allowlist schema to include signer identity and trust chain validation.
- Target Resolution: Before PRR.
