/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*
*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
*																				*
* FGServer			FGServer is the Class, that communicates headpose-data		*
*					to FlightGear, using UDP.				         			*
*					It is based on the (Linux) example made by Melchior FRANZ.	*
********************************************************************************/
#include "ftnoir_protocol_fg.h"
#include "facetracknoir/global-settings.h"
#include <ftnoir_tracker_base/ftnoir_tracker_types.h>

// For Todd and Arda Kutlu

void FTNoIR_Protocol::reloadSettings()
{
    s.b->reload();
}

void FTNoIR_Protocol::sendHeadposeToGame(const double* headpose) {
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

bool FTNoIR_Protocol::checkServerInstallationOK()
{   
    return outSocket.bind(QHostAddress::Any, 0, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocol* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Protocol;
}
