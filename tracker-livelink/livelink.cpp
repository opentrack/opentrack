/* Copyright (c) 2014, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "livelink.h"
#include "api/plugin-api.hpp"

#include <QDebug>

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
    datagram.resize(sizeof(last_recv_pose));

    while (!isInterruptionRequested())
    {
        if (sock.hasPendingDatagrams())
        {
            QMutexLocker foo(&mutex);

            bool ok = false;

            do
            {
                const qint64 sz = sock.pendingDatagramSize();
                // sock.readDatagram(reinterpret_cast<char*>(last_recv_pose2), sz);
                QByteArray temp;
                temp.resize(sz);
                sock.readDatagram(temp.data(), sz);
                temp = temp.right(36);
                temp = temp.left(12);
                // qDebug() << temp.toHex();
                if (temp.size() > 0)
                {
                    ok = true;
                    QDataStream dataStream(&temp, QIODevice::ReadOnly);
                    dataStream.setByteOrder(QDataStream::BigEndian);
                    dataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
                    int i = 0;
                    while (!dataStream.atEnd())
                    {
                        dataStream >> last_recv_pose2[i];
                        // qDebug() << last_recv_pose2[i];
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
    for (int i = 0; i < 3; i++) {
        data[i+3] = last_recv_pose[i];
        // qDebug() << last_recv_pose[i];
    }

    int fixedValues[] = {
        90,
        90,
        30,
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
            data[Yaw + i] *= fixedValues[i];
            data[Yaw + i] += values[k];
    }
}


OPENTRACK_DECLARE_TRACKER(livelink, dialog_livelink, livelink_receiver_dll)
