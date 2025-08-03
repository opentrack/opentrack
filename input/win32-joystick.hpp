/* Copyright (c) 2015-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */
#pragma once

#include "dinput.hpp"
#include "compat/timer.hpp"
#include "export.hpp"
#include "compat/qhash.hpp"

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>
#include <iterator>

#include <QString>
#include <QMutex>

namespace win32_joy_impl {

static constexpr unsigned max_buttons = 128;
static constexpr unsigned max_pov_hats = 4;
static constexpr unsigned pov_hat_directions = 8;

// cf. https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee416628(v=vs.85)
// see also remarks on the page
// no need to check for pos == unsigned(-1) || pos == 0xffff,
// this logic doesn't require that
static constexpr unsigned value_per_pov_hat_direction = 36000 / pov_hat_directions;
static constexpr unsigned max_buttons_and_pov_hats = max_buttons + max_pov_hats * pov_hat_directions;

//static_assert(pov_hat_directions == 4 || pov_hat_directions == 8);

// XXX how many axis update events can we reasonably get in a short time frame?
static constexpr unsigned num_buffers = 128;

//#define WIN32_JOY_DEBUG

struct OTR_INPUT_EXPORT win32_joy_ctx final
{
    using fn = std::function<void(const QString& guid, int btn, bool held)>;

    struct joy final
    {
        IDirectInputDevice8A* joy_handle;
        QString guid, name;
        bool last_state[max_buttons_and_pov_hats] {};

        joy(IDirectInputDevice8A* handle, const QString& guid, const QString& name);
        ~joy();

        void release();
        bool poll(fn const& f);
    };

    using joys_t = std::unordered_map<QString, std::shared_ptr<joy>>;

    static constexpr int joy_axis_size = 65536;

    struct joy_info
    {
        QString name, guid;
    };

    void poll(fn const& f);
    bool poll_axis(const QString& guid, int* axes);
    std::vector<joy_info> get_joy_info();

    win32_joy_ctx(const win32_joy_ctx&) = delete;
    win32_joy_ctx& operator=(const win32_joy_ctx&) = delete;

    win32_joy_ctx();
    void refresh();

private:
    static QString guid_to_string(const GUID& guid);

    class OTR_INPUT_EXPORT enum_state final
    {
        std::vector<QString> all;
        joys_t joys;
        di_t di;

        static BOOL __stdcall EnumJoysticksCallback(const DIDEVICEINSTANCEA* pdidInstance, void* pContext);
        static BOOL __stdcall EnumObjectsCallback(const DIDEVICEOBJECTINSTANCEA* pdidoi, void* ctx);

    public:
        static QMutex mtx;

        enum_state();
        ~enum_state();
        void refresh();
        const joys_t& get_joys() const;

        enum_state(enum_state const&) = delete;
    };

    static enum_state enumerator;
};

} // ns win32_joy_impl

using win32_joy_ctx = win32_joy_impl::win32_joy_ctx;
