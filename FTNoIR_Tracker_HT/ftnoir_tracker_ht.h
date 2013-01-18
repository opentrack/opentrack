/* Copyright (c) 2013 Stanis³aw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_HT_H
#define FTNOIR_TRACKER_HT_H

#include "stdafx.h"
#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#include "headtracker-ftnoir.h"
#include "ui_TrackerControls.h"

class VideoWidget : public QWidget
{
	Q_OBJECT
public:
	VideoWidget(QWidget* parent);
	~VideoWidget();
	ht_video_t videoFrame;
	HANDLE hMutex;
protected:
	void paintEvent(QPaintEvent* e);
private:
	QTimer timer;
};

class Tracker : public ITracker
{
public:
	Tracker();
	~Tracker();
	void Initialize(QFrame *videoframe);
	void StartTracker(HWND parent_window);
	void StopTracker(bool exit);
	bool GiveHeadPoseData(THeadPoseData *data);
	bool enableTX, enableTY, enableTZ, enableRX, enableRY, enableRZ;
	ht_shm_t* shm;
private:
	HANDLE hMutex, hMapFile;
	bool paused;
	QProcess subprocess;
	VideoWidget* videoWidget;
	QHBoxLayout* layout;
};

// Widget that has controls for FTNoIR protocol client-settings.
class TrackerControls : public QWidget, public ITrackerDialog
{
    Q_OBJECT
public:

	explicit TrackerControls();
    virtual ~TrackerControls();
	void showEvent ( QShowEvent * event );

    void Initialize(QWidget *parent);
	void registerTracker(ITracker *tracker) {};
	void unRegisterTracker() {};

private:
	Ui::Form ui;
	void loadSettings();
	void save();
	bool settingsDirty;

private slots:
	void doOK();
	void doCancel();
	void settingChanged() { settingsDirty = true; };
	void settingChanged(int) { settingsDirty = true; };
	void settingChanged(double) { settingsDirty = true; };
};

#endif

