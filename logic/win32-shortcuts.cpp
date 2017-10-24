/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#if defined(_WIN32)
#   undef DIRECTINPUT_VERSION
#   define DIRECTINPUT_VERSION 0x800
#   include <dinput.h>

#include "win32-shortcuts.h"
#include <QList>
#include <QVariant>
#include <QDebug>

QList<win_key> windows_key_mods {
        {DIK_LCONTROL, Qt::Key_Control},
        {DIK_RCONTROL, Qt::Key_Control},
        {DIK_LALT, Qt::Key_Alt},
        {DIK_RALT, Qt::Key_Alt},
        {DIK_LSHIFT, Qt::Key_Shift},
        {DIK_RSHIFT, Qt::Key_Shift},
        {DIK_LWIN, Qt::Key_Super_L},
        {DIK_RWIN, Qt::Key_Super_R},
};

QList<win_key> windows_key_sequences {
       { DIK_F1, Qt::Key_F1 },
       { DIK_F2, Qt::Key_F2 },
       { DIK_F3, Qt::Key_F3 },
       { DIK_F4, Qt::Key_F4 },
       { DIK_F5, Qt::Key_F5 },
       { DIK_F6, Qt::Key_F6 },
       { DIK_F7, Qt::Key_F7 },
       { DIK_F8, Qt::Key_F8 },
       { DIK_F9, Qt::Key_F9 },
       { DIK_F10, Qt::Key_F10 },
       { DIK_F11, Qt::Key_F11 },
       { DIK_F12, Qt::Key_F12 },
       { DIK_LEFT, Qt::Key_Left },
       { DIK_RIGHT, Qt::Key_Right },
       { DIK_UP, Qt::Key_Up },
       { DIK_DOWN, Qt::Key_Down },
       { DIK_PRIOR, Qt::Key_PageUp },
       { DIK_NEXT, Qt::Key_PageDown },
       { DIK_HOME, Qt::Key_Home },
       { DIK_END, Qt::Key_End },
       { DIK_BACK, Qt::Key_Backspace },
       { DIK_COMMA, Qt::Key_Comma },
       { DIK_PERIOD, Qt::Key_Period },
       { DIK_LBRACKET, Qt::Key_BracketLeft },
       { DIK_RBRACKET, Qt::Key_BracketRight },
       { DIK_SEMICOLON, Qt::Key_Semicolon },
       { DIK_SLASH, Qt::Key_Slash },
       { DIK_BACKSLASH, Qt::Key_Backslash },
       { DIK_BACKSPACE, Qt::Key_Backspace },
       { DIK_APOSTROPHE, Qt::Key_Apostrophe },
       { DIK_GRAVE, Qt::Key_QuoteLeft },
       { DIK_MINUS, Qt::Key_Minus },
       { DIK_EQUALS, Qt::Key_Equal },
       { DIK_PERIOD, Qt::Key_Period },
       { DIK_F1, Qt::Key_F1 },
       { DIK_F2, Qt::Key_F2 },
       { DIK_F3, Qt::Key_F3 },
       { DIK_F4, Qt::Key_F4 },
       { DIK_F5, Qt::Key_F5 },
       { DIK_F6, Qt::Key_F6 },
       { DIK_F7, Qt::Key_F7 },
       { DIK_F8, Qt::Key_F8 },
       { DIK_F9, Qt::Key_F9 },
       { DIK_F10, Qt::Key_F10 },
       { DIK_F11, Qt::Key_F11 },
       { DIK_F12, Qt::Key_F12 },
       { DIK_0, Qt::Key_0 },
       { DIK_1, Qt::Key_1 },
       { DIK_2, Qt::Key_2 },
       { DIK_3, Qt::Key_3 },
       { DIK_4, Qt::Key_4 },
       { DIK_5, Qt::Key_5 },
       { DIK_6, Qt::Key_6 },
       { DIK_7, Qt::Key_7 },
       { DIK_8, Qt::Key_8 },
       { DIK_9, Qt::Key_9 },
       { DIK_A, Qt::Key_A },
       { DIK_B, Qt::Key_B },
       { DIK_C, Qt::Key_C },
       { DIK_D, Qt::Key_D },
       { DIK_E, Qt::Key_E },
       { DIK_F, Qt::Key_F },
       { DIK_G, Qt::Key_G },
       { DIK_H, Qt::Key_H },
       { DIK_I, Qt::Key_I },
       { DIK_J, Qt::Key_J },
       { DIK_K, Qt::Key_K },
       { DIK_L, Qt::Key_L },
       { DIK_M, Qt::Key_M },
       { DIK_N, Qt::Key_N },
       { DIK_O, Qt::Key_O },
       { DIK_P, Qt::Key_P },
       { DIK_Q, Qt::Key_Q },
       { DIK_R, Qt::Key_R },
       { DIK_S, Qt::Key_S },
       { DIK_T, Qt::Key_T },
       { DIK_U, Qt::Key_U },
       { DIK_V, Qt::Key_V },
       { DIK_W, Qt::Key_W },
       { DIK_X, Qt::Key_X },
       { DIK_Y, Qt::Key_Y },
       { DIK_Z, Qt::Key_Z },
       { DIK_TAB, Qt::Key_Tab },
       { DIK_RETURN, Qt::Key_Return},
       { DIK_INSERT, Qt::Key_Insert},
       { DIK_DELETE, Qt::Key_Delete},
       { DIK_SPACE, Qt::Key_Space},
       { DIK_SYSRQ, Qt::Key_Print},
       { DIK_SCROLL, Qt::Key_ScrollLock},
       { DIK_PAUSE, Qt::Key_Pause},
       { DIK_NUMLOCK, Qt::Key_NumLock},
       { DIK_CAPSLOCK, Qt::Key_CapsLock},
#define mod(x, y) static_cast<Qt::Key>(x | y)
       { DIK_NUMPAD0,      mod(Qt::Key_0,        Qt::KeypadModifier)},
       { DIK_NUMPAD0,      mod(Qt::Key_0,        Qt::KeypadModifier)},
       { DIK_NUMPAD1,      mod(Qt::Key_1,        Qt::KeypadModifier)},
       { DIK_NUMPAD2,      mod(Qt::Key_2,        Qt::KeypadModifier)},
       { DIK_NUMPAD3,      mod(Qt::Key_3,        Qt::KeypadModifier)},
       { DIK_NUMPAD4,      mod(Qt::Key_4,        Qt::KeypadModifier)},
       { DIK_NUMPAD5,      mod(Qt::Key_5,        Qt::KeypadModifier)},
       { DIK_NUMPAD6,      mod(Qt::Key_6,        Qt::KeypadModifier)},
       { DIK_NUMPAD7,      mod(Qt::Key_7,        Qt::KeypadModifier)},
       { DIK_NUMPAD8,      mod(Qt::Key_8,        Qt::KeypadModifier)},
       { DIK_NUMPAD9,      mod(Qt::Key_9,        Qt::KeypadModifier)},
       { DIK_NUMPADCOMMA,  mod(Qt::Key_Comma,    Qt::KeypadModifier)},
       { DIK_NUMPADENTER,  mod(Qt::Key_Enter,    Qt::KeypadModifier)},
       { DIK_NUMPADEQUALS, mod(Qt::Key_Equal,    Qt::KeypadModifier)},
       { DIK_NUMPADMINUS,  mod(Qt::Key_Minus,    Qt::KeypadModifier)},
       { DIK_NUMPADPERIOD, mod(Qt::Key_Period,   Qt::KeypadModifier)},
       { DIK_NUMPADPLUS,   mod(Qt::Key_Plus,     Qt::KeypadModifier)},
       { DIK_NUMPADSLASH,  mod(Qt::Key_Slash,    Qt::KeypadModifier)},
       { DIK_NUMPADSTAR,   mod(Qt::Key_multiply, Qt::KeypadModifier)},
};

