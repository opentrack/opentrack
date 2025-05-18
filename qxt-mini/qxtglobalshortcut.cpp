#include "qxtglobalshortcut.h"
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

#include "qxtglobalshortcut_p.h"

#include <QAbstractEventDispatcher>
#include <QApplication>
#include <QtDebug>
#include <QtGlobal>

#ifdef __GNUG__
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

QMultiMap<QPair<quint32, quint32>, QxtGlobalShortcut*> QxtGlobalShortcutPrivate::shortcuts;

void QxtGlobalShortcutPrivate::event_filter_installer::ensure_event_filter()
{
#ifndef Q_OS_MAC
    QAbstractEventDispatcher* instance = QAbstractEventDispatcher::instance();
    if (instance)
    {
        static QxtGlobalShortcutPrivate filter(QxtGlobalShortcutPrivate::tag {});
        static bool installed =
            ((void)instance->installNativeEventFilter(&filter), true);
        Q_UNUSED(installed)
    }
#endif
}

QxtGlobalShortcutPrivate::QxtGlobalShortcutPrivate(QxtGlobalShortcutPrivate::tag) :
    keystate(false), enabled(false), key(Qt::Key(0)), mods(Qt::NoModifier)
{
    //qDebug() << "qxt-mini: adding event filter";
}

QxtGlobalShortcutPrivate::QxtGlobalShortcutPrivate() :
    keystate(false), enabled(true), key(Qt::Key(0)), mods(Qt::NoModifier)
{
    QxtGlobalShortcutPrivate::event_filter_installer::ensure_event_filter();
}

QxtGlobalShortcutPrivate::~QxtGlobalShortcutPrivate()
{
    unsetShortcut();
}

bool QxtGlobalShortcutPrivate::setShortcut(const QKeySequence& shortcut)
{
    (void) unsetShortcut();

    if (shortcut.toString() == "") return false;
    Qt::KeyboardModifiers allMods = Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier | Qt::KeypadModifier;
    key = shortcut.isEmpty() ? Qt::Key(0) : Qt::Key((unsigned)shortcut[0] & ~allMods);
    mods = shortcut.isEmpty() ? Qt::KeyboardModifier(0) : Qt::KeyboardModifiers((unsigned)shortcut[0] & allMods);

    const quint32 nativeKey = nativeKeycode(key);
    const quint32 nativeMods = nativeModifiers(mods);
    const bool res = registerShortcut(nativeKey, nativeMods);
    if (res)
    {
#ifndef Q_OS_MAC
        shortcuts.insertMulti(qMakePair(nativeKey, nativeMods), &qxt_p());
#else
        shortcuts.insert(qMakePair(nativeKey, nativeMods), &qxt_p());
#endif
    }
    else
        qWarning() << "QxtGlobalShortcut failed to register:" << shortcut;
    return res;
}

bool QxtGlobalShortcutPrivate::unsetShortcut()
{
    if (key == Qt::Key(0))
        return true;

    const quint32 nativeKey = nativeKeycode(key);
    const quint32 nativeMods = nativeModifiers(mods);
#ifdef Q_OS_MAC
    bool res = false;
    if (shortcuts.value(qMakePair(nativeKey, nativeMods)) == &qxt_p())
        res = unregisterShortcut(nativeKey, nativeMods);
    if (res)
        shortcuts.remove(qMakePair(nativeKey, nativeMods));
    else
        qWarning() << "QxtGlobalShortcut failed to unregister:" << QKeySequence(key + mods).toString();
#else
    using IT = decltype(shortcuts.end());
    const auto pair = qMakePair(nativeKey, nativeMods);
    IT it = shortcuts.find(pair);
    bool found = false;
    for (; it != shortcuts.end(); it++)
    {
        if (it.key() != pair) // DO NOT REMOVE
            break;
        if (*it == &qxt_p())
        {
            found = true;
            shortcuts.erase(it);
            break;
        }

    }
    if (!found)
        qDebug() << "qxt-mini: can't find shortcut for" << key << mods;
    bool res = unregisterShortcut(nativeKey, nativeMods);
#endif
    key = Qt::Key(0);
    mods = Qt::KeyboardModifier(0);
    return res;
}

