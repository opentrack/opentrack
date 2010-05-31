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

using namespace sm::faceapi;
using namespace sm::faceapi::qt;

//
// Setup the Main Dialog
//
FaceTrackNoIR::FaceTrackNoIR(QWidget *parent, Qt::WFlags flags) : 
QMainWindow(parent, flags)
{	
	cameraDetected = false;
	_engine_controls = 0;
	tracker = 0;
	_display = 0;
	l = 0;

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

	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));

	connect(ui.actionVideoWidget, SIGNAL(triggered()), this, SLOT(showVideoWidget()));
	connect(ui.actionHeadPoseWidget, SIGNAL(triggered()), this, SLOT(showHeadPoseWidget()));
	connect(ui.btnShowEngineControls, SIGNAL(clicked()), this, SLOT(showEngineControls()));

	// button methods connect with methods in this class
	connect(ui.btnStartTracker, SIGNAL(clicked()), this, SLOT(startTracker()));
	connect(ui.btnStopTracker, SIGNAL(clicked()), this, SLOT(stopTracker()));

	// Connect sliders for sensitivity
	connect(ui.sensYaw, SIGNAL(valueChanged(int)), this, SLOT(setSensYaw(int)));
	connect(ui.sensRoll, SIGNAL(valueChanged(int)), this, SLOT(setSensRoll(int)));
	connect(ui.sensPitch, SIGNAL(valueChanged(int)), this, SLOT(setSensPitch(int)));
	connect(ui.sensX, SIGNAL(valueChanged(int)), this, SLOT(setSensX(int)));
	connect(ui.sensY, SIGNAL(valueChanged(int)), this, SLOT(setSensY(int)));
	connect(ui.sensZ, SIGNAL(valueChanged(int)), this, SLOT(setSensZ(int)));

	connect(ui.chkInvertYaw, SIGNAL(stateChanged(int)), this, SLOT(setInvertYaw(int)));
	connect(ui.chkInvertRoll, SIGNAL(stateChanged(int)), this, SLOT(setInvertRoll(int)));
	connect(ui.chkInvertPitch, SIGNAL(stateChanged(int)), this, SLOT(setInvertPitch(int)));
	connect(ui.chkInvertX, SIGNAL(stateChanged(int)), this, SLOT(setInvertX(int)));
	connect(ui.chkInvertY, SIGNAL(stateChanged(int)), this, SLOT(setInvertY(int)));
	connect(ui.chkInvertZ, SIGNAL(stateChanged(int)), this, SLOT(setInvertZ(int)));

	connect(ui.slideNeutralZone, SIGNAL(valueChanged(int)), this, SLOT(setNeutralZone(int)));

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
	iniFile.setValue ( "NeutralZone", ui.slideNeutralZone->value() );
	iniFile.setValue ( "sensYaw", ui.sensYaw->value() );
	iniFile.setValue ( "sensPitch", ui.sensPitch->value() );
	iniFile.setValue ( "sensRoll", ui.sensRoll->value() );
	iniFile.setValue ( "sensX", ui.sensX->value() );
	iniFile.setValue ( "sensY", ui.sensY->value() );
	iniFile.setValue ( "sensZ", ui.sensZ->value() );
	iniFile.setValue ( "invertYaw", ui.chkInvertYaw->isChecked() );
	iniFile.setValue ( "invertPitch", ui.chkInvertPitch->isChecked() );
	iniFile.setValue ( "invertRoll", ui.chkInvertRoll->isChecked() );
	iniFile.setValue ( "invertX", ui.chkInvertX->isChecked() );
	iniFile.setValue ( "invertY", ui.chkInvertY->isChecked() );
	iniFile.setValue ( "invertZ", ui.chkInvertZ->isChecked() );
	iniFile.endGroup ();

	iniFile.beginGroup ( "GameProtocol" );
	iniFile.setValue ( "Selection", ui.iconcomboBox->currentIndex() );
	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Get the new name of the INI-file and save the settings to it.
