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
 *  - Data Header: 12 bytes
 *  - Gyro data: 12 bytes (3 floats: x, y, z)
 *  - Accelerometer data: 12 bytes (3 floats: x, y, z)
 *  - Additional sensor data: 24 bytes
 *  - Data Type: 6 bytes
 *  - Data Footer: 20 bytes
 *  - Unknown: 16 bytes
 *  - Footer: 14 bytes
 */
#pragma pack(push, 1)
struct XRealOneIMUFrame
{
    static constexpr int Size = 134;

    static constexpr uint8_t DefaultHeader[6]          = { 0x28, 0x36, 0x00, 0x00, 0x00, 0x80 };

    static constexpr uint8_t DefaultGyroAccelType[6]   = { 0x00, 0x40, 0x1f, 0x00, 0x00, 0x40 };

    // Offset 0-5: Frame header (6 bytes)
    uint8_t header[6];

    // Offset 6-13: Session ID (8 bytes)
    uint64_t session_id;

    // Offset 14-21: Timestamp in nanoseconds (8 bytes)
    uint64_t timestamp_ns;

    // Offset 22-33: Data header (12 bytes)
    uint8_t data_header[12];

    // Offset 34-45: Gyroscope data (3 floats, 12 bytes)
    float gyro_x;
    float gyro_y;
    float gyro_z;

    // Offset 46-57: Acceleration data (3 floats, 12 bytes)
    float accel_x;
    float accel_y;
    float accel_z;

    // Offset 58-77: Additional sensor data (20 bytes)
    // Might be magnetometer or second set of sensors
    uint8_t additional_sensor_data[20];

    // Offset 78-83: Data type indicator (6 bytes)
    uint8_t data_type[6];

    // Offset 84-103: Data footer (20 bytes)
    uint8_t data_footer[20];

    // Offset 104-119: Unknown/Reserved (16 bytes)
    // Usually just zeros with single `1` byte
    uint8_t reserved[16];

    // Offset 120-133: Frame footer (14 bytes)
    uint8_t footer[14];
};
#pragma pack(pop)

static_assert(sizeof(XRealOneIMUFrame) == XRealOneIMUFrame::Size, "XRealOneIMUFrame must be exactly 134 bytes");