void QxtGlobalShortcutPrivate::activateShortcut(quint32 nativeKey, quint32 nativeMods, bool is_down)
{
#ifndef Q_OS_MAC
    using IT = decltype(shortcuts.end());
    const auto pair = qMakePair(nativeKey, nativeMods);
    IT it = shortcuts.find(pair);

    bool once = false;

    for (; it != shortcuts.end(); it++)
    {
        if (it.key() != pair) // DO NOT REMOVE
            break;

        auto ptr = *it;
        auto& priv = ptr->qxt_d();

        if (priv.keystate == is_down)
        {
            continue;
        }

        if (!once)
        {
            once = true;
            qDebug() << "qxt-mini:" << (is_down ? "keydown" : "keyup") << priv.key << priv.mods;
        }

        priv.keystate = is_down;

        if (ptr->isEnabled())
            emit ptr->activated(is_down);
    }
#else
    QxtGlobalShortcut* shortcut = shortcuts.value(qMakePair(nativeKey, nativeMods));

    if (shortcut)
    {
        shortcut->qxt_d().keystate = false;

        if (shortcut->isEnabled())
            emit shortcut->activated(is_down);
    }
#endif
}

/*!
    \class QxtGlobalShortcut
    \inmodule QxtWidgets
    \brief The QxtGlobalShortcut class provides a global shortcut aka "hotkey".

    A global shortcut triggers even if the application is not active. This
    makes it easy to implement applications that react to certain shortcuts
    still if some other application is active or if the application is for
    example minimized to the system tray.

    Example usage:
    \code
    QxtGlobalShortcut* shortcut = new QxtGlobalShortcut(window);
    connect(shortcut, SIGNAL(activated()), window, SLOT(toggleVisibility()));
    shortcut->setShortcut(QKeySequence("Ctrl+Shift+F12"));
    \endcode

    \bold {Note:} Since Qxt 0.6 QxtGlobalShortcut no more requires QxtApplication.
 */

/*!
    \fn void QxtGlobalShortcut::activated(bool keydown=true)

    This signal is emitted when the user types the shortcut's key sequence.

    \sa shortcut
 */

/*!
    Constructs a new QxtGlobalShortcut with \a parent.
 */
QxtGlobalShortcut::QxtGlobalShortcut(QObject* parent)
        : QObject(parent)
{
    QXT_INIT_PRIVATE(QxtGlobalShortcut)
}

/*!
    Constructs a new QxtGlobalShortcut with \a shortcut and \a parent.
 */
QxtGlobalShortcut::QxtGlobalShortcut(const QKeySequence& shortcut, QObject* parent)
        : QObject(parent)
{
    QXT_INIT_PRIVATE(QxtGlobalShortcut)
    setShortcut(shortcut);
}

/*!
    Destructs the QxtGlobalShortcut.
 */
QxtGlobalShortcut::~QxtGlobalShortcut()
{
    if (qxt_d().key != 0)
        qxt_d().unsetShortcut();
}

/*!
    \property QxtGlobalShortcut::shortcut
    \brief the shortcut key sequence

    \note Notice that corresponding key press and release events are not
    delivered for registered global shortcuts even if they are disabled.
    Also, comma separated key sequences are not supported.
    Only the first part is used:

    \code
    qxtShortcut->setShortcut(QKeySequence("Ctrl+Alt+A,Ctrl+Alt+B"));
    Q_ASSERT(qxtShortcut->shortcut() == QKeySequence("Ctrl+Alt+A"));
    \endcode
 */
QKeySequence QxtGlobalShortcut::shortcut() const
{
    return QKeySequence((int)(qxt_d().key | qxt_d().mods));
}

bool QxtGlobalShortcut::setShortcut(const QKeySequence& shortcut)
{
    if (qxt_d().key != 0)
        qxt_d().unsetShortcut();
    return qxt_d().setShortcut(shortcut);
}

/*!
    \property QxtGlobalShortcut::enabled
    \brief whether the shortcut is enabled

    A disabled shortcut does not get activated.

    The default value is \c true.

    \sa setDisabled()
 */
bool QxtGlobalShortcut::isEnabled() const
{
    return qxt_d().enabled;
}

void QxtGlobalShortcut::setEnabled(bool enabled)
{
    qxt_d().enabled = enabled;
}

/*!
    Sets the shortcut \a disabled.

    \sa enabled
 */
void QxtGlobalShortcut::setDisabled(bool disabled)
{
    qxt_d().enabled = !disabled;
}
