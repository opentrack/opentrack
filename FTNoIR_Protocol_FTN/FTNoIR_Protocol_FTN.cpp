/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010-2011	Wim Vriend (Developing)								*
*							Ron Hendriks (Researching and Testing)				*
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
* FTNServer			FTNServer is the Class, that communicates headpose-data		*
*					to another FaceTrackNoIR program, using UDP.       			*
*					It is based on the (Linux) example made by Melchior FRANZ.	*
********************************************************************************/
/*
	Modifications (last one on top):
	20110401 - WVR: Moved protocol to a DLL, convenient for installation etc.
	20101224 - WVR: Base class is no longer inheriting QThread. sendHeadposeToGame
					is called from run() of Tracker.cpp
*/
#include "ftnoir_protocol_ftn.h"
#include <QFile>

/** constructor **/
FTNoIR_Protocol_FTN::FTNoIR_Protocol_FTN()
{
	loadSettings();
}

/** destructor **/
FTNoIR_Protocol_FTN::~FTNoIR_Protocol_FTN()
{
	if (inSocket != 0) {
		inSocket->close();
		delete inSocket;
	}
	
	if (outSocket != 0) {
		outSocket->close();
		delete outSocket;
	}
}

/** helper to Auto-destruct **/
void FTNoIR_Protocol_FTN::Release()
{
    delete this;
}

void FTNoIR_Protocol_FTN::Initialize()
{
	return;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Protocol_FTN::loadSettings() {
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "FTN" );

	QString destAddr = iniFile.value ( "IP-1", 192 ).toString() + "." + iniFile.value ( "IP-2", 168 ).toString() + "." + iniFile.value ( "IP-3", 2 ).toString() + "." + iniFile.value ( "IP-4", 1 ).toString();
	destIP = QHostAddress( destAddr );
	destPort = iniFile.value ( "PortNumber", 5550 ).toInt();

	iniFile.endGroup ();
}

//
// Update Headpose in Game.
//
void FTNoIR_Protocol_FTN::sendHeadposeToGame( THeadPoseData *headpose, THeadPoseData *rawheadpose ) {
int no_bytes;
QHostAddress sender;
quint16 senderPort;

	//
	// Copy the Raw measurements directly to the client.
	//
	frame_counter += 1;
	TestData = *headpose;
	TestData.frame_number = frame_counter;

	//
	// Try to send an UDP-message to the receiver
	//

	//! [1]
	if (outSocket != 0) {
		no_bytes = outSocket->writeDatagram((const char *) &TestData, sizeof( TestData ), destIP, destPort);
		if ( no_bytes > 0) {
//		qDebug() << "FTNServer::writePendingDatagrams says: bytes send =" << no_bytes << sizeof( double );
		}
		else {
			qDebug() << "FTNServer::writePendingDatagrams says: nothing sent!";
		}
	}

	//
	// Receiver may send data, so we must read that here.
	//
	if (inSocket != 0) {
		while (inSocket->hasPendingDatagrams()) {

			QByteArray datagram;
			datagram.resize(inSocket->pendingDatagramSize());

			inSocket->readDatagram( (char * ) &cmd, sizeof(cmd), &sender, &senderPort);

			fg_cmd = cmd;									// Let's just accept that command for now...
			if ( cmd > 0 ) {
				qDebug() << "FTNServer::sendHeadposeToGame hasPendingDatagrams, cmd = " << cmd;
//				headTracker->handleGameCommand ( cmd );		// Send it upstream, for the Tracker to handle
			}
		}
	}
}

//
// Check if the Client DLL exists and load it (to test it), if so.
// Returns 'true' if all seems OK.
//
bool FTNoIR_Protocol_FTN::checkServerInstallationOK( HANDLE handle )
{   
	// Init. the data
	TestData.x = 0.0f;
	TestData.y = 0.0f;
	TestData.z = 0.0f;
	TestData.yaw = 0.0f;
	TestData.pitch = 0.0f;
	TestData.roll = 0.0f;
	fg_cmd = 1;

	inSocket = 0;
	outSocket = 0;

	frame_counter = 0;

	//
	// Create UDP-sockets.
	//
	if (inSocket == 0) {
		qDebug() << "FGServer::sendHeadposeToGame creating insocket";
		inSocket = new QUdpSocket();

		// Connect the inSocket to the port, to receive messages
		if (!inSocket->bind(QHostAddress::Any, destPort+1, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
			QMessageBox::warning(0,"FaceTrackNoIR Error", "Unable to bind UDP-port",QMessageBox::Ok,QMessageBox::NoButton);
			delete inSocket;
			inSocket = 0;
			return false;
		}
	}

	if (outSocket == 0) {
		outSocket = new QUdpSocket();
	}

	return true;
}

//
// Return a name, if present the name from the Game, that is connected...
//
void FTNoIR_Protocol_FTN::getNameFromGame( char *dest )
{   
	sprintf_s(dest, 99, "FaceTrackNoIR");
	return;
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Protocol object.

// Export both decorated and undecorated names.
//   GetProtocol     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetProtocol@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetProtocol=_GetProtocol@0")

FTNOIR_PROTOCOL_BASE_EXPORT IProtocolPtr __stdcall GetProtocol()
{
	return new FTNoIR_Protocol_FTN;
}
