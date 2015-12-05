/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once
#include <QObject>
#include <tuple>
#include <vector>
#include <functional>

#ifdef BUILD_api
#   include "opentrack-compat/export.hpp"
#else
#   include "opentrack-compat/import.hpp"
#endif

#include "qxt-mini/QxtGlobalShortcut"
#include "opentrack-compat/options.hpp"
#include "opentrack/main-settings.hpp"

#ifdef _WIN32
#   include "keybinding-worker.hpp"
#endif

using namespace options;

struct OPENTRACK_EXPORT Shortcuts : public QObject {
    Q_OBJECT

public:
    using K =
#ifndef _WIN32
    mem<QxtGlobalShortcut>
#else
    Key
#endif
    ;
    
    using fun = std::function<void(void)>;
    using tt = std::tuple<K, fun>;
    std::vector<tt> keys;
#ifdef _WIN32
    KeybindingWorker::Token key_token;
#endif
    
    struct key_opts {
        value<QString> keycode, guid;
        value<int> button;
    
        key_opts(pbundle b, const QString& name) :
            keycode(b, QString("keycode-%1").arg(name), ""),
            guid(b, QString("guid-%1").arg(name), ""),
            button(b, QString("button-%1").arg(name), -1)
        {}
    };

    struct settings : opts {
        key_opts center, toggle, zero, start_tracking, stop_tracking, toggle_tracking;
        main_settings s_main;
        settings() :
            opts("keyboard-shortcuts"),
            center(b, "center"),
            toggle(b, "toggle"),
            zero(b, "zero"),
            start_tracking(b, "start-tracking"),
            stop_tracking(b, "stop-tracking"),
            toggle_tracking(b, "toggle-tracking")
        {}
    } s;

    Shortcuts() :
#ifdef _WIN32
        key_token([&](const Key& k) { receiver(k); })
#endif
    {}

    void reload(const std::vector<std::tuple<Shortcuts::key_opts &, Shortcuts::fun> > &keys);
private:
    void bind_keyboard_shortcut(K &key, const key_opts& k);
#ifdef _WIN32
    void receiver(const Key& k);
#endif
};
