/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#ifdef _WIN32

#include "keybinding-worker.hpp"
#include "compat/macros.h"
#include "compat/thread-name.hpp"

#include <cstdint>
#include <QDebug>
#include <QMutexLocker>

#include <dinput.h>

using std::intptr_t;

static void destroy(IDirectInputDevice8A*& dev)
{
    if (dev)
        dev->Release();
    dev = nullptr;
}

KeybindingWorker::~KeybindingWorker()
{
    requestInterruption();
    wait();

    destroy(dinkeyboard);
    destroy(dinmouse);
}

bool KeybindingWorker::init_(IDirectInputDevice8A*& dev, const char* name, const GUID& guid, const DIDATAFORMAT& fmt)
{
    if (dev)
        return true;

    if (!din)
    {
        qDebug() << "dinput: can't create dinput handle";
        goto fail;
    }

    if (auto hr = din->CreateDevice(guid, &dev, nullptr); hr != DI_OK)
    {
        qDebug() << "dinput: create" << name << "failed" << (void*)(intptr_t)hr;
        goto fail;
    }

    if (auto hr = dev->SetDataFormat(&fmt); hr != DI_OK)
    {
        qDebug() << "dinput:" << name << "SetDataFormat" << (void*)(intptr_t)hr;
        goto fail;
    }

    if (auto hr = dev->SetCooperativeLevel((HWND) fake_main_window.winId(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
        hr != DI_OK)
    {
        qDebug() << "dinput:" << name << "SetCooperativeLevel" << (void*)(intptr_t)hr;
        goto fail;
    }

    return true;
fail:
    destroy(dev);
    return false;
}

bool KeybindingWorker::init()
{
    bool ret = init_(dinkeyboard, "keyboard", GUID_SysKeyboard, c_dfDIKeyboard) &&
               init_(dinmouse, "mouse", GUID_SysMouse, c_dfDIMouse2);

    if (!ret)
        goto fail;

    {
        DIPROPDWORD dipdw;
        dipdw.dwData = num_keyboard_states;
        dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
        dipdw.diph.dwHow = DIPH_DEVICE;
        dipdw.diph.dwObj = 0;
        dipdw.diph.dwSize = sizeof(dipdw);

        if (auto hr = dinkeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph); hr != DI_OK)
        {
            qDebug() << "dinput: keyboard DIPROP_BUFFERSIZE" << (void*)(intptr_t)hr;
            goto fail;
        }
    }

    return true;

fail:
    destroy(dinkeyboard);
    destroy(dinmouse);
    return false;
}

KeybindingWorker::KeybindingWorker()
{
    fake_main_window.setAttribute(Qt::WA_NativeWindow);

    if (init())
        start(QThread::HighPriority);
}

KeybindingWorker& KeybindingWorker::make()
{
    static KeybindingWorker k;
    return k;
}

void KeybindingWorker::run()
{
    portable::set_curthread_name("keybinding worker");

    while (!isInterruptionRequested())
    {
        {
            QMutexLocker l(&mtx);

            if (!receivers.empty())
            {
                bool ok = true;

                ok &= run_keyboard_nolock();
                ok &= run_mouse_nolock();
                ok &= run_joystick_nolock();
                ok &= run_gamepad_nolock();

                if (!ok)
                    Sleep(500);
            }
        }

        Sleep(20);
    }
}

void KeybindingWorker::emit_key(const Key& k)
{
    for (auto& r : receivers)
        (*r)(k);
}

bool KeybindingWorker::run_gamepad_nolock()
{
#if OPENTRACK_HAS_GAMEINPUT
    if (gi)
        gi->poll(gamepad_fn, mods);
#endif

    return true;
}

bool KeybindingWorker::run_mouse_nolock()
{
    DIMOUSESTATE2 state;

    if (!di_t::poll_device(dinmouse))
        eval_once(qDebug() << "dinput: mouse poll failed");

    if (auto hr = dinmouse->GetDeviceState(sizeof(state), &state); hr != DI_OK)
    {
        eval_once(qDebug() << "dinput: mouse GetDeviceState failed" << (void*)(intptr_t)hr << GetLastError());
        return false;
    }

    Key k;
    k.guid = QStringLiteral("mouse");

    for (int i = first_mouse_button; i < num_mouse_buttons; i++)
    {
        const bool new_state = state.rgbButtons[i] & 0x80;
        k.held = new_state;
        k.keycode = i;
        bool& old_state = mouse_state[i - first_mouse_button];
        if (old_state != new_state)
        {
            emit_key(k);
        }
        old_state = new_state;
    }
    return true;
}

bool KeybindingWorker::run_keyboard_nolock()
{
    /* There are some problems reported on various forums
     * with regard to key-up events. But that's what I dug up:
     *
     * https://www.gamedev.net/forums/topic/633011-keyboard-getdevicedata-buffered-never-releases-keys/
     *
     * "Over in the xna forums (http://xboxforums.create.msdn.com/forums/p/108722/642144.aspx#642144)
     *  we discovered this behavior is caused by calling Unacquire in your event processing loop.
     *  Funnily enough only the keyboard seems to be affected."
     *
     * Key-up events work on my end.
     */

    if (!di_t::poll_device(dinkeyboard))
        eval_once(qDebug() << "dinput: keyboard poll failed");

    DIDEVICEOBJECTDATA keyboard_states[num_keyboard_states];

    DWORD sz = num_keyboard_states;
    HRESULT hr = dinkeyboard->GetDeviceData(sizeof(*keyboard_states), keyboard_states, &sz, 0);

    if (FAILED(hr))
    {
        eval_once(qDebug() << "dinput: keyboard GetDeviceData failed" << (void*)(intptr_t)hr);
        return false;
    }

#if OPENTRACK_HAS_GAMEINPUT
    mods = {
        .ctrl  = (keystate[DIK_LCONTROL] | keystate[DIK_RCONTROL]) != 0,
        .alt   = (keystate[DIK_LALT] | keystate[DIK_RALT]) != 0,
        .shift = (keystate[DIK_LSHIFT] | keystate[DIK_RSHIFT]) != 0,
    };
#endif

    for (unsigned k = 0; k < sz; k++)
    {
        const int idx = keyboard_states[k].dwOfs & 0xff; // defensive programming
        const bool held = !!(keyboard_states[k].dwData & 0x80);

        if (held == keystate[idx])
            continue;
        keystate[idx] = held;

        switch (idx)
        {
        case DIK_LCONTROL:
        case DIK_RCONTROL:
        case DIK_LSHIFT:
        case DIK_RSHIFT:
        case DIK_LALT:
        case DIK_RALT:
        case DIK_LWIN:
        case DIK_RWIN:
            break;
        default: {
            Key key;
            key.held = held;
            key.shift = keystate[DIK_LSHIFT] | keystate[DIK_RSHIFT];
            key.alt = keystate[DIK_LALT] | keystate[DIK_RALT];
            key.ctrl = keystate[DIK_LCONTROL] | keystate[DIK_RCONTROL];
            key.keycode = idx;
            emit_key(key);
            //qDebug() << "KeybindingWorker: key from dinput" << (held ? "+" : "-") << key.keycode;
            break;
        }
        }
    }

    return true;
}

bool KeybindingWorker::run_joystick_nolock()
{
    using joy_fn = std::function<void(const QString& guid, int idx, bool held)>;

    joy_fn f = [&](const QString& guid, int idx, bool held) {
        Key k;
        k.keycode = idx;
        k.shift = keystate[DIK_LSHIFT] | keystate[DIK_RSHIFT];
        k.alt = keystate[DIK_LALT] | keystate[DIK_RALT];
        k.ctrl = keystate[DIK_LCONTROL] | keystate[DIK_RCONTROL];
        k.guid = guid;
        k.held = held;
        emit_key(k);
    };

    joy_ctx.poll(f);

    return true;
}

KeybindingWorker::fun* KeybindingWorker::add_receiver(fun& receiver)
{
    QMutexLocker l(&mtx);
    receivers.push_back(std::make_unique<fun>(receiver));
    fun* f = &*receivers[receivers.size() - 1];
    //qDebug() << "add receiver" << (long) f;
    joy_ctx.refresh();
    return f;
}

void KeybindingWorker::remove_receiver(KeybindingWorker::fun* pos)
{
    QMutexLocker l(&mtx);
    bool ok = false;

    using s = int;

    for (int i = s(receivers.size()) - 1; i >= 0; i--)
    {
        using u = unsigned;
        if (&*receivers[u(i)] == pos)
        {
            ok = true;
            //qDebug() << "remove receiver" << (long) pos;
            receivers.erase(receivers.begin() + i);
            break;
        }
    }
    if (!ok)
    {
        qDebug() << "bad remove receiver" << (void*) pos;
    }
}

#endif
