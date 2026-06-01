/* Copyright (c) 2014, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "livelink.h"
#include "api/plugin-api.hpp"

#include <cmath>
#include <iterator>

livelink::livelink() :
    last_recv_pose { 0,0,0 },
    last_recv_pose2 { 0,0,0 }
{}

livelink::~livelink()
{
    requestInterruption();
    wait();
}

void livelink::run()
{
    QByteArray datagram;

    while (!isInterruptionRequested())
    {
        if (sock.hasPendingDatagrams())
        {
            QMutexLocker foo(&mutex);

            bool ok = false;

            do
            {
                const qint64 sz = sock.pendingDatagramSize();
                datagram.resize(sz);
                sock.readDatagram(datagram.data(), sz); // Grab the datagram

                // If the IPhone loses face tracking, it will send a smaller datagram. Full face-tracking data has 61 floats,
                // so make we actually have the data before we do anything.
                if (sz > sizeof(float[61]))
                {
                    // There is static data with length depending on IPhone name at the left of the datagram.
                    // Our three head rotation values are located 9th from the right of the datagram.
                    // Discard the rest of the face data
                    datagram = datagram.right(sizeof(float[9])).left(sizeof(float[3]));
                    ok = true;
                    QDataStream dataStream(&datagram, QIODevice::ReadOnly);
                    dataStream.setByteOrder(QDataStream::BigEndian);
                    dataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
                    int i = 0;
                    while (!dataStream.atEnd())
                    {
                        dataStream >> last_recv_pose2[i];
                        i ++;
                    }
                }
            }
            while (sock.hasPendingDatagrams());

            if (ok)
            {
                for (unsigned i = 0; i < 3; i++)
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
                for (unsigned i = 0; i < 3; i++)
                    last_recv_pose[i] = last_recv_pose2[i];
            }
        }

        (void) sock.waitForReadyRead(73);
    }
}

module_status livelink::start_tracker(QFrame*)
{
    if (!sock.bind(QHostAddress::Any, quint16(s.port), QUdpSocket::DontShareAddress))
        return error(tr("Can't bind socket -- %1").arg(sock.errorString()));

    sock.moveToThread(this);
    start();

    return status_ok();
}

void livelink::data(double *data)
{
    QMutexLocker foo(&mutex);
    for (int i = 3; i < 6; i++) {
        data[i] = last_recv_pose[i];
    }

    // All values are 0-1, multiply yaw by -90, pitch by 90, and roll by -30
    int axisMultipliers[] = {
        -90,
        90,
        -30,
    };
    int values[] = {
        0,
        90,
        -90,
        180,
        -180,
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
            data[Yaw + i] *= axisMultipliers[i];
            data[Yaw + i] += values[k];
    }
}


OPENTRACK_DECLARE_TRACKER(livelink, dialog_livelink, meta_livelink)
