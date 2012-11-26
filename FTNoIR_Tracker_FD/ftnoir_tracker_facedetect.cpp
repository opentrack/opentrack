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

VideoWidget::VideoWidget(HANDLE hMutex, unsigned char* data, struct face_detect_shm* shm) {
	this->hMutex = hMutex;
	this->data = data;
	this->shm = shm;
}

void VideoWidget::paintEvent(QPaintEvent*) {
	WaitForSingleObject(hMutex, INFINITE);
	if (!this->shm->settings.widgetp) {
		ReleaseMutex(hMutex);
		return;
	}
	QPainter painter(this);
	QImage image(data, FD_VIDEO_WIDTH, FD_VIDEO_HEIGHT, QImage::Format_RGB888);
	QRectF rect(0, 0, FD_VIDEO_WIDTH, FD_VIDEO_HEIGHT);
	painter.paintEngine()->drawImage(rect, image.rgbSwapped(), rect);
	ReleaseMutex(hMutex);
}

FTNoIR_Tracker::FTNoIR_Tracker()
{
	qDebug("making tracker FaceDetect");

	hMutex = CreateMutex(NULL, false, fd_mutex_name);
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct face_detect_shm), fd_shm_name);
	shm = (struct face_detect_shm*) MapViewOfFile(hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(struct face_detect_shm));
	memset(shm, 0, sizeof(struct face_detect_shm));
	activep = 0;
	procInfo.hProcess = INVALID_HANDLE_VALUE;
	ctrl = NULL;
	qframe = NULL;
}

void FTNoIR_Tracker::TerminateTracker() {
	if (procInfo.hProcess != INVALID_HANDLE_VALUE) {
		shm->settings.quit = 1;
		//TerminateProcess(procInfo.hProcess, 42);
		CloseHandle(procInfo.hProcess);
		CloseHandle(procInfo.hThread);
		procInfo.hProcess = INVALID_HANDLE_VALUE;
	}
}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
	WaitForSingleObject(hMutex, INFINITE);
	TerminateTracker();
	UnmapViewOfFile(shm);
	//CloseHandle(hMapFile);
	ReleaseMutex(hMutex);
	//CloseHandle(hMutex);
}

void FTNoIR_Tracker::Initialize( QFrame *videoframe )
{
	qDebug("FTNoIR_Tracker::Initialize()");
	WaitForSingleObject(hMutex, INFINITE);
	videoframe->setAttribute(Qt::WA_NativeWindow);
	videoframe->show();
	ctrl = new VideoWidget(hMutex, shm->pixels, shm);
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(ctrl);
	videoframe->setLayout(layout);
	ctrl->resize(FD_VIDEO_WIDTH, FD_VIDEO_HEIGHT);
	qframe = videoframe;
	loadSettings();
	ReleaseMutex(hMutex);
}

void FTNoIR_Tracker::refreshVideo() {
	QWidget* w;
	WaitForSingleObject(hMutex, INFINITE);
	w = ctrl;
	ReleaseMutex(hMutex);
	if (w != NULL)
		w->update();
}

void FTNoIR_Tracker::StartTracker( HWND parent_window )
{
	WaitForSingleObject(hMutex, INFINITE);
	qDebug("* tracker starting");
	activep = true;
	ReleaseMutex(hMutex);
}

void FTNoIR_Tracker::StopTracker( bool exit )
{
	WaitForSingleObject(hMutex, INFINITE);
	qDebug("* tracker stopping");
	activep = false;
	if (exit) {
		TerminateTracker();
		if (qframe && qframe->layout()) {
			delete qframe->layout();
			qframe = NULL;
		}
		if (ctrl) {
			delete ctrl;
			ctrl = NULL;
		}
	}
	ReleaseMutex(hMutex);
}

bool FTNoIR_Tracker::notifyZeroed() {
	qDebug("notifying of zero");
	WaitForSingleObject(hMutex, INFINITE);
	shm->zerop = 1;
	ReleaseMutex(hMutex);
	return true;
}

bool FTNoIR_Tracker::GiveHeadPoseData(THeadPoseData *data)
{
	WaitForSingleObject(hMutex, INFINITE);
	if (procInfo.hProcess == INVALID_HANDLE_VALUE) {
		STARTUPINFO si;
		SECURITY_ATTRIBUTES sa;
		sa.bInheritHandle = 1;
		sa.lpSecurityDescriptor = NULL;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		memset(&si, 0, sizeof(STARTUPINFO));
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.cb = sizeof(STARTUPINFO);
		si.hStdOutput = NULL;
		si.hStdError = NULL;
		si.hStdInput = NULL;
		if (!CreateProcess(prog_cmdline, NULL, NULL, NULL, true, 0, NULL, NULL, &si, &procInfo)) {
			qDebug("Badness! %d", GetLastError());
		}
	}

	shm->received = 1;

	if (activep) {
		shm->settings.newOutput = 0;
		data->x = shm->data[3];
		data->y = shm->data[4];
		data->z = shm->data[5];
		data->yaw = shm->data[0];
		data->pitch = shm->data[1];
		data->roll = shm->data[2];
		ReleaseMutex(hMutex);
		return true;
	}
	ReleaseMutex(hMutex);
	return false;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Tracker::loadSettings() {
	load_settings(&shm->settings);
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker object.

// Export both decorated and undecorated names.
//   GetTracker     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetTracker@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetTracker=_GetTracker@0")

FTNOIR_TRACKER_BASE_EXPORT ITrackerPtr __stdcall GetTracker()
{
	return new FTNoIR_Tracker;
}
