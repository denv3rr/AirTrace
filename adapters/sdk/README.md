# Adapter SDK Skeleton

This folder provides a minimal adapter SDK skeleton and build template.
Copy this folder to `adapters/<adapter_id>` when starting a new adapter.

Contents:
- `include/adapter_sdk.h` versioned interface header (skeleton).
- `src/adapter_sdk.cpp` placeholder implementation file.
- `CMakeLists.txt` template for adapter builds.

Notes:
- This SDK is a template and is not built by default.
- Adapters must use the contract and allowlist rules in `docs/adapter_contract.md`.
- Adapter UI extensions must follow `docs/adapters/ui_data_points.md`.
