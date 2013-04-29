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
#include "ftnoir_protocol_ftn.h"
#include <QDebug>
#include "facetracknoir/global-settings.h"

//*******************************************************************************************************
// FaceTrackNoIR Client Settings-dialog.
//*******************************************************************************************************

//
// Constructor for server-settings-dialog
//
FTNControls::FTNControls() :
QWidget()
{
	ui.setupUi( this );

	QPoint offsetpos(100, 100);
	//if (parent) {
	//	this->move(parent->pos() + offsetpos);
	//}

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
FTNControls::~FTNControls() {
	qDebug() << "~FTNControls() says: started";
}

void FTNControls::Release()
{
    delete this;
}

//
// Initialize tracker-client-dialog
//
void FTNControls::Initialize(QWidget *parent) {

	QPoint offsetpos(100, 100);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}
	show();
}

//
// OK clicked on server-dialog
//
void FTNControls::doOK() {
	save();
	this->close();
}

// override show event
void FTNControls::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void FTNControls::doCancel() {
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
void FTNControls::loadSettings() {
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
void FTNControls::save() {
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

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Protocol-settings dialog object.

// Export both decorated and undecorated names.
//   GetProtocolDialog     - Undecorated name, which can be easily used with GetProcAddress
//                          Win32 API function.
//   _GetProtocolDialog@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetProtocolDialog=_GetProtocolDialog@0")

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocolDialog* CALLING_CONVENTION GetDialog( )
{
    return new FTNControls;
}
