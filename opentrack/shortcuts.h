/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once
#include <QObject>
#include <QWidget>
#include <QElapsedTimer>
#include <QThread>
#include <QMessageBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSettings>
#include <QMutex>

#include "qxt-mini/QxtGlobalShortcut"
#include "opentrack/plugin-support.hpp"
#include "opentrack/options.hpp"
#include "opentrack/main-settings.hpp"

using namespace options;

extern QList<QString> global_key_sequences;

struct key_opts {
    value<QString> keycode;

    key_opts(pbundle b, const QString& name) :
        keycode(b, QString("keycode-%1").arg(name), "")
    {}
};

#if defined(_WIN32)
extern QList<int> global_windows_key_sequences;
#   undef DIRECTINPUT_VERSION
#   define DIRECTINPUT_VERSION 0x0800
#   include <windows.h>
#   include <dinput.h>

struct Key {
    BYTE keycode;
    bool shift;
    bool ctrl;
    bool alt;
    QElapsedTimer timer;
public:
    Key() : keycode(0), shift(false), ctrl(false), alt(false)
    {
    }

    bool should_process()
    {
        return !timer.isValid() ? (timer.start(), true) : timer.restart() > 100;
    }
};
#else
typedef unsigned char BYTE;
struct Key { int foo; };
#endif

struct Shortcuts;

struct KeybindingWorker : public QThread {
#ifdef _WIN32
private:
    LPDIRECTINPUT8 din;
    LPDIRECTINPUTDEVICE8 dinkeyboard;
    QMutex mtx;
public:
    volatile bool should_quit;
    std::function<void(Key&)> receiver;
    ~KeybindingWorker();
    KeybindingWorker(std::function<void(Key&)> receiver, WId h);
    void run();
#else
public:
    KeybindingWorker(Key, Key, Key, WId) {}
    void run() {}
#endif
};

struct Shortcuts : public QObject {
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


