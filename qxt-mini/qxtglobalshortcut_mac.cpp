#ifdef __APPLE__
#include <Carbon/Carbon.h>
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

#pragma GCC diagnostic ignored "-Wfour-char-constants"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "qxtglobalshortcut_p.h"
#include <QMap>
#include <QHash>
#include <QtDebug>
#include <QApplication>

typedef QPair<uint, uint> Identifier;
static QMap<quint32, EventHotKeyRef> keyRefs;
static QHash<Identifier, quint32> keyIDs;
static quint32 hotKeySerial = 0;

OSStatus qxt_mac_handle_hot_key(EventHandlerCallRef nextHandler, EventRef event, void* data)
{
    Q_UNUSED(nextHandler);
    Q_UNUSED(data);
    if (GetEventClass(event) == kEventClassKeyboard)
    {
        int kind = GetEventKind(event);
        bool is_down;

        if (kind == kEventHotKeyPressed)
            is_down = true;
        else if (kind == kEventHotKeyReleased)
            is_down = false;
        else
            return noErr;

        EventHotKeyID keyID;
        GetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, NULL, sizeof(keyID), NULL, &keyID);
        Identifier id = keyIDs.key(keyID.id);
        if(id != Identifier())
            QxtGlobalShortcutPrivate::activateShortcut(id.second, id.first, is_down);
    }
    return noErr;
}

// Constants found in NSEvent.h from AppKit.framework
// cf. https://stackoverflow.com/a/16125341
// cf. https://web.archive.org/web/20100501161453/http://www.classicteck.com/rbarticles/mackeyboard.php

static struct {
    Qt::Key qt_key;
    unsigned native_key;
} const key_list[] = {
    { Qt::Key_Escape                    , 0x35 },
    { Qt::Key_Return                    , 0x24 },
    { Qt::Key_Return                    , 0x4c },
    { Qt::Key_Tab                       , 0x30 },
    { Qt::Key_Space                     , 0x31 },
    { Qt::Key_Delete                    , 0x33 },
    { Qt::Key_F17                       , 0x40 },
    { Qt::Key_VolumeUp                  , 0x48 },
    { Qt::Key_VolumeDown                , 0x49 },
    { Qt::Key_Mute                      , 0x4A },
    { Qt::Key_F1                        , 0x7A },
    { Qt::Key_F2                        , 0x78 },
    { Qt::Key_F3                        , 0x63 },
    { Qt::Key_F4                        , 0x76 },
    { Qt::Key_F5                        , 0x60 },
    { Qt::Key_F6                        , 0x61 },
    { Qt::Key_F7                        , 0x62 },
    { Qt::Key_F8                        , 0x64 },
    { Qt::Key_F9                        , 0x65 },
    { Qt::Key_F10                       , 0x6D },
    { Qt::Key_F11                       , 0x67 },
    { Qt::Key_F12                       , 0x6F },
    { Qt::Key_Help                      , 0x72 },
    { Qt::Key_PageUp                    , 0x74 },
    { Qt::Key_PageDown                  , 0x79 },
    { Qt::Key_Delete                    , 0x33 },
    { Qt::Key_Backspace                 , 0x75 },
    { Qt::Key_Home                      , 0x73 },
    { Qt::Key_End                       , 0x77 },
    { Qt::Key_LeftArrow                 , 0x7B },
    { Qt::Key_RightArrow                , 0x7C },
    { Qt::Key_DownArrow                 , 0x7D },
    { Qt::Key_UpArrow                   , 0x7E },
};

static struct modifiers {
    Qt::KeyboardModifiers qt_mods;
    unsigned native_mods;
} const modifier_list[] = {
    { Qt::ShiftModifier,    56 },
    { Qt::ControlModifier,  59 },
    { Qt::AltModifier,      55 },
    { Qt::MetaModifier,     58 },
    //{ Qt::KeypadModifier,   kEventKeyModifierNumLockMask },
};

quint32 QxtGlobalShortcutPrivate::nativeModifiers(Qt::KeyboardModifiers modifiers)
{
    quint32 native = 0;

    for (const auto& m : modifiers)
        if (modifiers & m.qt_mods)
            native |= m.native_mods;

    return native;
}

