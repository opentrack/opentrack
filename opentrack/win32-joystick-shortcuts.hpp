#pragma once

#ifdef _WIN32

#include <cstring>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>
#ifndef DIRECTINPUT_VERSION
#   define DIRECTINPUT_VERSION 0x800
#endif
#include <dinput.h>
#include <windows.h>
#include "opentrack-compat/timer.hpp"
#include <QString>
#include <QDebug>

struct win32_joy_ctx
{
    using fn = std::function<void(const QString& guid, int btn, bool held)>;

    void poll(fn f)
    {
        refresh(false);
        for (int i = joys.size() - 1; i >= 0; i--)
        {
            if (!joys[i]->poll(f))
                joys.erase(joys.begin() + i);
        }
    }

    struct joy
    {
        LPDIRECTINPUTDEVICE8 joy_handle;
        QString guid;
        bool pressed[128];
        Timer first_timer;
        bool first;
        
        enum { first_event_delay_ms = 3000 };

        joy(LPDIRECTINPUTDEVICE8 handle, const QString& guid, bool first) : joy_handle(handle), guid(guid), first(first)
        {
            qDebug() << "got joy" << guid;
            for (int i = 0; i < 128; i++)
                pressed[i] = false;
        }

        ~joy()
        {
            qDebug() << "nix joy" << guid;
            release();
        }

        void release()
        {
            if (joy_handle)
            {
                (void) joy_handle->Unacquire();
                joy_handle->Release();
                joy_handle = nullptr;
            }
        }

        bool poll(fn f)
        {
            HRESULT hr;
            bool ok = false;
            
            if (first_timer.elapsed_ms() > first_event_delay_ms)
                first = true;

            for (int i = 0; i < 5; i++)
            {
                if (!FAILED(joy_handle->Poll()))
                {
                    ok = true;
                    break;
                }
                if ((hr = joy_handle->Acquire()) != DI_OK)
                    continue;
                else
                    ok = true;
                break;
            }

            if (!ok)
            {
                qDebug() << "joy acquire failed" << guid << hr;
                return false;
            }

            DIJOYSTATE2 js;
            memset(&js, 0, sizeof(js));

            if (FAILED(hr = joy_handle->GetDeviceState(sizeof(js), &js)))
            {
                qDebug() << "joy get state failed" << guid << hr;
                return false;
            }

            for (int i = 0; i < 128; i++)
            {
                const bool state = !!(js.rgbButtons[i] & 0x80);
                if (state != pressed[i] && first)
                {
                    f(guid, i, state);
                    qDebug() << "btn" << guid << i << state;
                }
                pressed[i] = state;
            }

            return true;
        }
    };

    static QString guid_to_string(const GUID guid)
    {
        char buf[40] = {0};
        wchar_t szGuidW[40] = {0};

        StringFromGUID2(guid, szGuidW, 40);
        WideCharToMultiByte(0, 0, szGuidW, -1, buf, 40, NULL, NULL);

        return QString(buf);
    }

    win32_joy_ctx() : dinput_handle(nullptr)
    {
        (void) CoInitialize(nullptr);

        HRESULT hr;

        if (FAILED(hr = DirectInput8Create(GetModuleHandle(nullptr),
                                           DIRECTINPUT_VERSION,
                                           IID_IDirectInput8,
                                           (void**) &dinput_handle,
                                           nullptr)))
            goto fail;

        refresh(true);

        return;
fail:
        qDebug() << "dinput8 failed for shortcuts" << hr;

        release();
    }

    ~win32_joy_ctx()
    {
        release();
    }

    void release()
    {
        joys = std::vector<std::shared_ptr<joy>>();
        if (dinput_handle)
        {
            dinput_handle->Release();
            dinput_handle = nullptr;
        }
    }

    void refresh(bool first)
    {
        if (!dinput_handle)
            return;

        if (!first)
        {
            if (timer_joylist.elapsed_ms() < joylist_refresh_ms)
                return;
            timer_joylist.start();
        }

        enum_state st(dinput_handle, joys, first);
    }

