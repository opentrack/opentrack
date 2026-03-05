# PR: Add Alpha Spectrum filter with Hydra heads, Rényi/MTM composition, and simplified+advanced UI

## Summary
This PR adds a new `filter-alpha-spectrum` module and integrates it into opentrack.

Key capabilities included:
- Hydra head composition: `EMA`, `Brownian`, `Adaptive`, `Predictive`
- Rényi/Tsallis neck scoring + MTM shoulder composition
- NGC translational coupling (`kappa`, `nominal Z`) controls
- Two-mode UI:
  - Simplified mode (default): 2 consolidated sliders + `Adaptive` + `Advanced`
  - Advanced mode: full parameter-level controls (greyed out until enabled)
- Diagnostics focused on per-head contribution in simplified flow
- Factory reset behavior fixed to persist defaults and re-sync simplified controls immediately

## User-facing controls
Simplified (default):
- `EMA Min/Max Range`
- `Curve/Deadzone Coupling`
- `Adaptive`
- `Advanced`

Advanced:
- Full EMA, Hydra, MTM, NGC sliders
- Individual head toggles

## Default baseline
Factory defaults are aligned to the current baseline profile:
- Adaptive: `off`
- Rotation/Translation Min: `0.085`
- Rotation/Translation Max: `0.218`
- Rotation/Translation Curve: `4.26`
- Rotation Deadzone: `0.156`
- Translation Deadzone: `1.041`

## Build validation
Validated with CMake Tools build on Linux:
- `Build_CMakeTools` returned success for full build
- `opentrack-filter-alpha-spectrum` target builds clean

## Scope notes
- No GUI binary execution was used for validation in integrated terminal.
- Plugin-only refresh workflow used for fast iteration and stability.

## Checklist
- [x] New filter module added and wired
- [x] UI/diagnostics wiring complete
- [x] Simplified/advanced mode behavior implemented
- [x] Factory reset semantics corrected
- [x] README updated for current behavior
- [x] Linux build validation completed
