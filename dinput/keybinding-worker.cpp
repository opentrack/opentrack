/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#ifdef _WIN32

#include "compat/sleep.hpp"
#include "compat/util.hpp"
#include "keybinding-worker.hpp"

#include <QDebug>
#include <QMutexLocker>

#include <windows.h>

Key::Key() {}

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

    {
        DIPROPDWORD dipdw;
        dipdw.dwData = 128;
        dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
        dipdw.diph.dwHow = DIPH_DEVICE;
        dipdw.diph.dwObj = 0;
        dipdw.diph.dwSize = sizeof(dipdw);
        if ( dinkeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph) != DI_OK)
        {
            qDebug() << "setup keyboard buffer mode failed!";
            dinkeyboard->Release();
            dinkeyboard = 0;
            return false;
        }
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
        start(QThread::HighPriority);
}

KeybindingWorker& KeybindingWorker::make()
{
    static KeybindingWorker k;
    return k;
}

void KeybindingWorker::run()
{
    while (!isInterruptionRequested())
    {
        {
            QMutexLocker l(&mtx);

            if (receivers.size())
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

                {
                    DWORD sz = num_keyboard_states;
                    const HRESULT hr = dinkeyboard->GetDeviceData(sizeof(*keyboard_states), keyboard_states, &sz, 0);

                    if (hr != DI_OK)
                    {
                        qDebug() << "Tracker::run GetDeviceData function failed!" << hr;
                        Sleep(25);
                        continue;
                    }
                    else
                    {
                        for (unsigned k = 0; k < sz; k++)
                        {
                            const unsigned idx = keyboard_states[k].dwOfs & 0xff; // defensive programming
                            const bool held = !!(keyboard_states[k].dwData & 0x80);

                            switch (idx)
                            {
                            case DIK_LCONTROL:
                            case DIK_LSHIFT:
                            case DIK_LALT:
                            case DIK_RCONTROL:
                            case DIK_RSHIFT:
                            case DIK_RALT:
                            case DIK_LWIN:
                            case DIK_RWIN:
                                break;
                            default:
                            {
                                Key k;
                                k.shift = keystate[DIK_LSHIFT] | keystate[DIK_RSHIFT];
                                k.alt = keystate[DIK_LALT] | keystate[DIK_RALT];
                                k.ctrl = keystate[DIK_LCONTROL] | keystate[DIK_RCONTROL];
                                k.keycode = idx;
                                k.held = held;

                                for (auto& r : receivers)
                                    (*r)(k);
                                break;
                            }
                            }
                            keystate[idx] = held;
                        }
                    }
                }

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

                        for (auto& r : receivers)
                            (*r)(k);
                    };

                    joy_ctx.poll(f);
                }
            }
        }

        portable::sleep(100);
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
