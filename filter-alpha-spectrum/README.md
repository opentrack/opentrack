# Alpha Spectrum filter tuning guide

This filter adapts smoothing strength from recent motion energy per axis.

- Low motion -> alpha stays near **Min** (more smoothing, less jitter)
- High motion -> alpha moves toward **Max** (faster response)

`Adaptive Mode` adds a second stage: when sustained motion is detected, it temporarily boosts responsiveness beyond the base Min/Max/Curve trajectory.

The runtime pipeline is now split into independently toggleable components:

- `EMA head`: applies smoothing update to output
- `Brownian interface`: contributes Brownian signal into adaptive drive and MTM measurement
- `Rényi MTM shoulders`: contributes mode-probability expectation and posterior updates

Internally, each axis is updated as:

- `output += alpha * (input - output)`
- `alpha = min + (max - min) * motion^curve`

where `motion` is a normalized innovation-energy estimate in `[0, 1]`.

## Parameters

## UI layout

The settings panel is split into two columns:

- **Left: Hydra / NGC controls**
  - Brownian Head Gain
  - Adaptive Threshold Lift
  - Predictive Head Gain
  - MTM Shoulder Base
  - NGC Kappa
  - NGC Nominal Z
- **Right: Core EMA controls**
  - Rotation/Translation Min, Max, Curve, Deadzone

The default workflow is now **Simplified Mode** at the top of the dialog:

- `Adaptive` checkbox
- `Advanced` checkbox
- `EMA Min/Max Range` slider
- `Curve/Deadzone Coupling` slider

When `Advanced` is off, detailed controls are disabled (greyed out). Turn `Advanced` on to access full per-parameter tuning.

### Adaptive Mode

- Default: `disabled`
- Effect:
  - **Enabled**: adds a motion-history responsiveness boost during sustained movement.
  - **Disabled**: uses only the base Min/Max/Curve mapping.
- When to use:
  - Enable for fast gameplay and frequent large turns.
  - Disable for predictable "always same" feel or precision slow-scan use.

### Component toggles

In Simplified Mode, only `Adaptive` and `Advanced` are intended for routine use.

In Advanced Mode, detailed head toggles are available:

- **Enable EMA smoothing head**
  - **On**: uses `output += alpha * (input - output)`.
  - **Off**: pure pass-through (`output = input`) while status info still updates.
- **Enable Brownian interface contribution**
  - **On**: Brownian term contributes to both adaptive drive and MTM measurement.
  - **Off**: Brownian status info still computes, but Brownian contribution to control is disabled.
- **Enable Rényi MTM shoulder composition**
  - **On**: mode diffusion + posterior update influence adaptive responsiveness.
  - **Off**: MTM influence is fully removed (`mode_e` does not affect response).

### Reset to defaults

- Use **Reset sliders to defaults** in the settings dialog to restore factory values.
- This action writes default values to the active profile settings immediately (not just in-memory reload).

### Rotation Min / Max

- Range: `0.005 .. 0.4` (Min), `0.02 .. 1.0` (Max)
- Default: `0.085` (Min), `0.218` (Max)
- Effect:
  - Higher **Rotation Min**: less rotational lag at rest, but more visible micro-jitter.
  - Lower **Rotation Min**: steadier view when still, but softer initial response.
  - Higher **Rotation Max**: snappier turns and faster catch-up.

In Simplified Mode, this is driven by **EMA Min/Max Range** and applied together to both rotation and translation:

- `Min`: `0.5% .. 40%` (`0.005 .. 0.4`)
- `Max`: `2% .. 100%` (`0.02 .. 1.0`)

### Rotation Curve

- Range: `0.2 .. 8.0`
- Default: `4.26`
- Effect:
  - Higher curve (>1): keeps heavy smoothing longer, then opens up late during stronger motion.
  - Lower curve (<1): opens up earlier, feeling more immediate.

### Rotation Deadzone

- Range: `0.0 .. 0.3` degrees
- Default: `0.156` degrees
- Effect:
  - Suppresses tiny rotational deltas before adaptation.
  - Too high can make tiny head movements feel ignored.

### Translation Min / Max

