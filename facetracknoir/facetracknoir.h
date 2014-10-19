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

#include "opentrack/options.hpp"
#include "opentrack/main-settings.hpp"
#include "opentrack/plugin-support.h"
#include "opentrack/tracker.h"
#include "./shortcuts.h"
#include "./curve-config.h"

using namespace options;

class FaceTrackNoIR : public QMainWindow
{
    Q_OBJECT
public:
    pbundle b;
    main_settings s;
    ptr<Tracker> tracker;
        
    // XXX move kbd handling into class its own -sh 20141019
#ifndef _WIN32
    void bind_keyboard_shortcut(QxtGlobalShortcut&, key_opts& k);
#endif
private:
#if defined(_WIN32)
    Key keyCenter;
    Key keyToggle;
    KeybindingWorker* keybindingWorker;
#else
    QxtGlobalShortcut keyCenter;
    QxtGlobalShortcut keyToggle;
#endif
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
    
    QShortcut kbd_quit;
    QPixmap no_feed_pixmap;
    
    QList<ptr<dylib>> filter_modules;
    QList<ptr<dylib>> tracker_modules;
    QList<ptr<dylib>> protocol_modules;
    
    QList<ptr<dylib>> module_list;
    
    // XXX this shit stinks -sh 20141004
    // TODO move to separate class representing running tracker state
    ptr<dylib> current_tracker()
    {
        return tracker_modules.value(ui.iconcomboTrackerSource->currentIndex(), nullptr);
    }
    ptr<dylib> current_protocol()
    {
        return protocol_modules.value(ui.iconcomboProtocol->currentIndex(), nullptr);
    }
    ptr<dylib> current_filter()
    {
        return filter_modules.value(ui.iconcomboFilter->currentIndex(), nullptr);
    }

    void createIconGroupBox();
    void loadSettings();
    void updateButtonState(bool running, bool inertialp);

    void fill_combobox(dylib::Type t, QList<ptr<dylib>> list, QList<ptr<dylib> > &out_list, QComboBox* cbx);
    void fill_profile_combobox();
    
    QFrame *video_frame();
public slots:
    void shortcutRecentered();
    void shortcutToggled();
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
    FaceTrackNoIR();
    ~FaceTrackNoIR();
    void save_mappings();
    void load_mappings();
    void bindKeyboardShortcuts();
};
