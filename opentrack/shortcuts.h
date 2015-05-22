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
#include "opentrack/plugin-support.h"
#include "opentrack/options.hpp"
#include "opentrack/main-settings.hpp"

using namespace options;

extern QList<QString> global_key_sequences;

struct key_opts {
    value<int> key_index;
    value<bool> ctrl, alt, shift;

    key_opts(pbundle b, const QString& name) :
        key_index(b,  QString("key-index-%1").arg(name), 0),
        ctrl(b,  QString("key-ctrl-%1").arg(name), 0),
        alt(b,  QString("key-alt-%1").arg(name), 0),
        shift(b,  QString("key-shift-%1").arg(name), 0)
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
    Shortcuts& sc;
    LPDIRECTINPUT8 din;
    LPDIRECTINPUTDEVICE8 dinkeyboard;
    Key kCenter;
    Key kToggle;
    Key kZero;
    QMutex mtx;
public:
    volatile bool should_quit;
    ~KeybindingWorker();
    KeybindingWorker(Key keyCenter, Key keyToggle, Key keyZero, WId handle, Shortcuts& sc);
    void run();
    void set_keys(Key kCenter, Key kToggle, Key kZero);
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
signals:
    void center();
    void toggle();
    void zero();
};


