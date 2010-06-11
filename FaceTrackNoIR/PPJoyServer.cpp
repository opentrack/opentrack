/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay for				*
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
* PPJoyServer		PPJoyServer is the Class, that communicates headpose-data	*
*					to the Virtual Joystick, created by Deon van der Westhuysen.*
********************************************************************************/
#include <QtGui>
#include <QtNetwork>
#include "PPJoyServer.h"
#include "Tracker.h"
#include <Winsock.h>

/** constructor **/
PPJoyServer::PPJoyServer( Tracker *parent ) {

	// Save the parent
	headTracker = parent;

	// Create events
	m_StopThread = CreateEvent(0, TRUE, FALSE, 0);
	m_WaitThread = CreateEvent(0, TRUE, FALSE, 0);
}

/** QThread run @override **/
void PPJoyServer::run() {

	//// Init. the data
	//TestData.x = 0.0f;
	//TestData.y = 0.0f;
	//TestData.z = 0.0f;
	//TestData.h = 0.0f;
	//TestData.p = 0.0f;
	//TestData.r = 0.0f;
	//TestData.status = 0;
	//fg_cmd = 0;

	// Create UDP-sockets
	//inSocket = new QUdpSocket();
	//outSocket = new QUdpSocket();

	//// Connect the inSocket to the port, to receive readyRead messages
	//inSocket->bind(QHostAddress::LocalHost, 5551);

 //   // Connect the inSocket to the member-function, to read FlightGear commands
	//connect(inSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()), Qt::DirectConnection);
	exec();								// Exec only returns, when the thread terminates...
}

/** QThread terminate @override **/
void PPJoyServer::terminate() {

}

//END
