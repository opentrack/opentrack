/**
 * Copyright (C) 2026 DimKa <xstuff88@gmail.com>
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”),
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "tracker_xreal_one.h"
#include "tracker_xreal_one_imu_frame.h"

#include "api/plugin-api.hpp"

#include <cmath>
#include <iterator>
#include <QDebug>


XRealOneTracker::XRealOneTracker()
{
    static_assert(sizeof(header) == sizeof(XRealOneIMUFrame::DefaultHeader), "Header size mismatch");
    static_assert(sizeof(ga_marker) == sizeof(XRealOneIMUFrame::DefaultGyroAccelType), "Gyro/Accel marker size mismatch");

    memcpy(header, XRealOneIMUFrame::DefaultHeader, sizeof(header));
    memcpy(ga_marker, XRealOneIMUFrame::DefaultGyroAccelType, sizeof(ga_marker));
}

XRealOneTracker::~XRealOneTracker()
{
    requestInterruption();
    wait();
}

module_status XRealOneTracker::start_tracker(QFrame*)
{
    update_from_settings(true);

    start();

    return status_ok();
}

void XRealOneTracker::run()
{
    QByteArray buffer;
    buffer.reserve(XRealOneIMUFrame::Size * 10);

    while (!isInterruptionRequested())
    {
        // Try to connect to the XReal One device
        qDebug() << "XRealOneTracker::run: Connecting to" << s.ip_address << ":" << s.port;
        sock.connectToHost(s.ip_address, static_cast<quint16>(s.port));

        if (!sock.waitForConnected(5000))
        {
            qWarning() << "XReal One Tracker: Connection failed, re-connecting after 5s..." << sock.errorString();
            sock.abort();

            // Wait 5 seconds before retry
            for (int i = 0; i < 50 && !isInterruptionRequested(); ++i)
            {
                msleep(100);
            }
            continue;
        }

        qDebug() << "XRealOneTracker::run: Connected successfully";

        initialize_fusion_ahrs();

        // Clear buffer on new connection
        buffer.clear();

        // Read data from socket in infinite loop
        while (!isInterruptionRequested())
        {
            // XReal One sends data at approximately 1000 Hz
            sock.waitForReadyRead();

            QByteArray data = sock.readAll();
            if (!data.isEmpty())
            {
                // Append received data to buffer
                buffer.append(data);

                // Process all complete packets in the buffer
                while (buffer.size() >= XRealOneIMUFrame::Size)
                {
                    // Check for settings update
                    update_from_settings();

                    // Extract and parse the packet
                    parse_packet(buffer.left(XRealOneIMUFrame::Size));

                    // Remove processed bytes from buffer
                    buffer.remove(0, XRealOneIMUFrame::Size);
                }
            }

            // Check if socket is still connected
            if (sock.state() != QAbstractSocket::ConnectedState)
            {
                qWarning() << "XReal One Tracker: Disconnected from device, attempting to reconnect...";
                break;
            }
        }

        sock.disconnectFromHost();
        if (sock.state() != QAbstractSocket::UnconnectedState)
        {
            sock.waitForDisconnected(1000);
        }
    }
}

void XRealOneTracker::initialize_fusion_ahrs()
{
    ahrs = {};
    offset = {};
    euler_angles = {};

    // Initialise Fusion algorithms
    FusionOffsetInitialise(&offset, AvgSampleRateHz);

    FusionAhrsInitialise(&ahrs);

    // Set AHRS algorithm settings
    const FusionAhrsSettings settings = {
        .convention = FusionConventionEnu,
        .gain = 0.5f,
        .gyroscopeRange = 2000.0f, // TODO: verify gyroscope range for XReal One
        .accelerationRejection = 10.0f,
        .magneticRejection = 0.0f,
        .recoveryTriggerPeriod = AvgSampleRateHz / 10,
    };
    FusionAhrsSetSettings(&ahrs, &settings);
}

void XRealOneTracker::update_from_settings(bool force /*= false*/)
{
    if (s.changed || force)
    {
        auto header_array = QByteArray::fromHex(s.header_value().toUtf8());
        if (header_array.size() == sizeof(header))
        {
            memcpy(header, header_array.data(), sizeof(header));
        }
        else
        {
            qWarning() << "XRealOneTracker::update_from_settings: Invalid header value, using default";
        }

        auto ga_marker_hex = QByteArray::fromHex(s.ga_marker_value().toUtf8());
        if (ga_marker_hex.size() == sizeof(ga_marker))
        {
            memcpy(ga_marker, ga_marker_hex.data(), sizeof(ga_marker));
        }
        else
        {
            qWarning() << "XRealOneTracker::update_from_settings: Invalid GA marker value, using default";
        }
    }

    s.changed = false;
}

