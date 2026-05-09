# opentrack X-Plane plugin

A 258-line C plugin (`opentrack.xpl`) that runs inside X-Plane and drives
the six pilot-head-pose datarefs from head tracking data written by
opentrack. It is the X-Plane-side counterpart to the **"X-Plane"** output
protocol in opentrack; the two processes communicate through a POSIX
shared-memory block, not a network socket.

Supported platforms: Linux, macOS (arm64 and x86_64), Windows.

## How the two halves fit together

```
opentrack process                              X-Plane process
┌─────────────────────────────┐                ┌─────────────────────────────┐
│  "X-Plane" output protocol   │                │  opentrack.xpl:              │
│  (proto-wine built with      │ ── POSIX ──>  │   flock()s the shm, reads 6  │
│  -DOTR_WINE_NO_WRAPPER):     │  shm_open +    │   doubles per frame, drives  │
│    writes {x,y,z,yaw,pitch,  │  mmap          │   sim/graphics/view/         │
│    roll} into shm block      │                │    pilots_head_{x,y,z,psi,   │
│    /facetracknoir-wine-shm   │                │    the,phi}                  │
└─────────────────────────────┘                └─────────────────────────────┘
```

The shared-memory block is named `facetracknoir-wine-shm` (same name used
by the Wine freetrack proxy, so one plugin binary serves both use cases).

## Building

The plugin is only compiled when the X-Plane SDK path is provided via the
`SDK_XPLANE` CMake cache variable; with an empty path it is silently
skipped.

1. Download the X-Plane SDK from
   [developer.x-plane.com/sdk/plugin-sdk-downloads/](https://developer.x-plane.com/sdk/plugin-sdk-downloads/)
   (free, ~2 MB zipped). Current version at time of writing: `XPSDK430.zip`.

2. Unzip it somewhere; note the `SDK/` directory inside the zip.

3. Configure the opentrack build tree with the SDK path:

   ```bash
   cmake -S . -B build -DSDK_XPLANE=/path/to/XPSDK430/SDK
   cmake --build build --target opentrack-xplane-plugin opentrack-proto-wine
   ```

   - `opentrack-xplane-plugin` → produces `opentrack.xpl` (the X-Plane side).
   - `opentrack-proto-wine` → produces `opentrack-proto-wine.dylib`/`.so`/
     `.dll`, the **"X-Plane"** entry in opentrack's output dropdown.

   Both targets need `SDK_XPLANE` set. Without it, the output dropdown
   shows only non-X-Plane protocols.

The output bundle is emitted at `x-plane-plugin/opentrack.xpl`.

## Installing the plugin into X-Plane

Copy `opentrack.xpl` into the standard X-Plane plugin layout. The
subdirectory name is platform-specific:

```
X-Plane 12/
  Resources/
    plugins/
      opentrack/
        mac_x64/   ← macOS (arm64 and x86_64)
        64/        ← Linux / Windows
          opentrack.xpl
```

One-liner for a typical macOS install:

```bash
XP12=~/X-Plane\ 12
mkdir -p "$XP12/Resources/plugins/opentrack/mac_x64"
cp x-plane-plugin/opentrack.xpl "$XP12/Resources/plugins/opentrack/mac_x64/"
```

Verify which path X-Plane actually picked up by grepping its
`X-Plane 12/Log.txt` for `opentrack.xpl` after the next launch;
if both `mac_x64/` and `64/` trees exist, X-Plane loads one and
silently ignores the other.

On first launch X-Plane's `Log.txt` (in the sim's root folder) should
contain a line like `opentrack init complete` emitted by
`XPluginStart`. If you instead see `opentrack failed to init SHM!`, the
plugin couldn't open `/facetracknoir-wine-shm` for RW — check your
filesystem permissions on `/dev/shm` (Linux) or System Settings →
Privacy sandboxing (macOS).

## Using it from opentrack

