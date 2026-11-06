# Building opentrack on macOS

Tested on Apple Silicon (macOS 27, Xcode 27, Homebrew). Intel Macs should
work the same with minor path differences.

## Prerequisites

1. Xcode from the App Store, then accept its license (required once after
   every Xcode update, otherwise every compile fails with a confusing
   linker error):

   ```
   sudo xcodebuild -license accept
   ```

2. Homebrew packages:

   ```
   brew install qt opencv ninja onnxruntime
   ```

   - `onnxruntime` is optional, without it the NeuralNet face tracker is
     skipped. Everything else builds fine without it.
   - `libomp` (OpenMP) arrives as a dependency of opencv, you do not need
     to install it separately.

## Configure

```
cmake -GNinja -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DQt6_DIR=$(brew --prefix qt)/lib/cmake/Qt6 \
    -DOpenCV_DIR=$(brew --prefix opencv)/lib/cmake/opencv5 \
    -DONNXRuntime_DIR=$(brew --prefix onnxruntime) \
    -DOpenMP_ROOT=$(brew --prefix libomp)
```

Notes:

- If a first configure ran before all dependencies were present, CMake
  caches the failed lookups. Re-run with `-U "OpenMP*"` (and delete
  `build/CMakeCache.txt` if anything else is stuck as `NOTFOUND`).
- `cmake/apple.cmake` is a legacy toolchain file for Qt5-era builds. It is
  not used by default; do not pass it.

## Build and install

```
cmake --build build --config Release --target install
```

Always use the `install` target. The plain build produces
`build/opentrack/opentrack.app`, which is only the binary without plugins
or bundled libraries: it opens, but every module fails to load with
"Library load failure".

The finished application is:

```
build/install/opentrack.app
```

Launch it with:

```
open build/install/opentrack.app
```

## What the install step does

Beyond copying files, the install step runs `macosx/make-app-bundle.sh`,
which:

1. copies plugins into the app bundle,
2. runs `macdeployqt` to pull Qt libraries in,
3. runs `macosx/fixup-bundle.sh`, which makes the bundle self-contained:
   - copies Homebrew-referenced libraries (OpenCV, ONNX Runtime, and their
     dependency trees) into `Contents/Frameworks`,
   - rewrites absolute references to `@executable_path/../Frameworks/...`,
   - removes run paths pointing at the build machine. Without this the
     bundle loads a second copy of libraries from Homebrew and dies at
     startup with OpenMP's "duplicate libomp" error,
   - adds `@executable_path/../Frameworks` where `@rpath` entries need it,
   - fails the build if any reference still points outside the bundle,
4. builds the application icon (uses `sips`, no ImageMagick needed),
5. ad-hoc signs the bundle. Unsigned bundles are killed by macOS at
   launch with a code signature error,
6. optionally creates a DMG if `create-dmg` is installed
   (`brew install create-dmg`). Without it the install still succeeds.

## First run

macOS will ask for permissions:

- **Camera** for the webcam trackers,
- **Local Network** if you use a UDP output (this prompt appears once;
  it covers sending to localhost as well).

Profile files live in `~/Documents/opentrack-2.3/`.

## Troubleshooting

### `ld: tapi error: malformed file` during configure or build

Your Xcode/Command Line Tools are older than the SDK the compiler expects.
Update Xcode from the App Store, then re-run `sudo xcodebuild -license
accept`.

### `OpenMP_CXX_FLAGS: NOTFOUND`

The first configure ran before `libomp` was available. Fix with:

```
cmake -U "OpenMP*" -DOpenMP_ROOT=$(brew --prefix libomp) -S . -B build
```

### App opens, Start fails with "Library load failure"

Either the build-tree app was launched (see above, use the install
target), or the profile has empty module selections left over from such a
launch. Pick the tracker/protocol/filter in the dropdowns again, or delete
`~/Documents/opentrack-2.3/default.ini`.

### App is killed at launch with a signature error

Something modified the bundle after installation. Re-run the install
target to rebuild and re-sign it.

### NeuralNet tracker missing from the tracker list

ONNX Runtime was missing at configure time. Install it and reconfigure
(see the `-DONNXRuntime_DIR` flag above), then rebuild and reinstall.