- Range: `0.005 .. 0.4` (Min), `0.02 .. 1.0` (Max)
- Default: `0.085` (Min), `0.218` (Max)
- Effect:
  - Higher **Translation Min**: less positional lag but more shake.
  - Lower **Translation Min**: calmer center position but softer micro-movement response.
  - Higher **Translation Max**: faster positional response on larger movement.

### Translation Curve

- Range: `0.2 .. 8.0`
- Default: `4.26`
- Effect:
  - Same shape behavior as rotation curve, for X/Y/Z translation.

### Translation Deadzone

- Range: `0.0 .. 2.0` mm
- Default: `1.041` mm
- Effect:
  - Suppresses tiny translational jitter.
  - Too high can create a sticky center feel.

In Simplified Mode, **Curve/Deadzone Coupling** controls all four together:

- Rotation Curve (`0.2 .. 8.0`)
- Translation Curve (`0.2 .. 8.0`)
- Rotation Deadzone (`0.0 .. 0.3°`)
- Translation Deadzone (`0.0 .. 2.0 mm`)

The slider maps these parameters one-to-one from their minimum to maximum ranges.

## Practical tuning workflow

Use this order to avoid chasing interactions:

1. Start with **Adaptive Mode** off; enable it only if you want extra sustained-motion responsiveness.
2. Observe status info while moving and while still.
3. Tune one family at a time (rotation, then translation).
4. Optionally fine-tune one or two sliders for personal preference.

Make small moves (`~5-10%` of slider range), then test with:

- still head pose (jitter check)
- slow pan
- fast snap turn
- lean in/out

### Two-column quick-start (recommended)

Use this order with the current two-column layout:

1. **Left column (Hydra / NGC)**
  - Start with defaults.
  - Tune **MTM Shoulder Base** first for overall trust level.
  - Tune **Brownian Head Gain** to balance jitter suppression vs response.
  - Tune **Adaptive Threshold Lift** to control when adaptive boosting engages.
  - Tune **Predictive Head Gain** while watching **Predictive error (rot/pos)**.
  - Tune **NGC Kappa** and **NGC Nominal Z** last for translational coupling behavior.
2. **Right column (Core EMA)**
  - Set rotation Min/Max/Curve/Deadzone for desired rotational feel.
  - Set translation Min/Max/Curve/Deadzone for positional feel.
3. **Validation pass**
  - Re-test still pose, slow pan, fast snap turn, and lean in/out.
  - If tuning regresses, use **Reset sliders to defaults** and repeat in the same order.

## Recommended starting profiles

### Stable / simulator focus

- Rotation Min/Max: `0.03 / 0.55`
- Rotation Curve: `1.8`
- Rotation Deadzone: `0.04°`
- Translation Min/Max: `0.04 / 0.65`
- Translation Curve: `1.5`
- Translation Deadzone: `0.12 mm`

### Balanced (factory default)

- Rotation Min/Max: `0.085 / 0.218`
- Rotation Curve: `4.26`
- Rotation Deadzone: `0.156°`
- Translation Min/Max: `0.085 / 0.218`
- Translation Curve: `4.26`
- Translation Deadzone: `1.041 mm`

### Responsive / fast action

- Rotation Min/Max: `0.06 / 0.85`
- Rotation Curve: `0.9`
- Rotation Deadzone: `0.02°`
- Translation Min/Max: `0.07 / 0.90`
- Translation Curve: `0.8`
- Translation Deadzone: `0.06 mm`

## Troubleshooting

- **View jitters while still**
  - Increase Min or deadzone slightly and watch brownian damped status info.
- **Feels laggy during turns**
  - Raise Max, lower Curve, or keep Adaptive Mode enabled.
- **Feels too twitchy**
  - Lower Max, raise Curve, or disable Adaptive Mode.
- **Small intentional movement is ignored**
  - Reduce deadzone.

## Notes

- Rotation wrap-around is handled (`±180°` crossing is unwrapped).
- Recenter resets internal state on the next frame.
- Brownian status info reports `raw / filtered / delta / damped` where `delta = raw - filtered`.
- Contribution status info reports per-family drives: `EMA`, `Brownian`, `Adaptive`, `Predictive`, `MTM`.

## Status format

