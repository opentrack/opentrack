#pragma once

#include <cstdint>
#include <string>

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

    // Flags of frames with gyroscope and accelerometer data. Magnetometer frames (about 400 per second, interleaved
    // with the 1000 per second gyro/accel frames) have 0x04 and no gyro/accel data. The flags look like a mask of
    // the sensors in a frame, but which bit is which isn't known, so a frame counts as gyro/accel when it has all of
    // these bits, whatever else is set.
    static constexpr uint32_t GyroAccelFlags = 0x0b;

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
};
#pragma pack(pop)

static_assert(sizeof(XRealOneIMUFrame) == XRealOneIMUFrame::Size, "XRealOneIMUFrame must be exactly 134 bytes");
