#include "ftnoir_protocol_libevdev.h"
#include "facetracknoir/global-settings.h"
//#include "ftnoir_tracker_base/ftnoir_tracker_types.h"
#include <cstdio>
#include <algorithm>

#include <sys/types.h>
#include <sys/stat.h>

#define CHECK_LIBEVDEV(expr) if ((error = (expr)) != 0) goto error;

static const int max_input = 65535;
static const int mid_input = 32767;
static const int min_input = 0;

FTNoIR_Protocol::FTNoIR_Protocol() : dev(NULL), uidev(NULL)
{
    int error = 0;

    dev = libevdev_new();

    if (!dev)
        goto error;

    CHECK_LIBEVDEV(libevdev_enable_property(dev, INPUT_PROP_BUTTONPAD));

    libevdev_set_name(dev, "opentrack headpose");

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
error:
    if (uidev)
        libevdev_uinput_destroy(uidev);
    if (dev)
        libevdev_free(dev);
    if (error)
        fprintf(stderr, "libevdev error: %d\n", error);
    uidev = NULL;
    dev = NULL;
}

FTNoIR_Protocol::~FTNoIR_Protocol()
{
    if (uidev)
        libevdev_uinput_destroy(uidev);
    if (dev)
        libevdev_free(dev);
}

void FTNoIR_Protocol::sendHeadposeToGame(const double* headpose) {
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
        int value = headpose[i] * mid_input / max_value[i] + mid_input;
        int normalized = std::max(std::min(max_input, value), min_input);
        (void) libevdev_uinput_write_event(uidev, EV_ABS, axes[i], normalized);
    }

    (void) libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocol* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Protocol;
}
