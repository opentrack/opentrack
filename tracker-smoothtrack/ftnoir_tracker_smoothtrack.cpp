/* Copyright (c) 2025
 *
 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "ftnoir_tracker_smoothtrack.h"
#include "api/plugin-api.hpp"

#include <QSocketNotifier>
#include <cmath>
#include <iterator>

smoothtrack::smoothtrack() : last_recv_pose{ 0, 0, 0, 0, 0, 0 }, last_recv_pose2{ 0, 0, 0, 0, 0, 0 }
{
}

smoothtrack::~smoothtrack()
{
    requestInterruption();
    wait();
    disconnect_from_device();
}

bool smoothtrack::connect_to_device()
{
    // Try to connect to iOS device via usbmuxd
    usbmuxd_device_info_t* device_list = nullptr;
    int device_count = usbmuxd_get_device_list(&device_list);

    if (device_count < 1)
    {
        if (device_list)
            usbmuxd_device_list_free(&device_list);
        return false;
    }

    // Use the first available device
    uint32_t device_handle = device_list[0].handle;
    usbmuxd_device_list_free(&device_list);

    // Connect to the SmoothTrack port on the iOS device
    usbmuxd_fd = usbmuxd_connect(device_handle, static_cast<uint16_t>(s.port));

    if (usbmuxd_fd < 0)
    {
        return false;
    }

    return true;
}

void smoothtrack::disconnect_from_device()
{
    if (sock)
    {
        sock->close();
        sock->deleteLater();
        sock = nullptr;
    }

    if (usbmuxd_fd >= 0)
    {
        close(usbmuxd_fd);
        usbmuxd_fd = -1;
    }
}

void smoothtrack::run()
{
    // Connect to the iOS device
    if (!connect_to_device())
    {
        return;
    }

    // Wrap the native socket in a QTcpSocket
    sock = new QTcpSocket();
    if (!sock->setSocketDescriptor(usbmuxd_fd))
    {
        disconnect_from_device();
        return;
    }

    QByteArray buffer;
    buffer.resize(sizeof(last_recv_pose));

    while (!isInterruptionRequested())
    {
        // Wait for data to be available
        if (!sock->waitForReadyRead(100))
        {
            // Check if we're still connected
            if (sock->state() != QAbstractSocket::ConnectedState)
            {
                break;
            }
            continue;
        }

        // Read available data
        while (sock->bytesAvailable() >= static_cast<qint64>(sizeof(last_recv_pose2)))
        {
            QMutexLocker foo(&mutex);

            bool ok = false;

            // Read exactly 48 bytes (6 doubles)
            const qint64 sz = sock->read(reinterpret_cast<char*>(last_recv_pose2), sizeof(double[6]));

            if (sz == sizeof(double[6]))
            {
                ok = true;

                // Validate the data (check for NaN and Infinite values)
                for (unsigned i = 0; i < 6; i++)
                {
                    int val = std::fpclassify(last_recv_pose2[i]);
                    if (val == FP_NAN || val == FP_INFINITE)
                    {
                        ok = false;
                        break;
                    }
                }
            }

            if (ok)
            {
                for (unsigned i = 0; i < 6; i++)
                    last_recv_pose[i] = last_recv_pose2[i];
            }
        }
    }

    disconnect_from_device();
}

module_status smoothtrack::start_tracker(QFrame*)
{
    // Start the worker thread which will handle the usbmuxd connection
    start();

    // Give the thread a moment to try connecting
    msleep(100);

    // Check if connection was successful
    if (usbmuxd_fd < 0)
    {
        return error(tr("Can't connect to iOS device. Make sure:\n"
                        "1. SmoothTrack app is running on your iOS device\n"
                        "2. Device is connected via USB\n"
                        "3. usbmuxd is running\n"
                        "4. Port number matches SmoothTrack settings"));
    }

    return status_ok();
}

void smoothtrack::data(double* data)
{
    QMutexLocker foo(&mutex);
    for (int i = 0; i < 6; i++)
        data[i] = last_recv_pose[i];

    int values[] = {
        0, 90, -90, 180, -180,
    };
    int indices[] = {
        s.add_yaw,
        s.add_pitch,
        s.add_roll,
    };

    for (int i = 0; i < 3; i++)
    {
        const int k = indices[i];
        if (k >= 0 && k < std::distance(std::begin(values), std::end(values)))
            data[Yaw + i] += values[k];
    }
}

OPENTRACK_DECLARE_TRACKER(smoothtrack, dialog_smoothtrack, smoothtrack_metadata)
