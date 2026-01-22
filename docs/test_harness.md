# Test Harness (Non-Interactive UI Flow)

This harness drives the same UI flow as the main TUI without interactive input.
It is test-only and requires explicit build-time gating and runtime enablement.

## Safety and Gating
- Build-time: harness runner is built with `AIRTRACE_TEST_HARNESS`.
- Runtime: set `AIRTRACE_TEST_HARNESS=1`.
- If the command file is missing, invalid, or mismatched, the harness fails closed.
- When enabled, the harness allows menu flows without a TTY for test execution.

## Command File Format
Each command is a single line with pipe delimiters:

- `select|<menu title>|<index>`
- `input_int|<prompt>|<value>`

Comments begin with `#`.

## Example Command File
See `docs/harness_commands.example.txt`.

## Running the Harness
Windows (PowerShell):
```
$env:AIRTRACE_TEST_HARNESS="1"
$env:AIRTRACE_HARNESS_COMMANDS="configs/harness_commands.txt"
.\build\AirTraceHarnessRunner.exe
```

macOS/Linux (sh):
```
AIRTRACE_TEST_HARNESS=1 \
AIRTRACE_HARNESS_COMMANDS=configs/harness_commands.txt \
./build/AirTraceHarnessRunner
```

## Mid-Run Edits
Append new commands to the command file while the harness is running.
Commands are consumed in order.

## Failure Modes
- Prompt or menu title mismatch: harness fails closed and exits.
- Out-of-range selection or input: harness fails closed and exits.
- Missing command file: harness fails closed and exits.
