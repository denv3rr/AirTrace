# Official Adapter Notes

This folder contains design notes for each official platform adapter and
the shared UI data-point contract. Each adapter note is a compliance
artifact and must be kept current with requirements and verification.

Official adapters:
- `docs/adapters/air.md`
- `docs/adapters/ground.md`
- `docs/adapters/maritime.md`
- `docs/adapters/space.md`
- `docs/adapters/handheld.md`
- `docs/adapters/fixed_site.md`
- `docs/adapters/subsea.md`

Shared UI data-point contract:
- `docs/adapters/ui_data_points.md`

Registry artifacts:
- Official adapter manifests: `adapters/official/<platform>/manifest.json`
- Allowlist: `adapters/allowlist.json`
Note: Current official adapter artifacts are manifest skeletons; runtime I/O bindings remain pending.

Rules:
- Use explicit units, ranges, and error behavior for adapter-specific fields.
- Keep all content ASCII.
- Ensure traceability links are updated when fields change.
