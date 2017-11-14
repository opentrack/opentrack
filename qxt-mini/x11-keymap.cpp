#include "x11-keymap.hpp"

#if !defined __APPLE__ && !defined _WIN32

#include <QMutex>

#define XK_MISCELLANY
#define XK_LATIN1

#include <X11/keysymdef.h>


struct tt {
    Qt::Key qt;
    quint32 keysym;
};

static const tt keymap[] =
{
    { Qt::Key_Return, XK_Return },
    { Qt::Key_Enter, XK_Return },
    { Qt::Key_Delete, XK_Delete },
    { Qt::Key_Pause, XK_Pause },
    { Qt::Key_SysReq, XK_Sys_Req },
    { Qt::Key_Home, XK_Home },
    { Qt::Key_Insert, XK_Insert },
    { Qt::Key_End, XK_End },
    { Qt::Key_Left, XK_Left },
    { Qt::Key_Up, XK_Up },
    { Qt::Key_Right, XK_Right },
    { Qt::Key_Down, XK_Down },
    { Qt::Key_PageUp, XK_Prior },
    { Qt::Key_PageDown, XK_Next },
    { Qt::Key_Tab, XK_Tab },

    { Qt::Key_F1, XK_F1 },
    { Qt::Key_F2, XK_F2 },
    { Qt::Key_F3, XK_F3 },
    { Qt::Key_F4, XK_F4 },
    { Qt::Key_F5, XK_F5 },
    { Qt::Key_F6, XK_F6 },
    { Qt::Key_F7, XK_F7 },
    { Qt::Key_F8, XK_F8 },
    { Qt::Key_F9, XK_F9 },
    { Qt::Key_F10, XK_F10 },
    { Qt::Key_F11, XK_F11 },
    { Qt::Key_F12, XK_F12 },

    { Qt::Key_Space, XK_space },
    { Qt::Key_QuoteDbl, XK_quotedbl },

#if 1
    { Qt::Key_Exclam, XK_exclam },
    { Qt::Key_At, XK_at },
    { Qt::Key_NumberSign, XK_numbersign },
    { Qt::Key_Dollar, XK_dollar },
    { Qt::Key_Percent, XK_percent },
    { Qt::Key_AsciiCircum, XK_asciicircum },
    { Qt::Key_Ampersand, XK_ampersand },
    { Qt::Key_Asterisk, XK_asterisk },
    { Qt::Key_ParenLeft, XK_parenleft },
    { Qt::Key_ParenRight, XK_parenright },
#else
    { Qt::Key_Exclam, XK_1 },
    { Qt::Key_At, XK_2 },
    { Qt::Key_NumberSign, XK_3 },
    { Qt::Key_Dollar, XK_4 },
    { Qt::Key_Percent, XK_5 },
    { Qt::Key_AsciiCircum, XK_6 },
    { Qt::Key_Ampersand, XK_7 },
    { Qt::Key_Asterisk, XK_8 },
    { Qt::Key_ParenLeft, XK_9 },
    { Qt::Key_ParenRight, XK_0 },
#endif
    { Qt::Key_Minus, XK_minus },
    { Qt::Key_Equal, XK_equal },
    { Qt::Key_Apostrophe, XK_apostrophe },
    { Qt::Key_Plus, XK_plus },
    { Qt::Key_Comma, XK_comma },
    { Qt::Key_Period, XK_period },
    { Qt::Key_Slash, XK_slash },

    { Qt::Key_0, XK_0 },
    { Qt::Key_1, XK_1 },
    { Qt::Key_2, XK_2 },
    { Qt::Key_3, XK_3 },
    { Qt::Key_4, XK_4 },
    { Qt::Key_5, XK_5 },
    { Qt::Key_6, XK_6 },
    { Qt::Key_7, XK_7 },
    { Qt::Key_8, XK_8 },
    { Qt::Key_9, XK_9 },

    { Qt::Key_Colon, XK_colon },
    { Qt::Key_Semicolon, XK_semicolon },
    { Qt::Key_Less, XK_less },
    { Qt::Key_Greater, XK_greater },
    { Qt::Key_Question, XK_question },

    { Qt::Key_A, XK_a },
    { Qt::Key_B, XK_b },
    { Qt::Key_C, XK_c },
    { Qt::Key_D, XK_d },
    { Qt::Key_E, XK_e },
    { Qt::Key_F, XK_f },
    { Qt::Key_G, XK_g },
    { Qt::Key_H, XK_h },
    { Qt::Key_I, XK_i },
    { Qt::Key_J, XK_j },
    { Qt::Key_K, XK_k },
    { Qt::Key_L, XK_l },
    { Qt::Key_M, XK_m },
    { Qt::Key_N, XK_n },
    { Qt::Key_O, XK_o },
    { Qt::Key_P, XK_p },
    { Qt::Key_Q, XK_q },
    { Qt::Key_R, XK_r },
    { Qt::Key_S, XK_s },
    { Qt::Key_T, XK_t },
    { Qt::Key_U, XK_u },
    { Qt::Key_V, XK_v },
    { Qt::Key_W, XK_w },
    { Qt::Key_X, XK_x },
    { Qt::Key_Y, XK_y },
    { Qt::Key_Z, XK_z },

    { Qt::Key_BracketLeft, XK_bracketleft },
    { Qt::Key_Backslash, XK_backslash },
    { Qt::Key_BracketRight, XK_bracketright },
    { Qt::Key_Underscore, XK_underscore },
    { Qt::Key_QuoteLeft, XK_grave },
#if 0
};
static tt numpad_keymap[] = {
#endif
    { Qt::Key_0, XK_KP_0 },
    { Qt::Key_1, XK_KP_1 },
    { Qt::Key_2, XK_KP_2 },
    { Qt::Key_3, XK_KP_3 },
    { Qt::Key_4, XK_KP_4 },
    { Qt::Key_5, XK_KP_5 },
    { Qt::Key_6, XK_KP_6 },
    { Qt::Key_7, XK_KP_7 },
    { Qt::Key_8, XK_KP_8 },
    { Qt::Key_9, XK_KP_9 },

    { Qt::Key_Space, XK_KP_Space },
    { Qt::Key_Tab, XK_KP_Tab },
    { Qt::Key_F1, XK_KP_F1 },
    { Qt::Key_F2, XK_KP_F2 },
    { Qt::Key_F3, XK_KP_F3 },
    { Qt::Key_F4, XK_KP_F4 },

    { Qt::Key_1, XK_KP_End },
    { Qt::Key_2, XK_KP_Down },
    { Qt::Key_3, XK_KP_Page_Down },
    { Qt::Key_4, XK_KP_Left },
    { Qt::Key_5, XK_KP_Begin },
    { Qt::Key_6, XK_KP_Right },
    { Qt::Key_7, XK_KP_Home },
    { Qt::Key_8, XK_KP_Up },
    { Qt::Key_9, XK_KP_Page_Up },
    { Qt::Key_0, XK_KP_Insert },

    { Qt::Key_Delete, XK_KP_Delete },
    { Qt::Key_Equal, XK_KP_Equal },
    { Qt::Key_Asterisk, XK_KP_Multiply },
    { Qt::Key_Plus, XK_KP_Add },
    { Qt::Key_Comma, XK_KP_Separator },
    { Qt::Key_Minus, XK_KP_Subtract },
    { Qt::Key_Period, XK_KP_Decimal },
    { Qt::Key_Slash, XK_KP_Divide },

    { Qt::Key_ScrollLock, XK_Scroll_Lock },
};

