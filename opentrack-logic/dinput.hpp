#pragma once

#ifdef _WIN32

#ifndef DIRECTINPUT_VERSION
#   define DIRECTINPUT_VERSION 0x800
#endif
#include "export.hpp"
#include <dinput.h>
#include <windows.h>

struct OPENTRACK_LOGIC_EXPORT dinput_handle final
{
    using di_t = LPDIRECTINPUT8;
private:
    static dinput_handle self;
    dinput_handle();
    ~dinput_handle();
    static di_t init_di();
    di_t handle;
public:
     static di_t make_di();

     dinput_handle(const dinput_handle&) = delete;
     dinput_handle(dinput_handle&&) = delete;
};

#endif
