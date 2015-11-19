/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "shortcuts.h"
#include "win32-shortcuts.h"

void Shortcuts::bind_keyboard_shortcut(K &key, key_opts& k)
{
#if !defined(_WIN32)
    using sh = QxtGlobalShortcut;
    if (key)
    {
        key->setEnabled(false);
        key->setShortcut(QKeySequence::UnknownKey);
        std::shared_ptr<sh> tmp(nullptr);
        key.swap(tmp);
    }

    key = std::make_shared<sh>();

    if (k.keycode != "")
    {
        key->setShortcut(QKeySequence::fromString(k.keycode, QKeySequence::PortableText));
        key->setEnabled();
    }
}
#else
    key = K();
    int idx = 0;
    QKeySequence code;
    
    if (k.guid != "")
    {
        key.guid = k.guid;
        key.keycode = k.button & ~Qt::KeyboardModifierMask;
        key.ctrl = !!(k.button & Qt::ControlModifier);
        key.alt = !!(k.button & Qt::AltModifier);
        key.shift = !!(k.button & Qt::ShiftModifier);
    }
    else
    {
        if (k.keycode == "")
            code = QKeySequence(Qt::Key_unknown);
        else
            code = QKeySequence::fromString(k.keycode, QKeySequence::PortableText);
    
        Qt::KeyboardModifiers mods = Qt::NoModifier;
        if (code != Qt::Key_unknown)
            win_key::from_qt(code, idx, mods);
        key.shift = !!(mods & Qt::ShiftModifier);
        key.alt = !!(mods & Qt::AltModifier);
        key.ctrl = !!(mods & Qt::ControlModifier);
        key.keycode = idx;
    }
}
#endif

#ifdef _WIN32
void Shortcuts::receiver(Key &k)
{
    std::vector<K*> ks { &keyCenter, &keyToggle, &keyZero };
    for (K* k_ : ks)
    {
        if (k.guid != k_->guid)
            continue;
        if (k.keycode != k_->keycode)
            continue;
        if (!k.held)
            continue;
        if (!k_->should_process())
            continue;
        if (k_->alt && !k.alt) continue;
        if (k_->ctrl && !k.ctrl) continue;
        if (k_->shift && !k.shift) continue;
        
        if (k_ == &keyCenter)
            emit center();
        else if (k_ == &keyToggle)
            emit toggle();
        else if (k_ == &keyZero)
            emit zero();
    }
}
#endif

void Shortcuts::reload() {
    bind_keyboard_shortcut(keyCenter, s.center);
    bind_keyboard_shortcut(keyToggle, s.toggle);
    bind_keyboard_shortcut(keyZero, s.zero);
#ifdef _WIN32
    bool is_new = keybindingWorker == nullptr;
    if (is_new)
    {
        keybindingWorker = std::make_shared<KeybindingWorker>([&](Key& k) { receiver(k); }, handle);
        keybindingWorker->start();
    }
#endif
}
