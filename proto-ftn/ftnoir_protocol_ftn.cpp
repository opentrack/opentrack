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
#include "ftnoir_protocol_ftn.h"
#include <QFile>
#include "api/plugin-api.hpp"

udp::udp()
{
}

void udp::pose(const double *headpose) {
    int destPort = s.port;
    QHostAddress destIP(QString("%1.%2.%3.%4").arg(
                            QString::number(static_cast<int>(s.ip1)),
                            QString::number(static_cast<int>(s.ip2)),
                            QString::number(static_cast<int>(s.ip3)),
                            QString::number(static_cast<int>(s.ip4))));
    outSocket.writeDatagram((const char *) headpose, sizeof( double[6] ), destIP, destPort);
}

bool udp::correct()
{   
    return outSocket.bind(QHostAddress::Any, 0, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
}

OPENTRACK_DECLARE_PROTOCOL(udp, FTNControls, udpDll)