bool win_key::to_qt(const Key& k, QKeySequence& qt_, Qt::KeyboardModifiers &mods)
{
    for (auto& wk : windows_key_sequences)
    {
        if (wk.win == k.keycode)
        {
            qt_ = wk.qt;
            mods = Qt::NoModifier;
            if (k.ctrl) mods |= Qt::ControlModifier;
            if (k.shift) mods |= Qt::ShiftModifier;
            if (k.alt) mods |= Qt::AltModifier;
            return true;
        }
    }
    return false;
}

bool win_key::from_qt(QKeySequence qt_, int& dik, Qt::KeyboardModifiers& mods)
{
    // CAVEAT don't use QVariant::toUInt() or conversion fails
    const unsigned qt = static_cast<unsigned>(QVariant(qt_).toInt());
    const unsigned our_mods = unsigned(qt & Qt::KeyboardModifierMask);

    {
        for (const win_key& wk : windows_key_sequences)
        {
            if (unsigned(wk.qt) == qt)
            {
                dik = wk.win;
                mods = Qt::NoModifier;
                return true;
            }
        }
    }
    {
        const unsigned key = qt & ~Qt::KeyboardModifierMask;
        for (const win_key& wk : windows_key_sequences)
        {
            if (unsigned(wk.qt) == key)
            {
                dik = wk.win;
                mods = static_cast<Qt::KeyboardModifiers>(our_mods);
                return true;
            }
        }
    }
    return false;
}

#endif
