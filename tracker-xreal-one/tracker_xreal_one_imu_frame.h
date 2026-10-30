#pragma once

#include <bit>
#include <cstdint>
#include <limits>
#include <string>

// True unless v is NaN or infinite. Tests the exponent bits rather than calling std::isfinite(), which -ffast-math
// (set in opentrack's Release toolchain files) lets the compiler replace with true.
inline bool xreal_one_is_finite(float v)
{
    static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 floats expected");
    return (std::bit_cast<uint32_t>(v) & 0x7f800000u) != 0x7f800000u;
}

/**
 *  Frame structure for XReal One IMU data packets.
 *  Total size: 134 bytes
 *
 *  Layout based on reverse engineering:
 *  - Header: 6 bytes (0x28 0x36 0x00 0x00 0x00 0x80)
 *  - Session ID: 8 bytes
 *  - Timestamp: 8 bytes (nanoseconds)
 *  - Sensor counter: 8 bytes
 *  - Flags: 4 bytes (frame type)
 *  - Gyro data: 12 bytes (3 floats: x, y, z)
 *  - Accelerometer data: 12 bytes (3 floats: x, y, z)
 *  - Additional sensor data: 17 bytes
 *  - Frame ID: 4 bytes
 *  - Unknown: 55 bytes
 */
#pragma pack(push, 1)
struct XRealOneIMUFrame
{
    static constexpr int Size = 134;

    static constexpr uint8_t DefaultHeader[6]          = { 0x28, 0x36, 0x00, 0x00, 0x00, 0x80 };

    // Flags of frames with gyroscope and accelerometer data (1000 per second). The flags look like a mask of the
    // sensors in a frame, but which bit is which isn't known, so a frame counts as gyro/accel when it has all of these
    // bits, whatever else is set.
    static constexpr uint32_t GyroAccelFlags = 0x0b;

    // Flags of the magnetometer frames (about 400 per second, interleaved with the gyro/accel frames), which have no
    // gyro/accel data
    static constexpr uint32_t MagnetometerFlags = 0x04;

    // Offset 0-5: Frame header (6 bytes)
    uint8_t header[6];

    // Offset 6-13: Session ID (8 bytes)
    uint64_t session_id;

    // Offset 14-21: Timestamp in nanoseconds (8 bytes)
    uint64_t timestamp_ns;

    // Offset 22-29: Sensor counter (8 bytes), 0 in magnetometer frames
    uint64_t sensor_counter;

    // Offset 30-33: Flags (4 bytes), the frame type
    uint32_t flags;

    // Offset 34-45: Gyroscope data (3 floats, 12 bytes)
    float gyro_x;
    float gyro_y;
    float gyro_z;

    // Offset 46-57: Acceleration data (3 floats, 12 bytes)
    float accel_x;
    float accel_y;
    float accel_z;

    // Offset 58-74: Additional sensor data (17 bytes)
    // Might be magnetometer or second set of sensors
    uint8_t additional_sensor_data[17];

    // Offset 75-78: Frame ID (4 bytes). Gyro/accel and magnetometer frames count separately, from shortly after the
    // glasses boot; the gyro/accel count passes 2^24 about 4 h 40 min after boot.
    uint32_t frame_id;

    // Offset 79-133: Unknown (55 bytes)
    uint8_t unknown[55];

    // True if the flags mark a frame with gyroscope and accelerometer data
    bool is_gyro_accel() const
    {
        return (flags & GyroAccelFlags) == GyroAccelFlags;
    }

    // True if the flags mark a magnetometer frame
    bool is_magnetometer() const
    {
        return !is_gyro_accel() && (flags & MagnetometerFlags) != 0;
    }

    // True if all gyroscope and accelerometer values are finite (magnetometer frames have NaN there)
    bool has_finite_gyro_accel() const
    {
        return xreal_one_is_finite(gyro_x) && xreal_one_is_finite(gyro_y) && xreal_one_is_finite(gyro_z) &&
               xreal_one_is_finite(accel_x) && xreal_one_is_finite(accel_y) && xreal_one_is_finite(accel_z);
    }
};
#pragma pack(pop)

static_assert(sizeof(XRealOneIMUFrame) == XRealOneIMUFrame::Size, "XRealOneIMUFrame must be exactly 134 bytes");
