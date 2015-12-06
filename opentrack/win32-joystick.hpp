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
#include <QMainWindow>

namespace std {
template<>
struct hash<QString>
{
    inline std::size_t operator()(const QString& value) const
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
    std::vector<joy_info> get_joy_info();
    
    win32_joy_ctx(const win32_joy_ctx&) = delete;
    win32_joy_ctx& operator=(const win32_joy_ctx&) = delete;
    
    win32_joy_ctx();
    ~win32_joy_ctx();
    
private:
    enum { joylist_refresh_ms = 100 };
    
    QMutex mtx;
    Timer timer_joylist;
    QMainWindow fake_main_window;
    LPDIRECTINPUT8 di;
    
    static QString guid_to_string(const GUID guid);
    void release();
    void refresh(bool first);
    
    struct joy
    {
        LPDIRECTINPUTDEVICE8 joy_handle;
        QString guid, name;
        bool pressed[128];
        Timer first_timer;
        DIJOYSTATE2 js_old;

        joy(LPDIRECTINPUTDEVICE8 handle, const QString& guid, const QString& name);
        ~joy();

        void release();
        bool poll(fn f);
    };
    
    std::unordered_map<QString, std::shared_ptr<joy>> joys;

    class enum_state
    {
        std::unordered_map<QString, std::shared_ptr<joy>> joys;
        QMainWindow& fake_main_window;
        LPDIRECTINPUT8 di;
        
        std::vector<QString> all;
        static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
        static BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* ctx);
    public:
        enum_state(std::unordered_map<QString, std::shared_ptr<joy>>& joys, QMutex &mtx, QMainWindow& fake_main_window, LPDIRECTINPUT8 di);
    };
};

#endif
