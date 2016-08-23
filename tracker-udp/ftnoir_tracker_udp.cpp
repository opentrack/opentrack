/* Copyright (c) 2014, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "ftnoir_tracker_udp.h"
#include "api/plugin-api.hpp"
#include "compat/nan.hpp"
#include "compat/util.hpp"

FTNoIR_Tracker::FTNoIR_Tracker() :
    last_recv_pose { 0,0,0, 0,0,0 },
    last_recv_pose2 { 0,0,0, 0,0,0 },
    should_quit(false)
{}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
    should_quit = true;
    wait();
}

void FTNoIR_Tracker::run()
{
    QByteArray datagram;
    datagram.resize(sizeof(last_recv_pose));

    should_quit = !sock.bind(QHostAddress::Any, quint16(s.port), QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    while (!should_quit)
    {
        if (sock.hasPendingDatagrams())
        {
            QMutexLocker foo(&mutex);

            bool ok = false;

            do
            {
                const qint64 sz = sock.readDatagram(reinterpret_cast<char*>(last_recv_pose2), sizeof(double[6]));
                if (sz > 0)
                    ok = true;
            }
            while (sock.hasPendingDatagrams());

            if (ok &&
                progn(
                    for (unsigned i = 0; i < 6; i++)
                    {
                        if (nanp(last_recv_pose2[i]))
                        {
                            return false;
                        }
                    }
                    return true;
               ))
            {
                for (unsigned i = 0; i < 6; i++)
                    last_recv_pose[i] = last_recv_pose2[i];
            }
        }

        (void) sock.waitForReadyRead(73);
    }
}

void FTNoIR_Tracker::start_tracker(QFrame*)
{
    start();
    sock.moveToThread(this);
}

void FTNoIR_Tracker::data(double *data)
{
    QMutexLocker foo(&mutex);
    for (int i = 0; i < 6; i++)
        data[i] = last_recv_pose[i];

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
        const unsigned k = clamp(unsigned(indices[i]), 0u, sizeof(values)/sizeof(*values) - 1u);
        data[Yaw + i] += values[k];
    }
}


OPENTRACK_DECLARE_TRACKER(FTNoIR_Tracker, TrackerControls, FTNoIR_TrackerDll)
