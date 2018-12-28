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
    Timer timer;
    int keycode = 0;
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
    bool held = true;
    bool enabled = true;
public:
    Key();

    bool should_process();
};

struct OTR_DINPUT_EXPORT KeybindingWorker : private QThread
{
    using fun = std::function<void(const Key&)>;

    KeybindingWorker(const KeybindingWorker&) = delete;
    KeybindingWorker& operator=(KeybindingWorker&) = delete;

private:
    LPDIRECTINPUTDEVICE8 dinkeyboard { nullptr };
    win32_joy_ctx joy_ctx;
    std::vector<std::unique_ptr<fun>> receivers;
    QMutex mtx;
    QMainWindow fake_main_window;
    di_t din;

    bool keystate[256] {};

    void run() override;
    bool run_keyboard_nolock();
    bool run_joystick_nolock();

    bool init();
    KeybindingWorker();

    static KeybindingWorker& make();
    fun* add_receiver(fun& receiver);
    void remove_receiver(fun* pos);
    ~KeybindingWorker() override;

    static constexpr int num_keyboard_states = 64;
    DIDEVICEOBJECTDATA keyboard_states[num_keyboard_states] {};
public:
    class Token
    {
        fun* pos;
    public:
        Token(const Token&) = delete;
        Token& operator=(Token&) = delete;

        ~Token()
        {
            make().remove_receiver(pos);
        }
        Token(fun receiver)
        {
            pos = make().add_receiver(receiver);
        }
    };
};
