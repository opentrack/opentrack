#ifndef __APPLE__
#include "qxtglobalshortcut_p.h"
/****************************************************************************
** Copyright (c) 2006 - 2011, the LibQxt project.
** See the Qxt AUTHORS file for a list of authors and copyright holders.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the LibQxt project nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
** <http://libqxt.org>  <foundation@libqxt.org>
*****************************************************************************/

// qt must go first or #error
#include <QHash>
#include <QMutex>
#include <QDebug>
#include <QPair>
#include <QKeyEvent>
#include <QApplication>
#include "qplatformnativeinterface.h"

#include "x11-keymap.hpp"

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <xcb/xcb.h>

#include "compat/powerset.hpp"
#include "compat/util.hpp"

#include <iterator>
#include <vector>
#include <type_traits>
#include <utility>
#include <cinttypes>
#include <array>

static auto evil_mods = make_powerset(LockMask, Mod5Mask, Mod2Mask);

static inline quint32 filter_evil_mods(quint32 mods)
{
    for (auto mod : evil_mods.elements())
    {
        mods &= quint32(~mod);
    }
    return mods;
}

using pair = QPair<quint32, quint32>;

struct keybinding final
{
    quint32 code, mods;
    int refcnt;

    static QHash<pair, keybinding> list;
    static QMutex lock;

    static bool incf(quint32 code, quint32 mods);
    static bool decf(quint32 code, quint32 mods);

    ~keybinding();

private:
    keybinding(quint32 code, quint32 mods);
};

static std::vector<pair> native_key(Qt::Key key, Qt::KeyboardModifiers modifiers);
typedef int (*X11ErrorHandler)(Display *display, XErrorEvent *event);

class QxtX11ErrorHandler {
public:
    static bool error;

    static int qxtX11ErrorHandler(Display *display, XErrorEvent *event)
    {
        Q_UNUSED(display);
        switch (event->error_code)
        {
            case BadAccess:
            case BadValue:
            case BadWindow:
                if (event->request_code == 33 /* X_GrabKey */ ||
                    event->request_code == 34 /* X_UngrabKey */)
                {
                    error = true;
                }
        }
        return 0;
    }

    QxtX11ErrorHandler()
    {
        error = false;
        m_previousErrorHandler = XSetErrorHandler(qxtX11ErrorHandler);
    }

    ~QxtX11ErrorHandler()
    {
        XSetErrorHandler(m_previousErrorHandler);
    }

private:
    X11ErrorHandler m_previousErrorHandler;
};

class QxtX11Data {
public:
    QxtX11Data()
    {
        QPlatformNativeInterface *native = qApp->platformNativeInterface();
        void *display = native->nativeResourceForScreen(QByteArray("display"),
                                                        QGuiApplication::primaryScreen());
        m_display = reinterpret_cast<Display *>(display);
    }

    bool isValid()
    {
        return m_display != 0;
    }

    Display *display()
    {
        Q_ASSERT(isValid());
        return m_display;
    }

    Window rootWindow()
    {
        return DefaultRootWindow(display());
    }

    bool grabKey(quint32 code, quint32 mods)
    {
        const std::vector<pair> keycodes = native_key(Qt::Key(code), Qt::KeyboardModifiers(mods));
        bool ret = true;

        for (pair x : keycodes)
        {
            int native_code = x.first, native_mods = x.second;

            native_code = XKeysymToKeycode(display(), native_code);

            if (keybinding::incf(native_code, native_mods))
            {
                QxtX11ErrorHandler errorHandler;

                XGrabKey(display(), native_code, native_mods, rootWindow(), True, GrabModeAsync, GrabModeAsync);

                for (const auto& set : evil_mods.sets())
                {
                    quint32 m = native_mods;

                    for (auto value : set)
                        m |= value;

                    XGrabKey(display(), native_code, m, rootWindow(), True, GrabModeAsync, GrabModeAsync);
                }

                if (errorHandler.error)
                {
                    qDebug() << "qxt-mini: error while binding to" << code << mods;
                    ungrabKey(code, mods);
                    ret = false;
                }
            }
        }

        return ret;
    }

    bool ungrabKey(quint32 code, quint32 mods)
    {
        const std::vector<pair> keycodes = native_key(Qt::Key(code), Qt::KeyboardModifiers(mods));
        bool ret = true;

        for (pair x : keycodes)
        {
            int native_code = x.first, native_mods = x.second;
            native_code = XKeysymToKeycode(display(), native_code);

            if (keybinding::decf(native_code, native_mods))
            {
                QxtX11ErrorHandler errorHandler;
                XUngrabKey(display(), native_code, native_mods, rootWindow());

                for (const auto& set : evil_mods.sets())
                {
                    quint32 m = mods;

                    for (auto value : set)
                        m |= value;

                    XUngrabKey(display(), code, m, rootWindow());
                }

                if (errorHandler.error)
                {
                    qDebug() << "qxt-mini: error while unbinding" << code << mods;
                    ret = false;
                }
            }
        }
        return ret;
    }

private:
    Display *m_display;
};

static std::vector<pair> native_key(Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    std::vector<pair> ret;

    QxtX11Data x11;
    if (!x11.isValid())
        return ret;

    std::vector<quint32> keycodes = qt_key_to_x11(x11.display(), key, modifiers);
    unsigned mods = qt_mods_to_x11(modifiers);
    mods = filter_evil_mods(mods);

    for (quint32 code : keycodes)
        ret.push_back(pair(code, mods));

    return ret;
}

