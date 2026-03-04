# Sources Menu Registry Workflow

The `M.1 AIM and U.S. Air-to-Air Missiles` block in `sources_menu.md` is generated
from a machine-validated JSON registry.

## Files

- `docs/sources_menu_registry.json`: source-of-truth menu entries.
- `docs/sources_menu_registry.schema.json`: JSON schema used for validation.
- `scripts/sync_sources_menu_registry.mjs`: validator + markdown generator.

## Update Process

1. Edit `docs/sources_menu_registry.json`.
2. Run `node ./scripts/sync_sources_menu_registry.mjs`.
3. Confirm the generated block in `sources_menu.md`.
4. Run `node ./scripts/sync_sources_menu_registry.mjs --check` before commit.

## Entry Rules

- Keep `entry_id` stable once introduced.
- Keep `order` values unique within a section for deterministic output.
- Use `url` for a single source link.
- Use `links` for grouped links (for example archive bundles).
- Keep URLs as `https://` values.
