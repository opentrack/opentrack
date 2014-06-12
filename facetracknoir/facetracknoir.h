/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
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

#ifndef FaceTrackNoIR_H
#define FaceTrackNoIR_H

#include <QMainWindow>
#include <QApplication>
#include <QFileDialog>
#include <QListView>
#include <QPainter>
#include <QWidget>
#include <QDialog>
#include <QUrl>
#include <QList>
#include <QKeySequence>
#include <QtGui>
#include <QString>
#include <QByteArray>
#include <QShortcut>
#include <vector>
#if !defined(_WIN32)
#	include "qxt-mini/QxtGlobalShortcut"
#else
#	include <windows.h>
#endif
#include <QThread>
#include <QDebug>

#include "ui_facetracknoir.h"

#include "facetracknoir/options.h"
using namespace options;

#include "facetracknoir/main-settings.hpp"

#include "global-settings.h"
#include "tracker.h"
#include "facetracknoir/shortcuts.h"

#include "ftnoir_protocol_base/ftnoir_protocol_base.h"
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ftnoir_filter_base/ftnoir_filter_base.h"

#include "opentrack-version.h"

class Tracker;				// pre-define class to avoid circular includes
class FaceTrackNoIR;

class KeybindingWorker;

class FaceTrackNoIR : public QMainWindow, IDynamicLibraryProvider
{
	Q_OBJECT

public:
    FaceTrackNoIR(QWidget *parent = 0);
	~FaceTrackNoIR();

    QFrame *get_video_widget();					// Get a pointer to the video-widget, to use in the DLL
    Tracker *tracker;
    void bindKeyboardShortcuts();
    DynamicLibrary* current_tracker1() {
        return dlopen_trackers.value(ui.iconcomboTrackerSource->currentIndex(), (DynamicLibrary*) NULL);
    }
    DynamicLibrary* current_tracker2() {
        return dlopen_trackers.value(ui.cbxSecondTrackerSource->currentIndex() - 1, (DynamicLibrary*) NULL);
    }
    DynamicLibrary* current_protocol() {
        return dlopen_protocols.value(ui.iconcomboProtocol->currentIndex(), (DynamicLibrary*) NULL);
    }
    DynamicLibrary* current_filter() {
        return dlopen_filters.value(ui.iconcomboFilter->currentIndex(), (DynamicLibrary*) NULL);
    }
    THeadPoseDOF& axis(int idx) {
        return *pose.axes[idx];
    }

#if defined(_WIN32)
    Key keyCenter;
    Key keyToggle;
    KeybindingWorker* keybindingWorker;
#else 
    QxtGlobalShortcut keyCenter;
    QxtGlobalShortcut keyToggle;
#endif
    pbundle b;
    main_settings s;
public slots:
    void shortcutRecentered();
    void shortcutToggled();

private:
    HeadPoseData pose;
    Ui::OpentrackUI ui;
	QTimer timUpdateHeadPose;					// Timer to display headpose

    ITrackerDialog* pTrackerDialog;			// Pointer to Tracker dialog instance (in DLL)
    ITrackerDialog* pSecondTrackerDialog;		// Pointer to the second Tracker dialog instance (in DLL)
    IProtocolDialog* pProtocolDialog;			// Pointer to Protocol dialog instance (in DLL)
    IFilterDialog* pFilterDialog;				// Pointer to Filter dialog instance (in DLL)

	QWidget *_keyboard_shortcuts;
	QWidget *_curve_config;

	void createIconGroupBox();

	void GetCameraNameDX();
	void loadSettings();
    void updateButtonState(bool);

    QList<DynamicLibrary*> dlopen_filters;
    QList<DynamicLibrary*> dlopen_trackers;
    QList<DynamicLibrary*> dlopen_protocols;
    QShortcut kbd_quit;

#ifndef _WIN32
    void bind_keyboard_shortcut(QxtGlobalShortcut&, key_opts& k);
#endif
    void fill_profile_cbx();
    bool looping;
    
private slots:
    void open();
    void save();
    void saveAs();
    void exit();
    void profileSelected(int index);
    
    void showTrackerSettings();
    void showSecondTrackerSettings();
    
    void showServerControls();
    void showFilterControls();
    void showKeyboardShortcuts();
    void showCurveConfiguration();
    
    void showHeadPose();
    
    void startTracker();
    void stopTracker();
};

#endif // FaceTrackNoIR_H
