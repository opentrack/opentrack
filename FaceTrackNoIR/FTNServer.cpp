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
* FTNServer			FTNServer is the Class, that communicates headpose-data		*
*					to another FaceTrackNoIR program, using UDP.       			*
*					It is based on the (Linux) example made by Melchior FRANZ.	*
********************************************************************************/
/*
	Modifications (last one on top):
	20101224 - WVR: Base class is no longer inheriting QThread. sendHeadposeToGame
					is called from run() of Tracker.cpp
*/
#include <QtGui>
#include <QtNetwork>
#include "FTNServer.h"
#include "Tracker.h"
#include <Winsock.h>

/** constructor **/
FTNServer::FTNServer( Tracker *parent ) {

	// Save the parent
	headTracker = parent;
	loadSettings();
}

/** destructor **/
FTNServer::~FTNServer() {
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
// Update Headpose in Game.
//
void FTNServer::sendHeadposeToGame() {
int no_bytes;
QHostAddress sender;
quint16 senderPort;

	//
	// Create UDP-sockets if they don't exist already.
	// They must be created here, because they must be in the Tracker thread (Tracker::run())
	//
	if (inSocket == 0) {
		qDebug() << "FTNServer::sendHeadposeToGame creating sockets";
		inSocket = new QUdpSocket();
		// Connect the inSocket to the port, to receive messages
		if (!inSocket->bind(QHostAddress::Any, destPort+1)) {
			qDebug() << "FTNServer::writePendingDatagrams says: unable to bind inSocket!";
			delete inSocket;
			inSocket = 0;
		}
	}

	if (outSocket == 0) {
		outSocket = new QUdpSocket();
	}

	//
	// Copy the Raw measurements directly to the client.
	//
	TestData.x = virtPosX;
	TestData.y = virtPosY;
	TestData.z = virtPosZ;
	TestData.pitch = virtRotX;
	TestData.yaw = virtRotY;
	TestData.roll = virtRotZ;

	//
	// Try to send an UDP-message to the receiver
	//

	//! [1]
	no_bytes = outSocket->writeDatagram((const char *) &TestData, sizeof( TestData ), destIP, destPort);
	if ( no_bytes > 0) {
//		qDebug() << "FTNServer::writePendingDatagrams says: bytes send =" << no_bytes << sizeof( double );
	}
	else {
		qDebug() << "FTNServer::writePendingDatagrams says: nothing sent!";
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
				headTracker->handleGameCommand ( cmd );		// Send it upstream, for the Tracker to handle
			}
		}
	}
}

//
// Check if the Client DLL exists and load it (to test it), if so.
// Returns 'true' if all seems OK.
//
bool FTNServer::checkServerInstallationOK( HANDLE handle )
{   
	// Init. the data
	TestData.x = 0.0f;
	TestData.y = 0.0f;
	TestData.z = 0.0f;
	TestData.yaw = 0.0f;
	TestData.pitch = 0.0f;
	TestData.roll = 0.0f;
//	TestData.status = 0;
	fg_cmd = 1;

	inSocket = 0;
	outSocket = 0;

	return true;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNServer::loadSettings() {

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
// Constructor for server-settings-dialog
//
FTNServerControls::FTNServerControls( QWidget *parent, Qt::WindowFlags f ) :
QWidget( parent , f)
{
	ui.setupUi( this );

	QPoint offsetpos(100, 100);
	this->move(parent->pos() + offsetpos);

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
	connect(ui.spinIPFirstNibble, SIGNAL(valueChanged(int)), this, SLOT(settingChanged()));
	connect(ui.spinIPSecondNibble, SIGNAL(valueChanged(int)), this, SLOT(settingChanged()));
	connect(ui.spinIPThirdNibble, SIGNAL(valueChanged(int)), this, SLOT(settingChanged()));
	connect(ui.spinIPFourthNibble, SIGNAL(valueChanged(int)), this, SLOT(settingChanged()));
	connect(ui.spinPortNumber, SIGNAL(valueChanged(int)), this, SLOT(settingChanged()));

	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
FTNServerControls::~FTNServerControls() {
	qDebug() << "~FTNServerControls() says: started";
}

//
// OK clicked on server-dialog
//
void FTNServerControls::doOK() {
	save();
	this->close();
}

// override show event
void FTNServerControls::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void FTNServerControls::doCancel() {
	//
	// Ask if changed Settings should be saved
	//
	if (settingsDirty) {
		int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Discard );

		qDebug() << "doCancel says: answer =" << ret;

		switch (ret) {
			case QMessageBox::Save:
				save();
				this->close();
				break;
			case QMessageBox::Discard:
				this->close();
				break;
			case QMessageBox::Cancel:
				// Cancel was clicked
				break;
			default:
				// should never be reached
			break;
		}
	}
	else {
		this->close();
	}
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNServerControls::loadSettings() {

//	qDebug() << "loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

//	qDebug() << "loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "FTN" );
	ui.spinIPFirstNibble->setValue( iniFile.value ( "IP-1", 192 ).toInt() );
	ui.spinIPSecondNibble->setValue( iniFile.value ( "IP-2", 168 ).toInt() );
	ui.spinIPThirdNibble->setValue( iniFile.value ( "IP-3", 2 ).toInt() );
	ui.spinIPFourthNibble->setValue( iniFile.value ( "IP-4", 1 ).toInt() );

	ui.spinPortNumber->setValue( iniFile.value ( "PortNumber", 5550 ).toInt() );
	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void FTNServerControls::save() {

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "FTN" );
	iniFile.setValue ( "IP-1", ui.spinIPFirstNibble->value() );
	iniFile.setValue ( "IP-2", ui.spinIPSecondNibble->value() );
	iniFile.setValue ( "IP-3", ui.spinIPThirdNibble->value() );
	iniFile.setValue ( "IP-4", ui.spinIPFourthNibble->value() );
	iniFile.setValue ( "PortNumber", ui.spinPortNumber->value() );
	iniFile.endGroup ();

	settingsDirty = false;
}

//END
