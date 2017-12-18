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
    FlightData.x = -headpose[TX] * 1e-2;
    FlightData.y = headpose[TY] * 1e-2;
    FlightData.z = headpose[TZ] * 1e-2;
    FlightData.p = headpose[Pitch];
    FlightData.h = -headpose[Yaw];
    FlightData.r = -headpose[Roll];
    FlightData.status = 1;
    QHostAddress destIP(quint32(s.ip1 << 24 | s.ip2 << 16 | s.ip3 << 8 | s.ip4));
    (void) outSocket.writeDatagram(reinterpret_cast<const char*>(&FlightData), sizeof(FlightData), destIP, static_cast<quint16>(s.port));
}

module_status flightgear::initialize()
{   
    if (outSocket.bind(QHostAddress::Any, 0, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
        return status_ok();
    else
        return error(otr_tr("Can't bind to [%1.%2.%3.%4]:%5")
                    .arg(s.ip1).arg(s.ip2).arg(s.ip3).arg(s.ip4)
                    .arg(s.port));
}

OPENTRACK_DECLARE_PROTOCOL(flightgear, FGControls, flightgearDll)
