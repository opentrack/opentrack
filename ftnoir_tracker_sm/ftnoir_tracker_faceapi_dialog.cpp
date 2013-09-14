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
#include "ftnoir_tracker_sm.h"
#include <QtGui>
#include "facetracknoir/global-settings.h"

//*******************************************************************************************************
// faceAPI Client Settings-dialog.
//*******************************************************************************************************

//
// Constructor for server-settings-dialog
//
TrackerControls::TrackerControls() :
    QWidget(),
    shm(SM_MM_DATA, SM_MUTEX, sizeof(TFaceData))
{
    pMemData = (SMMemMap*) shm.mem;
    
	ui.setupUi( this );

	theTracker = NULL;
	prev_state = -1;

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
	connect(ui.btnEngineStart, SIGNAL(clicked()), this, SLOT(doStartEngine()));
	connect(ui.btnEngineStop, SIGNAL(clicked()), this, SLOT(doStopEngine()));
	connect(ui.btnSave, SIGNAL(clicked()), this, SLOT(save()));

	ui.cbxFilterSetting->addItem("None");
	ui.cbxFilterSetting->addItem("Normal");
	ui.cbxFilterSetting->addItem("High");
	connect(ui.cbxFilterSetting, SIGNAL(currentIndexChanged(int)), this, SLOT(doSetFilter( int )));
	connect(ui.btnCameraSettings, SIGNAL(clicked()), this, SLOT(doShowCam()));

	//Setup the timer for showing the headpose.
	timUpdateSettings = new QTimer(this);
    connect(timUpdateSettings, SIGNAL(timeout()), this, SLOT(doTimUpdate()));
	timUpdateSettings->start(100);
	connect(this, SIGNAL(stateChanged( int )), this, SLOT(showSettings( int )));

	connect(ui.chkEnableRoll, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.chkEnablePitch, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.chkEnableYaw, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.chkEnableX, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.chkEnableY, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.chkEnableZ, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
}

//
// Destructor for server-dialog
//
TrackerControls::~TrackerControls() {
	qDebug() << "~TrackerControls() says: started";
}

//
// Initialize tracker-client-dialog
//
void TrackerControls::Initialize(QWidget *parent) {

	QPoint offsetpos(200, 200);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}

	// Load the settings from the current .INI-file
	loadSettings();

	show();
}

//
// OK clicked on server-dialog
//
void TrackerControls::doOK() {
	save();
	this->close();
}

// override show event
void TrackerControls::showEvent ( QShowEvent * event ) {
	prev_state = -1;
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void TrackerControls::doCancel() {
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
void TrackerControls::loadSettings() {

//	qDebug() << "loadSettings says: Starting ";
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

//	qDebug() << "loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "SMTracker" );
	ui.cbxFilterSetting->setCurrentIndex(iniFile.value ( "FilterLevel", 1 ).toInt());

	ui.chkEnableRoll->setChecked(iniFile.value ( "EnableRoll", 1 ).toBool());
	ui.chkEnablePitch->setChecked(iniFile.value ( "EnablePitch", 1 ).toBool());
	ui.chkEnableYaw->setChecked(iniFile.value ( "EnableYaw", 1 ).toBool());
	ui.chkEnableX->setChecked(iniFile.value ( "EnableX", 1 ).toBool());
	ui.chkEnableY->setChecked(iniFile.value ( "EnableY", 1 ).toBool());
	ui.chkEnableZ->setChecked(iniFile.value ( "EnableZ", 1 ).toBool());

	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void TrackerControls::save() {

	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "SMTracker" );
	iniFile.setValue ( "FilterLevel", ui.cbxFilterSetting->currentIndex() );

	iniFile.setValue ( "EnableRoll", ui.chkEnableRoll->isChecked() );
	iniFile.setValue ( "EnablePitch", ui.chkEnablePitch->isChecked() );
	iniFile.setValue ( "EnableYaw", ui.chkEnableYaw->isChecked() );
	iniFile.setValue ( "EnableX", ui.chkEnableX->isChecked() );
	iniFile.setValue ( "EnableY", ui.chkEnableY->isChecked() );
	iniFile.setValue ( "EnableZ", ui.chkEnableZ->isChecked() );

	iniFile.endGroup ();

	//
	// If the Tracker is active, let it load the new Settings.
	//
	if (theTracker) {
		theTracker->loadSettings();
	}

	settingsDirty = false;
}

// Show the current engine-settings etc.
//
void TrackerControls::doTimUpdate()
{
	int state = pMemData->state;
	if ( state != prev_state) {
		emit stateChanged(state);
		prev_state = state;
	}
}

//
// Show the current engine-settings etc.
//
void TrackerControls::showSettings( int newState )
{
	qDebug() << "TrackerControls::showSettings says: Starting Function";
    switch (newState)
    {
    case SM_API_ENGINE_STATE_TERMINATED:
        ui._engine_state_label->setText("TERMINATED");
        break;
    case SM_API_ENGINE_STATE_INVALID:
        ui._engine_state_label->setText("INVALID");
        break;
    case SM_API_ENGINE_STATE_IDLE:
        ui._engine_state_label->setText("IDLE");
        break;
    case SM_API_ENGINE_STATE_HT_INITIALIZING:
        ui._engine_state_label->setText("INITIALIZING");
        break;
    case SM_API_ENGINE_STATE_HT_TRACKING:
        ui._engine_state_label->setText("TRACKING");
        break;
    case SM_API_ENGINE_STATE_HT_SEARCHING:
        ui._engine_state_label->setText("SEARCHING");
        break;
    default:
        ui._engine_state_label->setText("Unknown State!");
        break;
    }

	ui.cbxFilterSetting->setEnabled( (newState == SM_API_ENGINE_STATE_IDLE) );
}

//
// Send a command without parameter-value to the tracking Engine.
//
void TrackerControls::doCommand(int command)
{
    shm.lock();
    pMemData->command = command;
    shm.unlock();
}

//
// Send a command with integer parameter-value to the tracking Engine.
//
void TrackerControls::doCommand(int command, int value)
{
    shm.lock();
    pMemData->command = command;					// Send command
    pMemData->par_val_int = value;
    shm.unlock();
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker-settings dialog object.

// Export both decorated and undecorated names.
//   GetTrackerDialog     - Undecorated name, which can be easily used with GetProcAddress
//                          Win32 API function.
//   _GetTrackerDialog@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetTrackerDialog=_GetTrackerDialog@0")

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITrackerDialog* CALLING_CONVENTION GetDialog()
{
	return new TrackerControls;
}
