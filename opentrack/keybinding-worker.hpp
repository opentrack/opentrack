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
#include "opentrack/win32-joystick.hpp"
#include <QThread>
#include <QMutex>
#include <QWidget>
#include <functional>
#include <vector>

#ifdef _WIN32
#   undef DIRECTINPUT_VERSION
#   define DIRECTINPUT_VERSION 0x0800
#   include <windows.h>
#   include <dinput.h>
struct Key {
    BYTE keycode;
    QString guid;
    bool shift;
    bool ctrl;
    bool alt;
    bool held;
    Timer timer;
public:
    Key() : keycode(0), shift(false), ctrl(false), alt(false), held(true)
    {
    }

    bool should_process()
    {
        if (keycode == 0 && guid == "")
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

struct OPENTRACK_EXPORT KeybindingWorker : private QThread
{
private:
    LPDIRECTINPUT8 din;
    LPDIRECTINPUTDEVICE8 dinkeyboard;
    win32_joy_ctx& joy_ctx;
    volatile bool should_quit;
    using fun = std::function<void(Key&)>;
    std::vector<fun> receivers;
    QMutex mtx;
    
    void run() override;
    KeybindingWorker();
    
    KeybindingWorker(const KeybindingWorker&) = delete;
    KeybindingWorker& operator=(KeybindingWorker&) = delete;
    static KeybindingWorker& make();
    fun* _add_receiver(fun receiver);
    void remove_receiver(fun* pos);
    ~KeybindingWorker();
public:
    class Token
    {
        friend class KeybindingWorker;
        fun* pos;
        //Token(const Token&) = delete;
        Token& operator=(Token&) = delete;
    public:
        Token(fun receiver)
        {
            pos = make()._add_receiver(receiver);
        }
        ~Token()
        {
            make().remove_receiver(pos);
        }
    };
    friend class Token;
    static Token add_receiver(fun receiver)
    {
        return Token(receiver);
    }
};
