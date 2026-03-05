# OpenHMD to OpenTrack Coordinate System Conversion Analysis

## Problem Statement
The OpenHMD tracker quaternion-to-Euler conversion has been patched iteratively without understanding the fundamental coordinate system conventions. This document establishes the ground truth.

## Key Facts

### 1. OpenHMD Quaternion Format (CONFIRMED FROM API DOCS)
From `OpenHMD/include/openhmd.h` line 101:
```c
/** float[4] (get): Absolute rotation of the device, in space, as a quaternion (x, y, z, w). */
OHMD_ROTATION_QUAT = 1,
```

**Critical: OpenHMD returns quaternion as [x, y, z, w], NOT [w, x, y, z]**

The current buggy code reads:
```cpp
const double w = q[0], x = q[1], y = q[2], z = q[3];  // WRONG!
```

Should be:
```cpp
const double x = q[0], y = q[1], z = q[2], w = q[3];  // CORRECT
```

### 2. OpenHMD Coordinate System
OpenHMD provides "OpenGL style" matrices (per API docs), indicating OpenGL's right-handed coordinate system:
- **+X axis**: Right
- **+Y axis**: Up  
- **+Z axis**: Towards viewer (out of screen)

### 3. OpenTrack Coordinate System
From `compat/hamilton-tools.cpp` lines 27-29:
```cpp
Q  = QuatFromAngleAxe( -YPR[2], {0, 0, 1} ); //Roll,  Z axe
Qp = QuatFromAngleAxe( -YPR[1], {1, 0, 0} ); //Pitch, X axe
Qy = QuatFromAngleAxe( -YPR[0], {0, 1, 0} ); //Yaw,   Y axe
```

OpenTrack defines:
- **Yaw**: Rotation around **Y axis** (vertical)
- **Pitch**: Rotation around **X axis** (horizontal left/right)
- **Roll**: Rotation around **Z axis** (depth/forward-back)

### 4. Reference Implementation: Oculus Rift SDK (tracker-rift-140)
From `tracker-rift-140/rift-140.cpp` lines 80-82:
```cpp
data[Yaw] =     double(yaw)   * -d2r;  // NEGATED
data[Pitch] =   double(pitch) *  d2r;  // POSITIVE
data[Roll] =    double(roll)  *  d2r;  // POSITIVE
```

Pattern: **Yaw typically needs negation**, pitch and roll are typically positive.

## Standard Quaternion to Euler Conversion

For quaternion (w, x, y, z) in **ZYX Tait-Bryan** rotation order:

```cpp
// Roll: rotation around X-axis
double sinr_cosp = 2 * (w * x + y * z);
double cosr_cosp = 1 - 2 * (x * x + y * y);
double roll = std::atan2(sinr_cosp, cosr_cosp);

// Pitch: rotation around Y-axis  
double sinp = 2 * (w * y - z * x);
double pitch = std::asin(sinp);  // with gimbal lock check

// Yaw: rotation around Z-axis
double siny_cosp = 2 * (w * z + x * y);
double cosy_cosp = 1 - 2 * (y * y + z * z);
double yaw = std::atan2(siny_cosp, cosy_cosp);
```

## Hypothesis: Correct Conversion for OpenHMD → OpenTrack

Given:
1. OpenHMD uses OpenGL coordinates (+Y up, +X right, +Z toward viewer)  
2. OpenTrack expects standard aviation coordinates
3. Oculus Rift reference shows yaw negation pattern
4. User reports "pitch is now correct"

**Proposed conversion:**
```cpp
void quat_to_euler(const float q[4], double &yaw, double &pitch, double &roll)
{
    // CRITICAL: OpenHMD returns [x, y, z, w], not [w, x, y, z]!
    const double x = q[0];
    const double y = q[1];
    const double z = q[2];
    const double w = q[3];
    
    // Standard ZYX Tait-Bryan conversion
    
    // Roll: X-axis rotation (unchanged)
    double sinr_cosp = 2.0 * (w * x + y * z);
    double cosr_cosp = 1.0 - 2.0 * (x * x + y * y);
    roll = std::atan2(sinr_cosp, cosr_cosp) * (180.0 / M_PI);
    
    // Pitch: Y-axis rotation (OpenTrack pitch = Y-axis in aviation coords)
    double sinp = 2.0 * (w * y - z * x);
    if (std::abs(sinp) >= 1.0)
        pitch = std::copysign(90.0, sinp); // Gimbal lock
    else
        pitch = std::asin(sinp) * (180.0 / M_PI);
    
    // Yaw: Z-axis rotation (negated following Oculus Rift pattern)
    double siny_cosp = 2.0 * (w * z + x * y);
    double cosy_cosp = 1.0 - 2.0 * (y * y + z * z);
    yaw = -std::atan2(siny_cosp, cosy_cosp) * (180.0 / M_PI);
}
```

## Testing Strategy

1. **Start from clean slate**: Remove all previous ad-hoc patches
2. **Use correct quaternion indexing**: x=q[0], y=q[1], z=q[2], w=q[3]
3. **Apply standard conversion**: ZYX Tait-Bryan formula
4. **Test with hardware**: Validate yaw, pitch, roll separately
5. **Adjust sign conventions only after empirical testing**

## References
- OpenHMD API: `/home/nos/headtracking/OpenHMD/include/openhmd.h`
- OpenTrack conventions: `/home/nos/headtracking/opentrack/compat/hamilton-tools.cpp`
- Oculus reference: `/home/nos/headtracking/opentrack/tracker-rift-140/rift-140.cpp`
- Euler utils: `/home/nos/headtracking/opentrack/compat/euler.cpp`
