/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage:			http://facetracknoir.sourceforge.net/home/default.htm		*
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
********************************************************************************/
#include "ftnoir_tracker_udp.h"
#include "facetracknoir/plugin-support.h"

FTNoIR_Tracker::FTNoIR_Tracker() : last_recv_pose { 0,0,0, 0,0,0 }, should_quit(false) {}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
    should_quit = true;
    wait();
}

void FTNoIR_Tracker::run() {
    QByteArray datagram;
    datagram.resize(sizeof(last_recv_pose));
    for (;;) {
        if (should_quit)
            break;
        QMutexLocker foo(&mutex);
        while (sock.hasPendingDatagrams()) {
            sock.readDatagram((char * ) last_recv_pose, sizeof(double[6]));
        }
        msleep(1);
    }
}

void FTNoIR_Tracker::StartTracker(QFrame*)
{
    (void) sock.bind(QHostAddress::Any, (int) s.port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
	start();
}

void FTNoIR_Tracker::GetHeadPoseData(double *data)
{
    QMutexLocker foo(&mutex);
    for (int i = 0; i < 6; i++)
        data[i] = last_recv_pose[i];
}

extern "C" OPENTRACK_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Tracker;
}
