/* Copyright (c) 2012 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_fd.h"
#include <Qt>
#include <QPainter>
#include <QPaintEngine>

//*******************************************************************************************************
// faceDetect Settings-dialog.
//*******************************************************************************************************

static void load_settings(struct face_detect_settings* out) {
	qDebug("[!] load_settings()");
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "FaceDetectTracker" );
	out->redetect_ms = iniFile.value("RedetectMs", 500).toInt();
	out->camera_id = iniFile.value("CameraId", 0).toInt();
	out->quit = 0;
	out->newOutput = 0;
	out->magic = FD_MAGIC;
	out->widgetp = iniFile.value("VideoWidget", true).toBool();
	iniFile.endGroup ();
}

static void save_settings(const struct face_detect_settings* in) {

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "FaceDetectTracker" );
	iniFile.setValue("RedetectMs", in->redetect_ms);
	iniFile.setValue("CameraId", in->camera_id);
	iniFile.setValue("VideoWidget", in->widgetp);
	iniFile.endGroup ();
}


//
// Constructor for server-settings-dialog
//
TrackerControls::TrackerControls() :
QWidget()
{
	qDebug("[!] TrackerControls::TrackerControls()");
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct face_detect_shm), fd_shm_name);
	shm = (struct face_detect_shm*) MapViewOfFile(hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(struct face_detect_shm));
	hMutex = CreateMutex(NULL, false, fd_mutex_name);
	ui.setupUi( this );

	load_settings(&shm->settings);

	ui.redetect_ms->setValue(shm->settings.redetect_ms);
	ui.cameraId->setValue(shm->settings.camera_id);
	ui.videoWidget->setChecked(shm->settings.widgetp);

	settingsDirty = false;

	// what a load of boilerplate...
	QObject::connect(ui.okButton, SIGNAL(clicked()), this, SLOT(doOK()));
	QObject::connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(doCancel()));
	QObject::connect(ui.redetect_ms, SIGNAL(valueChanged(int)), this, SLOT(doSetRedetectMs(int)));
	QObject::connect(ui.cameraId, SIGNAL(valueChanged(int)), this, SLOT(doSetCameraId(int)));
	QObject::connect(ui.videoWidget, SIGNAL(toggled(bool)), this, SLOT(doSetVideoWidget(bool)));

	//populate the description strings
	trackerFullName = "faceDetect V1.0.0";
	trackerShortName = "faceDetect";
	trackerDescription = "Stans' faceDetect V1.0.0";

}

void TrackerControls::save() {
	save_settings(&shm->settings);
	settingsDirty = false;
}

void TrackerControls::doSetCameraId(int val) {
	settingsDirty = true;
	WaitForSingleObject(hMutex, INFINITE);
	shm->settings.camera_id = val;
	ReleaseMutex(hMutex);
}

void TrackerControls::doSetVideoWidget(bool val) {
	settingsDirty = true;
	WaitForSingleObject(hMutex, INFINITE);
	shm->settings.widgetp = val;
	ReleaseMutex(hMutex);
}

void TrackerControls::doSetRedetectMs(int val) {
	settingsDirty = true;
	WaitForSingleObject(hMutex, INFINITE);
	shm->settings.redetect_ms = val;
	ReleaseMutex(hMutex);
}

//
// Destructor for server-dialog
//
TrackerControls::~TrackerControls() {
	UnmapViewOfFile(shm);
	//CloseHandle(hMutex);
	//CloseHandle(hMapFile);
}

void TrackerControls::Release()
{
    delete this;
}

//
// Initialize tracker-client-dialog
//
void TrackerControls::Initialize(QWidget *parent) {

	QPoint offsetpos(200, 200);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}
	show();
}

//
// OK clicked on server-dialog
//
void TrackerControls::doOK() {
	save();
	this->close();
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

void TrackerControls::getFullName(QString *strToBeFilled)
{
	*strToBeFilled = trackerFullName;
};


void TrackerControls::getShortName(QString *strToBeFilled)
{
	*strToBeFilled = trackerShortName;
};


void TrackerControls::getDescription(QString *strToBeFilled)
{
	*strToBeFilled = trackerDescription;
};

void TrackerControls::getIcon(QIcon *icon)
{
	*icon = QIcon(":/images/FaceDetect.ico");
};

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker-settings dialog object.

// Export both decorated and undecorated names.
//   GetTrackerDialog     - Undecorated name, which can be easily used with GetProcAddress
//                          Win32 API function.
//   _GetTrackerDialog@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetTrackerDialog=_GetTrackerDialog@0")

FTNOIR_TRACKER_BASE_EXPORT TRACKERDIALOGHANDLE __stdcall GetTrackerDialog( )
{
	return new TrackerControls;
}
