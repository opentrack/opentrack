/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay for				*
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
*********************************************************************************/

#include "FaceTrackNoIR.h"
#include "tracker.h"
#include "PPJoyServer.h"
#include "FSUIPCServer.h"
#include "FTIRServer.h"

using namespace sm::faceapi;
using namespace sm::faceapi::qt;

//
// Setup the Main Dialog
//
FaceTrackNoIR::FaceTrackNoIR(QWidget *parent, Qt::WFlags flags) : 
QMainWindow(parent, flags)
{	
	cameraDetected = false;

	//
	// Initialize Widget handles, to prevent memory-access errors.
	//
	_engine_controls = 0;
	_server_controls = 0;
	_keyboard_shortcuts = 0;
	_preferences = 0;
	_keyboard_shortcuts = 0;
	_curve_config = 0;

	tracker = 0;
	_display = 0;
	l = 0;
	trayIcon = 0;

	setupFaceTrackNoIR();
}

/** sets up all objects and connections to buttons */
void FaceTrackNoIR::setupFaceTrackNoIR() {
	
	ui.setupUi(this);

	ui.headPoseWidget->hide();
	ui.video_frame->hide();

	// menu objects will be connected with the functions in FaceTrackNoIR class
	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(open()));
	connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(save()));
	connect(ui.actionSave_As, SIGNAL(triggered()), this, SLOT(saveAs()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(exit()));

	connect(ui.actionPreferences, SIGNAL(triggered()), this, SLOT(showPreferences()));
	connect(ui.actionKeyboard_Shortcuts, SIGNAL(triggered()), this, SLOT(showKeyboardShortcuts()));
	connect(ui.actionCurve_Configuration, SIGNAL(triggered()), this, SLOT(showCurveConfiguration()));
	connect(ui.btnEditCurves, SIGNAL(clicked()), this, SLOT(showCurveConfiguration()));

	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));

	connect(ui.actionVideoWidget, SIGNAL(triggered()), this, SLOT(showVideoWidget()));
	connect(ui.actionHeadPoseWidget, SIGNAL(triggered()), this, SLOT(showHeadPoseWidget()));
	connect(ui.btnShowEngineControls, SIGNAL(clicked()), this, SLOT(showEngineControls()));
	connect(ui.btnShowServerControls, SIGNAL(clicked()), this, SLOT(showServerControls()));

	// button methods connect with methods in this class
	connect(ui.btnStartTracker, SIGNAL(clicked()), this, SLOT(startTracker()));
	connect(ui.btnStopTracker, SIGNAL(clicked()), this, SLOT(stopTracker()));

	// Connect checkboxes
	connect(ui.chkInvertYaw, SIGNAL(stateChanged(int)), this, SLOT(setInvertYaw(int)));
	connect(ui.chkInvertRoll, SIGNAL(stateChanged(int)), this, SLOT(setInvertRoll(int)));
	connect(ui.chkInvertPitch, SIGNAL(stateChanged(int)), this, SLOT(setInvertPitch(int)));
	connect(ui.chkInvertX, SIGNAL(stateChanged(int)), this, SLOT(setInvertX(int)));
	connect(ui.chkInvertY, SIGNAL(stateChanged(int)), this, SLOT(setInvertY(int)));
	connect(ui.chkInvertZ, SIGNAL(stateChanged(int)), this, SLOT(setInvertZ(int)));

	connect(ui.chkUseEWMA, SIGNAL(stateChanged(int)), this, SLOT(setUseFilter(int)));

	// Connect sliders for reduction factor
	connect(ui.redYaw, SIGNAL(valueChanged(int)), this, SLOT(setRedYaw(int)));
	connect(ui.redRoll, SIGNAL(valueChanged(int)), this, SLOT(setRedRoll(int)));
	connect(ui.redPitch, SIGNAL(valueChanged(int)), this, SLOT(setRedPitch(int)));
	connect(ui.redX, SIGNAL(valueChanged(int)), this, SLOT(setRedX(int)));
	connect(ui.redY, SIGNAL(valueChanged(int)), this, SLOT(setRedY(int)));
	connect(ui.redZ, SIGNAL(valueChanged(int)), this, SLOT(setRedZ(int)));

	// Connect slider for smoothing
	connect(ui.slideSmoothing, SIGNAL(valueChanged(int)), this, SLOT(setSmoothing(int)));

	//read the camera-name, using DirectShow
	GetCameraNameDX();
	
	//Create the system-tray and connect the events for that.
	createIconGroupBox();
	createActions();
	createTrayIcon();

	connect(ui.iconcomboTrackerSource, SIGNAL(currentIndexChanged(int)), this, SLOT(trackingSourceSelected(int)));
	connect(ui.iconcomboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setIcon(int)));
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

	//Load the tracker-settings, from the INI-file
	loadSettings();
	trayIcon->show();

	//Setup the timer for automatically minimizing after StartTracker.
	timMinimizeFTN = new QTimer(this);
    connect(timMinimizeFTN, SIGNAL(timeout()), this, SLOT(showMinimized()));
}

/** destructor stops the engine and quits the faceapi **/
FaceTrackNoIR::~FaceTrackNoIR() {

	//
	// Stop the tracker, by simulating a button-push
	//
	stopTracker();

	//
	// Ask if changed Settings should be saved
	//
	if (settingsDirty) {
		int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard, QMessageBox::Discard );

		switch (ret) {
			case QMessageBox::Save:
				saveAs();
				break;
			case QMessageBox::Discard:
				// Don't Save was clicked
				break;
			case QMessageBox::Cancel:
				// Cancel was clicked
				break;
			default:
				// should never be reached
			break;
		}
	}

}

//
// Get the ProgramName from a connected game and display it.
//
void FaceTrackNoIR::getGameProgramName() {
	if ( tracker != NULL ) {
		ui.cameraName->setText( tracker->getGameProgramName() );
	}
}

