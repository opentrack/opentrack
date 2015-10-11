/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <QMainWindow>
#include <QKeySequence>
#include <QShortcut>
#include <QPixmap>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QString>
#include <QMenu>

#include "ui_main.h"

#include "opentrack/options.hpp"
#include "opentrack/main-settings.hpp"
#include "opentrack/plugin-support.hpp"
#include "opentrack/tracker.h"
#include "opentrack/shortcuts.h"
#include "opentrack/work.hpp"
#include "opentrack/state.hpp"
#include "curve-config.h"
#include "options-dialog.hpp"
#include "process_detector.h"
#include "facetracknoir/software-update-dialog.hpp"

using namespace options;

class MainWindow : public QMainWindow, private State
{
    Q_OBJECT

    Ui::OpentrackUI ui;
    mem<QSystemTrayIcon> tray;
    QTimer pose_update_timer;
    QTimer det_timer;
    QTimer config_list_timer;
    mem<OptionsDialog> options_widget;
    mem<MapWidget> mapping_widget;
    QShortcut kbd_quit;
    QPixmap no_feed_pixmap;
    mem<IProtocolDialog> pProtocolDialog;
    process_detector_worker det;
    QMenu profile_menu;
    bool is_refreshing_profiles;
    QTimer save_timer;
    update_dialog::query update_query;

    mem<dylib> current_protocol()
    {
        return modules.protocols().value(ui.iconcomboProtocol->currentIndex(), nullptr);
    }

    void changeEvent(QEvent* e) override;

    void load_settings();
    void updateButtonState(bool running, bool inertialp);
    void display_pose(const double* mapped, const double* raw);
    void ensure_tray();
    void set_title(const QString& game_title = QStringLiteral(""));
    static bool get_new_config_name_from_dialog(QString &ret);
    void set_profile(const QString& profile);
    void maybe_save();
    bool maybe_not_close_tracking();
    void closeEvent(QCloseEvent *e) override;
private slots:
    void _save();
    void save();
    void exit();
    void profileSelected(QString name);

    void showProtocolSettings();
    void show_options_dialog();
    void showCurveConfiguration();
    void showHeadPose();

    void restore_from_tray(QSystemTrayIcon::ActivationReason);
    void maybe_start_profile_from_executable();

    void make_empty_config();
    void make_copied_config();
    void open_config_directory();
    void refresh_config_list();

    void startTracker();
    void stopTracker();
    void reload_options();
public slots:
    void shortcutRecentered();
    void shortcutToggled();
    void shortcutZeroed();
public:
    MainWindow();
    ~MainWindow();
    void save_mappings();
    void load_mappings();
    static void set_working_directory();
};