quint32 QxtGlobalShortcutPrivate::nativeKeycode(Qt::Key keys)
{
    UTF16Char ch;

    for (k const& : key_list)
        if (k.qt_key == keys)
            return k.native_key;

    if (keys == Qt::Key_Escape)	ch = 27;
    else if (keys == Qt::Key_Return) ch = 13;
    else if (keys == Qt::Key_Enter) ch = 3;
    else if (keys == Qt::Key_Tab) ch = 9;
    else ch = keys;

    CFDataRef currentLayoutData;
    TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();

    if (currentKeyboard == NULL)
        return 0;

    currentLayoutData = (CFDataRef)TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
    CFRelease(currentKeyboard);
    if (currentLayoutData == NULL)
        return 0;

    UCKeyboardLayout* header = (UCKeyboardLayout*)CFDataGetBytePtr(currentLayoutData);
    UCKeyboardTypeHeader* table = header->keyboardTypeList;

    uint8_t *data = (uint8_t*)header;
    // God, would a little documentation for this shit kill you...
    for (quint32 i=0; i < header->keyboardTypeCount; i++)
    {
        UCKeyStateRecordsIndex* stateRec = 0;
        if (table[i].keyStateRecordsIndexOffset != 0)
        {
            stateRec = reinterpret_cast<UCKeyStateRecordsIndex*>(data + table[i].keyStateRecordsIndexOffset);
            if (stateRec->keyStateRecordsIndexFormat != kUCKeyStateRecordsIndexFormat) stateRec = 0;
        }

        UCKeyToCharTableIndex* charTable = reinterpret_cast<UCKeyToCharTableIndex*>(data + table[i].keyToCharTableIndexOffset);
        if (charTable->keyToCharTableIndexFormat != kUCKeyToCharTableIndexFormat) continue;

        for (quint32 j=0; j < charTable->keyToCharTableCount; j++)
        {
            UCKeyOutput* keyToChar = reinterpret_cast<UCKeyOutput*>(data + charTable->keyToCharTableOffsets[j]);
            for (quint32 k=0; k < charTable->keyToCharTableSize; k++)
            {
                if (keyToChar[k] & kUCKeyOutputTestForIndexMask)
                {
                    long idx = keyToChar[k] & kUCKeyOutputGetIndexMask;
                    if (stateRec && idx < stateRec->keyStateRecordCount)
                    {
                        UCKeyStateRecord* rec = reinterpret_cast<UCKeyStateRecord*>(data + stateRec->keyStateRecordOffsets[idx]);
                        if (rec->stateZeroCharData == ch) return k;
                    }
                }
                else if (!(keyToChar[k] & kUCKeyOutputSequenceIndexMask) && keyToChar[k] < 0xFFFE)
                {
                    if (keyToChar[k] == ch) return k;
                }
            } // for k
        } // for j
    } // for i
    return 0;
}

static bool register_event_handler()
{
    EventTypeSpec specs[2] = {
        { kEventClassKeyboard, kEventHotKeyPressed  },
        { kEventClassKeyboard, kEventHotKeyReleased },
    };

    (void)InstallApplicationEventHandler(&qxt_mac_handle_hot_key, 2, &t, NULL, NULL);

    return true;
}

bool QxtGlobalShortcutPrivate::registerShortcut(quint32 nativeKey, quint32 nativeMods)
{
    static const bool once = register_event_handler();

    EventHotKeyID keyID;
    keyID.signature = 'cute';
    keyID.id = ++hotKeySerial;

    EventHotKeyRef ref = 0;
    bool rv = !RegisterEventHotKey(nativeKey, nativeMods, keyID, GetApplicationEventTarget(), 0, &ref);
    if (rv)
    {
        keyIDs.insert(Identifier(nativeMods, nativeKey), keyID.id);
        keyRefs.insert(keyID.id, ref);
    }
    return rv;
}

bool QxtGlobalShortcutPrivate::unregisterShortcut(quint32 nativeKey, quint32 nativeMods)
{
    Identifier id{nativeMods, nativeKey};
    if (!keyIDs.contains(id))
        return false;

    EventHotKeyRef ref = keyRefs.take(keyIDs[id]);
    keyIDs.remove(id);
    return !UnregisterEventHotKey(ref);
}

bool QxtGlobalShortcutPrivate::nativeEventFilter(const QByteArray & eventType, void *message, long *result)
{
    return false;
}
#endif
