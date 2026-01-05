/* Copyright (c) 2013-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "opentrack/defs.hpp"
#include "api/plugin-support.hpp"
#include "gui/mapping-dialog.hpp"
#include "gui/options-dialog.hpp"
#include "gui/process_detector.h"
#include "logic/main-settings.hpp"
#include "logic/pipeline.hpp"
#include "input/shortcuts.h"
#include "logic/work.hpp"
#include "logic/state.hpp"
#include "options/options.hpp"
#include "compat/qt-signal.hpp"

#include <QMainWindow>
#include <QKeySequence>
#include <QShortcut>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QString>
#include <QMenu>
#include <QAction>
#include <QList>

#include <memory>

#include "ui_main-window.h"

class main_window final : public QMainWindow, private State
{
    Q_DECLARE_TR_FUNCTIONS(main_window)

    Ui::main_window ui;

    std::unique_ptr<QSystemTrayIcon> tray;
    QMenu tray_menu { this };

    QTimer pose_update_timer { this };
    QTimer det_timer;
    QTimer profile_list_timer;

    std::unique_ptr<Shortcuts> global_shortcuts;
    QShortcut kbd_quit { QKeySequence("Ctrl+Q"), this };

#ifdef UI_NO_VIDEO_FEED
    QWidget fake_video_frame_parent;
    QFrame fake_video_frame{&fake_video_frame_parent};
#endif

    std::unique_ptr<options_dialog> options_widget;
    std::unique_ptr<mapping_dialog> mapping_widget;

    std::unique_ptr<IFilterDialog> pFilterDialog;
    std::unique_ptr<IProtocolDialog> pProtocolDialog;
    std::unique_ptr<ITrackerDialog> pTrackerDialog;

    process_detector_worker det;
    QMenu profile_menu;

    QList<QString> profile_list;

    QAction menu_action_header   { &tray_menu },
            menu_action_show     { &tray_menu },
            menu_action_exit     { &tray_menu },
            menu_action_tracker  { &tray_menu },
            menu_action_filter   { &tray_menu },
            menu_action_proto    { &tray_menu },
            menu_action_options  { &tray_menu },
            menu_action_mappings { &tray_menu };

    bool exiting_already { false };

    qt_signal<void> start_tracker { this, &main_window::start_tracker_, Qt::QueuedConnection };
    qt_signal<void> stop_tracker { this, &main_window::stop_tracker_, Qt::QueuedConnection };
    qt_signal<void> toggle_tracker { this, &main_window::toggle_tracker_, Qt::QueuedConnection };
    qt_signal<void> restart_tracker { this, &main_window::restart_tracker_, Qt::QueuedConnection };

public:
    void init_dylibs();
    void init_tray_menu();
    void init_profiles();
    void init_buttons();

    void init_shortcuts();
    void register_shortcuts();
    void set_keys_enabled(bool flag);

    void update_button_state(bool running, bool inertialp);

#if !defined _WIN32
    void annoy_if_root();
#endif

    void changeEvent(QEvent* e) override;
    bool maybe_hide_to_tray(QEvent* e);
    void closeEvent(QCloseEvent *event) override;
    bool event(QEvent *event) override;

    void show_tracker_settings_(bool show);
    void show_proto_settings_(bool show);
    void show_filter_settings_(bool show);
    void show_tracker_settings() { show_tracker_settings_(true); }
    void show_proto_settings() { show_proto_settings_(true); }
    void show_filter_settings() { show_filter_settings_(true); }

    void show_options_dialog(bool show);
    void show_mapping_window();

    void show_pose();
    void show_pose_(const double* mapped, const double* raw);
    void set_title(const QString& game_title = QString());

    void start_tracker_();
    void stop_tracker_();
    void restart_tracker_();
    void toggle_tracker_();

    [[nodiscard]] bool create_profile_from_preset(const QString& name);
    void set_profile(const QString& new_name, bool migrate = true);
    void set_profile_in_registry(const QString& profile);
    void refresh_profile_list();
    void die_on_profile_not_writable();
    void maybe_start_profile_from_executable();
    [[nodiscard]] static bool profile_name_from_dialog(QString& ret);
    void copy_presets();

    void create_empty_profile();
    void create_copied_profile();
    void open_profile_directory();

    void ensure_tray();
    void toggle_restore_from_tray(QSystemTrayIcon::ActivationReason e);
    bool tray_enabled();
    bool start_in_tray();

    void save_modules();
    bool module_tabs_enabled() const;

    void exit(int status = EXIT_SUCCESS);

public:
    main_window();
    ~main_window() override;
};
