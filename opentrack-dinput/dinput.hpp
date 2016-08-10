/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#ifdef _WIN32

#include "export.hpp"

#ifndef DIRECTINPUT_VERSION
#   define DIRECTINPUT_VERSION 0x800
#endif
#include <dinput.h>

#include <atomic>

class OPENTRACK_DINPUT_EXPORT dinput_handle final
{
public:
    class di_t;

private:
    static std::atomic<int> refcnt;
    static std::atomic_flag init_lock;
    static di_t handle;

    static LPDIRECTINPUT8& init_di();
public:
    class di_t final
    {
        friend class dinput_handle;

        LPDIRECTINPUT8& handle;

        di_t(LPDIRECTINPUT8& handle);
        void free_di();

    public:
        LPDIRECTINPUT8 operator->() { return handle; }
        operator LPDIRECTINPUT8() { return handle; }
        LPDIRECTINPUT8 di() { return handle; }

        ~di_t();
    };

    static di_t make_di();
    dinput_handle() = delete;
};

#endif
