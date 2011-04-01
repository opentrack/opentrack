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

/** constructor **/
FTNoIR_Protocol_FG::FTNoIR_Protocol_FG()
{
	loadSettings();
}

/** destructor **/
FTNoIR_Protocol_FG::~FTNoIR_Protocol_FG()
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
void FTNoIR_Protocol_FG::Release()
{
    delete this;
}

void FTNoIR_Protocol_FG::Initialize()
{
	return;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Protocol_FG::loadSettings() {
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

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
void FTNoIR_Protocol_FG::sendHeadposeToGame( T6DOF *headpose ) {
int no_bytes;
QHostAddress sender;
quint16 senderPort;

	//
	// Copy the Raw measurements directly to the client.
	//
	FlightData.x = headpose->position.x / 100.0f;
	FlightData.y = headpose->position.y / 100.0f;
	FlightData.z = headpose->position.z / 100.0f;
	FlightData.p = headpose->position.pitch;
	FlightData.h = headpose->position.yaw;
	FlightData.r = headpose->position.roll;
	FlightData.status = fg_cmd;

	//
	// Try to send an UDP-message to the FlightGear
	//

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
		}
	}
}

//
// Check if the Client DLL exists and load it (to test it), if so.
// Returns 'true' if all seems OK.
//
bool FTNoIR_Protocol_FG::checkServerInstallationOK( HANDLE handle )
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
#pragma comment(linker, "/export:GetProtocol=_GetProtocol@0")

FTNOIR_PROTOCOL_BASE_EXPORT PROTOCOLHANDLE __stdcall GetProtocol()
{
	return new FTNoIR_Protocol_FG;
}

//*******************************************************************************************************
// FaceTrackNoIR Client Settings-dialog.
//*******************************************************************************************************

//
// Constructor for server-settings-dialog
//
FGControls::FGControls( QWidget *parent, Qt::WindowFlags f ) :
QWidget( parent , f)
{
	ui.setupUi( this );

	QPoint offsetpos(100, 100);
	this->move(parent->pos() + offsetpos);

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
	connect(ui.chkLocalPC, SIGNAL(stateChanged(int)), this, SLOT(chkLocalPCOnlyChanged()));
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
FGControls::~FGControls() {
	qDebug() << "~FGControls() says: started";
}

void FGControls::Release()
{
    delete this;
}

//
// Initialize tracker-client-dialog
//
void FGControls::Initialize(QWidget *parent) {

	QPoint offsetpos(100, 100);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}
	show();
}

//
// OK clicked on server-dialog
//
void FGControls::doOK() {
	save();
	this->close();
}

// override show event
void FGControls::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void FGControls::doCancel() {
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
void FGControls::loadSettings() {
//	qDebug() << "loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

//	qDebug() << "loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "FG" );
	ui.chkLocalPC->setChecked (iniFile.value ( "LocalPCOnly", 1 ).toBool());

	ui.spinIPFirstNibble->setValue( iniFile.value ( "IP-1", 192 ).toInt() );
	ui.spinIPSecondNibble->setValue( iniFile.value ( "IP-2", 168 ).toInt() );
	ui.spinIPThirdNibble->setValue( iniFile.value ( "IP-3", 2 ).toInt() );
	ui.spinIPFourthNibble->setValue( iniFile.value ( "IP-4", 1 ).toInt() );

	ui.spinPortNumber->setValue( iniFile.value ( "PortNumber", 5550 ).toInt() );
	iniFile.endGroup ();

	chkLocalPCOnlyChanged();	
	settingsDirty = false;
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void FGControls::save() {
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "FG" );
	iniFile.setValue ( "LocalPCOnly", ui.chkLocalPC->isChecked() );
	iniFile.setValue ( "IP-1", ui.spinIPFirstNibble->value() );
	iniFile.setValue ( "IP-2", ui.spinIPSecondNibble->value() );
	iniFile.setValue ( "IP-3", ui.spinIPThirdNibble->value() );
	iniFile.setValue ( "IP-4", ui.spinIPFourthNibble->value() );
	iniFile.setValue ( "PortNumber", ui.spinPortNumber->value() );
	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Handle change of the checkbox.
//
void FGControls::chkLocalPCOnlyChanged() {

	if ( ui.chkLocalPC->isChecked() ) {
		ui.spinIPFirstNibble->setValue( 127 );
		ui.spinIPFirstNibble->setEnabled ( false );
		ui.spinIPSecondNibble->setValue( 0 );
		ui.spinIPSecondNibble->setEnabled ( false );
		ui.spinIPThirdNibble->setValue( 0 );
		ui.spinIPThirdNibble->setEnabled ( false );
		ui.spinIPFourthNibble->setValue( 1 );
		ui.spinIPFourthNibble->setEnabled ( false );
	}
	else {
		ui.spinIPFirstNibble->setEnabled ( true );
		ui.spinIPSecondNibble->setEnabled ( true );
		ui.spinIPThirdNibble->setEnabled ( true );
		ui.spinIPFourthNibble->setEnabled ( true );
	}

	settingsDirty = true;
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Protocol-settings dialog object.

// Export both decorated and undecorated names.
//   GetProtocolDialog     - Undecorated name, which can be easily used with GetProcAddress
//                          Win32 API function.
//   _GetProtocolDialog@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetProtocolDialog=_GetProtocolDialog@0")

FTNOIR_PROTOCOL_BASE_EXPORT PROTOCOLDIALOGHANDLE __stdcall GetProtocolDialog( )
{
	return new FGControls;
}
