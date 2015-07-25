#include <QList>
#include <QKeySequence>
#include <QVariant>
#include <QDebug>
#include "global-shortcuts.h"

#if defined(_WIN32)
#   ifndef DIRECTINPUT_VERSION
#       define DIRECTINPUT_VERSION 0x800
#   endif
#   include <windows.h>
#   include <dinput.h>

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
    });

bool win_key::from_qt(QKeySequence qt_, int& dik, Qt::KeyboardModifiers& mods)
{
    auto qt = static_cast<QVariant>(qt_).toInt();
    auto our_mods = qt & Qt::KeyboardModifierMask;
#ifdef _WIN32
    const auto our_mods_ = our_mods;
    our_mods |= Qt::ShiftModifier;
    switch (qt & ~Qt::ShiftModifier)
    {
    case Qt::Key::Key_BraceLeft: qt = Qt::Key::Key_BracketLeft; break;
    case Qt::Key::Key_BraceRight: qt = Qt::Key::Key_BracketRight; break;
    case Qt::Key::Key_ParenLeft: qt = Qt::Key::Key_9; break;
    case Qt::Key::Key_ParenRight: qt = Qt::Key::Key_0; break;

    case Qt::Key::Key_Exclam: qt = Qt::Key::Key_1; break;
    case Qt::Key::Key_At: qt = Qt::Key::Key_2; break;
    case Qt::Key::Key_NumberSign: qt = Qt::Key::Key_3; break;
    case Qt::Key::Key_Dollar: qt = Qt::Key::Key_4; break;
    case Qt::Key::Key_Percent: qt = Qt::Key::Key_5; break;
    case Qt::Key::Key_AsciiCircum: qt = Qt::Key::Key_6; break;
    case Qt::Key::Key_Ampersand: qt = Qt::Key::Key_7; break;
    case Qt::Key::Key_Asterisk: qt = Qt::Key::Key_8; break;

    case Qt::Key::Key_Underscore: qt = Qt::Key::Key_Minus; break;
    case Qt::Key::Key_Plus: qt = Qt::Key::Key_Equal; break;

    case Qt::Key::Key_Colon: qt = Qt::Key::Key_Semicolon; break;
    case Qt::Key::Key_QuoteDbl: qt = Qt::Key::Key_Apostrophe; break;
    case Qt::Key::Key_Less: qt = Qt::Key::Key_Comma; break;
    case Qt::Key::Key_Question: qt = Qt::Key::Key_Slash; break;
    case Qt::Key::Key_Bar: qt = Qt::Key::Key_Backslash; break;
    default: our_mods = our_mods_; break;
    }
#endif

    const auto key = qt & ~Qt::KeyboardModifierMask;
    for (auto& wk : windows_key_sequences)
    {
        if (wk.qt == key)
        {
            dik = wk.win;
            mods = static_cast<Qt::KeyboardModifiers>(our_mods);
            return true;
        }
    }
    return false;
}

#endif
