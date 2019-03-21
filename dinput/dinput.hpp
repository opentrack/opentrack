/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <QMutex>

#include "export.hpp"

#undef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x800

#include <dinput.h>

// XXX TODO -sh 20190209
// keybinding_worker and joystick context are badly named
// add namespaces and rename, including inner joystick device struct

using diptr = IDirectInput8A*;

class OTR_DINPUT_EXPORT di_t final
{
    static diptr handle;
    static QMutex lock;
    static diptr init_di_();
    static diptr init_di();

public:
    di_t();
    di_t(const di_t&) : di_t() {}
    di_t& operator=(const di_t&) = default;

    diptr operator->() const;
    operator bool() const;
    operator diptr() const;

    static bool poll_device(LPDIRECTINPUTDEVICE8 dev);
};
