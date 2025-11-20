/* Copyright (c) 2025
 *
 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "ftnoir_tracker_smoothtrack.h"
#include "api/plugin-api.hpp"

#include <QDebug>
#include <cmath>

smoothtrack::smoothtrack() : last_recv_pose{ 0, 0, 0, 0, 0, 0 }
{
}

smoothtrack::~smoothtrack()
{
    requestInterruption();
    wait();
}

bool smoothtrack::connect_to_device()
{
    usbmuxd_device_info_t* device_list = nullptr;
    const int device_count = usbmuxd_get_device_list(&device_list);

    if (device_count < 1)
    {
        qDebug() << "smoothtrack: no iOS device found, usbmuxd_get_device_list returned" << device_count;
        if (device_list)
            usbmuxd_device_list_free(&device_list);
        return false;
    }

    // Use the first available device
    const uint32_t device_handle = device_list[0].handle;
    usbmuxd_device_list_free(&device_list);

    const int fd = usbmuxd_connect(device_handle, device_port);

    if (fd < 0)
    {
        qDebug() << "smoothtrack: usbmuxd_connect to port" << device_port << "failed with" << fd;
        return false;
    }

    // setSocketDescriptor() takes ownership of the descriptor, so from here on
    // it's sock that closes it. Only this failure path still owns it, and
    // usbmuxd_disconnect() is the portable counterpart to usbmuxd_connect().
    if (!sock.setSocketDescriptor(fd))
    {
        qDebug() << "smoothtrack: setSocketDescriptor failed --" << sock.errorString();
        usbmuxd_disconnect(fd);
        return false;
    }

    return true;
}

void smoothtrack::run()
{
    double pose[6];

    while (!isInterruptionRequested())
    {
        if (!sock.waitForReadyRead(10))
        {
            if (sock.state() != QAbstractSocket::ConnectedState)
            {
                qDebug() << "smoothtrack: connection lost --" << sock.errorString();
                break;
            }
            continue;
        }

        while (sock.bytesAvailable() >= qint64(sizeof(pose)))
        {
            const qint64 sz = sock.read(reinterpret_cast<char*>(pose), sizeof(pose));

            if (sz != qint64(sizeof(pose)))
            {
                qDebug() << "smoothtrack: read returned" << sz << "--" << sock.errorString();
                break;
            }

            // Validate the data (check for NaN and Infinite values)
            bool ok = true;

            for (unsigned i = 0; i < 6; i++)
            {
                const int val = std::fpclassify(pose[i]);
                if (val == FP_NAN || val == FP_INFINITE)
                {
                    ok = false;
                    break;
                }
            }

            if (ok)
            {
                QMutexLocker foo(&mutex);
                for (unsigned i = 0; i < 6; i++)
                    last_recv_pose[i] = pose[i];
            }
        }
    }

    sock.close();
}

module_status smoothtrack::start_tracker(QFrame*)
{
    // Connect here rather than from run(): the caller needs a definitive
    // answer, and waiting for the worker thread to publish one would mean
    // either blocking the UI thread or racing on a half-written member.
    if (!connect_to_device())
        return error(tr("Can't connect to iOS device. Make sure:\n"
                        "1. SmoothTrack app is running on your iOS device\n"
                        "2. \"Activate USB Connection\" is tapped in the app\n"
                        "3. Device is connected via USB\n"
                        "4. usbmuxd is running"));

    sock.moveToThread(this);
    start();

    return status_ok();
}

void smoothtrack::data(double* data)
{
    QMutexLocker foo(&mutex);
    for (int i = 0; i < 6; i++)
        data[i] = last_recv_pose[i];
}

OPENTRACK_DECLARE_TRACKER(smoothtrack, dialog_smoothtrack, smoothtrack_metadata)
