/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "export.hpp"

#include "compat/timer.hpp"
#include "win32-joystick.hpp"
#include "dinput.hpp"
#include <QThread>
#include <QMutex>
#include <QWidget>
#include <QMainWindow>
#include <QDebug>
#include <functional>
#include <vector>

struct OTR_DINPUT_EXPORT Key
{
    QString guid;
    int keycode;
    bool shift;
    bool ctrl;
    bool alt;
    bool held;
    bool enabled;
    Timer timer;
public:
    Key() : keycode(0), shift(false), ctrl(false), alt(false), held(true), enabled(true) {}

    bool should_process();
};

struct OTR_DINPUT_EXPORT KeybindingWorker : private QThread
{
    using fun = std::function<void(const Key&)>;

private:
    LPDIRECTINPUTDEVICE8 dinkeyboard;
    win32_joy_ctx joy_ctx;
    std::vector<std::unique_ptr<fun>> receivers;
    QMutex mtx;
    QMainWindow fake_main_window;
    dinput_handle::di_t din;

    void run() override;
    bool init();
    KeybindingWorker();

    static KeybindingWorker& make();
    fun* _add_receiver(fun &receiver);
    void remove_receiver(fun* pos);
    ~KeybindingWorker();

    KeybindingWorker(const KeybindingWorker&) = delete;
    KeybindingWorker& operator=(KeybindingWorker&) = delete;
public:
    class Token
    {
        fun* pos;
        Token(const Token&) = delete;
        Token& operator=(Token&) = delete;
    public:
        ~Token()
        {
            make().remove_receiver(pos);
        }
        Token(fun receiver)
        {
            pos = make()._add_receiver(receiver);
        }
    };
};
