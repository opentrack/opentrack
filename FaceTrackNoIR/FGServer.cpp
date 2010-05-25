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
* FGServer			FGServer is the Class, that communicates headpose-data		*
*					to FlightGear, using UDP.				         			*
*					It is based on the (Linux) example made by Melchior FRANZ.	*
********************************************************************************/
#include <QtGui>
#include <QtNetwork>
#include "FGServer.h"
#include "Tracker.h"
#include <Winsock.h>

/** constructor **/
FGServer::FGServer( Tracker *parent ) {

	// Save the parent
	headTracker = parent;

	// Create events
	m_StopThread = CreateEvent(0, TRUE, FALSE, 0);
	m_WaitThread = CreateEvent(0, TRUE, FALSE, 0);
}

//
// Callback function registered in the run() method.
// Retrieve all pending Datagrams (typically 1) and send a reply, containing the headpose-data.
//
void FGServer::readPendingDatagrams()
{
QHostAddress sender;
quint16 senderPort;
qint32 cmd;
int no_bytes;

	//
	// Read data from FlightGear
	//
	while (inSocket->hasPendingDatagrams()) {
		QByteArray datagram;
		datagram.resize(inSocket->pendingDatagramSize());

//		qDebug() << "FGServer::readPendingDatagrams says: size =" << inSocket->pendingDatagramSize();

		inSocket->readDatagram( (char * ) &cmd, sizeof(cmd), &sender, &senderPort);
//		qDebug() << "FGServer::readPendingDatagrams says: data =" << cmd;

		fg_cmd = cmd;									// Let's just accept that command for now...
		if ( cmd > 0 ) {
			headTracker->handleGameCommand ( cmd );		// Send it upstream, for the Tracker to handle
		}
	}

	//
	// Copy the Raw measurements directly to the client.
	//
	TestData.x = virtPosX;
	TestData.y = virtPosY;
	TestData.z = virtPosZ;
	TestData.p = virtRotX;
	TestData.h = virtRotY;
	TestData.r = virtRotZ;
	TestData.status = fg_cmd;

	//
	// Try to send an UDP-message to the FlightGear
	//

	//! [1]
	no_bytes = outSocket->writeDatagram((const char *) &TestData, sizeof( TestData ), 
		QHostAddress::LocalHost, 5550);
	if ( no_bytes > 0) {
//		qDebug() << "FGServer::writePendingDatagrams says: bytes send =" << no_bytes << sizeof( double );
	}
	else {
		qDebug() << "FGServer::writePendingDatagrams says: nothing send!";
	}

}

/** QThread run @override **/
void FGServer::run() {

	// Init. the data
	TestData.x = 0.0f;
	TestData.y = 0.0f;
	TestData.z = 0.0f;
	TestData.h = 0.0f;
	TestData.p = 0.0f;
	TestData.r = 0.0f;
	TestData.status = 0;
	fg_cmd = 0;

	// Create UDP-sockets
	inSocket = new QUdpSocket();
	outSocket = new QUdpSocket();

	// Connect the inSocket to the port, to receive readyRead messages
	inSocket->bind(QHostAddress::LocalHost, 5551);

    // Connect the inSocket to the member-function, to read FlightGear commands
	connect(inSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()), Qt::DirectConnection);
	exec();								// Exec only returns, when the thread terminates...
}

/** QThread terminate @override **/
void FGServer::terminate() {

	inSocket->disconnectFromHost();
	inSocket->waitForDisconnected();
	outSocket->disconnectFromHost();
	outSocket->waitForDisconnected();

	delete inSocket;
	delete outSocket;
}

//END