//
// Get the ProgramName from a connected game and display it.
//
void FaceTrackNoIR::updateSettings() {
	if ( tracker != NULL ) {
		tracker->loadSettings();
	}
}

/** read the name of the first video-capturing device at start up **/
/** FaceAPI can only use this first one... **/
void FaceTrackNoIR::GetCameraNameDX() {
	
////	ui.widget->setCameraName("No video-capturing device was found in your system: check if it's connected!");

	ui.cameraName->setText("No video-capturing device was found in your system: check if it's connected!");

	// Create the System Device Enumerator.
	HRESULT hr;
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		qDebug() << "GetWDM says: CoCreateInstance Failed!";
		return;
	}

	qDebug() << "GetWDM says: CoCreateInstance succeeded!";
	
	// Obtain a class enumerator for the video compressor category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK) {
		qDebug() << "GetWDM says: CreateClassEnumerator succeeded!";

		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		if (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) {
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
			if (SUCCEEDED(hr))	{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					// Display the name in your UI somehow.
					QString str((QChar*)varName.bstrVal, wcslen(varName.bstrVal));
					qDebug() << "GetWDM says: Moniker found:" << str;
					ui.cameraName->setText(str);
					return;
				}
				VariantClear(&varName);

				////// To create an instance of the filter, do the following:
				////IBaseFilter *pFilter;
				////hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
				////	(void**)&pFilter);
				// Now add the filter to the graph. 
				//Remember to release pFilter later.
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();

}

//
// Open an INI-file with the QFileDialog
// If succesfull, the settings in it will be read
//
void FaceTrackNoIR::open() {
	 QFileDialog::Options options;
	 QFileDialog::FileMode mode;

     options |= QFileDialog::DontUseNativeDialog;
	 mode = QFileDialog::ExistingFile;
     QString selectedFilter;
	 QStringList fileNames = QFileDialog::getOpenFileNames(
								this,
                                 tr("Select one FTNoir settings file"),
								 QCoreApplication::applicationDirPath() + "/Settings",
                                 tr("Settings file (*.ini);;All Files (*)"));

	//
	// If a file was selected, save it's name and read it's contents.
	//
	if (! fileNames.isEmpty() ) {
		QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)
		settings.setValue ("SettingsFile", fileNames.at(0));
		loadSettings();
	}
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void FaceTrackNoIR::save() {

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "Tracking" );
	iniFile.setValue ( "Smooth", ui.slideSmoothing->value() );
	iniFile.setValue ( "invertYaw", ui.chkInvertYaw->isChecked() );
	iniFile.setValue ( "invertPitch", ui.chkInvertPitch->isChecked() );
	iniFile.setValue ( "invertRoll", ui.chkInvertRoll->isChecked() );
	iniFile.setValue ( "invertX", ui.chkInvertX->isChecked() );
	iniFile.setValue ( "invertY", ui.chkInvertY->isChecked() );
	iniFile.setValue ( "invertZ", ui.chkInvertZ->isChecked() );
	iniFile.setValue ( "useEWMA", ui.chkUseEWMA->isChecked() );
	iniFile.setValue ( "redYaw", ui.redYaw->value() );
	iniFile.setValue ( "redPitch", ui.redPitch->value() );
	iniFile.setValue ( "redRoll", ui.redRoll->value() );
	iniFile.setValue ( "redX", ui.redX->value() );
	iniFile.setValue ( "redY", ui.redY->value() );
	iniFile.setValue ( "redZ", ui.redZ->value() );
	iniFile.endGroup ();

	iniFile.beginGroup ( "GameProtocol" );
	iniFile.setValue ( "Selection", ui.iconcomboBox->currentIndex() );
	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Get the new name of the INI-file and save the settings to it.
//
// The user may choose to overwrite an existing file. This will be deleted, before copying the current file to it.
//
void FaceTrackNoIR::saveAs()
{
	//
	// Get the current filename of the INI-file.
	//
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)
	QString oldFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();

	//
	// Get the new filename of the INI-file.
	//
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"),
													QCoreApplication::applicationDirPath() + "/Settings",
													tr("Settings file (*.ini);;All Files (*)"));
	if (!fileName.isEmpty()) {

		//
		// Remove the file, if it already exists.
		//
		QFileInfo newFileInfo ( fileName );
		if (newFileInfo.exists()) {
			QFile newFileFile ( fileName );
			newFileFile.remove();
		}

		//
		// Copy the current INI-file to the new name.
		//
		QFileInfo oldFileInfo ( oldFile );
		if (oldFileInfo.exists()) {
			QFile oldFileFile ( oldFile );
			oldFileFile.copy( fileName );
		}

		//
		// Write the new name to the Registry and save the other INI-values.
		//
		settings.setValue ("SettingsFile", fileName);
		save();

		// Put the filename in the window-title
		setWindowTitle ( "FaceTrackNoIR (1.4) - " + newFileInfo.fileName() );
	}
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FaceTrackNoIR::loadSettings() {

	qDebug() << "loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "Tracking" );
	ui.slideSmoothing->setValue (iniFile.value ( "Smooth", 10 ).toInt());
	ui.chkInvertYaw->setChecked (iniFile.value ( "invertYaw", 0 ).toBool());
	ui.chkInvertPitch->setChecked (iniFile.value ( "invertPitch", 0 ).toBool());
	ui.chkInvertRoll->setChecked (iniFile.value ( "invertRoll", 0 ).toBool());
	ui.chkInvertX->setChecked (iniFile.value ( "invertX", 0 ).toBool());
	ui.chkInvertY->setChecked (iniFile.value ( "invertY", 0 ).toBool());
	ui.chkInvertZ->setChecked (iniFile.value ( "invertZ", 0 ).toBool());
	ui.chkUseEWMA->setChecked (iniFile.value ( "useEWMA", 1 ).toBool());
	ui.redYaw->setValue (iniFile.value ( "redYaw", 70 ).toInt());
	ui.redPitch->setValue (iniFile.value ( "redPitch", 70 ).toInt());
	ui.redRoll->setValue (iniFile.value ( "redRoll", 70 ).toInt());
	ui.redX->setValue (iniFile.value ( "redX", 70 ).toInt());
	ui.redY->setValue (iniFile.value ( "redY", 70 ).toInt());
	ui.redZ->setValue (iniFile.value ( "redZ", 70 ).toInt());
	iniFile.endGroup ();

	iniFile.beginGroup ( "GameProtocol" );
	ui.iconcomboBox->setCurrentIndex(iniFile.value ( "Selection", 0 ).toInt());
	setIcon( ui.iconcomboBox->currentIndex() );
	iniFile.endGroup ();

	settingsDirty = false;

	// Put the filename in the window-title
    QFileInfo pathInfo ( currentFile );
    setWindowTitle ( "FaceTrackNoIR (1.4) - " + pathInfo.fileName() );

}

