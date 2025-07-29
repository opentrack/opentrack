/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#ifdef _WIN32
#   undef DIRECTINPUT_VERSION
#   define DIRECTINPUT_VERSION 0x800
#   include <dinput.h>

#include "win32-shortcuts.h"
#include <QVariant>
#include <QDebug>
#include <winuser.h>

namespace {

constexpr auto MODS = (unsigned)(Qt::KeyboardModifierMask & ~Qt::KeypadModifier);

#if 0
win_key const windows_key_mods[] {
        {DIK_LCONTROL, Qt::Key_Control},
        {DIK_RCONTROL, Qt::Key_Control},
        {DIK_LALT, Qt::Key_Alt},
        {DIK_RALT, Qt::Key_Alt},
        {DIK_LSHIFT, Qt::Key_Shift},
        {DIK_RSHIFT, Qt::Key_Shift},
        {DIK_LWIN, Qt::Key_Super_L},
        {DIK_RWIN, Qt::Key_Super_R},
};
#endif

constexpr win_key windows_key_sequences[] {
    { Qt::Key_F1,                       DIK_F1,             VK_F1,         },
    { Qt::Key_F2,                       DIK_F2,             VK_F2,         },
    { Qt::Key_F3,                       DIK_F3,             VK_F3,         },
    { Qt::Key_F4,                       DIK_F4,             VK_F4,         },
    { Qt::Key_F5,                       DIK_F5,             VK_F5,         },
    { Qt::Key_F6,                       DIK_F6,             VK_F6,         },
    { Qt::Key_F7,                       DIK_F7,             VK_F7,         },
    { Qt::Key_F8,                       DIK_F8,             VK_F8,         },
    { Qt::Key_F9,                       DIK_F9,             VK_F9,         },
    { Qt::Key_F10,                      DIK_F10,            VK_F10,        },
    { Qt::Key_F11,                      DIK_F11,            VK_F11,        },
    { Qt::Key_F12,                      DIK_F12,            VK_F12,        },
    { Qt::Key_F13,                      DIK_F13,            VK_F13,        },
    { Qt::Key_F14,                      DIK_F14,            VK_F14,        },
    { Qt::Key_F15,                      DIK_F15,            VK_F15,        },
    { Qt::Key_Left,                     DIK_LEFT,           VK_LEFT,       },
    { Qt::Key_Right,                    DIK_RIGHT,          VK_RIGHT,      },
    { Qt::Key_Up,                       DIK_UP,             VK_UP,         },
    { Qt::Key_Down,                     DIK_DOWN,           VK_DOWN,       },
    { Qt::Key_PageUp,                   DIK_PRIOR,          VK_PRIOR,      },
    { Qt::Key_PageDown,                 DIK_NEXT,           VK_NEXT,       },
    { Qt::Key_Home,                     DIK_HOME,           VK_HOME,       },
    { Qt::Key_End,                      DIK_END,            VK_END,        },
    { Qt::Key_Backspace,                DIK_BACK,           VK_BACK,       },
    { Qt::Key_Comma,                    DIK_COMMA,          VK_OEM_COMMA,  },
    { Qt::Key_Period,                   DIK_PERIOD,         VK_OEM_PERIOD, },
    { Qt::Key_BracketLeft,              DIK_LBRACKET,       VK_OEM_4,      },
    { Qt::Key_BracketRight,             DIK_RBRACKET,       VK_OEM_6,      },
    { Qt::Key_Semicolon,                DIK_SEMICOLON,      VK_OEM_1,      },
    { Qt::Key_Slash,                    DIK_SLASH,          VK_OEM_2,      },
    { Qt::Key_Backslash,                DIK_BACKSLASH,      VK_OEM_5,      },
    { Qt::Key_Apostrophe,               DIK_APOSTROPHE,     VK_OEM_7,      },
    { Qt::Key_QuoteLeft,                DIK_GRAVE,          VK_OEM_3,      },
    { Qt::Key_Minus,                    DIK_MINUS,          VK_OEM_MINUS,  },
    { Qt::Key_Equal,                    DIK_EQUALS,         VK_OEM_PLUS,   },
    { Qt::Key_0,                        DIK_0,              '0',           },
    { Qt::Key_1,                        DIK_1,              '1',           },
    { Qt::Key_2,                        DIK_2,              '2',           },
    { Qt::Key_3,                        DIK_3,              '3',           },
    { Qt::Key_4,                        DIK_4,              '4',           },
    { Qt::Key_5,                        DIK_5,              '5',           },
    { Qt::Key_6,                        DIK_6,              '6',           },
    { Qt::Key_7,                        DIK_7,              '7',           },
    { Qt::Key_8,                        DIK_8,              '8',           },
    { Qt::Key_9,                        DIK_9,              '9',           },
    { Qt::Key_A,                        DIK_A,              'A',           },
    { Qt::Key_B,                        DIK_B,              'B',           },
    { Qt::Key_C,                        DIK_C,              'C',           },
    { Qt::Key_D,                        DIK_D,              'D',           },
    { Qt::Key_E,                        DIK_E,              'E',           },
    { Qt::Key_F,                        DIK_F,              'F',           },
    { Qt::Key_G,                        DIK_G,              'G',           },
    { Qt::Key_H,                        DIK_H,              'H',           },
    { Qt::Key_I,                        DIK_I,              'I',           },
    { Qt::Key_J,                        DIK_J,              'J',           },
    { Qt::Key_K,                        DIK_K,              'K',           },
    { Qt::Key_L,                        DIK_L,              'L',           },
    { Qt::Key_M,                        DIK_M,              'M',           },
    { Qt::Key_N,                        DIK_N,              'N',           },
    { Qt::Key_O,                        DIK_O,              'O',           },
    { Qt::Key_P,                        DIK_P,              'P',           },
    { Qt::Key_Q,                        DIK_Q,              'Q',           },
    { Qt::Key_R,                        DIK_R,              'R',           },
    { Qt::Key_S,                        DIK_S,              'S',           },
    { Qt::Key_T,                        DIK_T,              'T',           },
    { Qt::Key_U,                        DIK_U,              'U',           },
    { Qt::Key_V,                        DIK_V,              'V',           },
    { Qt::Key_W,                        DIK_W,              'W',           },
    { Qt::Key_X,                        DIK_X,              'X',           },
    { Qt::Key_Y,                        DIK_Y,              'Y',           },
    { Qt::Key_Z,                        DIK_Z,              'Z',           },
    { Qt::Key_Tab,                      DIK_TAB,            VK_TAB,        },
    { Qt::Key_Return,                   DIK_RETURN,         VK_RETURN,     },
    { Qt::Key_Insert,                   DIK_INSERT,         VK_INSERT,     },
    { Qt::Key_Delete,                   DIK_DELETE,         VK_DELETE,     },
    { Qt::Key_Space,                    DIK_SPACE,          VK_SPACE,      },
    { Qt::Key_Print,                    DIK_SYSRQ,          VK_SNAPSHOT,   },
    { Qt::Key_Print,                    DIK_SYSRQ,          VK_PRINT,      },
    { Qt::Key_ScrollLock,               DIK_SCROLL,         VK_SCROLL,     },
    { Qt::Key_Pause,                    DIK_PAUSE,          VK_PAUSE,      },
    { Qt::Key_NumLock,                  DIK_NUMLOCK,        VK_NUMLOCK,    },
    { Qt::Key_CapsLock,                 DIK_CAPSLOCK,       VK_CAPITAL,    },
#define key_mod(x, y) Qt::Key(int((x)) | int((y)))
#define key_numpad(x) key_mod((x), (Qt::KeypadModifier))
    { key_numpad(Qt::Key_0),            DIK_NUMPAD0,        VK_NUMPAD0,    },
    { key_numpad(Qt::Key_1),            DIK_NUMPAD1,        VK_NUMPAD1,    },
    { key_numpad(Qt::Key_2),            DIK_NUMPAD2,        VK_NUMPAD2,    },
    { key_numpad(Qt::Key_3),            DIK_NUMPAD3,        VK_NUMPAD3,    },
    { key_numpad(Qt::Key_4),            DIK_NUMPAD4,        VK_NUMPAD4,    },
    { key_numpad(Qt::Key_5),            DIK_NUMPAD5,        VK_NUMPAD5,    },
    { key_numpad(Qt::Key_6),            DIK_NUMPAD6,        VK_NUMPAD6,    },
    { key_numpad(Qt::Key_7),            DIK_NUMPAD7,        VK_NUMPAD7,    },
    { key_numpad(Qt::Key_8),            DIK_NUMPAD8,        VK_NUMPAD8,    },
    { key_numpad(Qt::Key_9),            DIK_NUMPAD9,        VK_NUMPAD9,    },
    { key_numpad(Qt::Key_Comma),        DIK_NUMPADCOMMA,    VK_SEPARATOR,  },
    { key_numpad(Qt::Key_Enter),        DIK_NUMPADENTER,    VK_RETURN,     },
    { key_numpad(Qt::Key_Minus),        DIK_NUMPADMINUS,    VK_SUBTRACT,   },
    { key_numpad(Qt::Key_Period),       DIK_NUMPADPERIOD,   VK_DECIMAL,    },
    { key_numpad(Qt::Key_Plus),         DIK_NUMPADPLUS,     VK_ADD,        },
    { key_numpad(Qt::Key_Slash),        DIK_NUMPADSLASH,    VK_DIVIDE,     },
    { key_numpad(Qt::Key_multiply),     DIK_NUMPADSTAR,     VK_MULTIPLY,   },
};

} // namespace

