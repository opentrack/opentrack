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
#include <QFile>
#include "facetracknoir/global-settings.h"
#include <ftnoir_tracker_base/ftnoir_tracker_types.h>

// For Todd and Arda Kutlu

FTNoIR_Protocol::FTNoIR_Protocol()
{
    loadSettings();
}

FTNoIR_Protocol::~FTNoIR_Protocol()
{
}

void FTNoIR_Protocol::loadSettings() {
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
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
    destPort = iniFile.value ( "PortNumber", 5542 ).toInt();

	iniFile.endGroup ();

}

void FTNoIR_Protocol::sendHeadposeToGame( double *headpose, double *rawheadpose ) {
    int no_bytes;
    QHostAddress sender;
    quint16 senderPort;

    FlightData.x = headpose[TX] * 1e-2;
    FlightData.y = headpose[TY] * 1e-2;
    FlightData.z = headpose[TZ] * 1e-2;
    FlightData.p = headpose[Pitch];
    FlightData.h = headpose[Yaw];
    FlightData.r = headpose[Roll];
    FlightData.status = 1;
    (void) outSocket.writeDatagram(reinterpret_cast<const char*>(&FlightData), sizeof(FlightData), destIP, static_cast<quint16>(destPort));
}

//
// Check if the Client DLL exists and load it (to test it), if so.
// Returns 'true' if all seems OK.
//
bool FTNoIR_Protocol::checkServerInstallationOK()
{   
    return outSocket.bind(QHostAddress::Any, 0, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocol* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Protocol;
}