/** show about dialog **/
void FaceTrackNoIR::about() {
	aboutDialog.move(this->width()/2-135,
					this->height()/2-220);
	
	aboutDialog.show();

	/** ABOUT DIALOG **/
	aboutDialog.setBaseSize(270, 440);

	aboutDialog.setMaximumWidth(270);
	aboutDialog.setMaximumHeight(440);

	aboutDialog.setMinimumWidth(270);
	aboutDialog.setMinimumHeight(440);
	aboutDialog.setStyleSheet("background:#fff url(UIElements/aboutFaceTrackNoIR.png) no-repeat;");
}

/** start tracking the face **/
void FaceTrackNoIR::startTracker( ) {	

	//
	// Create the Tracker and setup
	//
	tracker = new Tracker ( ui.iconcomboBox->currentIndex() );

	// Show the video widget
	ui.video_frame->show();
	_display = new VideoDisplayWidget( tracker->getEngine(), ui.video_frame );
	l = new QVBoxLayout(ui.video_frame);
	l->setMargin(0);
	l->setSpacing(0);
	l->addWidget(_display);

	//
	// Setup the Tracker and send the settings.
	// This is necessary, because the events are only triggered 'on change'
	//
	tracker->setup( ui.headPoseWidget , this);
	tracker->setSmoothing ( ui.slideSmoothing->value() );
	tracker->setUseFilter (ui.chkUseEWMA->isChecked() );

	tracker->setInvertYaw (ui.chkInvertYaw->isChecked() );
	tracker->setInvertPitch (ui.chkInvertPitch->isChecked() );
	tracker->setInvertRoll (ui.chkInvertRoll->isChecked() );
	tracker->setInvertX (ui.chkInvertX->isChecked() );
	tracker->setInvertY (ui.chkInvertY->isChecked() );
	tracker->setInvertZ (ui.chkInvertZ->isChecked() );

	tracker->setRedYaw (ui.redYaw->value() );
	tracker->setRedPitch (ui.redPitch->value() );
	tracker->setRedRoll (ui.redRoll->value() );
	tracker->setRedX (ui.redX->value() );
	tracker->setRedY (ui.redY->value() );
	tracker->setRedZ (ui.redZ->value() );

	tracker->start( QThread::TimeCriticalPriority );

	ui.headPoseWidget->show();

	// 
	ui.btnStartTracker->setEnabled ( false );
	ui.btnStopTracker->setEnabled ( true );

	// Engine controls
	ui.btnShowEngineControls->setEnabled ( true );
	ui.iconcomboBox->setEnabled ( false );

	// Enable/disable Protocol-server Settings
	ui.btnShowServerControls->setEnabled ( false );

	//
	// Update the camera-name, FaceAPI can only use the 1st one found!
	//
	GetCameraNameDX();


	//
	// Get the TimeOut value for minimizing FaceTrackNoIR
	// Only start the Timer if value > 0
	//
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)
	int timevalue = settings.value ( "AutoMinimizeTime", 0 ).toInt() * 1000;

	if (timevalue > 0) {
		timMinimizeFTN->setSingleShot( true );
		timMinimizeFTN->start(timevalue);
	}
}

/** stop tracking the face **/
void FaceTrackNoIR::stopTracker( ) {	

	//
	// Delete the video-display.
	//
	if ( _display ) {
		_display->disconnect();
		delete _display;
		_display = 0;
		delete l;
		l = 0;
		qDebug() << "stopTracker says: display deleted";
	}
	ui.video_frame->hide();

	if ( tracker ) {
		qDebug() << "stopTracker says: Deleting tracker!";
		delete tracker;
		tracker = 0;
	}
	ui.btnStartTracker->setEnabled ( true );
	ui.btnStopTracker->setEnabled ( false );
	ui.btnShowEngineControls->setEnabled ( false );
	ui.iconcomboBox->setEnabled ( true );

	// Enable/disable Protocol-server Settings
	ui.btnShowServerControls->setEnabled ( true );
}

/** set the invert from the checkbox **/
void FaceTrackNoIR::setInvertYaw( int invert ) {
	Tracker::setInvertYaw ( (invert != 0)?true:false );
	settingsDirty = true;
}

/** set the invert from the checkbox **/
void FaceTrackNoIR::setInvertPitch( int invert ) {
	Tracker::setInvertPitch ( (invert != 0)?true:false );
	settingsDirty = true;
}

/** set the invert from the checkbox **/
void FaceTrackNoIR::setInvertRoll( int invert ) {
	Tracker::setInvertRoll ( (invert != 0)?true:false );
	settingsDirty = true;
}

/** set the invert from the checkbox **/
void FaceTrackNoIR::setInvertX( int invert ) {
	Tracker::setInvertX ( (invert != 0)?true:false );
	settingsDirty = true;
}

/** set the invert from the checkbox **/
void FaceTrackNoIR::setInvertY( int invert ) {
	Tracker::setInvertY ( (invert != 0)?true:false );
	settingsDirty = true;
}

