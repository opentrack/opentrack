/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#if defined(_WIN32)
#   ifndef DIRECTINPUT_VERSION
#       define DIRECTINPUT_VERSION 0x800
#   endif
#   include <dinput.h>

#include "win32-shortcuts.h"
#include <QList>
#include <QVariant>
#include <QDebug>

QList<win_key> windows_key_mods =
    QList<win_key>({
        win_key(DIK_LCONTROL, Qt::Key::Key_Control),
        win_key(DIK_RCONTROL, Qt::Key::Key_Control),
        win_key(DIK_LALT, Qt::Key::Key_Alt),
        win_key(DIK_RALT, Qt::Key::Key_Alt),
        win_key(DIK_LSHIFT, Qt::Key::Key_Shift),
        win_key(DIK_RSHIFT, Qt::Key::Key_Shift),
        win_key(DIK_LWIN, Qt::Key::Key_unknown),
        win_key(DIK_RWIN, Qt::Key::Key_unknown)
    });

QList<win_key> windows_key_sequences =
    QList<win_key>({
       win_key(DIK_F1, Qt::Key::Key_F1 ),
       win_key(DIK_F2, Qt::Key::Key_F2 ),
       win_key(DIK_F3, Qt::Key::Key_F3 ),
       win_key(DIK_F4, Qt::Key::Key_F4 ),
       win_key(DIK_F5, Qt::Key::Key_F5 ),
       win_key(DIK_F6, Qt::Key::Key_F6 ),
       win_key(DIK_F7, Qt::Key::Key_F7 ),
       win_key(DIK_F8, Qt::Key::Key_F8 ),
       win_key(DIK_F9, Qt::Key::Key_F9 ),
       win_key(DIK_F10, Qt::Key::Key_F10 ),
       win_key(DIK_F11, Qt::Key::Key_F11 ),
       win_key(DIK_F12, Qt::Key::Key_F12 ),
       win_key(DIK_LEFT, Qt::Key::Key_Left ),
       win_key(DIK_RIGHT, Qt::Key::Key_Right ),
       win_key(DIK_UP, Qt::Key::Key_Up ),
       win_key(DIK_DOWN, Qt::Key::Key_Down ),
       win_key(DIK_PRIOR, Qt::Key::Key_PageUp ),
       win_key(DIK_NEXT, Qt::Key::Key_PageDown ),
       win_key(DIK_HOME, Qt::Key::Key_Home ),
       win_key(DIK_END, Qt::Key::Key_End ),
       win_key(DIK_BACK, Qt::Key::Key_Backspace ),
       win_key(DIK_COMMA, Qt::Key::Key_Comma ),
       win_key(DIK_PERIOD, Qt::Key::Key_Period ),
       win_key(DIK_LBRACKET, Qt::Key::Key_BracketLeft ),
       win_key(DIK_RBRACKET, Qt::Key::Key_BracketRight ),
       win_key(DIK_SEMICOLON, Qt::Key::Key_Semicolon ),
       win_key(DIK_SLASH, Qt::Key::Key_Slash ),
       win_key(DIK_BACKSLASH, Qt::Key::Key_Backslash ),
       win_key(DIK_BACKSPACE, Qt::Key::Key_Backspace ),
       win_key(DIK_APOSTROPHE, Qt::Key::Key_Apostrophe ),
       win_key(DIK_GRAVE, Qt::Key::Key_QuoteLeft ),
       win_key(DIK_MINUS, Qt::Key::Key_Minus ),
       win_key(DIK_EQUALS, Qt::Key::Key_Equal ),
       win_key(DIK_PERIOD, Qt::Key::Key_Period ),
       win_key(DIK_F1, Qt::Key::Key_F1 ),
       win_key(DIK_F2, Qt::Key::Key_F2 ),
       win_key(DIK_F3, Qt::Key::Key_F3 ),
       win_key(DIK_F4, Qt::Key::Key_F4 ),
       win_key(DIK_F5, Qt::Key::Key_F5 ),
       win_key(DIK_F6, Qt::Key::Key_F6 ),
       win_key(DIK_F7, Qt::Key::Key_F7 ),
       win_key(DIK_F8, Qt::Key::Key_F8 ),
       win_key(DIK_F9, Qt::Key::Key_F9 ),
       win_key(DIK_F10, Qt::Key::Key_F10 ),
       win_key(DIK_F11, Qt::Key::Key_F11 ),
       win_key(DIK_F12, Qt::Key::Key_F12 ),
       win_key(DIK_0, Qt::Key::Key_0 ),
       win_key(DIK_1, Qt::Key::Key_1 ),
       win_key(DIK_2, Qt::Key::Key_2 ),
       win_key(DIK_3, Qt::Key::Key_3 ),
       win_key(DIK_4, Qt::Key::Key_4 ),
       win_key(DIK_5, Qt::Key::Key_5 ),
       win_key(DIK_6, Qt::Key::Key_6 ),
       win_key(DIK_7, Qt::Key::Key_7 ),
       win_key(DIK_8, Qt::Key::Key_8 ),
       win_key(DIK_9, Qt::Key::Key_9 ),
       win_key(DIK_A, Qt::Key::Key_A ),
       win_key(DIK_B, Qt::Key::Key_B ),
       win_key(DIK_C, Qt::Key::Key_C ),
       win_key(DIK_D, Qt::Key::Key_D ),
       win_key(DIK_E, Qt::Key::Key_E ),
       win_key(DIK_F, Qt::Key::Key_F ),
       win_key(DIK_G, Qt::Key::Key_G ),
       win_key(DIK_H, Qt::Key::Key_H ),
       win_key(DIK_I, Qt::Key::Key_I ),
       win_key(DIK_J, Qt::Key::Key_J ),
       win_key(DIK_K, Qt::Key::Key_K ),
       win_key(DIK_L, Qt::Key::Key_L ),
       win_key(DIK_M, Qt::Key::Key_M ),
       win_key(DIK_N, Qt::Key::Key_N ),
       win_key(DIK_O, Qt::Key::Key_O ),
       win_key(DIK_P, Qt::Key::Key_P ),
       win_key(DIK_Q, Qt::Key::Key_Q ),
       win_key(DIK_R, Qt::Key::Key_R ),
       win_key(DIK_S, Qt::Key::Key_S ),
       win_key(DIK_T, Qt::Key::Key_T ),
       win_key(DIK_U, Qt::Key::Key_U ),
       win_key(DIK_V, Qt::Key::Key_V ),
       win_key(DIK_W, Qt::Key::Key_W ),
       win_key(DIK_X, Qt::Key::Key_X ),
       win_key(DIK_Y, Qt::Key::Key_Y ),
       win_key(DIK_Z, Qt::Key::Key_Z ),
       win_key(DIK_RETURN, Qt::Key::Key_Return),
       win_key(DIK_INSERT, Qt::Key::Key_Insert),
       win_key(DIK_DELETE, Qt::Key::Key_Delete),
       win_key(DIK_SPACE, Qt::Key::Key_Space),
       win_key(DIK_SYSRQ, Qt::Key::Key_Print),
       win_key(DIK_SCROLL, Qt::Key::Key_ScrollLock),
       win_key(DIK_PAUSE, Qt::Key::Key_Pause),
       win_key(DIK_NUMLOCK, Qt::Key::Key_NumLock),
#define mod(x, y) static_cast<Qt::Key>(x | y)
       win_key(DIK_NUMPAD0,      mod(Qt::Key::Key_0,        Qt::KeypadModifier)),
       win_key(DIK_NUMPAD0,      mod(Qt::Key::Key_0,        Qt::KeypadModifier)),
       win_key(DIK_NUMPAD1,      mod(Qt::Key::Key_1,        Qt::KeypadModifier)),
       win_key(DIK_NUMPAD2,      mod(Qt::Key::Key_2,        Qt::KeypadModifier)),
       win_key(DIK_NUMPAD3,      mod(Qt::Key::Key_3,        Qt::KeypadModifier)),
       win_key(DIK_NUMPAD4,      mod(Qt::Key::Key_4,        Qt::KeypadModifier)),
       win_key(DIK_NUMPAD5,      mod(Qt::Key::Key_5,        Qt::KeypadModifier)),
       win_key(DIK_NUMPAD6,      mod(Qt::Key::Key_6,        Qt::KeypadModifier)),
       win_key(DIK_NUMPAD7,      mod(Qt::Key::Key_7,        Qt::KeypadModifier)),
       win_key(DIK_NUMPAD8,      mod(Qt::Key::Key_8,        Qt::KeypadModifier)),
       win_key(DIK_NUMPAD9,      mod(Qt::Key::Key_9,        Qt::KeypadModifier)),
       win_key(DIK_NUMPADCOMMA,  mod(Qt::Key::Key_Comma,    Qt::KeypadModifier)),
       win_key(DIK_NUMPADENTER,  mod(Qt::Key::Key_Enter,    Qt::KeypadModifier)),
       win_key(DIK_NUMPADEQUALS, mod(Qt::Key::Key_Equal,    Qt::KeypadModifier)),
       win_key(DIK_NUMPADMINUS,  mod(Qt::Key::Key_Minus,    Qt::KeypadModifier)),
       win_key(DIK_NUMPADPERIOD, mod(Qt::Key::Key_Period,   Qt::KeypadModifier)),
       win_key(DIK_NUMPADPLUS,   mod(Qt::Key::Key_Plus,     Qt::KeypadModifier)),
       win_key(DIK_NUMPADSLASH,  mod(Qt::Key::Key_Slash,    Qt::KeypadModifier)),
       win_key(DIK_NUMPADSTAR,   mod(Qt::Key::Key_multiply, Qt::KeypadModifier)),
    });

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
        const auto key_ = qt;
        for (auto& wk : windows_key_sequences)
        {
            if (wk.qt == key_)
            {
                dik = wk.win;
                mods = Qt::NoModifier;
                return true;
            }
        }
    }
    {
        const unsigned key = qt & ~Qt::KeyboardModifierMask;
        for (auto& wk : windows_key_sequences)
        {
            if (wk.qt == key)
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
