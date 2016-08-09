/* Copyright (c) 2013-2016, Stanislaw Halik <sthalik@misaki.pl>

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
#include <QEvent>

#include <vector>
#include <tuple>

#include "ui_main-window.h"

#include "opentrack-compat/options.hpp"
#include "opentrack-logic/main-settings.hpp"
#include "opentrack/plugin-support.hpp"
#include "opentrack-logic/tracker.h"
#include "opentrack-logic/shortcuts.h"
#include "opentrack-logic/work.hpp"
#include "opentrack-logic/state.hpp"
#include "mapping-window.hpp"
#include "options-dialog.hpp"
#include "process_detector.h"

using namespace options;

class MainWindow : public QMainWindow, private State
{
    Q_OBJECT

    Ui::main_window ui;

    Shortcuts global_shortcuts;
    module_settings m;
    mem<QSystemTrayIcon> tray;
    QTimer pose_update_timer;
    QTimer det_timer;
    QTimer config_list_timer;
    mem<OptionsDialog> options_widget;
    mem<MapWidget> mapping_widget;
    QShortcut kbd_quit;
    mem<IFilterDialog> pFilterDialog;
    mem<IProtocolDialog> pProtocolDialog;
    mem<ITrackerDialog> pTrackerDialog;
    process_detector_worker det;
    QMenu profile_menu;
    bool is_refreshing_profiles;

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

    void load_settings();
    void load_mappings();
    void updateButtonState(bool running, bool inertialp);
    void display_pose(const double* mapped, const double* raw);
    void ensure_tray();
    void set_title(const QString& game_title = QStringLiteral(""));
    static bool get_new_config_name_from_dialog(QString &ret);
    void set_profile(const QString& profile);
    void register_shortcuts();
    void set_keys_enabled(bool flag);

    void changeEvent(QEvent* e) override;
    bool maybe_hide_to_tray(QEvent* e);

private slots:
    void save_modules();
    void exit();
    void profile_selected(const QString& name);

    void showTrackerSettings();
    void showProtocolSettings();
    void showFilterSettings();
    void show_options_dialog();
    void showCurveConfiguration();
    void showHeadPose();

    void maybe_start_profile_from_executable();

    void make_empty_config();
    void make_copied_config();
    void open_config_directory();
    void refresh_config_list();

    void startTracker();
    void stopTracker();
    void reload_options();

    void toggle_restore_from_tray(QSystemTrayIcon::ActivationReason e);

signals:
    void emit_start_tracker();
    void emit_stop_tracker();
    void emit_toggle_tracker();
    void emit_restart_tracker();
public:
    MainWindow();
    ~MainWindow();
    static void set_working_directory();
    void warn_on_config_not_writable();
    bool is_tray_enabled();
};
