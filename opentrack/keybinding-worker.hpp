/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#ifdef BUILD_api
#   include "opentrack-compat/export.hpp"
#else
#   include "opentrack-compat/import.hpp"
#endif

#include "opentrack-compat/timer.hpp"
#include <QThread>
#include <QMutex>
#include <QWidget>
#include <functional>

#ifdef _WIN32
#   undef DIRECTINPUT_VERSION
#   define DIRECTINPUT_VERSION 0x0800
#   include <windows.h>
#   include <dinput.h>
struct Key {
    BYTE keycode;
    bool shift;
    bool ctrl;
    bool alt;
    Timer timer;
public:
    Key() : keycode(0), shift(false), ctrl(false), alt(false)
    {
    }

    bool should_process()
    {
        if (keycode == 0)
            return false;
        bool ret = timer.elapsed_ms() > 100;
        timer.start();
        return ret;
    }
};
#else
typedef unsigned char BYTE;
struct Key { int foo; };
#endif

struct OPENTRACK_EXPORT KeybindingWorker : public QThread {
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