//
void FaceTrackNoIR::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"),
													QCoreApplication::applicationDirPath() + "/Settings",
													tr("Settings file (*.ini);;All Files (*)"));
	if (!fileName.isEmpty()) {
		QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)
		settings.setValue ("SettingsFile", fileName);
		save();

		// Put the filename in the window-title
	    QFileInfo pathInfo ( fileName );
		setWindowTitle ( "FaceTrackNoIR - " + pathInfo.fileName() );
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
	ui.slideNeutralZone->setValue (iniFile.value ( "NeutralZone", 5 ).toInt());
	ui.sensYaw->setValue (iniFile.value ( "sensYaw", 100 ).toInt());
	ui.sensPitch->setValue (iniFile.value ( "sensPitch", 100 ).toInt());
	ui.sensRoll->setValue (iniFile.value ( "sensRoll", 100 ).toInt());
	ui.sensX->setValue (iniFile.value ( "sensX", 100 ).toInt());
	ui.sensY->setValue (iniFile.value ( "sensY", 100 ).toInt());
	ui.sensZ->setValue (iniFile.value ( "sensZ", 100 ).toInt());
	ui.chkInvertYaw->setChecked (iniFile.value ( "invertYaw", 0 ).toBool());
	ui.chkInvertPitch->setChecked (iniFile.value ( "invertPitch", 0 ).toBool());
	ui.chkInvertRoll->setChecked (iniFile.value ( "invertRoll", 0 ).toBool());
	ui.chkInvertX->setChecked (iniFile.value ( "invertX", 0 ).toBool());
	ui.chkInvertY->setChecked (iniFile.value ( "invertY", 0 ).toBool());
	ui.chkInvertZ->setChecked (iniFile.value ( "invertZ", 0 ).toBool());
	iniFile.endGroup ();

	iniFile.beginGroup ( "GameProtocol" );
	ui.iconcomboBox->setCurrentIndex(iniFile.value ( "Selection", 0 ).toInt());
	iniFile.endGroup ();

	settingsDirty = false;

	// Put the filename in the window-title
    QFileInfo pathInfo ( currentFile );
    setWindowTitle ( "FaceTrackNoIR - " + pathInfo.fileName() );

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
	tracker = new Tracker;

	// Show the video widget
	ui.video_frame->show();
	_display = new VideoDisplayWidget( tracker->getEngine(), ui.video_frame );
	l = new QVBoxLayout(ui.video_frame);
	l->setMargin(0);
	l->setSpacing(0);
	l->addWidget(_display);

	tracker->setup( ui.headPoseWidget , this);
	tracker->setSmoothing ( ui.slideSmoothing->value() );

	ui.headPoseWidget->show();

	ui.btnStartTracker->setEnabled ( false );
	ui.btnStopTracker->setEnabled ( true );
	ui.btnShowEngineControls->setEnabled ( true );
	ui.iconcomboBox->setEnabled ( false );

	//
	// Update the camera-name, FaceAPI can only use the 1st one found!
	//
	GetCameraNameDX();
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
}

/** set the sensibility from the slider **/
void FaceTrackNoIR::setSensYaw( int sens ) {
	Tracker::setSensYaw ( sens );
	settingsDirty = true;
}

/** set the sensibility from the slider **/
void FaceTrackNoIR::setSensPitch( int sens ) {
	Tracker::setSensPitch ( sens );
	settingsDirty = true;
}

/** set the sensibility from the slider **/
void FaceTrackNoIR::setSensRoll( int sens ) {
	Tracker::setSensRoll ( sens );
	settingsDirty = true;
}

/** set the sensibility from the slider **/
void FaceTrackNoIR::setSensX( int sens ) {
	Tracker::setSensX ( sens );
	settingsDirty = true;
}

/** set the sensibility from the slider **/
void FaceTrackNoIR::setSensY( int sens ) {
	Tracker::setSensY ( sens );
	settingsDirty = true;
}

/** set the sensibility from the slider **/
void FaceTrackNoIR::setSensZ( int sens ) {
	Tracker::setSensZ ( sens );
	settingsDirty = true;
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

/** set the Neutral Zone for rotations from the slider **/
void FaceTrackNoIR::setNeutralZone( int angle ) {
	Tracker::setNeutralZone ( angle );
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
    if (!_engine_controls)
    {
		qDebug() << "showEngineControls says: No engine_controls yet!";
        _engine_controls = new EngineControls( tracker->getEngine(), true, false, this, Qt::Dialog );
		qDebug() << "showEngineControls says: After new!";
    }

	if (_engine_controls) {
		qDebug() << "showEngineControls says: Before show!";
		_engine_controls->show();
		_engine_controls->raise();
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
	ui.iconcomboBox->addItem(QIcon("images/Freetrack.ico"), tr("Freetrack"));
	ui.iconcomboBox->addItem(QIcon("images/FlightGear.ico"), tr("FlightGear"));
	ui.iconcomboBox->addItem(QIcon("images/FaceTrackNoIR.ico"), tr("FTNoir client"));

	ui.iconcomboTrackerSource->addItem(QIcon("images/SeeingMachines.ico"), tr("Face API"));
	ui.iconcomboTrackerSource->addItem(QIcon("images/FaceTrackNoIR.ico"), tr("FTNoir server"));
}

//
// Create the Actions in the System tray and connect them to Application events
//
void FaceTrackNoIR::createActions()
{
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    maximizeAction = new QAction(tr("Ma&ximize"), this);
    connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

//
// Create the SystemTray and set the default Icon
//
void FaceTrackNoIR::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    trayIcon->setIcon(QIcon("images/FaceTrackNoIR.ico"));
}

//
// Set the Tray icon, using the Game-protocol combobox as source
//
void FaceTrackNoIR::setIcon(int index)
{
    QIcon icon = ui.iconcomboBox->itemIcon(index);
    trayIcon->setIcon(icon);
    setWindowIcon(QIcon("images/FaceTrackNoIR.ico"));

    trayIcon->setToolTip(ui.iconcomboBox->itemText(index));
	settingsDirty = true;
}
 
//
// Handle SystemTray events
//
void FaceTrackNoIR::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
     switch (reason) {
     case QSystemTrayIcon::Trigger:
     case QSystemTrayIcon::DoubleClick:
         ui.iconcomboBox->setCurrentIndex((ui.iconcomboBox->currentIndex() + 1)
                                       % ui.iconcomboBox->count());
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
