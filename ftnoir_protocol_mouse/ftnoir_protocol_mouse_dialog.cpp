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
#include "ftnoir_protocol_mouse.h"
#include <QDebug>
#include "facetracknoir/global-settings.h"

//*******************************************************************************************************
// FaceTrackNoIR Client Settings-dialog.
//*******************************************************************************************************

//
// Constructor for server-settings-dialog
//
MOUSEControls::MOUSEControls() :
QWidget()
{
	ui.setupUi( this );

	//
	// Setup the choices
	//
	ui.cbxSelectMouseStyle->addItem("Absolute");
	ui.cbxSelectMouseStyle->addItem("Relative");

	ui.cbxSelectMouse_X->addItem("None");
	ui.cbxSelectMouse_X->addItem("Pitch");
	ui.cbxSelectMouse_X->addItem("Yaw");
	ui.cbxSelectMouse_X->addItem("Roll");
	ui.cbxSelectMouse_X->addItem("X");
	ui.cbxSelectMouse_X->addItem("Y");
	ui.cbxSelectMouse_X->addItem("Z");

	ui.cbxSelectMouse_Y->addItem("None");
	ui.cbxSelectMouse_Y->addItem("Pitch");
	ui.cbxSelectMouse_Y->addItem("Yaw");
	ui.cbxSelectMouse_Y->addItem("Roll");
	ui.cbxSelectMouse_Y->addItem("X");
	ui.cbxSelectMouse_Y->addItem("Y");
	ui.cbxSelectMouse_Y->addItem("Z");

	ui.cbxSelectMouse_Wheel->addItem("None");
	ui.cbxSelectMouse_Wheel->addItem("Pitch");
	ui.cbxSelectMouse_Wheel->addItem("Yaw");
	ui.cbxSelectMouse_Wheel->addItem("Roll");
	ui.cbxSelectMouse_Wheel->addItem("X");
	ui.cbxSelectMouse_Wheel->addItem("Y");
	ui.cbxSelectMouse_Wheel->addItem("Z");

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
	connect(ui.cbxSelectMouse_X, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged( int )));
	connect(ui.cbxSelectMouse_Y, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged( int )));
	connect(ui.cbxSelectMouse_Wheel, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged( int )));

	connect(ui.spinSensX, SIGNAL(valueChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.spinSensY, SIGNAL(valueChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.spinSensWheel, SIGNAL(valueChanged(int)), this, SLOT(settingChanged(int)));

	connect(ui.chkUseVirtualDesk, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));

	theProtocol = NULL;

	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
MOUSEControls::~MOUSEControls() {
	qDebug() << "~MOUSEControls() says: started";
}

//
// Initialize tracker-client-dialog
//
void MOUSEControls::Initialize(QWidget *parent) {

	QPoint offsetpos(100, 100);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}
	show();
}

//
// OK clicked on server-dialog
//
void MOUSEControls::doOK() {
	save();
	this->close();
}

// override show event
void MOUSEControls::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void MOUSEControls::doCancel() {
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
void MOUSEControls::loadSettings() {
	qDebug() << "loadSettings says: Starting ";
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "Mouse" );
	ui.cbxSelectMouseStyle->setCurrentIndex(iniFile.value ( "Style", 1 ).toInt() - 1);
	ui.cbxSelectMouse_X->setCurrentIndex(iniFile.value ( "Mouse_X", 1 ).toInt() - 1);
	ui.cbxSelectMouse_Y->setCurrentIndex(iniFile.value ( "Mouse_Y", 1 ).toInt() - 1);
	ui.cbxSelectMouse_Wheel->setCurrentIndex(iniFile.value ( "Mouse_Wheel", 1 ).toInt() - 1);

	ui.slideSensX->setValue(iniFile.value("SensX", 10).toInt());
	ui.slideSensY->setValue(iniFile.value("SensY", 10).toInt());
	ui.slideSensWheel->setValue(iniFile.value("SensWheel", 10).toInt());

	ui.chkUseVirtualDesk->setChecked( iniFile.value ( "useVirtualDesk", 0 ).toBool() );

	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void MOUSEControls::save() {
	qDebug() << "save() says: started";

	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "Mouse" );
	iniFile.setValue ( "Style", ui.cbxSelectMouseStyle->currentIndex() + 1 );
	iniFile.setValue ( "Mouse_X", ui.cbxSelectMouse_X->currentIndex() + 1 );
	iniFile.setValue ( "Mouse_Y", ui.cbxSelectMouse_Y->currentIndex() + 1 );
	iniFile.setValue ( "Mouse_Wheel", ui.cbxSelectMouse_Wheel->currentIndex() + 1 );

	iniFile.setValue ( "SensX", ui.slideSensX->value() );
	iniFile.setValue ( "SensY", ui.slideSensY->value() );
	iniFile.setValue ( "SensWheel", ui.slideSensWheel->value() );

	iniFile.setValue( "useVirtualDesk", ui.chkUseVirtualDesk->isChecked() );
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
    return new MOUSEControls;
}