/** set the invert from the checkbox **/
void FaceTrackNoIR::setInvertZ( int invert ) {
	Tracker::setInvertZ ( (invert != 0)?true:false );
	settingsDirty = true;
}

/** set Use Filter from the checkbox **/
void FaceTrackNoIR::setUseFilter( int set ) {
	Tracker::setUseFilter ( (set != 0)?true:false );
	settingsDirty = true;
}

/** set the redhold from the slider **/
void FaceTrackNoIR::setRedYaw( int redh ) {
	Tracker::setRedYaw ( redh );
	settingsDirty = true;
}

/** set the redhold from the slider **/
void FaceTrackNoIR::setRedPitch( int redh ) {
	Tracker::setRedPitch ( redh );
	settingsDirty = true;
}

/** set the redhold from the slider **/
void FaceTrackNoIR::setRedRoll( int redh ) {
	Tracker::setRedRoll ( redh );
	settingsDirty = true;
}

/** set the redhold from the slider **/
void FaceTrackNoIR::setRedX( int redh ) {
	Tracker::setRedX ( redh );
	settingsDirty = true;
}

/** set the redhold from the slider **/
void FaceTrackNoIR::setRedY( int redh ) {
	Tracker::setRedY ( redh );
	settingsDirty = true;
}

/** set the redhold from the slider **/
void FaceTrackNoIR::setRedZ( int redh ) {
	Tracker::setRedZ ( redh );
	settingsDirty = true;
}

/** set the smoothing from the slider **/
void FaceTrackNoIR::setSmoothing( int smooth ) {
	
	//
	// Pass the smoothing setting, if the Tracker exists.
	//
	if ( tracker ) {
		tracker->setSmoothing ( smooth );
		settingsDirty = true;
	}
}


/** toggles Video Widget **/
void FaceTrackNoIR::showVideoWidget() {
	if(ui.video_frame->isHidden())
		ui.video_frame->show();
	else
		ui.video_frame->hide();
}

/** toggles Video Widget **/
void FaceTrackNoIR::showHeadPoseWidget() {
	if(ui.headPoseWidget->isHidden())
		ui.headPoseWidget->show();
	else
		ui.headPoseWidget->hide();
}

/** toggles Engine Controls Dialog **/
void FaceTrackNoIR::showEngineControls() {

	// Create if new
    if (!_engine_controls)
    {
        _engine_controls = new EngineControls( tracker->getEngine(), true, false, this, Qt::Dialog );
    }

	// Show if already created
	if (_engine_controls) {
		_engine_controls->show();
		_engine_controls->raise();
	}
}

/** toggles Server Controls Dialog **/
void FaceTrackNoIR::showServerControls() {


	//
	// Delete the existing QDialog
	//
	if (_server_controls) {
		delete _server_controls;
		_server_controls = 0;
	}

	// Create if new
	if (!_server_controls)
    {


		// Show the appropriate Protocol-server Settings
		switch (ui.iconcomboBox->currentIndex()) {
		case FREE_TRACK:
		case FLIGHTGEAR:
		case FTNOIR:
		case SIMCONNECT:
			break;
		case PPJOY:
	        _server_controls = new PPJoyControls( this, Qt::Dialog );
			break;
		case FSUIPC:
	        _server_controls = new FSUIPCControls( this, Qt::Dialog );
			break;
		case TRACKIR:
	        _server_controls = new FTIRControls( this, Qt::Dialog );
			break;
		default:
			break;
		}
    }

	// Show if already created
	if (_server_controls) {
		_server_controls->show();
		_server_controls->raise();
	}
}

/** toggles FaceTrackNoIR Preferences Dialog **/
void FaceTrackNoIR::showPreferences() {

	// Create if new
	if (!_preferences)
    {
        _preferences = new PreferencesDialog( this, this, Qt::Dialog );
    }

	// Show if already created
	if (_preferences) {
		_preferences->show();
		_preferences->raise();
	}
}

/** toggles Keyboard Shortcut Dialog **/
void FaceTrackNoIR::showKeyboardShortcuts() {

	// Create if new
	if (!_keyboard_shortcuts)
    {
        _keyboard_shortcuts = new KeyboardShortcutDialog( this, this, Qt::Dialog );
    }

	// Show if already created
	if (_keyboard_shortcuts) {
		_keyboard_shortcuts->show();
		_keyboard_shortcuts->raise();
	}
}

/** toggles Curve Configuration Dialog **/
void FaceTrackNoIR::showCurveConfiguration() {

	// Create if new
	if (!_curve_config)
    {
        _curve_config = new CurveConfigurationDialog( this, this, Qt::Dialog );
    }

	// Show if already created
	if (_curve_config) {
		_curve_config->show();
		_curve_config->raise();
	}
}

/** exit application **/
void FaceTrackNoIR::exit() {
	QCoreApplication::exit(0);
}

//
// Setup the icons for the comboBoxes
//
void FaceTrackNoIR::createIconGroupBox()
{
	ui.iconcomboBox->addItem(QIcon(QCoreApplication::applicationDirPath() + "/images/Freetrack.ico"), tr("Freetrack"));
	ui.iconcomboBox->addItem(QIcon(QCoreApplication::applicationDirPath() + "/images/FlightGear.ico"), tr("FlightGear"));
	ui.iconcomboBox->addItem(QIcon(QCoreApplication::applicationDirPath() + "/images/FaceTrackNoIR.ico"), tr("FTNoir client"));
	ui.iconcomboBox->addItem(QIcon(QCoreApplication::applicationDirPath() + "/images/PPJoy.ico"), tr("Virtual Joystick"));
	ui.iconcomboBox->addItem(QIcon(QCoreApplication::applicationDirPath() + "/images/TrackIR.ico"), tr("Fake TrackIR"));
	ui.iconcomboBox->addItem(QIcon(QCoreApplication::applicationDirPath() + "/images/FSX.ico"), tr("SimConnect (FSX)"));
	ui.iconcomboBox->addItem(QIcon(QCoreApplication::applicationDirPath() + "/images/FS9.ico"), tr("FS2002/FS2004"));

	ui.iconcomboTrackerSource->addItem(QIcon(QCoreApplication::applicationDirPath() + "/images/SeeingMachines.ico"), tr("Face API"));
	ui.iconcomboTrackerSource->addItem(QIcon(QCoreApplication::applicationDirPath() + "/images/FaceTrackNoIR.ico"), tr("FTNoir server"));
}

