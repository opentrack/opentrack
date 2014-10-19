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

#pragma once

#include <QMainWindow>
#include <QApplication>
#include <QWidget>
#include <QDialog>
#include <QUrl>
#include <QList>
#include <QKeySequence>
#include <QShortcut>
#include <QLayout>
#include <QPixmap>
#include <QLabel>
#include <QTimer>
#if !defined(_WIN32)
#	include "qxt-mini/QxtGlobalShortcut"
#else
#	include <windows.h>
#endif

#include "ui_facetracknoir.h"

#include "./options.h"
#include "./main-settings.hpp"
#include "./plugin-support.h"
#include "./tracker.h"
#include "./shortcuts.h"
#include "./curve-config.h"

using namespace options;

class FaceTrackNoIR : public QMainWindow
{
    Q_OBJECT

public:
    FaceTrackNoIR();
    ~FaceTrackNoIR();

    QFrame *video_frame();
    ptr<Tracker> tracker;
    void bindKeyboardShortcuts();

    // XXX this shit stinks -sh 20141004
    // TODO move to separate class representing running tracker state
    ptr<DynamicLibrary> current_tracker() {
        return dlopen_trackers.value(ui.iconcomboTrackerSource->currentIndex(), nullptr);
    }
    ptr<DynamicLibrary> current_protocol() {
        return dlopen_protocols.value(ui.iconcomboProtocol->currentIndex(), nullptr);
    }
    ptr<DynamicLibrary> current_filter() {
        return dlopen_filters.value(ui.iconcomboFilter->currentIndex(), nullptr);
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
    // XXX move the shit outta the _widget_, establish a class
    // for running tracker state, etc -sh 20141014
    Mappings pose;
    Ui::OpentrackUI ui;
    QTimer timUpdateHeadPose;
    
    SelectedLibraries libs;
    ptr<ITrackerDialog> pTrackerDialog;
    ptr<IProtocolDialog> pProtocolDialog;
    ptr<IFilterDialog> pFilterDialog;

    ptr<QWidget> shortcuts_widget;
    ptr<MapWidget> mapping_widget;

    void createIconGroupBox();
    void loadSettings();
    void updateButtonState(bool running, bool inertialp);

    QList<ptr<DynamicLibrary>> dlopen_filters;
    QList<ptr<DynamicLibrary>> dlopen_trackers;
    QList<ptr<DynamicLibrary>> dlopen_protocols;
    
    QShortcut kbd_quit;
    QPixmap no_feed_pixmap;
    
#ifndef _WIN32
    void bind_keyboard_shortcut(QxtGlobalShortcut&, key_opts& k);
#endif
    void fill_profile_cbx();

private slots:
    void open();
    void save();
    void saveAs();
    void exit();
    void profileSelected(int index);

    void showTrackerSettings();

    void showServerControls();
    void showFilterControls();
    void showKeyboardShortcuts();
    void showCurveConfiguration();

    void showHeadPose();

    void startTracker();
    void stopTracker();

public:
    void save_mappings();
    void load_mappings();
};
