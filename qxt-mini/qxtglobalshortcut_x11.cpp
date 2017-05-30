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

#include <QHash>
#include <QMutex>
#include <QDebug>
#include <QPair>
#include <QKeyEvent>
#include <QApplication>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <xcb/xcb.h>
#include "qplatformnativeinterface.h"
#include "compat/util.hpp"

#include <iterator>
#include "x11-keymap.hpp"

static constexpr quint32 AllMods = ShiftMask|LockMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask;

static constexpr quint32 evil_mods[] = {
#if 0
    LockMask, // caps lock
    Mod3Mask, // scroll lock
#endif
    Mod2Mask, // num lock
    Mod5Mask, // altgr

    Mod2Mask | Mod5Mask,
};

typedef int (*X11ErrorHandler)(Display *display, XErrorEvent *event);

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

bool QxtX11ErrorHandler::error = false;

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

    static constexpr quint32 filter_evil_mods(quint32 mods)
    {
        for (quint32 mod : evil_mods)
            mods &= ~mod;
        return mods;
    }

    bool grabKey(quint32 code, quint32 mods, Window window)
    {
        // qDebug() << "grabbing key" << code << mods;

        mods = filter_evil_mods(mods);

        // qDebug() << "mods now" << mods;

        if (keybinding::incf(code, mods))
        {
            QxtX11ErrorHandler errorHandler;

            XGrabKey(display(), code, mods, window, True, GrabModeAsync, GrabModeAsync);

            for (quint32 evil : evil_mods)
            {
                quint32 m = mods | evil;

                XGrabKey(display(), code, m, window, True, GrabModeAsync, GrabModeAsync);
            }

            if (errorHandler.error)
            {
                qDebug() << "qxt-mini: error while binding to" << code << mods;
                ungrabKey(code, mods, window);
                return false;
            }
        }

        return true;
    }

    bool ungrabKey(quint32 code, quint32 mods, Window window)
    {
        mods = filter_evil_mods(mods);

        if (keybinding::decf(code, mods))
        {
            QxtX11ErrorHandler errorHandler;
            XUngrabKey(display(), code, mods, window);

            for (quint32 evil : evil_mods)
            {
                quint32 m = mods | evil;
                XUngrabKey(display(), code, m, window);
            }

            if (errorHandler.error)
            {
                qDebug() << "qxt-mini: error while unbinding" << code << mods;
                return false;
            }
        }
        return true;
    }

private:
    Display *m_display;
};

bool QxtGlobalShortcutPrivate::nativeEventFilter(const QByteArray & eventType,
    void *message, long *result)
{
    Q_UNUSED(result);

    {
        static bool once_ = false;
        if (!once_)
        {
            QxtX11Data x11;
            if (x11.isValid())
            {
                once_ = true;
                Bool val;

                (void) XkbSetDetectableAutoRepeat(x11.display(), True, &val);

                if (val)
                    qDebug() << "qxt-mini: fixed x11 autorepeat";
                else
                    qDebug() << "qxt-mini: can't fix x11 autorepeat";
            }
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
        unsigned int keystate = 0;
        if(kev->state & XCB_MOD_MASK_1) // alt
            keystate |= Mod1Mask;
        if(kev->state & XCB_MOD_MASK_CONTROL) // ctrl
            keystate |= ControlMask;
        if(kev->state & XCB_MOD_MASK_4) // super aka win key
            keystate |= Mod4Mask;
        if(kev->state & XCB_MOD_MASK_SHIFT) //shift
            keystate |= ShiftMask;
#if 0
        if(key->state & XCB_MOD_MASK_3) // alt gr aka right-alt or ctrl+left-alt -- what mask is it?
            keystate |= AltGrMask;
#endif

        // qDebug() << "qxt-mini:" << (is_release ? "keyup" : "keydown") << keycode << keystate;

        activateShortcut(keycode, keystate, !is_release);
    }
    return false;
}

quint32 QxtGlobalShortcutPrivate::nativeModifiers(Qt::KeyboardModifiers modifiers)
{
    quint32 native = 0;
    if (modifiers & Qt::AltModifier)
        native |= Mod1Mask;
    if (modifiers & Qt::ControlModifier)
        native |= ControlMask;
    if (modifiers & Qt::MetaModifier)
        native |= Mod4Mask;
    if (modifiers & Qt::ShiftModifier)
        native |= ShiftMask;
    if (modifiers & Qt::KeypadModifier)
        native |= Mod2Mask;
    if (modifiers & Qt::MetaModifier) // Super aka Windows key
        native |= Mod4Mask;
    if (modifiers & Qt::KeypadModifier) // numlock
        native |= Mod2Mask;

    native &= AllMods;

    return native;
}

quint32 QxtGlobalShortcutPrivate::nativeKeycode(Qt::Key key)
{
    QxtX11Data x11;

    if (x11.isValid())
        return qt_key_to_x11(x11.display(), key);

    return unsigned(-1);
}

bool QxtGlobalShortcutPrivate::registerShortcut(quint32 nativeKey, quint32 nativeMods)
{
    QxtX11Data x11;
    if (nativeKey == unsigned(-1))
        return false;
    return x11.isValid() && x11.grabKey(nativeKey, nativeMods, x11.rootWindow());
}

bool QxtGlobalShortcutPrivate::unregisterShortcut(quint32 nativeKey, quint32 nativeMods)
{
    QxtX11Data x11;
    if (nativeKey == unsigned(-1))
        return false;
    return x11.isValid() && x11.ungrabKey(nativeKey, nativeMods, x11.rootWindow());
}
#endif