//
// Create the Actions in the System tray and connect them to Application events
//
void FaceTrackNoIR::createActions()
{
    minimizeAction = new QAction(tr("Mi&nimize FaceTrackNoIR"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    //maximizeAction = new QAction(tr("Ma&ximize"), this);
    //connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

    restoreAction = new QAction(tr("&Restore FaceTrackNoIR"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(tr("&Quit FaceTrackNoIR"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

//
// Create the SystemTray and set the default Icon
//
void FaceTrackNoIR::createTrayIcon()
{
	if (QSystemTrayIcon::isSystemTrayAvailable()) {
		trayIconMenu = new QMenu(this);
		trayIconMenu->addAction(minimizeAction);
		trayIconMenu->addAction(restoreAction);
		trayIconMenu->addSeparator();
		trayIconMenu->addAction(quitAction);

		trayIcon = new QSystemTrayIcon(this);
		trayIcon->setContextMenu(trayIconMenu);

		trayIcon->setIcon(QIcon(QCoreApplication::applicationDirPath() + "/images/FaceTrackNoIR.ico"));
	}
}

//
// Set the Tray icon, using the Game-protocol combobox as source
//
void FaceTrackNoIR::setIcon(int index)
{
    QIcon icon = ui.iconcomboBox->itemIcon(index);
	if (trayIcon != 0) {
		trayIcon->setIcon(icon);
	    trayIcon->setToolTip(ui.iconcomboBox->itemText(index));
		trayIcon->show();
		trayIcon->showMessage( "FaceTrackNoIR", ui.iconcomboBox->itemText(index));
	}
    setWindowIcon(QIcon(QCoreApplication::applicationDirPath() + "/images/FaceTrackNoIR.ico"));

	settingsDirty = true;

	// Enable/disable Protocol-server Settings
	switch (ui.iconcomboBox->currentIndex()) {
	case FREE_TRACK:
	case FLIGHTGEAR:
	case FTNOIR:
	case SIMCONNECT:
		ui.btnShowServerControls->hide();
		break;
	case PPJOY:
	case FSUIPC:
	case TRACKIR:
		ui.btnShowServerControls->show();
		ui.btnShowServerControls->setEnabled ( true );
		break;

	default:
		break;
	}

}
 
//
// Handle SystemTray events
//
void FaceTrackNoIR::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
     switch (reason) {
     case QSystemTrayIcon::Trigger:
     case QSystemTrayIcon::DoubleClick:
         //ui.iconcomboBox->setCurrentIndex((ui.iconcomboBox->currentIndex() + 1)
         //                              % ui.iconcomboBox->count());
         break;
     ////case QSystemTrayIcon::MiddleClick:
     ////    showMessage();
     ////    break;
     default:
         ;
     }
 }

//
// Handle changes of the Tracking Source selection
//
void FaceTrackNoIR::trackingSourceSelected(int index)
{
	switch (ui.iconcomboTrackerSource->currentIndex()) {
	case 0:													// Face API
		break;
	case 1:													// FTNoir server
		ui.video_frame->hide();
		ui.headPoseWidget->show();
		break;
	default:
		break;
	}
}


//
// Constructor for FaceTrackNoIR=Preferences-dialog
//
PreferencesDialog::PreferencesDialog( FaceTrackNoIR *ftnoir, QWidget *parent, Qt::WindowFlags f ) :
QWidget( parent , f)
{
	ui.setupUi( this );

	QPoint offsetpos(100, 100);
	this->move(parent->pos() + offsetpos);

	mainApp = ftnoir;											// Preserve a pointer to FTNoIR

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));

	connect(ui.slideAutoMinimizeTime, SIGNAL(valueChanged(int)), this, SLOT(keyChanged(int)));

	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
PreferencesDialog::~PreferencesDialog() {
	qDebug() << "~PreferencesDialog() says: started";
}

//
// OK clicked on server-dialog
//
void PreferencesDialog::doOK() {
	save();
	this->close();
}

// override show event
void PreferencesDialog::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void PreferencesDialog::doCancel() {
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
void PreferencesDialog::loadSettings() {

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)
	ui.slideAutoMinimizeTime->setValue( settings.value ( "AutoMinimizeTime", 0 ).toInt() );

	settingsDirty = false;

}

//
// Save the current Settings to the currently 'active' INI-file.
//
void PreferencesDialog::save() {

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)
	settings.setValue( "AutoMinimizeTime", ui.slideAutoMinimizeTime->value() );

	//
	// Send a message to the main program, to update the Settings (for the tracker)
	//
	mainApp->updateSettings();

	settingsDirty = false;
}

//**************************************************************************************************//
//**************************************************************************************************//
//
// Constructor for Keyboard-shortcuts-dialog
//
KeyboardShortcutDialog::KeyboardShortcutDialog( FaceTrackNoIR *ftnoir, QWidget *parent, Qt::WindowFlags f ) :
QWidget( parent , f)
{
	ui.setupUi( this );

	QPoint offsetpos(100, 100);
	this->move(parent->pos() + offsetpos);

	mainApp = ftnoir;											// Preserve a pointer to FTNoIR

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));

	connect(ui.cbxCenterKey, SIGNAL(currentIndexChanged(int)), this, SLOT(keyChanged( int )));
	connect(ui.chkCenterShift, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkCenterCtrl, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkCenterAlt, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));

	connect(ui.cbxStartStopKey, SIGNAL(currentIndexChanged(int)), this, SLOT(keyChanged( int )));
	connect(ui.chkStartStopShift, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkStartStopCtrl, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkStartStopAlt, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));

	//
	// Clear the Lists with key-descriptions and keycodes and build the Lists
	// The strings will all be added to the ListBoxes for each Shortkey
	//
	stringList.clear();
	stringList.append("NONE");
	stringList.append("F1");
	stringList.append("F2");
	stringList.append("F3");
	stringList.append("F4");
	stringList.append("F5");
	stringList.append("F6");
	stringList.append("F7");
	stringList.append("F8");
	stringList.append("F9");
	stringList.append("F10");
	stringList.append("F11");
	stringList.append("F12");
	stringList.append("MINUS");
	stringList.append("EQUALS");
	stringList.append("BACK");
	stringList.append("A");
	stringList.append("B");
	stringList.append("C");
	stringList.append("D");
	stringList.append("E");
	stringList.append("F");
	stringList.append("G");
	stringList.append("H");
	stringList.append("I");
	stringList.append("J");
	stringList.append("K");
	stringList.append("L");
	stringList.append("M");
	stringList.append("N");
	stringList.append("O");
	stringList.append("P");
	stringList.append("Q");
	stringList.append("R");
	stringList.append("S");
	stringList.append("T");
	stringList.append("U");
	stringList.append("V");
	stringList.append("W");
	stringList.append("X");
	stringList.append("Y");
	stringList.append("Z");
	stringList.append("NUMPAD0");
	stringList.append("NUMPAD1");
	stringList.append("NUMPAD2");
	stringList.append("NUMPAD3");
	stringList.append("NUMPAD4");
	stringList.append("NUMPAD5");
	stringList.append("NUMPAD6");
	stringList.append("NUMPAD7");
	stringList.append("NUMPAD8");
	stringList.append("NUMPAD9");
	stringList.append("HOME");
	stringList.append("UP");
	stringList.append("PGUP");		/* PgUp on arrow keypad */
	stringList.append("LEFT");
	stringList.append("RIGHT");
	stringList.append("END");
	stringList.append("DOWN");
	stringList.append("PGDWN");		/* PgDn on arrow keypad */
	stringList.append("INSERT");
	stringList.append("DELETE");

	keyList.clear();
	keyList.append(0);				// NONE = 0
	keyList.append(DIK_F1);
	keyList.append(DIK_F2);
	keyList.append(DIK_F3);
	keyList.append(DIK_F4);
	keyList.append(DIK_F5);
	keyList.append(DIK_F6);
	keyList.append(DIK_F7);
	keyList.append(DIK_F8);
	keyList.append(DIK_F9);
	keyList.append(DIK_F10);
	keyList.append(DIK_F11);
	keyList.append(DIK_F12);
	keyList.append(DIK_MINUS);
	keyList.append(DIK_EQUALS);
	keyList.append(DIK_BACK);
	keyList.append(DIK_A);
	keyList.append(DIK_B);
	keyList.append(DIK_C);
	keyList.append(DIK_D);
	keyList.append(DIK_E);
	keyList.append(DIK_F);
	keyList.append(DIK_G);
	keyList.append(DIK_H);
	keyList.append(DIK_I);
	keyList.append(DIK_J);
	keyList.append(DIK_K);
	keyList.append(DIK_L);
	keyList.append(DIK_M);
	keyList.append(DIK_N);
	keyList.append(DIK_O);
	keyList.append(DIK_P);
	keyList.append(DIK_Q);
	keyList.append(DIK_R);
	keyList.append(DIK_S);
	keyList.append(DIK_T);
	keyList.append(DIK_U);
	keyList.append(DIK_V);
	keyList.append(DIK_W);
	keyList.append(DIK_X);
	keyList.append(DIK_Y);
	keyList.append(DIK_Z);
	keyList.append(DIK_NUMPAD0);
	keyList.append(DIK_NUMPAD1);
	keyList.append(DIK_NUMPAD2);
	keyList.append(DIK_NUMPAD3);
	keyList.append(DIK_NUMPAD4);
	keyList.append(DIK_NUMPAD5);
	keyList.append(DIK_NUMPAD6);
	keyList.append(DIK_NUMPAD7);
	keyList.append(DIK_NUMPAD8);
	keyList.append(DIK_NUMPAD9);
	keyList.append(DIK_HOME);
	keyList.append(DIK_UP);
	keyList.append(DIK_PRIOR);		/* PgUp on arrow keypad */
	keyList.append(DIK_LEFT);
	keyList.append(DIK_RIGHT);
	keyList.append(DIK_END);
	keyList.append(DIK_DOWN);
	keyList.append(DIK_NEXT);		/* PgDn on arrow keypad */
	keyList.append(DIK_INSERT);
	keyList.append(DIK_DELETE);

	//
	// Add strings to the Listboxes.
	//
	for ( int i = 0; i < stringList.size(); i++) {
		ui.cbxCenterKey->addItem(stringList.at(i));
		ui.cbxStartStopKey->addItem(stringList.at(i));
	}

	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
KeyboardShortcutDialog::~KeyboardShortcutDialog() {
	qDebug() << "~KeyboardShortcutDialog() says: started";
}

//
// OK clicked on server-dialog
//
void KeyboardShortcutDialog::doOK() {
	save();
	this->close();
}

// override show event
void KeyboardShortcutDialog::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void KeyboardShortcutDialog::doCancel() {
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
void KeyboardShortcutDialog::loadSettings() {
int keyindex;

	qDebug() << "loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "KB_Shortcuts" );
	
	// Center key
	keyindex = keyList.indexOf ( iniFile.value ( "Keycode_Center", 1 ).toInt() );
	if ( keyindex > 0 ) {
		ui.cbxCenterKey->setCurrentIndex( keyindex );
	}
	else {
		ui.cbxCenterKey->setCurrentIndex( 0 );
	}
	ui.chkCenterShift->setChecked (iniFile.value ( "Shift_Center", 0 ).toBool());
	ui.chkCenterCtrl->setChecked (iniFile.value ( "Ctrl_Center", 0 ).toBool());
	ui.chkCenterAlt->setChecked (iniFile.value ( "Alt_Center", 0 ).toBool());

	// Start/stop key
	keyindex = keyList.indexOf ( iniFile.value ( "Keycode_StartStop", 1 ).toInt() );
	if ( keyindex > 0 ) {
		ui.cbxStartStopKey->setCurrentIndex( keyindex );
	}
	else {
		ui.cbxStartStopKey->setCurrentIndex( 0 );
	}
	ui.chkStartStopShift->setChecked (iniFile.value ( "Shift_StartStop", 0 ).toBool());
	ui.chkStartStopCtrl->setChecked (iniFile.value ( "Ctrl_StartStop", 0 ).toBool());
	ui.chkStartStopAlt->setChecked (iniFile.value ( "Alt_StartStop", 0 ).toBool());

	iniFile.endGroup ();

	settingsDirty = false;

}

//
// Save the current Settings to the currently 'active' INI-file.
//
void KeyboardShortcutDialog::save() {

	qDebug() << "save() says: started";

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "KB_Shortcuts" );
	iniFile.setValue ( "Keycode_Center", keyList.at( ui.cbxCenterKey->currentIndex() ) );
	iniFile.setValue ( "Shift_Center", ui.chkCenterShift->isChecked() );
	iniFile.setValue ( "Ctrl_Center", ui.chkCenterCtrl->isChecked() );
	iniFile.setValue ( "Alt_Center", ui.chkCenterAlt->isChecked() );

	iniFile.setValue ( "Keycode_StartStop", keyList.at( ui.cbxStartStopKey->currentIndex() ) );
	iniFile.setValue ( "Shift_StartStop", ui.chkStartStopShift->isChecked() );
	iniFile.setValue ( "Ctrl_StartStop", ui.chkStartStopCtrl->isChecked() );
	iniFile.setValue ( "Alt_StartStop", ui.chkStartStopAlt->isChecked() );
	iniFile.endGroup ();

	settingsDirty = false;

	//
	// Send a message to the main program, to update the Settings (for the tracker)
	//
	mainApp->updateSettings();
}

//**************************************************************************************************//
//**************************************************************************************************//
//
// Constructor for Curve-configuration-dialog
//
CurveConfigurationDialog::CurveConfigurationDialog( FaceTrackNoIR *ftnoir, QWidget *parent, Qt::WindowFlags f ) :
QWidget( parent , f)
{
	ui.setupUi( this );

	QPoint offsetpos(120, 30);
	this->move(parent->pos() + offsetpos);

	mainApp = ftnoir;											// Preserve a pointer to FTNoIR

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));

	connect(ui.curveYaw, SIGNAL(BezierCurveChanged(bool)), this, SLOT(curveChanged(bool)));
	connect(ui.curvePitch, SIGNAL(BezierCurveChanged(bool)), this, SLOT(curveChanged(bool)));
	connect(ui.curveRoll, SIGNAL(BezierCurveChanged(bool)), this, SLOT(curveChanged(bool)));
	connect(ui.curveX, SIGNAL(BezierCurveChanged(bool)), this, SLOT(curveChanged(bool)));
	connect(ui.curveY, SIGNAL(BezierCurveChanged(bool)), this, SLOT(curveChanged(bool)));
	connect(ui.curveZ, SIGNAL(BezierCurveChanged(bool)), this, SLOT(curveChanged(bool)));

	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
