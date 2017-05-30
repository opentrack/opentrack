#pragma once

#if !defined __APPLE__ && !defined _WIN32

#undef QXT_X11_INCLUDED
#define QXT_X11_INCLUDED

#include <Qt>
#include <QDebug>
#include <QHash>
#include <QPair>

#include "qxtglobal.h"

#include <vector>
#include <tuple>

#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

QXT_EXPORT
std::vector<quint32> qt_key_to_x11(Display* disp,
                                   Qt::Key k, Qt::KeyboardModifiers m);

QXT_EXPORT
std::tuple<Qt::Key, Qt::KeyboardModifiers> x11_key_to_qt(Display* disp,
                                                         quint32 keycode, quint32 mods);

QXT_EXPORT QPair<KeySym, KeySym> keycode_to_keysym(Display* disp,
                                                   quint32 keycode, quint32 keystate,
                                                   xcb_key_press_event_t const* kev);

QXT_EXPORT quint32 qt_mods_to_x11(Qt::KeyboardModifiers modifiers);

QXT_GUI_EXPORT
Qt::KeyboardModifiers x11_mods_to_qt(quint32 mods);

QXT_GUI_EXPORT
quint32 xcb_mods_to_x11(quint32 mods);

#endif
