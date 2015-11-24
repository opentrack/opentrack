#pragma once

#ifdef _WIN32

#include <cstring>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>
#include <unordered_map>
#ifndef DIRECTINPUT_VERSION
#   define DIRECTINPUT_VERSION 0x800
#endif
#include <dinput.h>
#include <windows.h>
#include "opentrack-compat/timer.hpp"
#include <QString>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>

namespace std {
template<>
struct hash<QString>
{
    std::size_t operator()(const QString& value) const
    {
        return qHash(value);
    }
};
}

#ifdef BUILD_api
#   include "opentrack-compat/export.hpp"
#else
#   include "opentrack-compat/import.hpp"
#endif

struct OPENTRACK_EXPORT win32_joy_ctx
{
    using fn = std::function<void(const QString& guid, int btn, bool held)>;

    void poll(fn f)
    {
        QMutexLocker l(&mtx);

        refresh(false);
        for (auto& j : joys())
        {
            j.second->poll(f);
        }
    }
    
    bool poll_axis(const QString& guid, int axes[8])
    {
        QMutexLocker l(&mtx);
        
        refresh(false);
        
        auto iter = joys().find(guid);

        if (iter == joys().end() || iter->second->joy_handle == nullptr)
            return false;
        
        auto& j = iter->second;
        
        auto& joy_handle = j->joy_handle;
        bool ok = false;
        HRESULT hr;
        
        (void) joy_handle->Acquire();
        
        if (!FAILED(hr = joy_handle->Poll()))
            ok = true;
        
        if (!ok)
        {
            qDebug() << "joy acquire failed" << guid << hr;
            (void) joy_handle->Unacquire();
            return false;
        }
        
        DIJOYSTATE2 js;
        memset(&js, 0, sizeof(js));
        
        if (FAILED(hr = joy_handle->GetDeviceState(sizeof(js), &js)))
        {
            qDebug() << "joy get state failed" << guid << hr;
            return false;
        }
        
        const int values[] = {
            js.lX,
            js.lY,
            js.lZ,
            js.lRx,
            js.lRy,
            js.lRz,
            js.rglSlider[0],
            js.rglSlider[1]
        };
        
        for (int i = 0; i < 8; i++)
            axes[i] = values[i];
        
        return true;
    }
    
    ~win32_joy_ctx()
    {
        release();
    }
    
    enum { joy_axis_size = 65535 };
    
    struct joy_info
    {
        QString name, guid;
    };
    
    std::vector<joy_info> get_joy_info()
    {
        QMutexLocker l(&mtx);
        
        std::vector<joy_info> ret;
        
        for (auto& j : joys())
            ret.push_back(joy_info { j.second->name, j.first });
        
        return ret;
    }

    static win32_joy_ctx& make();
    
    win32_joy_ctx(const win32_joy_ctx&) = delete;
    win32_joy_ctx& operator=(const win32_joy_ctx&) = delete;
private:
    static QString guid_to_string(const GUID guid)
    {
        char buf[40] = {0};
        wchar_t szGuidW[40] = {0};

        StringFromGUID2(guid, szGuidW, 40);
        WideCharToMultiByte(0, 0, szGuidW, -1, buf, 40, NULL, NULL);

        return QString(buf);
    }
    
    static LPDIRECTINPUT8& dinput_handle();
    
    win32_joy_ctx()
    {
        refresh(true);
    }
    void release()
    {
        qDebug() << "release joystick dinput handle";
        joys() = std::unordered_map<QString, std::shared_ptr<joy>>();
        {
            auto& di = dinput_handle();
            di->Release();
            di = nullptr;
        }
    }

    void refresh(bool first)
    {
        if (!first)
        {
            if (timer_joylist.elapsed_ms() < joylist_refresh_ms)
                return;
            timer_joylist.start();
        }

        enum_state st(joys(), first);
    }
    QMutex mtx;
    Timer timer_joylist;
    enum { joylist_refresh_ms = 250 };
    
    struct joy
    {
        LPDIRECTINPUTDEVICE8 joy_handle;
        QString guid, name;
        bool pressed[128];
        Timer first_timer;
        bool first;
        
        enum { first_event_delay_ms = 3000 };

        joy(LPDIRECTINPUTDEVICE8 handle, const QString& guid, const QString& name, bool first) : joy_handle(handle), guid(guid), name(name), first(first)
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
            
            if (joy_handle == nullptr)
                return false;
            
            (void) joy_handle->Acquire();
            
            if (!FAILED(joy_handle->Poll()))
                ok = true;

            if (!ok)
            {
                qDebug() << "joy acquire failed" << guid << hr;
                (void) joy_handle->Unacquire();
                return false;
            }

            DIJOYSTATE2 js;
            memset(&js, 0, sizeof(js));

            if (FAILED(hr = joy_handle->GetDeviceState(sizeof(js), &js)))
            {
                qDebug() << "joy get state failed" << guid << hr;
                return false;
            }
            
            first |= first_timer.elapsed_ms() > first_event_delay_ms;

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

    struct enum_state
    {
        std::unordered_map<QString, std::shared_ptr<joy>>& joys;
        std::vector<QString> all;
        bool first;

        enum_state(std::unordered_map<QString, std::shared_ptr<joy>>& joys, bool first) : joys(joys), first(first)
        {
            HRESULT hr;
            LPDIRECTINPUT8 di = dinput_handle();

            if(FAILED(hr = di->EnumDevices(DI8DEVCLASS_GAMECTRL,
                                           EnumJoysticksCallback,
                                           this,
                                           DIEDFL_ATTACHEDONLY)))
            {
                qDebug() << "failed enum joysticks" << hr;
                return;
            }
            
            for (auto it = joys.begin(); it != joys.end(); )
            {
                if (std::find_if(all.cbegin(), all.cend(), [&](const QString& guid2) -> bool { return it->second->guid == guid2; }) == all.cend())
                    it = joys.erase(it);
                else
                    it++;
            }
        }

        static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
        {
            enum_state& state = *reinterpret_cast<enum_state*>(pContext);
            const QString guid = guid_to_string(pdidInstance->guidInstance);
            const QString name = QString(pdidInstance->tszInstanceName);
            
            auto it = state.joys.find(guid);
            const bool exists = it != state.joys.end() && it->second->joy_handle != nullptr;

            state.all.push_back(guid);

            if (!exists)
            {
                HRESULT hr;
                LPDIRECTINPUTDEVICE8 h;
                LPDIRECTINPUT8 di = dinput_handle();
                if (FAILED(hr = di->CreateDevice(pdidInstance->guidInstance, &h, nullptr)))
                {
                    qDebug() << "createdevice" << guid << hr;
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
                if (FAILED(hr = h->EnumObjects(EnumObjectsCallback, h, DIDFT_ALL)))
                {
                    qDebug() << "enum-objects";
                    h->Release();
                    goto end;
                }
                
                qDebug() << "add joy" << guid;
                state.joys[guid] = std::make_shared<joy>(h, guid, name, state.first);
            }

end:        return DIENUM_CONTINUE;
        }

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
                diprg.lMax = joy_axis_size;
                diprg.lMin = -joy_axis_size;
                
                HRESULT hr;

                if (FAILED(hr = reinterpret_cast<LPDIRECTINPUTDEVICE8>(ctx)->SetProperty(DIPROP_RANGE, &diprg.diph)))
                {
                    qDebug() << "DIPROP_RANGE" << hr;
                    return DIENUM_STOP;
                }
            }

            return DIENUM_CONTINUE;
        }
    };
    
    static std::unordered_map<QString, std::shared_ptr<joy>>& joys();
};

#endif
