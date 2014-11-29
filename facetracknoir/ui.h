/*******************************************************************************
* MainWindow         This program is a private project of the some enthusiastic
*                                       gamers from Holland, who don't like to pay much for
*                                       head-tracking.
*
* Copyright (C) 2010    Wim Vriend (Developing)
*                                               Ron Hendriks (Researching and Testing)
*
* Homepage
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 3 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, see <http://www.gnu.org/licenses/>.
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
#include <QSystemTrayIcon>

#if !defined(_WIN32)
#       include "qxt-mini/QxtGlobalShortcut"
#else
#       include <windows.h>
#endif

#include "ui_main.h"

#include "opentrack/options.hpp"
#include "opentrack/main-settings.hpp"
#include "opentrack/plugin-support.h"
#include "opentrack/tracker.h"
#include "opentrack/shortcuts.h"
#include "opentrack/work.hpp"
#include "opentrack/state.hpp"
#include "curve-config.h"

using namespace options;

class MainWindow : public QMainWindow, private State
{
    Q_OBJECT

    Ui::OpentrackUI ui;
    mem<QSystemTrayIcon> tray;
    QTimer pose_update_timer;
    mem<KeyboardShortcutDialog> shortcuts_widget;
    mem<MapWidget> mapping_widget;
    QShortcut kbd_quit;
    QPixmap no_feed_pixmap;
    mem<IFilterDialog> pFilterDialog;
    mem<IProtocolDialog> pProtocolDialog;
    mem<ITrackerDialog> pTrackerDialog;

    mem<dylib> current_tracker()
    {
        return modules.trackers().value(ui.iconcomboTrackerSource->currentIndex(), nullptr);
    }
    mem<dylib> current_protocol()
    {
        return modules.protocols().value(ui.iconcomboProtocol->currentIndex(), nullptr);
    }
    mem<dylib> current_filter()
    {
        return modules.filters().value(ui.iconcomboFilter->currentIndex(), nullptr);
    }

    void changeEvent(QEvent* e) override;

    void createIconGroupBox();
    void load_settings();
    void updateButtonState(bool running, bool inertialp);
    void fill_profile_combobox();
    void display_pose(const double* mapped, const double* raw);
    void ensure_tray();
public slots:
    void shortcutRecentered();
    void shortcutToggled();
    void shortcutZeroed();
    void bindKeyboardShortcuts();
private slots:
    void open();
    void save();
    void saveAs();
    void exit();
    void profileSelected(int index);

    void showTrackerSettings();
    void showProtocolSettings();
    void showFilterSettings();
    void showKeyboardShortcuts();
    void showCurveConfiguration();
    void showHeadPose();

    void startTracker();
    void stopTracker();

    void restore_from_tray(QSystemTrayIcon::ActivationReason);
public:
    MainWindow();
    ~MainWindow();
    void save_mappings();
    void load_mappings();
};
