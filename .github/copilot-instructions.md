# Copilot Instructions for opentrack

## Source of truth

- Session safety and crash-tracking rules are maintained at user level in VS Code settings (`github.copilot.chat.codeGeneration.instructions`).
- Use this repo file only for concise project-scoped defaults that are safe to commit.

## Linux-friendly build validation defaults

- Prefer file/build-system inspection and linker-path diagnostics over runtime launches.
- Keep changes focused to CMake/build files when the stated task is Linux build friendliness.
- Avoid broad test/launch sweeps unless requested.
