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

FTNoIR_Tracker::FTNoIR_Tracker() : should_quit(false)
{
    should_quit = false;

    for (int i = 0; i < 6; i++)
        newHeadPose[i] = 0;
}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
    should_quit = true;
    wait();
}

void FTNoIR_Tracker::run() {
	forever {
        if (should_quit)
            break;
        while (inSocket.hasPendingDatagrams()) {
                QMutexLocker foo(&mutex);
				QByteArray datagram;
                datagram.resize(sizeof(newHeadPose));
                inSocket.readDatagram((char * ) newHeadPose, sizeof(double[6]));
        }
		usleep(10000);
	}
}

void FTNoIR_Tracker::StartTracker(QFrame*)
{
    (void) inSocket.bind(QHostAddress::Any, (int) s.port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
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