1. In opentrack, pick **X-Plane** from the **Output** dropdown. This is
   `proto-wine` built in no-wrapper mode; its icon is the X-Plane logo.
   It writes the shared-memory block the plugin reads.
2. Start opentrack tracking as usual. The plugin is passive — it just
   mirrors whatever is currently in the shm block to X-Plane's head-pose
   datarefs every flight-loop tick.

## Plugin commands

The plugin registers two X-Plane commands that you can bind to joystick
buttons or keys from **Settings → Joystick** or **Settings → Keyboard**:

| Command                         | Description                                                                                       |
| ------------------------------- | ------------------------------------------------------------------------------------------------- |
| `opentrack/toggle`              | Enable/disable the flight-loop callback. When off, no data is sent to X-Plane's datarefs.         |
| `opentrack/toggle_translation`  | Enable/disable the translation part only (yaw/pitch/roll remain active). Re-centers on re-enable. |

Both commands operate on `xplm_CommandBegin`, so they fire on button
press (not release).

## Data conversions

The plugin reads six `double`s from the shm and writes to X-Plane:

| shm index | Semantic   | Dataref                                   | Conversion          |
| --------- | ---------- | ----------------------------------------- | ------------------- |
| 0 (TX)    | x (mm)     | `sim/graphics/view/pilots_head_x`         | `mm × 1e-3 + offset_x` |
| 1 (TY)    | y (mm)     | `sim/graphics/view/pilots_head_y`         | `mm × 1e-3 + offset_y` |
| 2 (TZ)    | z (mm)     | `sim/graphics/view/pilots_head_z`         | `mm × 1e-3 + offset_z` |
| 3 (Yaw)   | yaw (rad)  | `sim/graphics/view/pilots_head_psi`       | `rad × 180/π`       |
| 4 (Pitch) | pitch (rad)| `sim/graphics/view/pilots_head_the`       | `rad × 180/π`       |
| 5 (Roll)  | roll (rad) | `sim/graphics/view/pilots_head_phi`       | `rad × 180/π`       |

Translation offsets (`offset_{x,y,z}`) are captured at plugin start and
whenever translation is re-enabled; this lets the user move around their
sim cockpit from a home position that matches the current seat pose.

## Shared-memory layout

Defined in both `x-plane-plugin/plugin.c` and `proto-wine/wine-shm.h`:

```c
typedef struct WineSHM {
    double        data[6];    // x,y,z,yaw,pitch,roll (see conversions)
    int           gameid, gameid2;
    unsigned char table[8];
    bool          stop;
} volatile WineSHM;
```

The X-Plane plugin only reads `data[]`; the other fields are used by the
Wine freetrack flavor of proto-wine and are ignored here.

Access is guarded with a POSIX `flock(fd, LOCK_SH)` around the read —
cheap, contention-free for single-writer/single-reader.

## Limitations and notes

- **No UDP fallback in the plugin.** If you want to drive X-Plane from a
  different machine, use opentrack's **UDP over network** output and a
  different plugin on the X-Plane side. This plugin is local-only.
- **Head roll can affect the field-of-view dataref on some X-Plane
  builds.** The commented-out `field_of_view_roll_deg` line at the top of
  `XPluginStart` shows the older binding. If you dislike roll acting on
  FoV on your X-Plane version, swap the two lines and rebuild.
- **Plugin is not codesigned.** On macOS, X-Plane may warn about loading
  an unsigned plugin. Either accept the warning or ad-hoc-sign with
  `codesign --force -s - opentrack.xpl` before installing.

## Building with a different SDK version

The minimum API version is set in `x-plane-plugin/CMakeLists.txt`:

```cmake
-DXPLM200 -DXPLM210
```

That targets X-Plane 10.00 and newer. If you need features from a newer
SDK (e.g. `XPLM400` for X-Plane 12.04+ datarefs), add the corresponding
definition; see the SDK's
[version-define documentation](https://developer.x-plane.com/sdk/plugin-sdk-documents/)
for the full ladder. No new defines are needed for pose tracking as
used here.