QXT_GUI_EXPORT
quint32 qt_mods_to_x11(Qt::KeyboardModifiers modifiers)
{
    quint32 mods = 0;

    if (modifiers & Qt::AltModifier)
        mods |= Mod1Mask;
    if (modifiers & Qt::ControlModifier)
        mods |= ControlMask;
    if (modifiers & Qt::ShiftModifier)
        mods |= ShiftMask;
    if (modifiers & Qt::KeypadModifier)
        mods |= Mod2Mask;
    if (modifiers & Qt::MetaModifier) // Super aka Windows key
        mods |= Mod4Mask;

    return mods;
}

QXT_GUI_EXPORT
std::vector<quint32> qt_key_to_x11(Display*, Qt::Key k, Qt::KeyboardModifiers)
{
    std::vector<quint32> ret;

    for (const tt& tuple : keymap)
    {
        Qt::Key k_ = tuple.qt;
        unsigned keycode = tuple.keysym;

        if (k == k_)
            ret.push_back(keycode);
    }

    if (ret.size() == 0)
        qDebug() << "qxt-mini: no keysym for" << k;

    return ret;
}
QXT_GUI_EXPORT
Qt::KeyboardModifiers x11_mods_to_qt(quint32 mods)
{
    Qt::KeyboardModifiers ret(0);

    if (mods & Mod1Mask)
        ret |= Qt::AltModifier;
    if (mods & ControlMask)
        ret |= Qt::ControlModifier;
    if (mods & Mod4Mask)
        ret |= Qt::MetaModifier;
    if (mods & ShiftMask)
        ret |= Qt::ShiftModifier;

    return ret;
}

