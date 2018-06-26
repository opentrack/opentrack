/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "export.hpp"

#include <atomic>

#undef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x800

#include <dinput.h>

using diptr = IDirectInput8A*;

class OTR_DINPUT_EXPORT di_t final
{
    static void unref_di();
    static void ref_di();

    static diptr handle;
    static std::atomic<int> refcnt;
    static std::atomic_flag init_lock;
    static diptr init_di_();

public:
    di_t();
    ~di_t();
    di_t(const di_t&) : di_t() {}
    di_t& operator=(const di_t&) { return *this; }

    diptr operator->() const { return handle; }
    operator bool() { return handle; }

    // for debugging bad usages
    template<typename t = void>
    operator void*() const
    {
        static_assert(sizeof(t) == -1);
        static_assert(sizeof(t) == 0);

        return nullptr;
    }
};