CurveConfigurationDialog::~CurveConfigurationDialog() {
	qDebug() << "~CurveConfigurationDialog() says: started";
}

//
// OK clicked on server-dialog
//
void CurveConfigurationDialog::doOK() {
	save();
	this->close();
}

// override show event
void CurveConfigurationDialog::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void CurveConfigurationDialog::doCancel() {
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
void CurveConfigurationDialog::loadSettings() {
int NeutralZone;
int sensYaw, sensPitch, sensRoll;
int sensX, sensY, sensZ;
QPointF point1, point2, point3, point4;

	qDebug() << "loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "Tracking" );
	NeutralZone = iniFile.value ( "NeutralZone", 5 ).toInt();
	sensYaw = iniFile.value ( "sensYaw", 100 ).toInt();
	sensPitch = iniFile.value ( "sensPitch", 100 ).toInt();
	sensRoll = iniFile.value ( "sensRoll", 100 ).toInt();
	sensX = iniFile.value ( "sensX", 100 ).toInt();
	sensY = iniFile.value ( "sensY", 100 ).toInt();
	sensZ = iniFile.value ( "sensZ", 100 ).toInt();

	iniFile.endGroup ();

	iniFile.beginGroup ( "Curves" );
	getCurvePoints( &iniFile, "Yaw_", &point1, &point2, &point3, &point4, NeutralZone, sensYaw, 50, 180 );
	ui.curveYaw->setPointOne( point1 );
	ui.curveYaw->setPointTwo( point2 );
	ui.curveYaw->setPointThree( point3 );
	ui.curveYaw->setPointFour( point4 );

	getCurvePoints( &iniFile, "Pitch_", &point1, &point2, &point3, &point4, NeutralZone, sensPitch, 50, 180 );
	ui.curvePitch->setPointOne( point1 );
	ui.curvePitch->setPointTwo( point2 );
	ui.curvePitch->setPointThree( point3 );
	ui.curvePitch->setPointFour( point4 );

	getCurvePoints( &iniFile, "Roll_", &point1, &point2, &point3, &point4, NeutralZone, sensRoll, 50, 180 );
	ui.curveRoll->setPointOne( point1 );
	ui.curveRoll->setPointTwo( point2 );
	ui.curveRoll->setPointThree( point3 );
	ui.curveRoll->setPointFour( point4 );

	getCurvePoints( &iniFile, "X_", &point1, &point2, &point3, &point4, NeutralZone, sensX, 50, 180 );
	ui.curveX->setPointOne( point1 );
	ui.curveX->setPointTwo( point2 );
	ui.curveX->setPointThree( point3 );
	ui.curveX->setPointFour( point4 );

	getCurvePoints( &iniFile, "Y_", &point1, &point2, &point3, &point4, NeutralZone, sensY, 50, 180 );
	ui.curveY->setPointOne( point1 );
	ui.curveY->setPointTwo( point2 );
	ui.curveY->setPointThree( point3 );
	ui.curveY->setPointFour( point4 );

	getCurvePoints( &iniFile, "Z_", &point1, &point2, &point3, &point4, NeutralZone, sensZ, 50, 180 );
	ui.curveZ->setPointOne( point1 );
	ui.curveZ->setPointTwo( point2 );
	ui.curveZ->setPointThree( point3 );
	ui.curveZ->setPointFour( point4 );
	iniFile.endGroup ();

	settingsDirty = false;

}

//
// Save the current Settings to the currently 'active' INI-file.
//
void CurveConfigurationDialog::save() {

	qDebug() << "save() says: started";

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "Curves" );
	iniFile.setValue ("Yaw_point1", ui.curveYaw->getPointOne() );
	iniFile.setValue ("Yaw_point2", ui.curveYaw->getPointTwo() );
	iniFile.setValue ("Yaw_point3", ui.curveYaw->getPointThree() );
	iniFile.setValue ("Yaw_point4", ui.curveYaw->getPointFour() );

	iniFile.setValue ("Pitch_point1", ui.curvePitch->getPointOne() );
	iniFile.setValue ("Pitch_point2", ui.curvePitch->getPointTwo() );
	iniFile.setValue ("Pitch_point3", ui.curvePitch->getPointThree() );
	iniFile.setValue ("Pitch_point4", ui.curvePitch->getPointFour() );

	iniFile.setValue ("Roll_point1", ui.curveRoll->getPointOne() );
	iniFile.setValue ("Roll_point2", ui.curveRoll->getPointTwo() );
	iniFile.setValue ("Roll_point3", ui.curveRoll->getPointThree() );
	iniFile.setValue ("Roll_point4", ui.curveRoll->getPointFour() );
	
	iniFile.setValue ("X_point1", ui.curveX->getPointOne() );
	iniFile.setValue ("X_point2", ui.curveX->getPointTwo() );
	iniFile.setValue ("X_point3", ui.curveX->getPointThree() );
	iniFile.setValue ("X_point4", ui.curveX->getPointFour() );

	iniFile.setValue ("Y_point1", ui.curveY->getPointOne() );
	iniFile.setValue ("Y_point2", ui.curveY->getPointTwo() );
	iniFile.setValue ("Y_point3", ui.curveY->getPointThree() );
	iniFile.setValue ("Y_point4", ui.curveY->getPointFour() );

	iniFile.setValue ("Z_point1", ui.curveZ->getPointOne() );
	iniFile.setValue ("Z_point2", ui.curveZ->getPointTwo() );
	iniFile.setValue ("Z_point3", ui.curveZ->getPointThree() );
	iniFile.setValue ("Z_point4", ui.curveZ->getPointFour() );
	
	iniFile.endGroup ();
	settingsDirty = false;

	//
	// Send a message to the main program, to update the Settings (for the tracker)
	//
	mainApp->updateSettings();
}

