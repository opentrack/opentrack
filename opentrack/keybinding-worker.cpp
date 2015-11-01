/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "keybinding-worker.hpp"

#if defined(_WIN32)
#   include <functional>
#   include <windows.h>
#   include <QDebug>

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

KeybindingWorker::KeybindingWorker(std::function<void(Key&)> receiver, WId h) :
    should_quit(true), receiver(receiver)
{
    HWND handle = reinterpret_cast<HWND>(h);

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
    if (dinkeyboard->SetCooperativeLevel((HWND) handle, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK) {
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
}

void KeybindingWorker::run() {
    BYTE keystate[256];

    while (!should_quit)
    {
        if (dinkeyboard->GetDeviceState(256, (LPVOID)keystate) != DI_OK) {
            qDebug() << "Tracker::run GetDeviceState function failed!" << GetLastError();
            Sleep(25);
            continue;
        }

        QMutexLocker l(&mtx);

        for (int i = 0; i < 256; i++)
        {
            Key k;
            if (keystate[i] & 0x80)
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
                    receiver(k);
                    break;
                }
            }
        }

        // keypresses get dropped with high values
        Sleep(4);
    }
}
#endif