bool XRealOneTracker::is_header_valid(const XRealOneIMUFrame& frame) const
{
    return memcmp(frame.header, header, sizeof(header)) == 0;
}

bool XRealOneTracker::is_gyro_accel_data(const XRealOneIMUFrame& frame) const
{
    return memcmp(frame.data_type, ga_marker, sizeof(ga_marker)) == 0;
}

void XRealOneTracker::parse_packet(const QByteArray& packet)
{
    if (packet.size() != XRealOneIMUFrame::Size)
    {
        qDebug() << "XRealOneTracker::parse_packet: Invalid packet size";
        return;
    }

    XRealOneIMUFrame frame;
    memcpy(&frame, packet.data(), XRealOneIMUFrame::Size);

    if (!is_header_valid(frame))
    {
        qDebug() << "XRealOneTracker::parse_packet: Invalid header";
        return;
    }

    if (is_gyro_accel_data(frame))
    {
        process_ga_packet(frame);
    }
}

void XRealOneTracker::process_ga_packet(const XRealOneIMUFrame& frame)
{
    // At the moment, we do not apply any calibration
    static constexpr bool UseCalibration = false;

    const FusionVector gyroscope_rads = { frame.gyro_x, frame.gyro_y, frame.gyro_z };
    const FusionVector accelerometer_ms2 = { frame.accel_x, frame.accel_y, frame.accel_z };

    static const float rads2deg = 180.0f / static_cast<float>(M_PI);
    const float gravity_scale = static_cast<float>(1.0 / s.g_constant);

    auto gyroscope = FusionVectorMultiplyScalar(gyroscope_rads, rads2deg);
    auto accelerometer = FusionVectorMultiplyScalar(accelerometer_ms2, gravity_scale);

    // Apply calibration
    if constexpr (UseCalibration)
    {
        // TODO: Define calibration (replace with actual calibration data if available)
        const FusionMatrix gyroscopeMisalignment = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
        const FusionVector gyroscopeSensitivity = { 1.0f, 1.0f, 1.0f };
        const FusionVector gyroscopeOffset = { 0.0f, 0.0f, 0.0f };

        const FusionMatrix accelerometerMisalignment = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
        const FusionVector accelerometerSensitivity = { 1.0f, 1.0f, 1.0f };
        const FusionVector accelerometerOffset = { 0.0f, 0.0f, 0.0f };

        gyroscope = FusionCalibrationInertial(gyroscope, gyroscopeMisalignment, gyroscopeSensitivity, gyroscopeOffset);
        accelerometer = FusionCalibrationInertial(accelerometer, accelerometerMisalignment, accelerometerSensitivity, accelerometerOffset);
    }

    // Apply gyroscope offset correction (filtering)
    if (s.use_fusion_offset)
    {
        gyroscope = FusionOffsetUpdate(&offset, gyroscope);
    }

    static constexpr float DefaultDt = 1.0f / AvgSampleRateHz;

    // get delta time in seconds
    const float dt = (last_timestamp_ns == 0) ? (DefaultDt) : std::min((frame.timestamp_ns - last_timestamp_ns) * 1e-9f, DefaultDt * 10);
    last_timestamp_ns = frame.timestamp_ns;

    // Update gyroscope AHRS algorithm
    const FusionVector NoMagnetometer = { 0.0f, 0.0f, 0.0f };
    FusionAhrsUpdate(&ahrs, gyroscope, accelerometer, NoMagnetometer, dt);

    // Update internal Euler angles representation
    // NoMutex lock since we have 1 writer and 1 reader
    euler_angles = FusionEulerFrom(FusionAhrsGetQuaternion(&ahrs));
}

void XRealOneTracker::data(double *data)
{
    // no pose data
    data[0] = data[1] = data[2] = 0;

    // Orientation data (no mutex lock since we have 1 writer and 1 reader)
    const FusionEuler current = euler_angles;
    data[Yaw] = current.array[2];
    data[Pitch] = current.array[0];
    data[Roll] = current.array[1];
}

OPENTRACK_DECLARE_TRACKER(XRealOneTracker, XRealOneSettingsDialog, XRealOneMetadata)