void getCurvePoints(QSettings *iniFile, QString prefix, QPointF *point1, QPointF *point2, QPointF *point3, QPointF *point4, int NeutralZone, int Sensitivity, int MaxInput, int MaxOutput) {
bool setMax;
float newMax;

	setMax = FALSE;
	newMax = MaxInput;

	//
	// If Point 1 exists, read it from the file.
	// If not: get the y-coord from the global (deprecated) NeutralZone setting.
	//
	if (iniFile->contains(prefix + "point1")) {
		*point1 = iniFile->value ( prefix + "point1", 0 ).toPoint();
	}
	else {
		point1->setY(NeutralZone);
	}

	//
	// If Point 4 exists, read it from the file.
	// If not: derive the x-coord from the (deprecated) 'Sensitivity' setting and set y to max.
	//
	if (iniFile->contains(prefix + "point4")) {
		*point4 = iniFile->value ( prefix + "point4", 0 ).toPoint();
	}
	else {
		point4->setY(MaxInput);									// Max. Input for rotations
		point4->setX((Sensitivity/100.0f) * MaxInput);
		if (point4->x() > MaxOutput) {
			point4->setX(MaxOutput);
			setMax = TRUE;
		}
		else {
			newMax = (Sensitivity/100.0f) * MaxInput;
		}
	}

	//
	// If Point 2 exists, read it from the file.
	// If not: derive it from the (deprecated) 'Sensitivity' setting.
	//
	if (iniFile->contains(prefix + "point2")) {
		*point2 = iniFile->value ( prefix + "point2", 0 ).toPoint();
	}
	else {
		point2->setY(0.333f * MaxInput);						// Set the Point at 1/3 of Max. Input
		if (!setMax) {
			point2->setX(0.333f * newMax);
		}
		else {
			point2->setX(0.333f * MaxOutput);
		}
	}

	//
	// If Point 3 exists, read it from the file.
	// If not: derive it from the (deprecated) 'Sensitivity' setting.
	//
	if (iniFile->contains(prefix + "point3")) {
		*point3 = iniFile->value ( prefix + "point3", 0 ).toPoint();
	}
	else {
		point3->setY(0.666f * MaxInput);						// Set the Point at 2/3 of Max. Input
		if (!setMax) {
			point3->setX(0.666f * newMax);
		}
		else {
			point3->setX(0.666f * MaxOutput);
		}
	}
}
