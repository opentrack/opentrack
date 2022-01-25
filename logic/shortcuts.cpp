/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "shortcuts.h"
#include "win32-shortcuts.h"

#include <QString>

#include <tuple>

void Shortcuts::free_binding(K& key)
{
#ifndef _WIN32
    if (key)
    {
        //key->setEnabled(false);
        //key->setShortcut(QKeySequence());
        delete key;
        key = nullptr;
    }
#else
    key.keycode = 0;
    key.guid = "";
#endif
}

void Shortcuts::bind_shortcut(K& key, const key_opts& k, bool held)
{
#if !defined _WIN32
    (void)held;
    using sh = QxtGlobalShortcut;
    if (key)
    {
        free_binding(key);
    }

    key = new sh;

    if (k.keycode != "")
    {
        key->setShortcut(QKeySequence::fromString(k.keycode, QKeySequence::PortableText));
        key->setEnabled();
    }
#else
    key = {};
    int idx = 0;
    QKeySequence code(QKeySequence::UnknownKey);

    if (k.guid == QStringLiteral("mouse"))
    {
        key.guid = k.guid;
        key.keycode = k.button;
        key.held = held;
    }
    if (!k.guid->isEmpty())
    {
        key.guid = k.guid;
        key.keycode = k.button & ~Qt::KeyboardModifierMask;
        key.held = held;
        key.ctrl = !!(k.button & Qt::ControlModifier);
        key.alt = !!(k.button & Qt::AltModifier);
        key.shift = !!(k.button & Qt::ShiftModifier);
    }
    else
    {
        if (!k.keycode->isEmpty())
            code = QKeySequence::fromString(k.keycode, QKeySequence::PortableText);

        Qt::KeyboardModifiers mods = Qt::NoModifier;
        if (!code.isEmpty() &&
            code != QKeySequence{ QKeySequence::UnknownKey } &&
            win_key::from_qt(code, idx, mods))
        {
            key.guid = QString{};
            key.keycode = idx;
            key.held = held;
            key.ctrl = !!(mods & Qt::ControlModifier);
            key.alt = !!(mods & Qt::AltModifier);
            key.shift = !!(mods & Qt::ShiftModifier);
        }
    }
#endif
}

#ifdef _WIN32

void Shortcuts::receiver(const Key& key)
{
    const unsigned sz = keys.size();
    for (unsigned i = 0; i < sz; i++)
    {
        auto& [k, f, held] = keys[i];
        if (key.guid != k.guid)
            continue;
        if (key.keycode != k.keycode)
            continue;

        if (k.held && !key.held) continue;
        if (key.held)
        {
            if (k.alt != key.alt) continue;
            if (k.ctrl != key.ctrl) continue;
            if (k.shift != key.shift) continue;
            if (!k.should_process())
                continue;
        }
        f(key.held);
    }
}

#endif

Shortcuts::~Shortcuts()
{
    reload({});
}

void Shortcuts::reload(const t_keys& keys_)
{
    const unsigned sz = keys_.size();
#ifndef _WIN32
    for (tt& tuple : keys)
    {
        K k;
        std::tie(k, std::ignore, std::ignore) = tuple;
        delete k;
    }
#endif
    keys = std::vector<tt>();

    for (unsigned i = 0; i < sz; i++)
    {
        auto const&[opts, fun, held] = keys_[i];
#ifdef _WIN32
        K k;
#else
        K k(nullptr);
#endif
        bind_shortcut(k, opts, held);
        keys.emplace_back(k, fun, held);

#ifndef _WIN32
        const int idx = keys.size() - 1;
        tt& kk_ = keys[idx];
        auto fn = std::get<1>(kk_);
        bool held_ = held;
        connect(k, &QxtGlobalShortcut::activated, [fn, held_](bool keydown) {
            if (keydown || !held_)
                fn(keydown);
        });
#endif
    }
}
