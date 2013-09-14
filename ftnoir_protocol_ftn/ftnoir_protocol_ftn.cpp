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
#include "ftnoir_protocol_ftn.h"
#include <QFile>
#include "facetracknoir/global-settings.h"

/** constructor **/
FTNoIR_Protocol::FTNoIR_Protocol()
{
	loadSettings();
    outSocket = 0;
}

/** destructor **/
FTNoIR_Protocol::~FTNoIR_Protocol()
{
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

	iniFile.beginGroup ( "FTN" );

	QString destAddr = iniFile.value ( "IP-1", 192 ).toString() + "." + iniFile.value ( "IP-2", 168 ).toString() + "." + iniFile.value ( "IP-3", 2 ).toString() + "." + iniFile.value ( "IP-4", 1 ).toString();
	destIP = QHostAddress( destAddr );
	destPort = iniFile.value ( "PortNumber", 5550 ).toInt();

	iniFile.endGroup ();
}

//
// Update Headpose in Game.
//
void FTNoIR_Protocol::sendHeadposeToGame(double *headpose, double *rawheadpose ) {
    int no_bytes;
    double test_data[6];
	//
	// Copy the Raw measurements directly to the client.
	//
    for (int i = 0; i < 6; i++)
        test_data[i] = headpose[i];
	//
	// Try to send an UDP-message to the receiver
	//

	//! [1]
	if (outSocket != 0) {
        no_bytes = outSocket->writeDatagram((const char *) test_data, sizeof( test_data ), destIP, destPort);
		if ( no_bytes > 0) {
//		qDebug() << "FTNServer::writePendingDatagrams says: bytes send =" << no_bytes << sizeof( double );
		}
		else {
			qDebug() << "FTNServer::writePendingDatagrams says: nothing sent!";
		}
	}
}

//
// Check if the Client DLL exists and load it (to test it), if so.
// Returns 'true' if all seems OK.
//
bool FTNoIR_Protocol::checkServerInstallationOK()
{   
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
