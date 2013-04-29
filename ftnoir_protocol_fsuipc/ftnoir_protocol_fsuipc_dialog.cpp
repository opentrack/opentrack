/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
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
********************************************************************************/
/*
	Modifications (last one on top):
		20120830 - WVR: The Dialog class was used to get general info on the DLL. This
						had a big disadvantage: the complete dialog was loaded, just to get
						some data and then it was deleted again (without ever showing the dialog).
						The ProtocolDll class solves this.
						The functions to get the name(s) and icon were removed from the two other classes.
*/
#include "ftnoir_protocol_fsuipc.h"
#include "facetracknoir/global-settings.h"

//*******************************************************************************************************
// FaceTrackNoIR Client Settings-dialog.
//*******************************************************************************************************

//
// Constructor for server-settings-dialog
//
FSUIPCControls::FSUIPCControls() :
QWidget()
{
	ui.setupUi( this );

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
	connect(ui.btnFindDLL, SIGNAL(clicked()), this, SLOT(getLocationOfDLL()));

	theProtocol = NULL;

	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
FSUIPCControls::~FSUIPCControls() {
	qDebug() << "~FSUIPCControls() says: started";
}

void FSUIPCControls::Release()
{
    delete this;
}

//
// Initialize tracker-client-dialog
//
void FSUIPCControls::Initialize(QWidget *parent) {

	QPoint offsetpos(100, 100);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}
	show();
}

//
// OK clicked on server-dialog
//
void FSUIPCControls::doOK() {
	save();
	this->close();
}

// override show event
void FSUIPCControls::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void FSUIPCControls::doCancel() {
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
void FSUIPCControls::loadSettings() {

	qDebug() << "loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "FSUIPC" );
	ui.txtLocationOfDLL->setText(iniFile.value ( "LocationOfDLL", FSUIPC_FILENAME ).toString() );
	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void FSUIPCControls::save() {

	qDebug() << "save() says: started";

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "FSUIPC" );
	iniFile.setValue ( "LocationOfDLL", ui.txtLocationOfDLL->text() );
	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Show the Dialog to set the DLL's location
//
void FSUIPCControls::getLocationOfDLL()
{
	//
	// Get the new filename of the INI-file.
	//
	QString fileName = QFileDialog::getOpenFileName(this, tr("Locate file"),
													ui.txtLocationOfDLL->text(),
													tr("FSUIPC DLL file (FSUIPC*.dll);;All Files (*)"));
	if (!fileName.isEmpty()) {
		ui.txtLocationOfDLL->setText( fileName );
		settingsDirty = true;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Protocol-settings dialog object.

// Export both decorated and undecorated names.
//   GetProtocolDialog     - Undecorated name, which can be easily used with GetProcAddress
//                          Win32 API function.
//   _GetProtocolDialog@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetProtocolDialog=_GetProtocolDialog@0")

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocolDialog* CALLING_CONVENTION GetDialog(void)
{
    return new FSUIPCControls;
}
