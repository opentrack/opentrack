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
    
    enum { joy_axis_size = 65535 };
    
    struct joy_info
    {
        QString name, guid;
    };

    void poll(fn f);
    bool poll_axis(const QString& guid, int axes[8]);
    ~win32_joy_ctx();
    std::vector<joy_info> get_joy_info();
    static win32_joy_ctx& make();
    win32_joy_ctx(const win32_joy_ctx&) = delete;
    win32_joy_ctx& operator=(const win32_joy_ctx&) = delete;
    
private:
    enum { joylist_refresh_ms = 250 };
    
    QMutex mtx;
    Timer timer_joylist;
    
    static QString guid_to_string(const GUID guid);
    static LPDIRECTINPUT8& dinput_handle();
    win32_joy_ctx();
    void release();
    void refresh(bool first);
    
    struct joy;
    static std::unordered_map<QString, std::shared_ptr<joy>>& joys();
    
    struct joy
    {
        enum { first_event_delay_ms = 3000 };
        
        LPDIRECTINPUTDEVICE8 joy_handle;
        QString guid, name;
        bool pressed[128];
        Timer first_timer;
        bool first;

        joy(LPDIRECTINPUTDEVICE8 handle, const QString& guid, const QString& name, bool first)
            : joy_handle(handle), guid(guid), name(name), first(first)
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

        void release();
        bool poll(fn f);
    };

    class enum_state
    {
        std::unordered_map<QString, std::shared_ptr<joy>>& joys;
        bool first;
        
        std::vector<QString> all;
        static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
        static BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* ctx);
    public:
        enum_state(std::unordered_map<QString, std::shared_ptr<joy>>& joys, bool first);
    };
};

#endif
