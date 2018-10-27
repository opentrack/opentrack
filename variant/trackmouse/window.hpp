#pragma once

/* Copyright (c) 2013-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "ui_window.h"
#include "proto-mouse/mouse-settings.hpp"

#include "api/plugin-support.hpp"
#include "logic/main-settings.hpp"
#include "logic/pipeline.hpp"
#include "logic/shortcuts.h"
#include "logic/work.hpp"
#include "logic/state.hpp"
#include "options/options.hpp"

#include <tuple>
#include <memory>

#include <QMainWindow>
#include <QKeySequence>
#include <QShortcut>
#include <QPixmap>
#include <QTimer>
#include <QString>

class main_window final : public QMainWindow, private State
{
    Q_OBJECT

    Ui::window ui;

    QTimer save_settings_timer { this };

    Shortcuts global_shortcuts;
    module_settings m;
    mouse_settings mouse;

    QShortcut kbd_quit { QKeySequence("Ctrl+Q"), this };
    std::unique_ptr<IFilterDialog> pFilterDialog;
    std::unique_ptr<IProtocolDialog> pProtocolDialog;
    std::unique_ptr<ITrackerDialog> pTrackerDialog;
    bool exiting_already { false };

    using dylib_ptr = Modules::dylib_ptr;
    using dylib_list = Modules::dylib_list;

    static std::tuple<dylib_ptr, int> module_by_name(const QString& name, Modules::dylib_list& list);

    dylib_ptr current_tracker();
    dylib_ptr current_protocol();
    dylib_ptr current_filter();

    void update_button_state(bool running, bool inertialp);

    void set_title(const QString& game_title = QString());

    void set_profile_in_registry();
    void register_shortcuts();

    void closeEvent(QCloseEvent *event) override;

    bool maybe_die_on_config_not_writable(const QString& current);
    void die_on_config_not_writable();

    static constexpr inline int save_settings_interval_ms = 2500;

private slots:
    void save_modules();
    void exit(int status = EXIT_SUCCESS);
    bool set_profile();

    void start_tracker_();
    void stop_tracker_();
    void toggle_tracker_();

    static void set_working_directory();

signals:
    void start_tracker();
    void stop_tracker();

public:
    main_window();
    ~main_window() override;
};