QXT_GUI_EXPORT
std::tuple<Qt::Key, Qt::KeyboardModifiers> x11_key_to_qt(Display* disp, quint32 keycode, quint32 mods)
{
    (void)disp;
    using t = std::tuple<Qt::Key, Qt::KeyboardModifiers>;

    for (const tt& tuple : keymap)
    {
        const Qt::Key k = tuple.qt;

        if (keycode == tuple.keysym)
            return t(k, x11_mods_to_qt(mods));
    }

    return t(Qt::Key(0), Qt::KeyboardModifiers(0));
}


QXT_GUI_EXPORT
QPair<KeySym, KeySym> keycode_to_keysym(Display* disp,
                                        quint32 keycode, quint32 keystate,
                                        xcb_key_press_event_t const* kev)
{
    using pair = QPair<quint32, quint32>;

    static QMutex lock;
    static QHash<pair, QPair<KeySym, KeySym>> values;

    QMutexLocker l(&lock);

    auto it = values.find(pair(keycode, keystate));

    if (it != values.end())
        return *it;

    XKeyEvent ev{};
    ev.serial = kev->sequence;
    ev.send_event = False;
    ev.display = disp;
    ev.root = kev->root;
    ev.subwindow = kev->child;
    ev.window = kev->event;
    ev.time = kev->time;
    ev.x = kev->event_x;
    ev.y = kev->event_x;
    ev.x_root = kev->root_x;
    ev.y_root = kev->root_y;
    ev.state = keystate;
    ev.keycode = keycode;
    ev.same_screen = kev->same_screen;

    static char bytes[255+1];
    KeySym sym = 0;
    int len = XLookupString(&ev, bytes, 255, &sym, NULL);
    if (len <= 0 || len > 255)
    {
        len = 0;
        sym = 0;
    }

    KeySym sym2 = XLookupKeysym(&ev, 0);

    QPair<KeySym, KeySym> ret(sym, sym2);

    values[pair(keycode, keystate)] = ret;

    return ret;
}

QXT_GUI_EXPORT
quint32 xcb_mods_to_x11(quint32 mods)
{
    unsigned int keystate = 0;

    if(mods & XCB_MOD_MASK_1) // alt
        keystate |= Mod1Mask;
    if(mods & XCB_MOD_MASK_CONTROL) // ctrl
        keystate |= ControlMask;
    if(mods & XCB_MOD_MASK_4) // super aka win key
        keystate |= Mod4Mask;
    if(mods & XCB_MOD_MASK_SHIFT) //shift
        keystate |= ShiftMask;
    if(mods & XCB_MOD_MASK_2) // numlock
        keystate |= Mod2Mask;

    return keystate;
}
#endif