The status line uses a compact fixed-width format to avoid UI resizing jitter:

- `Mon|E1 B1 A1 P1 M1|rE0.350 rP0.350 pE0.534 pP0.500 k0.000`

Legend:

- `Mon`: status active
- `E/B/A/P/M`: EMA, Brownian, Adaptive, Predictive, MTM toggles (`1`=on, `0`=off)
- `rE`, `rP`: rotation mode expectation and peak
- `pE`, `pP`: translation mode expectation and peak
- `k`: live NGC coupling residual

Simplified status display emphasizes **head contribution lines** (rotation and translation) while keeping the rest of info secondary.

## EMA-head to Rényi-interface bridge

This is now implemented as a **Hydra-style 4-head composition**:

- EMA head proposes one candidate output.
- Brownian head proposes a second candidate output.
- Adaptive head proposes a third candidate output with a stricter activation gate.
- Predictive head proposes a cached next-frame estimate computed after prior-frame output.
- Rényi/Tsallis neck scores head candidates by generalized likelihood.
- MTM shoulder composes the neck-normalized heads and applies mode-strength gating.
- NGC commutator coupling injects depth-scale residual (`κ`) into translational likelihood.

Adaptive head gate detail:

- The Adaptive head uses about a 15% higher activation threshold before it contributes strongly.
- This makes it a higher-confidence head that activates later than base EMA/Brownian heads.

Predictive head detail:

- After each frame produces final filtered output, the filter computes and stores a one-step-ahead prediction.
- On the next frame, Predictive head is a direct memory lookup (`predicted_next_output`).
- Prediction assumes the next frame interval is the same as the measured current frame period.

Non-EMA tuning controls:

- Brownian Head Gain: scales Brownian head drive before alpha synthesis.
- Adaptive Threshold Lift: raises Adaptive head activation threshold.
- Predictive Head Gain: scales influence of cached next-frame prediction.
- MTM Shoulder Base: sets minimum shoulder trust before mode-dependent lift.
- NGC Kappa: controls depth-scale commutator coupling strength.
- NGC Nominal Z: reference distance used by NGC coupling term.

The design is head-capacity oriented in code so more heads can be added later without replacing the current composition contract.

## What is implemented now

This module is now in **Phase A** of a Rényi-MTM style implementation.

- Implemented now:
  - Per-axis adaptive EMA smoothing
  - Hydra-style four-head composition (EMA + Brownian + Adaptive + Predictive)
  - True Rényi/Tsallis neck scoring across enabled heads
  - MTM shoulder composition over neck-normalized head candidates
  - NGC depth-scale commutator coupling term in neck residual evaluation
  - Alpha-spectrum mode probability vector (7 bins) with per-frame diffusion
  - Measurement-driven mode likelihood updates and normalized posterior
  - Mode expectation + peak status info exposed in the dialog status line
  - Brownian raw/filtered/damped status info
  - Brownian-aware instantaneous adaptive drive
  - Head-share contribution status info (`EMA`, `Brownian`, `Adaptive`, `Predictive`) and shoulder contribution (`MTM`)
  - Live NGC coupling status info in status line (`κ`)
  - Predictive error status info (`rot/pos`) for next-frame lookup quality
  - Runtime status info surfaced in settings dialog
- Not implemented yet:
  - Full transition-matrix MTM with explicit regime semantics
  - Rényi-distance likelihood over more than the currently enabled head set
  - IMM-style mode mixing across explicit motion regimes

Code references:

- Core filter/update path: [filter-alpha-spectrum/ftnoir_filter_alpha_spectrum.cpp](filter-alpha-spectrum/ftnoir_filter_alpha_spectrum.cpp)
- Filter state and status definitions: [filter-alpha-spectrum/ftnoir_filter_alpha_spectrum.h](filter-alpha-spectrum/ftnoir_filter_alpha_spectrum.h)
- Settings dialog status presentation: [filter-alpha-spectrum/ftnoir_filter_alpha_spectrum_dialog.cpp](filter-alpha-spectrum/ftnoir_filter_alpha_spectrum_dialog.cpp)

## Author discussion

https://github.com/ganzuul/ancg_spectrum/blob/main/README.md
