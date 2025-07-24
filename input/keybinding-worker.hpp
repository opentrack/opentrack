/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#ifdef _WIN32
#include "export.hpp"

#include "key.hpp"
#include "win32-joystick.hpp"
#ifdef OPENTRACK_HAS_GAMEINPUT
#include "gameinput.hpp"
#endif
#include "dinput.hpp"

#include <functional>
#include <vector>

#include <QThread>
#include <QMutex>
#include <QWidget>
#include <QMainWindow>
#include <QDebug>

struct OTR_INPUT_EXPORT KeybindingWorker : private QThread
{
    using fun = std::function<void(const Key&)>;

    KeybindingWorker(const KeybindingWorker&) = delete;
    KeybindingWorker& operator=(const KeybindingWorker&) = delete;

private:
    void run() override;
    bool run_keyboard_nolock();
    bool run_joystick_nolock();
    bool run_mouse_nolock();
    bool run_gamepad_nolock();
    void emit_key(const Key& k);

    bool init();
    bool init_(IDirectInputDevice8A*& dev, const char* name, const GUID& guid, const DIDATAFORMAT& fmt);
    KeybindingWorker();

    static KeybindingWorker& make();
    fun* add_receiver(fun& receiver);
    void remove_receiver(fun* pos);
    ~KeybindingWorker() override;

    static constexpr int num_keyboard_states = 128;
    static constexpr int num_mouse_buttons = 8;
    static constexpr int first_mouse_button = 1;

    IDirectInputDevice8A* dinkeyboard = nullptr, *dinmouse = nullptr;
    win32_joy_ctx joy_ctx;
    std::vector<std::unique_ptr<fun>> receivers;
    QMutex mtx;
    QMainWindow fake_main_window;
    di_t din;
    bool keystate[256] {};
    bool mouse_state[num_mouse_buttons - first_mouse_button] = {};
#if OPENTRACK_HAS_GAMEINPUT
    std::function<void(const Key&)> gamepad_fn{ [this](const Key& k) { emit_key(k); } };
    std::unique_ptr<opentrack::gameinput::Worker> gi{opentrack::gameinput::make_gameinput_worker()};
    opentrack::gameinput::Mods mods;
#endif

public:
    class Token
    {
        fun* pos;
    public:
        Token(const Token&) = delete;
        Token& operator=(const Token&) = delete;
        Token(Token&&) = delete;
        Token& operator=(Token&&) = delete;

        Token(fun receiver) { pos = make().add_receiver(receiver); }
        ~Token() { make().remove_receiver(pos); }
    };
};
#endif