bool win_key::dik_to_qt(const Key& k, QKeySequence& qt, Qt::KeyboardModifiers &mods)
{
    for (auto& wk : windows_key_sequences)
    {
        if ((unsigned)wk.win == (unsigned)k.keycode)
        {
            qt = wk.qt;
            mods = Qt::NoModifier;
            if (k.ctrl) mods |= Qt::ControlModifier;
            if (k.shift) mods |= Qt::ShiftModifier;
            if (k.alt) mods |= Qt::AltModifier;
            return true;
        }
    }
    return false;
}

bool win_key::qt_to_dik(const QKeySequence& qtʹ, int& dik, Qt::KeyboardModifiers& mods)
{
    // CAVEAT don't use QVariant::toUInt() or conversion fails
    const unsigned qt = (unsigned)QVariant(qtʹ).toInt();
    const unsigned our_mods = qt & MODS;

    if (qt == 0)
        return false;

    for (const win_key& wk : windows_key_sequences)
    {
        if (unsigned(wk.qt) == qt)
        {
            dik = wk.win;
            mods = Qt::NoModifier;
            return true;
        }
    }

    {
        const unsigned key = qt & ~MODS;
        for (const win_key& wk : windows_key_sequences)
        {
            if (unsigned(wk.qt) == key)
            {
                dik = wk.win;
                mods = { (Qt::KeyboardModifier)our_mods };
                return true;
            }
        }
    }
    return false;
}