    struct enum_state
    {
        std::vector<std::shared_ptr<joy>>& joys;
        std::vector<QString> all;
        LPDIRECTINPUT8 dinput_handle;
        bool first;

        enum_state(LPDIRECTINPUT8 di, std::vector<std::shared_ptr<joy>>& joys, bool first) : joys(joys), dinput_handle(di), first(first)
        {
            HRESULT hr;

            if(FAILED(hr = dinput_handle->EnumDevices(DI8DEVCLASS_GAMECTRL,
                                                      EnumJoysticksCallback,
                                                      this,
                                                      DIEDFL_ATTACHEDONLY)))
            {
                qDebug() << "failed enum joysticks" << hr;
                return;
            }

            for (int i = joys.size() - 1; i >= 0; i--)
            {
                const auto& guid = joys[i]->guid;
                if (std::find_if(all.cbegin(), all.cend(), [&](const QString& guid2) -> bool { return guid == guid2; }) == all.cend())
                    joys.erase(joys.begin() + i);
            }
        }

        static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
        {
            enum_state& state = *reinterpret_cast<enum_state*>(pContext);
            const QString guid = guid_to_string(pdidInstance->guidInstance);
#if 0
            const QString name = QString(pdidInstance->tszInstanceName);
            // the logic here is that iff multiple joysticks of same name exist, then take guids into account at all
            const int cnt_names = std::count_if(state.joys.begin(), state.joys.end(), [&](const joy& j) -> bool { return j.name == name; });
            // this is potentially bad since replugged sticks can change guids (?)
#endif

            const bool exists = std::find_if(state.joys.cbegin(),
                                             state.joys.cend(),
                                             [&](const std::shared_ptr<joy>& j) -> bool { return j->guid == guid; }) != state.joys.cend();

            state.all.push_back(guid);

            if (!exists)
            {
                HRESULT hr;
                LPDIRECTINPUTDEVICE8 h;
                if (FAILED(hr = state.dinput_handle->CreateDevice(pdidInstance->guidInstance, &h, nullptr)))
                {
                    qDebug() << "create joystick breakage" << guid << hr;
                    goto end;
                }
                if (FAILED(h->SetDataFormat(&c_dfDIJoystick2)))
                {
                    qDebug() << "format";
                    h->Release();
                    goto end;
                }

                if (FAILED(h->SetCooperativeLevel((HWND) GetDesktopWindow(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
                {
                    qDebug() << "coop";
                    h->Release();
                    goto end;
                }
#if 0
                if (FAILED(hr = h->EnumObjects(EnumObjectsCallback, h, DIDFT_ALL)))
                {
                    qDebug() << "enum-objects";
                    h->Release();
                    goto end;
                }
#endif
                state.joys.push_back(std::make_shared<joy>(h, guid, state.first));
            }

end:        return DIENUM_CONTINUE;
        }

#if 0
        static BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* ctx)
        {
            if (pdidoi->dwType & DIDFT_AXIS)
            {
                DIPROPRANGE diprg;
                memset(&diprg, 0, sizeof(diprg));
                diprg.diph.dwSize = sizeof( DIPROPRANGE );
                diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER );
                diprg.diph.dwHow = DIPH_BYID;
                diprg.diph.dwObj = pdidoi->dwType;
                diprg.lMax = 32;
                diprg.lMin = -32;

                if (FAILED(reinterpret_cast<LPDIRECTINPUTDEVICE8>(ctx)->SetProperty(DIPROP_RANGE, &diprg.diph)))
                    return DIENUM_STOP;
            }

            return DIENUM_CONTINUE;
        }
#endif
    };

    LPDIRECTINPUT8 dinput_handle;
    std::vector<std::shared_ptr<joy>> joys;
    Timer timer_joylist;
    enum { joylist_refresh_ms = 250 };
};

#endif
