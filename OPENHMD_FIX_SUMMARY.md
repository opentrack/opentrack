# OpenHMD Tracker Fix: Clean Slate Implementation

## Date: March 5, 2026, 21:09

## The Fundamental Bug

**Root Cause**: The quaternion components were being read from the **wrong array indices**.

### What Was Wrong
Previous code assumed OpenHMD returns quaternion as `[w, x, y, z]`:
```cpp
// INCORRECT ASSUMPTION
const double w = q[0], x = q[1], y = q[2], z = q[3];
```

### What Is Correct
OpenHMD API documentation (`OpenHMD/include/openhmd.h` line 101) explicitly states:
```c
/** float[4] (get): Absolute rotation of the device, in space, as a quaternion (x, y, z, w). */
OHMD_ROTATION_QUAT = 1,
```

**The quaternion is returned as [x, y, z, w], NOT [w, x, y, z]!**

## Clean Implementation

### Fixed Quaternion Reading
```cpp
// CORRECT: OpenHMD API returns [x, y, z, w]
const double x = q[0];
const double y = q[1];
const double z = q[2];
const double w = q[3];
```

### Standard ZYX Tait-Bryan Conversion
Applied textbook quaternion-to-Euler conversion for ZYX rotation order:

```cpp
// Roll: rotation around X-axis
double sinr_cosp = 2.0 * (w * x + y * z);
double cosr_cosp = 1.0 - 2.0 * (x * x + y * y);
roll = std::atan2(sinr_cosp, cosr_cosp) * (180.0 / M_PI);

// Pitch: rotation around Y-axis (OpenTrack pitch = Y-axis rotation)
double sinp = 2.0 * (w * y - z * x);
if (std::abs(sinp) >= 1.0)
    pitch = std::copysign(90.0, sinp);  // Gimbal lock handling
else
    pitch = std::asin(sinp) * (180.0 / M_PI);

// Yaw: rotation around Z-axis (negated per OpenTrack convention)
double siny_cosp = 2.0 * (w * z + x * y);
double cosy_cosp = 1.0 - 2.0 * (y * y + z * z);
yaw = -std::atan2(siny_cosp, cosy_cosp) * (180.0 / M_PI);
```

### Sign Conventions Applied
- **Roll**: Positive (no negation)
- **Pitch**: Positive (no negation)
- **Yaw**: **Negated** (follows Oculus Rift SDK reference pattern from tracker-rift-140)

## What Changed From Previous Ad-Hoc Fixes

1. **Removed axis swaps**: No longer swapping Y↔Z components
2. **Removed pitch negation**: Pitch is now computed directly without sign inversion
3. **Fixed quaternion indexing**: This is the critical fix that makes everything else work
4. **Kept yaw negation**: This matches established OpenTrack convention (per Oculus reference)

## Expected Behavior After Fix

With correct quaternion component extraction:
- **Pitch**: Should remain correct (user reported working)
- **Yaw**: Should now move in correct direction with proper magnitude
- **Roll**: Should work correctly with standard formula

## Testing Instructions

1. Restart OpenTrack
2. Select "OpenHMD" tracker and "Fusion" with OpenHMD as rotation source
3. Enable Alpha-spectrum filter with high-rate mode
4. Test each axis independently:
   - **Pitch**: Nod head up/down
   - **Yaw**: Turn head left/right  
   - **Roll**: Tilt head side-to-side
5. Verify directionality matches physical movements

## Build Information

- **Source file**: `/home/nos/headtracking/opentrack/tracker-openhmd/ftnoir_tracker_openhmd.cpp`
- **Build output**: `/home/nos/headtracking/opentrack/build/tracker-openhmd/opentrack-tracker-openhmd.so`
- **Install location**: `/home/nos/headtracking/opentrack/build/install/libexec/opentrack/opentrack-tracker-openhmd.so`
- **Build timestamp**: 2026-03-05 21:08:24
- **Deploy timestamp**: 2026-03-05 21:09:13
- **File size**: 1.8M

## References

- Analysis document: `/home/nos/headtracking/opentrack/OPENHMD_COORDINATE_ANALYSIS.md`
- OpenHMD API: `/home/nos/headtracking/OpenHMD/include/openhmd.h`
- OpenTrack conventions: `/home/nos/headtracking/opentrack/compat/hamilton-tools.cpp`
- Oculus reference: `/home/nos/headtracking/opentrack/tracker-rift-140/rift-140.cpp`