bool operator==(const keybinding& k1, const keybinding& k2)
{
    return k1.code == k2.code && k1.mods == k2.mods;
}

inline bool operator!=(const keybinding& k1, const keybinding& k2)
{
    return !(k1 == k2);
}

uint qHash(const keybinding& k)
{
    return uint(k.code * 41) ^ qHash(k.mods);
}

uint qHash(const keybinding& k, uint seed)
{
    return qHash(uint(k.code * 41) ^ qHash(k.mods), seed);
}

keybinding::keybinding(quint32 code, quint32 mods) :
    code(code), mods(mods),
    refcnt(0)
{
}

keybinding::~keybinding()
{
}

bool keybinding::incf(quint32 code, quint32 mods)
{
    QMutexLocker l(&lock);

    keybinding k = list.value(pair(code, mods), keybinding(code, mods));

    const bool ret = k.refcnt == 0;

    if (ret)
    {
        //qDebug() << "qxt-mini: registered keybinding" << code;
    }

    k.refcnt++;
    list.insert(pair(code, mods), k);

    //qDebug() << "qxt-mini: incf: refcount for" << code << "now" << k.refcnt;

    return ret;
}

bool keybinding::decf(quint32 code, quint32 mods)
{
    QMutexLocker l(&lock);

    auto it = list.find(pair(code, mods));

    if (it == list.end())
    {
        qWarning() << "qxt-mini: spurious keybinding decf on" << code;
        return false;
    }

    keybinding& k = *it;
    k.refcnt--;

    if (k.refcnt == 0)
    {
        list.erase(it);
        //qDebug() << "qxt-mini: removed keybinding" << code;
        return true;
    }

    //qDebug() << "qxt-mini: decf: refcount for" << code << "now" << k.refcnt;

    return false;
}

QHash<pair, keybinding> keybinding::list;
QMutex keybinding::lock;

bool QxtX11ErrorHandler::error = false;

bool QxtGlobalShortcutPrivate::nativeEventFilter(const QByteArray & eventType,
    void *message, long *)
{
    QxtX11Data x11;

    if (!x11.isValid())
        return false;

    {
        static bool once_ = false;
        if (!once_)
        {
            once_ = true;
            Bool val = False;

            (void) XkbSetDetectableAutoRepeat(x11.display(), True, &val);

            if (val)
                qDebug() << "qxt-mini: fixed x11 autorepeat";
            else
                qDebug() << "qxt-mini: can't fix x11 autorepeat";
        }
    }

    bool is_release = false;

    xcb_key_press_event_t *kev = 0;
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t *ev = static_cast<xcb_generic_event_t *>(message);
        switch (ev->response_type & 127)
        {
            case XCB_KEY_RELEASE:
                is_release = true;
                /*FALLTHROUGH*/
            case XCB_KEY_PRESS:
                kev = static_cast<xcb_key_press_event_t *>(message);
                /*FALLTHROUGH*/
            default:
                break;
        }
    }

    if (kev) {
#if 0
        using event_type = decltype((xcb_key_press_event_t{}).detail);

        static event_type prev_event = 0;
        static bool prev_is_release = false;

        if (is_release == prev_is_release &&
            prev_event != 0 &&
            prev_event == kev->detail)
        {
            // ignore repeated keystrokes
            return false;
        }
        else
        {
            prev_event = kev->detail;
            prev_is_release = is_release;
        }
#endif

        unsigned int keycode = kev->detail;

        if (keycode == 0)
            return false;

        quint32 keystate = xcb_mods_to_x11(kev->state);

        keystate = filter_evil_mods(keystate);

        QPair<KeySym, KeySym> sym_ = keycode_to_keysym(x11.display(), keycode, keystate, kev);
        KeySym sym = sym_.first, sym2 = sym_.second;

        Qt::Key k; Qt::KeyboardModifiers mods;


        {
            std::tie(k, mods) = x11_key_to_qt(x11.display(), sym, keystate);

            if (k != 0)
                activateShortcut(k, mods, !is_release);
        }

        {
            std::tie(k, mods) = x11_key_to_qt(x11.display(), sym2, keystate);

            if (k != 0)
                activateShortcut(k, mods, !is_release);
        }
    }
    return false;
}

quint32 QxtGlobalShortcutPrivate::nativeModifiers(Qt::KeyboardModifiers modifiers)
{
    modifiers = x11_mods_to_qt(filter_evil_mods(qt_mods_to_x11(modifiers)));
    return quint32(modifiers);
}

quint32 QxtGlobalShortcutPrivate::nativeKeycode(Qt::Key key)
{
    return quint32(key);
}

bool QxtGlobalShortcutPrivate::registerShortcut(quint32 nativeKey, quint32 nativeMods)
{
    QxtX11Data x11;
    return x11.isValid() && x11.grabKey(nativeKey, nativeMods);
}

bool QxtGlobalShortcutPrivate::unregisterShortcut(quint32 nativeKey, quint32 nativeMods)
{
    QxtX11Data x11;
    return x11.isValid() && x11.ungrabKey(nativeKey, nativeMods);
}
#endif
