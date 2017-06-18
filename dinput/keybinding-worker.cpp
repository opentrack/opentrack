/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#ifdef _WIN32

#include "keybinding-worker.hpp"
#include "compat/util.hpp"
#include <functional>
#include <windows.h>
#include <QDebug>
#include <QMutexLocker>

bool Key::should_process()
{
    if (!enabled || (keycode == 0 && guid == ""))
        return false;
    bool ret = prog1(!held || timer.elapsed_ms() > 100,
                     timer.start());
    return ret;
}

KeybindingWorker::~KeybindingWorker()
{
    qDebug() << "exit: keybinding worker";

    requestInterruption();
    wait();
    if (dinkeyboard) {
        dinkeyboard->Unacquire();
        dinkeyboard->Release();
    }
}

bool KeybindingWorker::init()
{
    if (!din)
    {
        qDebug() << "can't create dinput handle";
        return false;
    }

    if (din->CreateDevice(GUID_SysKeyboard, &dinkeyboard, NULL) != DI_OK) {
        qDebug() << "setup CreateDevice function failed!" << GetLastError();
        return false;
    }

    if (dinkeyboard->SetDataFormat(&c_dfDIKeyboard) != DI_OK) {
        qDebug() << "setup SetDataFormat function failed!" << GetLastError();
        dinkeyboard->Release();
        dinkeyboard = 0;
        return false;
    }

    if (dinkeyboard->SetCooperativeLevel((HWND) fake_main_window.winId(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK) {
        dinkeyboard->Release();
        dinkeyboard = 0;
        qDebug() << "setup SetCooperativeLevel function failed!" << GetLastError();
        return false;
    }

    if (dinkeyboard->Acquire() != DI_OK)
    {
        dinkeyboard->Release();
        dinkeyboard = 0;
        qDebug() << "setup dinkeyboard Acquire failed!" << GetLastError();
        return false;
    }

    return true;
}

KeybindingWorker::KeybindingWorker() : dinkeyboard(nullptr), din(dinput_handle::make_di())
{
    if (init())
        start();
}

KeybindingWorker& KeybindingWorker::make()
{
    static KeybindingWorker k;
    return k;
}

void KeybindingWorker::run()
{
    unsigned char keystate[256] = {0};
    unsigned char old_keystate[256] = {0};

    while (!isInterruptionRequested())
    {
        {
            QMutexLocker l(&mtx);

            if (receivers.size())
            {
                {
                    const HRESULT hr = dinkeyboard->GetDeviceState(256, (LPVOID)keystate);

                    if (hr != DI_OK)
                    {
                        qDebug() << "Tracker::run GetDeviceState function failed!" << GetLastError();
                        Sleep(25);
                        continue;
                    }
                }

                {
                    using joy_fn = std::function<void(const QString& guid, int idx, bool held)>;

                    joy_fn f = [&](const QString& guid, int idx, bool held) -> void {
                        Key k;
                        k.keycode = idx;
                        k.shift = !!(keystate[DIK_LSHIFT] & 0x80 || keystate[DIK_RSHIFT] & 0x80);
                        k.alt = !!(keystate[DIK_LALT] & 0x80 || keystate[DIK_RALT] & 0x80);
                        k.ctrl = !!(keystate[DIK_LCONTROL] & 0x80 || keystate[DIK_RCONTROL] & 0x80);
                        k.guid = guid;
                        k.held = held;

                        for (auto& r : receivers)
                            (*r)(k);
                    };

                    joy_ctx.poll(f);
                }

                for (int i = 0; i < 256; i++)
                {
                    Key k;
                    if (old_keystate[i] != keystate[i])
                    {
                        const bool held = !!(keystate[i] & 0x80);
                        switch (i)
                        {
                        case DIK_LCONTROL:
                        case DIK_LSHIFT:
                        case DIK_LALT:
                        case DIK_RCONTROL:
                        case DIK_RSHIFT:
                        case DIK_RALT:
                            break;
                        default:
                            k.shift = !!((keystate[DIK_LSHIFT] & 0x80) || (keystate[DIK_RSHIFT] & 0x80));
                            k.alt = !!((keystate[DIK_LALT] & 0x80) || (keystate[DIK_RALT] & 0x80));
                            k.ctrl = !!((keystate[DIK_LCONTROL] & 0x80) || (keystate[DIK_RCONTROL] & 0x80));
                            k.keycode = i;
                            k.held = held;

                            for (auto& r : receivers)
                                (*r)(k);
                            break;
                        }
                    }
                    old_keystate[i] = keystate[i];
                }
            }
        }

        // keypresses get dropped with high values
        Sleep(4);
    }
}

KeybindingWorker::fun* KeybindingWorker::_add_receiver(fun& receiver)
{
    QMutexLocker l(&mtx);
    receivers.push_back(std::make_unique<fun>(receiver));
    fun* f = receivers[receivers.size() - 1].get();
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
        if (receivers[u(i)].get() == pos)
        {
            ok = true;
            //qDebug() << "remove receiver" << (long) pos;
            receivers.erase(receivers.begin() + i);
            break;
        }
    }
    if (!ok)
    {
        qDebug() << "bad remove receiver" << (long) pos;
    }
}

#endif
