/* Homepage         http://facetracknoir.sourceforge.net/home/default.htm        *
 *                                                                               *
 * ISC License (ISC)                                                             *
 *                                                                               *
 * Copyright (c) 2015, Wim Vriend                                                *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */
#include "ftnoir_protocol_fg.h"
#include "api/plugin-api.hpp"

// For Todd and Arda Kutlu

void flightgear::pose(const double* headpose) {
    FlightData.x = headpose[TX] * 1e-2;
    FlightData.y = headpose[TY] * 1e-2;
    FlightData.z = headpose[TZ] * 1e-2;
    FlightData.p = headpose[Pitch];
    FlightData.h = headpose[Yaw];
    FlightData.r = headpose[Roll];
    FlightData.status = 1;
    QHostAddress destIP(QString("%1.%2.%3.%4").arg(
                            QString::number(static_cast<int>(s.ip1)),
                            QString::number(static_cast<int>(s.ip2)),
                            QString::number(static_cast<int>(s.ip3)),
                            QString::number(static_cast<int>(s.ip4))));
    int destPort = s.port;
    (void) outSocket.writeDatagram(reinterpret_cast<const char*>(&FlightData), sizeof(FlightData), destIP, static_cast<quint16>(destPort));
}

bool flightgear::correct()
{   
    return outSocket.bind(QHostAddress::Any, 0, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
}

OPENTRACK_DECLARE_PROTOCOL(flightgear, FGControls, flightgearDll)
