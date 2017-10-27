/* Copyright (c) 2013-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "api/plugin-support.hpp"
#include "mapping-dialog.hpp"
#include "settings.hpp"
#include "process_detector.h"
#include "logic/main-settings.hpp"
#include "logic/pipeline.hpp"
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
    std::unique_ptr<QSystemTrayIcon> tray;
    QMenu tray_menu;
    QTimer pose_update_timer;
    QTimer det_timer;
    QTimer config_list_timer;
    std::unique_ptr<OptionsDialog> options_widget;
    std::unique_ptr<MapWidget> mapping_widget;
    QShortcut kbd_quit;
    std::unique_ptr<IFilterDialog> pFilterDialog;
    std::unique_ptr<IProtocolDialog> pProtocolDialog;
    std::unique_ptr<ITrackerDialog> pTrackerDialog;

    process_detector_worker det;
    QMenu profile_menu;

    QAction menu_action_header, menu_action_show, menu_action_exit,
            menu_action_tracker, menu_action_filter, menu_action_proto,
            menu_action_options, menu_action_mappings;

    std::shared_ptr<dylib> current_tracker()
    {
        return modules.trackers().value(ui.iconcomboTrackerSource->currentIndex(), nullptr);
    }
    std::shared_ptr<dylib> current_protocol()
    {
        return modules.protocols().value(ui.iconcomboProtocol->currentIndex(), nullptr);
    }
    std::shared_ptr<dylib> current_filter()
    {
        return modules.filters().value(ui.iconcomboFilter->currentIndex(), nullptr);
    }

    void update_button_state(bool running, bool inertialp);
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
    bool event(QEvent *event) override;
    bool maybe_hide_to_tray(QEvent* e);
#if !defined _WIN32 && !defined __APPLE__
    void annoy_if_root();
#endif

    // only use in impl file since no definition in header!
    template<typename t>
    bool mk_dialog(std::shared_ptr<dylib> lib, std::unique_ptr<t>& d);

    // idem
    template<typename t, typename... Args>
    inline bool mk_window(std::unique_ptr<t>& place, Args&&... params);

    // idem
    template<typename t, typename F>
    bool mk_window_common(std::unique_ptr<t>& d, F&& ctor);

private slots:
    void save_modules();
    void exit();
    bool set_profile(const QString& new_name);

    void show_tracker_settings();
    void show_proto_settings();
    void show_filter_settings();
    void show_options_dialog();
    void show_mapping_window();
    void show_pose();

    void maybe_start_profile_from_executable();

    void make_empty_config();
    void make_copied_config();
    void open_config_directory();
    bool refresh_config_list();

    void start_tracker_();
    void stop_tracker_();

    void toggle_restore_from_tray(QSystemTrayIcon::ActivationReason e);

signals:
    void start_tracker();
    void stop_tracker();
    void toggle_tracker();
    void restart_tracker();
public:
    MainWindow();
    ~MainWindow();
    static void set_working_directory();
    bool maybe_die_on_config_not_writable(const QString& current, QStringList* ini_list);
    void die_on_config_not_writable();
    bool is_tray_enabled();
    bool start_in_tray();
};
