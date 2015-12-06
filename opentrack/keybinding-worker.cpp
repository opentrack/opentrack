/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#ifdef _WIN32

#include "keybinding-worker.hpp"
#include <functional>
#include <windows.h>
#include <QDebug>
#include <QMutexLocker>

bool Key::should_process()
{
    if (keycode == 0 && guid == "")
        return false;
    bool ret = timer.elapsed_ms() > 100;
    timer.start();
    return ret;
}

KeybindingWorker::~KeybindingWorker() {
    should_quit = true;
    wait();
    if (dinkeyboard) {
        dinkeyboard->Unacquire();
        dinkeyboard->Release();
    }
    if (din)
        din->Release();
}

KeybindingWorker::KeybindingWorker() :
    should_quit(true)
{
    if (DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&din, NULL) != DI_OK) {
        qDebug() << "setup DirectInput8 Creation failed!" << GetLastError();
        return;
    }
    if (din->CreateDevice(GUID_SysKeyboard, &dinkeyboard, NULL) != DI_OK) {
        din->Release();
        din = 0;
        qDebug() << "setup CreateDevice function failed!" << GetLastError();
        return;
    }
    if (dinkeyboard->SetDataFormat(&c_dfDIKeyboard) != DI_OK) {
        qDebug() << "setup SetDataFormat function failed!" << GetLastError();
        dinkeyboard->Release();
        dinkeyboard = 0;
        din->Release();
        din = 0;
        return;
    }
    
    if (dinkeyboard->SetCooperativeLevel((HWND) fake_main_window.winId(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK) {
        dinkeyboard->Release();
        din->Release();
        din = 0;
        dinkeyboard = 0;
        qDebug() << "setup SetCooperativeLevel function failed!" << GetLastError();
        return;
    }
    if (dinkeyboard->Acquire() != DI_OK)
    {
        dinkeyboard->Release();
        din->Release();
        din = 0;
        dinkeyboard = 0;
        qDebug() << "setup dinkeyboard Acquire failed!" << GetLastError();
        return;
    }
    should_quit = false;
    start();
}

KeybindingWorker& KeybindingWorker::make()
{
    static KeybindingWorker k;
    return k;
}

void KeybindingWorker::run() {
    BYTE keystate[256] = {0};
    BYTE old_keystate[256] = {0};

    while (!should_quit)
    {
        {
            QMutexLocker l(&mtx);

            if (receivers.size())
            {
                {
                    const HRESULT hr = dinkeyboard->GetDeviceState(256, (LPVOID)keystate);

                    if (hr != DI_OK) {
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
                            r->operator()(k);
                    };

                    joy_ctx.poll(f);
                }

                for (int i = 0; i < 256; i++)
                {
                    Key k;
                    if (old_keystate[i] != keystate[i] && keystate[i] & 0x80)
                    {
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
                            k.shift = !!(keystate[DIK_LSHIFT] & 0x80) || !!(keystate[DIK_RSHIFT] & 0x80);
                            k.alt = !!(keystate[DIK_LALT] & 0x80) || !!(keystate[DIK_RALT] & 0x80);
                            k.ctrl = !!(keystate[DIK_LCONTROL] & 0x80) || !!(keystate[DIK_RCONTROL] & 0x80);
                            k.keycode = i;

                            for (auto& r : receivers)
                                r->operator()(k);
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
    receivers.push_back(std::unique_ptr<fun>(new fun(receiver)));
    fun* f = receivers[receivers.size() - 1].get();
    qDebug() << "add receiver" << (long) f;
    return f;
}

void KeybindingWorker::remove_receiver(KeybindingWorker::fun* pos)
{
    QMutexLocker l(&mtx);
    bool ok = false;

    for (int i = receivers.size() - 1; i >= 0; i--)
    {
        if (receivers[i].get() == pos)
        {
            ok = true;
            qDebug() << "remove receiver" << (long) pos;
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
