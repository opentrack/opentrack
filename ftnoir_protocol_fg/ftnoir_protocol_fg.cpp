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
/*
	Modifications (last one on top):
	20110401 - WVR: Moved protocol to a DLL, convenient for installation etc.
	20101224 - WVR: Base class is no longer inheriting QThread. sendHeadposeToGame
					is called from run() of Tracker.cpp
*/
#include "ftnoir_protocol_fg.h"
#include <QFile>
#include "facetracknoir/global-settings.h"
#include <ftnoir_tracker_base/ftnoir_tracker_types.h>

// For Todd and Arda Kutlu
//#define SEND_ASCII_DATA
//#define LOG_OUTPUT

/** constructor **/
FTNoIR_Protocol::FTNoIR_Protocol()
{
	blnConnectionActive = false;
	loadSettings();
}

/** destructor **/
FTNoIR_Protocol::~FTNoIR_Protocol()
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

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Protocol::loadSettings() {
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "FG" );

	bool blnLocalPC = iniFile.value ( "LocalPCOnly", 1 ).toBool();
	if (blnLocalPC) {
		destIP = QHostAddress::LocalHost;
	}
	else {
		QString destAddr = iniFile.value ( "IP-1", 192 ).toString() + "." + iniFile.value ( "IP-2", 168 ).toString() + "." + iniFile.value ( "IP-3", 2 ).toString() + "." + iniFile.value ( "IP-4", 1 ).toString();
		destIP = QHostAddress( destAddr );
	}
	destPort = iniFile.value ( "PortNumber", 5550 ).toInt();

	iniFile.endGroup ();

}

//
// Update Headpose in Game.
//
void FTNoIR_Protocol::sendHeadposeToGame( double *headpose, double *rawheadpose ) {
int no_bytes;
QHostAddress sender;
quint16 senderPort;

	//
	// Copy the Raw measurements directly to the client.
	//
    FlightData.x = headpose[TX];
    FlightData.y = headpose[RY];
    FlightData.z = headpose[TZ];
    FlightData.p = headpose[TY];
    FlightData.h = headpose[RX];
    FlightData.r = headpose[RZ];
	FlightData.status = fg_cmd;

	//
	// Try to send an UDP-message to the FlightGear
	//

#ifdef SEND_ASCII_DATA
	sprintf_s(data, "%.2f %.2f %.2f %.2f %.2f %.2f\n\0", FlightData.x, FlightData.y, FlightData.z, FlightData.p, FlightData.h, FlightData.r);

	if (outSocket != 0) {
		no_bytes = outSocket->writeDatagram((const char *) &data, strlen( data ), destIP, destPort);
		if ( no_bytes > 0) {
		qDebug() << "FGServer::writePendingDatagrams says: bytes send =" << data;
		}
		else {
			qDebug() << "FGServer::writePendingDatagrams says: nothing sent!";
		}
	}

#endif

	#ifdef LOG_OUTPUT
	//	Use this for some debug-output to file...
	QFile datafile(QCoreApplication::applicationDirPath() + "\\FG_output.txt");
	if (datafile.open(QFile::WriteOnly | QFile::Append)) {
		QTextStream out(&datafile);
		out << "output:\t" << FlightData.x << "\t" << FlightData.y << "\t" << FlightData.z << "\t" << FlightData.p << "\t" << FlightData.h << "\t" << FlightData.r << '\n';
	}
	#endif

	#ifndef SEND_ASCII_DATA
	//! [1]
//	no_bytes = outSocket->writeDatagram((const char *) &FlightData, sizeof( FlightData ), QHostAddress::LocalHost, 5550);
	if (outSocket != 0) {
        no_bytes = outSocket->writeDatagram((const char *) &FlightData, sizeof( FlightData ), destIP, destPort);
		if ( no_bytes > 0) {
	//		qDebug() << "FGServer::writePendingDatagrams says: bytes send =" << no_bytes << sizeof( double );
		}
		else {
			qDebug() << "FGServer::writePendingDatagrams says: nothing sent!";
		}
	}
	#endif

	//
	// FlightGear keeps sending data, so we must read that here.
	//
	if (inSocket != 0) {
		while (inSocket->hasPendingDatagrams()) {

			QByteArray datagram;
			datagram.resize(inSocket->pendingDatagramSize());

			inSocket->readDatagram( (char * ) &cmd, sizeof(cmd), &sender, &senderPort);

			fg_cmd = cmd;									// Let's just accept that command for now...
			if ( cmd > 0 ) {
				qDebug() << "FGServer::sendHeadposeToGame hasPendingDatagrams, cmd = " << cmd;
//				headTracker->handleGameCommand ( cmd );		// Send it upstream, for the Tracker to handle
			}

			if (!blnConnectionActive) {
				blnConnectionActive = true;
			}
		}
	}
}

//
// Check if the Client DLL exists and load it (to test it), if so.
// Returns 'true' if all seems OK.
//
bool FTNoIR_Protocol::checkServerInstallationOK()
{   
	// Init. the data
	FlightData.x = 0.0f;
	FlightData.y = 0.0f;
	FlightData.z = 0.0f;
	FlightData.h = 0.0f;
	FlightData.p = 0.0f;
	FlightData.r = 0.0f;
	FlightData.status = 0;
	fg_cmd = 1;

	inSocket = 0;
	outSocket = 0;

	//
	// Create UDP-sockets.
	//
	if (inSocket == 0) {
		qDebug() << "FGServer::sendHeadposeToGame creating insocket";
		inSocket = new QUdpSocket();

		// Connect the inSocket to the port, to receive messages
		if (!inSocket->bind(QHostAddress::Any, destPort+1)) {
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

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Protocol object.

// Export both decorated and undecorated names.
//   GetProtocol     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetProtocol@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetProtocol=_GetProtocol@0")

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocol* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Protocol;
}
