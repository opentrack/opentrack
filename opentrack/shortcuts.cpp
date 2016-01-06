/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "shortcuts.h"
#include "win32-shortcuts.h"

void Shortcuts::bind_keyboard_shortcut(K &key, const key_opts& k)
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
void Shortcuts::receiver(const Key& k)
{
    const int sz = keys.size();
    for (int i = 0; i < sz; i++)
    {
        K& k_ = std::get<0>(keys[i]);
        auto& fun = std::get<1>(keys[i]);
        if (k.guid != k_.guid)
            continue;
        if (k.keycode != k_.keycode)
            continue;
        if (!k.held)
            continue;
        if (!k_.should_process())
            continue;
        if (k_.alt && !k.alt) continue;
        if (k_.ctrl && !k.ctrl) continue;
        if (k_.shift && !k.shift) continue;
        
        fun();
    }
}
#endif

void Shortcuts::reload(const std::vector<std::tuple<key_opts&, fun> > &keys_)
{
    const int sz = keys_.size();
    keys = std::vector<tt>();
    
    for (int i = 0; i < sz; i++)
    {
        const auto& kk = keys_[i];
        const key_opts& opts = std::get<0>(kk);
        auto& fun = std::get<1>(kk);
        K k;
        bind_keyboard_shortcut(k, opts);
        keys.push_back(std::tuple<K, Shortcuts::fun>(k, fun));
#ifndef _WIN32
        connect(k.get(), &QxtGlobalShortcut::activated, fun);
#endif
    }
}
