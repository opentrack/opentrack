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
#include "facetracknoir/global-settings.h"

FTNoIR_Tracker::FTNoIR_Tracker()
{
	inSocket = 0;
	outSocket = 0;

	bEnableRoll = true;
	bEnablePitch = true;
	bEnableYaw = true;
	bEnableX = true;
	bEnableY = true;
	bEnableZ = true;
	portAddress = 5551;
    should_quit = false;

    for (int i = 0; i < 6; i++)
        newHeadPose[i] = 0;
}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
    should_quit = true;
    wait();
	if (inSocket) {
		inSocket->close();
		delete inSocket;
	}

	if (outSocket) {
		outSocket->close();
		delete outSocket;
	}
}

/** QThread run @override **/
void FTNoIR_Tracker::run() {

QHostAddress sender;
quint16 senderPort;

	//
	// Read the data that was received.
	//
	forever {
        if (should_quit)
            break;
		if (inSocket != 0) {
			while (inSocket->hasPendingDatagrams()) {

				QByteArray datagram;
				datagram.resize(inSocket->pendingDatagramSize());
                mutex.lock();
				inSocket->readDatagram( (char * ) &newHeadPose, sizeof(newHeadPose), &sender, &senderPort);
                mutex.unlock();
			}
		}
		else {
            break;
		}

		usleep(10000);
	}
}

void FTNoIR_Tracker::StartTracker(QFrame*)
{
    loadSettings();
    //
    // Create UDP-sockets if they don't exist already.
    // They must be created here, because they must be in the new thread (FTNoIR_Tracker::run())
    //
    if (inSocket == 0) {
        qDebug() << "FTNoIR_Tracker::Initialize() creating insocket";
        inSocket = new QUdpSocket();
        // Connect the inSocket to the port, to receive messages

        if (!inSocket->bind(QHostAddress::Any, (int) portAddress, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
            QMessageBox::warning(0,"FaceTrackNoIR Error", "Unable to bind UDP-port",QMessageBox::Ok,QMessageBox::NoButton);
            delete inSocket;
            inSocket = 0;
        }
    }
	start();
	return;
}

bool FTNoIR_Tracker::GiveHeadPoseData(double *data)
{
    mutex.lock();
	if (bEnableX) {
        data[TX] = newHeadPose[TX];
	}
	if (bEnableX) {
        data[TY] = newHeadPose[TY];
	}
	if (bEnableX) {
        data[TZ] = newHeadPose[TZ];
	}
    if (bEnableYaw) {
        data[Yaw] = newHeadPose[Yaw];
	}
    if (bEnablePitch) {
        data[Pitch] = newHeadPose[Pitch];
	}
    if (bEnableRoll) {
        data[Roll] = newHeadPose[Roll];
	}
    mutex.unlock();
	return true;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Tracker::loadSettings() {

	qDebug() << "FTNoIR_Tracker::loadSettings says: Starting ";
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Tracker::loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "FTNClient" );
	bEnableRoll = iniFile.value ( "EnableRoll", 1 ).toBool();
	bEnablePitch = iniFile.value ( "EnablePitch", 1 ).toBool();
	bEnableYaw = iniFile.value ( "EnableYaw", 1 ).toBool();
	bEnableX = iniFile.value ( "EnableX", 1 ).toBool();
	bEnableY = iniFile.value ( "EnableY", 1 ).toBool();
	bEnableZ = iniFile.value ( "EnableZ", 1 ).toBool();
	portAddress = (float) iniFile.value ( "PortNumber", 5550 ).toInt();
	iniFile.endGroup ();
}


////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker object.

// Export both decorated and undecorated names.
//   GetTracker     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetTracker@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetTracker=_GetTracker@0")

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Tracker;
}
