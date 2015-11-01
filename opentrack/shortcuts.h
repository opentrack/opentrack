/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once
#include <QObject>

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

    K keyCenter;
    K keyToggle;
    K keyZero;

    WId handle;
#ifdef _WIN32
    mem<KeybindingWorker> keybindingWorker;
#endif
    
    struct key_opts {
        value<QString> keycode;
    
        key_opts(pbundle b, const QString& name) :
            keycode(b, QString("keycode-%1").arg(name), "")
        {}
    };

    struct settings : opts {
        key_opts center, toggle, zero;
        main_settings s_main;
        settings() :
            opts("keyboard-shortcuts"),
            center(b, "center"),
            toggle(b, "toggle"),
            zero(b, "zero")
        {}
    } s;

    Shortcuts(WId handle) : handle(handle) { reload(); }

    void reload();
private:
    void bind_keyboard_shortcut(K &key, key_opts& k);
#ifdef _WIN32
    void receiver(Key& k);
#endif
signals:
    void center();
    void toggle();
    void zero();
};
