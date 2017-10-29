/* Copyright (c) 2015-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */
#pragma once

#ifdef _WIN32

#include "dinput.hpp"
#include "compat/timer.hpp"
#include "export.hpp"
#include <cstring>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>
#include <unordered_map>
#include <QString>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QWidget>

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

struct OTR_DINPUT_EXPORT win32_joy_ctx
{
    using fn = std::function<void(const QString& guid, int btn, bool held)>;

    struct joy
    {
        LPDIRECTINPUTDEVICE8 joy_handle;
        QString guid, name;
        enum { num_pressed_keys = 128 + 4 * 4 };
        //bool pressed[num_pressed_keys] {};
        Timer first_timer;

        static DIDEVICEOBJECTDATA keystate_buffers[256];

        joy(LPDIRECTINPUTDEVICE8 handle, const QString& guid, const QString& name);
        ~joy();

        void release();
        bool poll(fn f);
    };

    using joys_t = std::unordered_map<QString, std::shared_ptr<joy>>;

    static constexpr int joy_axis_size = 65536;

    struct joy_info
    {
        QString name, guid;
    };

    void poll(fn f);
    bool poll_axis(const QString& guid, int* axes);
    std::vector<joy_info> get_joy_info();

    win32_joy_ctx(const win32_joy_ctx&) = delete;
    win32_joy_ctx& operator=(const win32_joy_ctx&) = delete;

    win32_joy_ctx();
    void refresh();

    using di_t = dinput_handle::di_t;

private:
    static QString guid_to_string(const GUID& guid);

    class OTR_DINPUT_EXPORT enum_state final
    {
        std::vector<QString> all;
        joys_t joys;
        dinput_handle::di_t di;

        static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
        static BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* ctx);

    public:
        static QMutex mtx;

        enum_state();
        ~enum_state();
        void refresh();
        const joys_t& get_joys() const;
    };

    static enum_state enumerator;
};

#endif
