/* Copyright (c) 2013-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "api/plugin-support.hpp"
#include "mapping-window.hpp"
#include "options-dialog.hpp"
#include "process_detector.h"
#include "logic/main-settings.hpp"
#include "logic/tracker.h"
#include "logic/shortcuts.h"
#include "logic/work.hpp"
#include "logic/state.hpp"
#include "options/options.hpp"

#include <QObject>
#include <QWidget>
#include <QMainWindow>
#include <QKeySequence>
#include <QShortcut>
#include <QPixmap>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QString>
#include <QMenu>
#include <QAction>
#include <QEvent>
#include <QCloseEvent>

#include <vector>
#include <tuple>
#include <memory>

#include "ui_main-window.h"

using namespace options;

class MainWindow : public QMainWindow, private State
{
    Q_OBJECT

    Ui::main_window ui;

    Shortcuts global_shortcuts;
    module_settings m;
    ptr<QSystemTrayIcon> tray;
    QMenu tray_menu;
    QTimer pose_update_timer;
    QTimer det_timer;
    QTimer config_list_timer;
    ptr<OptionsDialog> options_widget;
    ptr<MapWidget> mapping_widget;
    QShortcut kbd_quit;
    ptr<IFilterDialog> pFilterDialog;
    ptr<IProtocolDialog> pProtocolDialog;
    ptr<ITrackerDialog> pTrackerDialog;
    process_detector_worker det;
    QMenu profile_menu;

    QAction menu_action_header, menu_action_show, menu_action_exit,
            menu_action_tracker, menu_action_filter, menu_action_proto,
            menu_action_options, menu_action_mappings;

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

    void updateButtonState(bool running, bool inertialp);
    void display_pose(const double* mapped, const double* raw);
    void ensure_tray();
    void set_title(const QString& game_title = QStringLiteral(""));
    static bool get_new_config_name_from_dialog(QString &ret);
    void set_profile_in_registry(const QString& profile);
    void register_shortcuts();
    void set_keys_enabled(bool flag);
    bool is_config_listed(const QString& name);

    void init_tray_menu();

    void changeEvent(QEvent* e) override;
    void closeEvent(QCloseEvent*) override;
    bool maybe_hide_to_tray(QEvent* e);

private slots:
    void save_modules();
    void exit();
    bool set_profile(const QString& new_name);

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
    bool refresh_config_list();

    void startTracker();
    void stopTracker();

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
    bool maybe_die_on_config_not_writable(const QString& current, QStringList* ini_list);
    void die_on_config_not_writable();
    bool is_tray_enabled();
    bool start_in_tray();
};
