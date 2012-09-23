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

FTNoIR_Tracker::FTNoIR_Tracker()
{
	inSocket = 0;
	outSocket = 0;

	// Create events
	m_StopThread = CreateEvent(0, TRUE, FALSE, 0);
	m_WaitThread = CreateEvent(0, TRUE, FALSE, 0);

	////allocate memory for the parameters
	//parameterValueAsFloat.clear();
	//parameterRange.clear();

	//// Add the parameters to the list
	//parameterRange.append(std::pair<float,float>(1000.0f,9999.0f));
	//parameterValueAsFloat.append(0.0f);
	//setParameterValue(kPortAddress,5551.0f);

	portAddress = 5551;

	newHeadPose.x = 0.0f;
	newHeadPose.y = 0.0f;
	newHeadPose.z = 0.0f;
	newHeadPose.yaw   = 0.0f;
	newHeadPose.pitch = 0.0f;
	newHeadPose.roll  = 0.0f;
}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
	// Trigger thread to stop
	::SetEvent(m_StopThread);

	// Wait until thread finished
	if (isRunning()) {
		::WaitForSingleObject(m_WaitThread, INFINITE);
	}

	// Close handles
	::CloseHandle(m_StopThread);
	::CloseHandle(m_WaitThread);

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

int no_bytes;
QHostAddress sender;
quint16 senderPort;

	//
	// Read the data that was received.
	//
	forever {

	    // Check event for stop thread
		if(::WaitForSingleObject(m_StopThread, 0) == WAIT_OBJECT_0)
		{
			// Set event
			::SetEvent(m_WaitThread);
			qDebug() << "FTNoIR_Tracker::run() terminated run()";
			return;
		}

		if (inSocket != 0) {
			while (inSocket->hasPendingDatagrams()) {

				QByteArray datagram;
				datagram.resize(inSocket->pendingDatagramSize());

				inSocket->readDatagram( (char * ) &newHeadPose, sizeof(newHeadPose), &sender, &senderPort);
			}
		}
		else {
			qDebug() << "FTNoIR_Tracker::run() insocket not ready: exit run()";
			return;
		}

		//for lower cpu load 
		usleep(10000);
//		yieldCurrentThread(); 
	}
}

void FTNoIR_Tracker::Initialize( QFrame *videoframe )
{
	qDebug() << "FTNoIR_Tracker::Initialize says: Starting ";
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

	return;
}

void FTNoIR_Tracker::StartTracker( HWND parent_window )
{
	start( QThread::TimeCriticalPriority );
	return;
}

void FTNoIR_Tracker::StopTracker( bool exit )
{
	//
	// OK, the thread is not stopped, doing this. That might be dangerous anyway...
	//
	if (exit || !exit) return;
	return;
}

bool FTNoIR_Tracker::GiveHeadPoseData(THeadPoseData *data)
{
	data->x = newHeadPose.x;
	data->y = newHeadPose.y;
	data->z = newHeadPose.z;
	data->yaw = newHeadPose.yaw;
	data->pitch = newHeadPose.pitch;
	data->roll = newHeadPose.roll;
	return true;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Tracker::loadSettings() {

	qDebug() << "FTNoIR_Tracker::loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Tracker::loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "FTNClient" );
	portAddress = (float) iniFile.value ( "PortNumber", 5550 ).toInt();
	iniFile.endGroup ();
}


////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker object.

// Export both decorated and undecorated names.
//   GetTracker     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetTracker@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetTracker=_GetTracker@0")

FTNOIR_TRACKER_BASE_EXPORT ITrackerPtr __stdcall GetTracker()
{
	return new FTNoIR_Tracker;
}
