/* Copyright (c) 2014, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "shortcuts.h"
#include <QMutexLocker>

#if defined(_WIN32)
#include <windows.h>
#include "win32-shortcuts.h"

void KeybindingWorker::set_keys(Key kCenter_, Key kToggle_, Key kZero_)
{
    QMutexLocker l(&mtx);

    kCenter = kCenter_;
    kToggle = kToggle_;
    kZero = kZero_;
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

KeybindingWorker::KeybindingWorker(Key keyCenter, Key keyToggle, Key keyZero, WId handle, Shortcuts& sc) :
    sc(sc), din(0), dinkeyboard(0), kCenter(keyCenter), kToggle(keyToggle), kZero(keyZero), should_quit(true)
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

static bool isKeyPressed( const Key *key, const BYTE *keystate ) {
    bool shift;
    bool ctrl;
    bool alt;

    if (key->keycode != 0 && keystate[key->keycode] & 0x80)
    {
        shift = ( (keystate[DIK_LSHIFT] & 0x80) || (keystate[DIK_RSHIFT] & 0x80) );
        ctrl  = ( (keystate[DIK_LCONTROL] & 0x80) || (keystate[DIK_RCONTROL] & 0x80) );
        alt   = ( (keystate[DIK_LALT] & 0x80) || (keystate[DIK_RALT] & 0x80) );

        if (key->shift && !shift) return false;
        if (key->ctrl && !ctrl) return false;
        if (key->alt && !alt) return false;

        return true;
    }
    return false;
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

        if (isKeyPressed(&kCenter, keystate) && kCenter.should_process())
            emit sc.center();

        if (isKeyPressed(&kToggle, keystate) && kToggle.should_process())
            emit sc.toggle();

        if (isKeyPressed(&kZero, keystate) && kZero.should_process())
            emit sc.zero();

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
        key->setEnabled(false);
        key->setShortcut(QKeySequence::UnknownKey);
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

void Shortcuts::reload() {
#ifndef _WIN32
    if (keyCenter)
    {
        keyCenter->setShortcut(QKeySequence::UnknownKey);
        keyCenter->setEnabled(false);
    }
    if (keyToggle)
    {
        keyToggle->setShortcut(QKeySequence::UnknownKey);
        keyToggle->setEnabled(false);
    }
    if (keyZero)
    {
        keyZero->setShortcut(QKeySequence::UnknownKey);
        keyZero->setEnabled(false);
    }
#endif
    bind_keyboard_shortcut(keyCenter, s.center);
    bind_keyboard_shortcut(keyToggle, s.toggle);
    bind_keyboard_shortcut(keyZero, s.zero);
#ifdef _WIN32
    bool is_new = keybindingWorker == nullptr;
    if (is_new)
    {
        keybindingWorker = std::make_shared<KeybindingWorker>(keyCenter, keyToggle, keyZero, handle, *this);
        keybindingWorker->start();
    }
    else
        keybindingWorker->set_keys(keyCenter, keyToggle, keyZero);
#endif
}
