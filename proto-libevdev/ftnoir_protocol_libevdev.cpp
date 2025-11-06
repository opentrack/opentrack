// strerror_r(3)

#ifdef __clang__
#   pragma clang diagnostic ignored "-Wreserved-id-macro"
#   pragma clang diagnostic ignored "-Wunused-macros"
#endif

#ifndef _POSIX_C_SOURCE
#   define _POSIX_C_SOURCE 200112L
#endif

#include "ftnoir_protocol_libevdev.h"
#include "api/plugin-api.hpp"
#include "compat/math.hpp"

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <algorithm>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>

#include <QDebug>

#define CHECK_LIBEVDEV(expr)                                    \
    do {                                                        \
        if (int error = (expr); error != 0)                     \
        {                                                       \
            error_code = -error;                                \
            error_expr = #expr;                                 \
            goto fail;                                          \
        }                                                       \
    } while (false)

static constexpr int max_input = 1 << 16;
static constexpr int mid_input = (1 << 15) - 1;
static constexpr int min_input = 0;

evdev::evdev()
{
    dev = libevdev_new();

    if (!dev)
    {
        error_code = errno;
        error_expr = "libevdev_new();";
        goto fail;
    }

    CHECK_LIBEVDEV(libevdev_enable_property(dev, INPUT_PROP_BUTTONPAD));

    libevdev_set_name(dev, "opentrack headpose");
    libevdev_set_id_vendor(dev, 1964);
    libevdev_set_id_product(dev, 9526);
    libevdev_set_id_bustype(dev, 3);
    libevdev_set_id_version(dev, 123);

    struct input_absinfo absinfo;

    absinfo.minimum = min_input;
    absinfo.maximum = max_input;
    absinfo.resolution = 1;
    absinfo.value = mid_input;
    absinfo.flat = 1;
    absinfo.fuzz = 0;

    CHECK_LIBEVDEV(libevdev_enable_event_type(dev, EV_ABS));
    CHECK_LIBEVDEV(libevdev_enable_event_code(dev, EV_ABS, ABS_X, &absinfo));
    CHECK_LIBEVDEV(libevdev_enable_event_code(dev, EV_ABS, ABS_Y, &absinfo));
    CHECK_LIBEVDEV(libevdev_enable_event_code(dev, EV_ABS, ABS_Z, &absinfo));
    CHECK_LIBEVDEV(libevdev_enable_event_code(dev, EV_ABS, ABS_RX, &absinfo));
    CHECK_LIBEVDEV(libevdev_enable_event_code(dev, EV_ABS, ABS_RY, &absinfo));
    CHECK_LIBEVDEV(libevdev_enable_event_code(dev, EV_ABS, ABS_RZ, &absinfo));

    /* do not remove next 3 lines or udev scripts won't assign 0664 permissions -sh */
    CHECK_LIBEVDEV(libevdev_enable_event_type(dev, EV_KEY));
    CHECK_LIBEVDEV(libevdev_enable_event_code(dev, EV_KEY, BTN_JOYSTICK, NULL));
    CHECK_LIBEVDEV(libevdev_enable_event_code(dev, EV_KEY, BTN_TRIGGER, NULL));

    CHECK_LIBEVDEV(libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev));

    return;

fail:
    if (uidev)
        libevdev_uinput_destroy(uidev);
    if (dev)
        libevdev_free(dev);

    qDebug() << "libevdev error" << error_code;

    uidev = nullptr;
    dev = nullptr;
}

evdev::~evdev()
{
    if (uidev)
        libevdev_uinput_destroy(uidev);
    if (dev)
        libevdev_free(dev);
}

void evdev::pose(const double* headpose, const double*) {
    static const int axes[] = {
        /* translation goes first */
        ABS_X, ABS_Y, ABS_Z, ABS_RX, ABS_RY, ABS_RZ
    };

    static const int max_value[] = {
        100,
        100,
        100,
        180,
        90,
        180
    };

    for (int i = 0; i < 6; i++)
    {
        int value = (int)(headpose[i] * mid_input / max_value[i] + mid_input);
        int normalized = std::clamp(value, min_input, max_input);
        (void) libevdev_uinput_write_event(uidev, EV_ABS, axes[i], normalized);
    }

    (void) libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
}

module_status evdev::initialize()
{
    if (error_code)
    {
        const char* msg;
        char buf[128] {};

        if (!(msg = strerror_r(error_code, buf, sizeof(buf))))
        {
            snprintf(buf, sizeof(buf), "%d", error_code);
            msg = buf;
        }

        return error(QStringLiteral("libevdev call '%1' failed with error '%2'")
                     .arg(!error_expr ? "<NULL>" : error_expr, msg));
    }
    else
        return {};
}

OPENTRACK_DECLARE_PROTOCOL(evdev, LibevdevControls, evdevDll)
