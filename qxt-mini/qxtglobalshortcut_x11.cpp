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
#include <QMutexLocker>
#include <QDebug>

#include <QApplication>
// include private header for great justice -sh 20131015
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include "qplatformnativeinterface.h"
#include "compat/util.hpp"

static constexpr quint32 AllMods = ShiftMask|LockMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask;

typedef int (*X11ErrorHandler)(Display *display, XErrorEvent *event);

struct keybinding final
{
    quint32 code;
    int refcnt;

    static QHash<quint32, keybinding> list;
    static QMutex lock;

    static bool incf(quint32 code);
    static bool decf(quint32 code);

    ~keybinding();

private:
    keybinding(quint32 code);
};

bool operator==(const keybinding& k1, const keybinding& k2)
{
    return k1.code == k2.code;
}

inline bool operator!=(const keybinding& k1, const keybinding& k2)
{
    return !(k1 == k2);
}

uint qHash(const keybinding& k)
{
    return qHash(k.code);
}

uint qHash(const keybinding& k, uint seed)
{
    return qHash(k.code, seed);
}

keybinding::keybinding(quint32 code) : code(code), refcnt(0)
{
}

keybinding::~keybinding()
{
}

bool keybinding::incf(quint32 code)
{
    QMutexLocker l(&lock);

    keybinding k = list.value(code, keybinding(code));

    const bool ret = k.refcnt == 0;

    if (ret)
    {
        //qDebug() << "qxt-mini: registered keybinding" << code;
    }

    k.refcnt++;
    list.insert(code, k);

    //qDebug() << "qxt-mini: incf: refcount for" << code << "now" << k.refcnt;

    return ret;
}

bool keybinding::decf(quint32 code)
{
    QMutexLocker l(&lock);

    auto it = list.find(code);

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

QHash<quint32, keybinding> keybinding::list;
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

    bool grabKey(quint32 keycode, Window window)
    {
        if (keybinding::incf(keycode))
        {
            QxtX11ErrorHandler errorHandler;

            XGrabKey(display(), keycode, AllMods, window, True,
                     GrabModeAsync, GrabModeAsync);

            if (errorHandler.error) {
                ungrabKey(keycode, window);
                return false;
            }
        }

        return true;
    }

    bool ungrabKey(quint32 keycode, Window window)
    {
        if (keybinding::decf(keycode))
        {
            QxtX11ErrorHandler errorHandler;
            XUngrabKey(display(), keycode, AllMods, window);
            return !errorHandler.error;
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

    xcb_key_press_event_t *kev = 0;
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t *ev = static_cast<xcb_generic_event_t *>(message);
        if ((ev->response_type & 127) == XCB_KEY_PRESS)
            kev = static_cast<xcb_key_press_event_t *>(message);
    }

    if (kev != 0) {
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
        if(kev->state & XCB_MOD_MASK_2) //numlock
            keystate |= Mod2Mask;
#if 0
        if(key->state & XCB_MOD_MASK_3) // alt gr aka right-alt or ctrl+left-alt -- what mask is it?
            keystate |= AltGrMask;
#endif

        activateShortcut(keycode, keystate);
    }
    return false;
}

quint32 QxtGlobalShortcutPrivate::nativeModifiers(Qt::KeyboardModifiers modifiers)
{
    // XXX TODO make a lookup table
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

#if 0
    if (modifiers & Qt::MetaModifier) // dunno the native mask
        native |= Mod4Mask;
#endif
    if (modifiers & Qt::KeypadModifier) // numlock
        native |= Mod2Mask;

    native &= AllMods;

    return native;
}

quint32 QxtGlobalShortcutPrivate::nativeKeycode(Qt::Key key)
{
    QxtX11Data x11;
    if (!x11.isValid())
        return 0;

    QByteArray tmp(QKeySequence(key).toString().toLatin1());

    KeySym keysym = XStringToKeysym(tmp.data());
    if (keysym == NoSymbol)
        keysym = static_cast<ushort>(key);

    const quint32 ret = XKeysymToKeycode(x11.display(), keysym);

    //qDebug() << "key is" << key << QKeySequence(key).toString(QKeySequence::PortableText) << ret;

    return ret;
}

bool QxtGlobalShortcutPrivate::registerShortcut(quint32 nativeKey, unused(quint32, nativeMods))
{
    QxtX11Data x11;
    return x11.isValid() && x11.grabKey(nativeKey, x11.rootWindow());
}

bool QxtGlobalShortcutPrivate::unregisterShortcut(quint32 nativeKey, unused(quint32, nativeMods))
{
    QxtX11Data x11;
    return x11.isValid() && x11.ungrabKey(nativeKey, x11.rootWindow());
}
#endif
