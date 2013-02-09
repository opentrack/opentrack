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
#include "ftnoir_protocol_ft.h"
#include <QDebug>
#include <QFileDialog>

//*******************************************************************************************************
// FaceTrackNoIR Client Settings-dialog.
//*******************************************************************************************************

//
// Constructor for server-settings-dialog
//
FTControls::FTControls() :
QWidget()
{
	QString aFileName;														// File Path and Name

	ui.setupUi( this );

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
	connect(ui.chkTIRViews, SIGNAL(stateChanged(int)), this, SLOT(chkTIRViewsChanged()));
	connect(ui.chkStartDummy, SIGNAL(stateChanged(int)), this, SLOT(settingChanged()));
	connect(ui.cbxSelectInterface, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged( int )));
	connect(ui.bntLocateNPClient, SIGNAL(clicked()), this, SLOT(selectDLL()));

	ui.cbxSelectInterface->addItem("Enable both");
	ui.cbxSelectInterface->addItem("Use FreeTrack, hide TrackIR");
	ui.cbxSelectInterface->addItem("Use TrackIR, hide FreeTrack");

	aFileName = QCoreApplication::applicationDirPath() + "/TIRViews.dll";
	if ( !QFile::exists( aFileName ) ) {
		ui.chkTIRViews->setChecked( false );
		ui.chkTIRViews->setEnabled ( false );
		save();
	}
	else {
		ui.chkTIRViews->setEnabled ( true );
	}

	theProtocol = NULL;

	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
FTControls::~FTControls() {
	qDebug() << "~FTControls() says: started";
}

//
// Initialize tracker-client-dialog
//
void FTControls::Initialize(QWidget *parent) {

	QPoint offsetpos(100, 100);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}
	show();
}

//
// OK clicked on server-dialog
//
void FTControls::doOK() {
	save();
	this->close();
}

// override show event
void FTControls::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void FTControls::doCancel() {
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
void FTControls::loadSettings() {
	qDebug() << "loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "FT" );
	ui.cbxSelectInterface->setCurrentIndex( iniFile.value ( "UsedInterface", 0 ).toInt() );
	iniFile.endGroup ();

	iniFile.beginGroup ( "FTIR" );
	ui.chkTIRViews->setChecked (iniFile.value ( "useTIRViews", 0 ).toBool());
	ui.chkStartDummy->setChecked (iniFile.value ( "useDummyExe", 1 ).toBool());
	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void FTControls::save() {
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "FT" );
	iniFile.setValue ( "UsedInterface", ui.cbxSelectInterface->currentIndex());
	iniFile.endGroup ();

	iniFile.beginGroup ( "FTIR" );
	iniFile.setValue ( "useTIRViews", ui.chkTIRViews->isChecked() );
	iniFile.setValue ( "useDummyExe", ui.chkStartDummy->isChecked() );
	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Select a NPClient.dll file, to repair the Location in the Registry.
// Several program distribute their own version of this file.
//
void FTControls::selectDLL() {
	QFileDialog::Options options;
	QFileDialog::FileMode mode;

    options |= QFileDialog::DontUseNativeDialog;
	mode = QFileDialog::ExistingFile;
    QString selectedFilter;
	QString fileName = QFileDialog::getOpenFileName( this, tr("Select the desired NPClient DLL"), QCoreApplication::applicationDirPath() + "/NPClient.dll", tr("Dll file (*.dll);;All Files (*)"));

	//
	// Write the location of the file in the required Registry-key.
	//
	if (! fileName.isEmpty() ) {
		if (fileName.endsWith("NPClient.dll", Qt::CaseInsensitive) ) {
			QSettings settingsTIR("NaturalPoint", "NATURALPOINT\\NPClient Location");			// Registry settings (in HK_USER)
			QString aLocation = fileName.left(fileName.length() - 12);							// Location of Client DLL

			settingsTIR.setValue( "Path" , aLocation );
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Protocol-settings dialog object.

// Export both decorated and undecorated names.
//   GetProtocolDialog     - Undecorated name, which can be easily used with GetProcAddress
//                          Win32 API function.
//   _GetProtocolDialog@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetProtocolDialog=_GetProtocolDialog@0")

FTNOIR_PROTOCOL_BASE_EXPORT IProtocolDialogPtr __stdcall GetProtocolDialog( )
{
	return new FTControls;
}