bool win_key::vk_to_qt(const Key& k, QKeySequence& qt, Qt::KeyboardModifiers& mods)
{
    for (auto& wk : windows_key_sequences)
    {
        if ((unsigned)wk.win == (unsigned)k.keycode)
        {
            qt = wk.qt;
            mods = Qt::NoModifier;
            if (k.ctrl) mods |= Qt::ControlModifier;
            if (k.shift) mods |= Qt::ShiftModifier;
            if (k.alt) mods |= Qt::AltModifier;
            return true;
        }
    }
    return false;
}

bool win_key::qt_to_vk(const QKeySequence& qtʹ, int& vk, Qt::KeyboardModifiers& mods)
{
    // CAVEAT don't use QVariant::toUInt() or conversion fails
    const unsigned qt = (unsigned)QVariant(qtʹ).toInt();
    const unsigned our_mods = qt & MODS;

    if (qt == 0)
        return false;

    for (const win_key& wk : windows_key_sequences)
    {
        if (unsigned(wk.qt) == qt)
        {
            vk = wk.vk;
            mods = Qt::NoModifier;
            return true;
        }
    }

    {
        const unsigned key = qt & ~MODS;
        for (const win_key& wk : windows_key_sequences)
        {
            if (unsigned(wk.qt) == key)
            {
                vk = wk.vk;
                mods = { (Qt::KeyboardModifier)our_mods };
                return true;
            }
        }
    }
    return false;
}

#endif
