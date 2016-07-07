#pragma once

#ifdef _WIN32

#include "dinput.hpp"
#include "opentrack-compat/timer.hpp"
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

struct OPENTRACK_LOGIC_EXPORT win32_joy_ctx
{
    using fn = std::function<void(const QString& guid, int btn, bool held)>;

    struct joy
    {
        LPDIRECTINPUTDEVICE8 joy_handle;
        QString guid, name;
        bool pressed[128 + 4 * 4];
        Timer first_timer;

        joy(LPDIRECTINPUTDEVICE8 handle, const QString& guid, const QString& name);
        ~joy();

        void release();
        bool poll(fn f);
    };

    using joys_t = std::unordered_map<QString, std::shared_ptr<joy>>;

    static constexpr int joy_axis_size = 65535;

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
    static QString guid_to_string(const GUID guid);

    class enum_state final
    {
        std::vector<QString> all;
        joys_t joys;

        static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
        static BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* ctx);

    public:
        static QMutex mtx;

        enum_state();
        ~enum_state();
        void refresh();
        const joys_t& get_joys() const { return joys; }
    };

    static enum_state enumerator;
};

#endif
