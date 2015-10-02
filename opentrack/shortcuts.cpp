/* Copyright (c) 2014, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "shortcuts.h"
#include <QMutexLocker>

#if defined(_WIN32)
#include <functional>
#include <windows.h>
#include "win32-shortcuts.h"

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
                    k.shift = !!(keystate[DIK_LSHIFT] & 0x80);
                    k.alt = !!(keystate[DIK_LALT] & 0x80);
                    k.ctrl = !!(keystate[DIK_LCONTROL] & 0x80);
                    k.keycode = i;
                    receiver(k);
                    break;
                }
            }
        }

        // keypresses get dropped with high values
        Sleep(15);
    }
}
#endif

void Shortcuts::bind_keyboard_shortcut(K &key, key_opts& k)
{
#if !defined(_WIN32)
    if (!key)
        key = std::make_shared<QxtGlobalShortcut>();
    else {
        key->setShortcut(QKeySequence::UnknownKey);
        key->setEnabled(false);
    }

    if (k.keycode != "")
    {
        key->setShortcut(QKeySequence::fromString(k.keycode, QKeySequence::PortableText));
        key->setEnabled();
    }
}
#else
    key = K();
    int idx = 0;
    QKeySequence code;

    if (k.keycode == "")
        code = QKeySequence(Qt::Key_unknown);
    else
        code = QKeySequence::fromString(k.keycode, QKeySequence::PortableText);

    Qt::KeyboardModifiers mods = Qt::NoModifier;
    if (code != Qt::Key_unknown)
        win_key::from_qt(code, idx, mods);
    key.shift = !!(mods & Qt::ShiftModifier);
    key.alt = !!(mods & Qt::AltModifier);
    key.ctrl = !!(mods & Qt::ControlModifier);
    key.keycode = idx;
}
#endif

#ifdef _WIN32
void Shortcuts::receiver(Key &k)
{
    std::vector<K*> ks { &keyCenter, &keyToggle, &keyZero };
    for (auto& k_ : ks)
    {
        if (k.keycode != k_->keycode)
            continue;
        if (!k_->should_process())
            return;
        if (k_->alt && !k.alt) return;
        if (k_->ctrl && !k.ctrl) return;
        if (k_->shift && !k.shift) return;

        if (k.keycode == keyCenter.keycode)
            emit center();
        else if (k.keycode == keyToggle.keycode)
            emit toggle();
        else if (k.keycode == keyZero.keycode)
            emit zero();
    }
}
#endif

void Shortcuts::reload() {
    bind_keyboard_shortcut(keyCenter, s.center);
    bind_keyboard_shortcut(keyToggle, s.toggle);
    bind_keyboard_shortcut(keyZero, s.zero);
#ifdef _WIN32
    bool is_new = keybindingWorker == nullptr;
    if (is_new)
    {
        keybindingWorker = std::make_shared<KeybindingWorker>([&](Key& k) { receiver(k); }, handle);
        keybindingWorker->start();
    }
#endif
}
