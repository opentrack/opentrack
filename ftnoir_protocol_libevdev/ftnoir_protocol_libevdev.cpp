#include "ftnoir_protocol_libevdev.h"
#include "facetracknoir/global-settings.h"
//#include "ftnoir_tracker_base/ftnoir_tracker_types.h"

#define CHECK_LIBEVDEV(expr) if ((expr) != 0) goto error;

FTNoIR_Protocol::FTNoIR_Protocol() : dev(NULL), uidev(NULL)
{
    int error;

    dev = libevdev_new();

    if (!dev)
        goto error;

    libevdev_set_name(dev, "opentrack headpose");

    struct input_absinfo absinfo;

    absinfo.minimum = -ABS_MAX;
    absinfo.maximum = ABS_MAX;
    absinfo.resolution= 1; /* units per radian? let's go shopping */
    absinfo.value = 0;
    absinfo.fuzz = 1; /* no filtering in evdev subsystem, we do our own */

    CHECK_LIBEVDEV(libevdev_enable_event_type(dev, EV_ABS))
    CHECK_LIBEVDEV(libevdev_enable_event_code(dev, EV_ABS, ABS_X, &absinfo))
    CHECK_LIBEVDEV(libevdev_enable_event_code(dev, EV_ABS, ABS_Y, &absinfo))
    CHECK_LIBEVDEV(libevdev_enable_event_code(dev, EV_ABS, ABS_Z, &absinfo))
    CHECK_LIBEVDEV(libevdev_enable_event_code(dev, EV_ABS, ABS_RX, &absinfo))
    CHECK_LIBEVDEV(libevdev_enable_event_code(dev, EV_ABS, ABS_RY, &absinfo))
    CHECK_LIBEVDEV(libevdev_enable_event_code(dev, EV_ABS, ABS_RZ, &absinfo))

    CHECK_LIBEVDEV(error = libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev))

    return;
error:
    if (uidev)
        libevdev_uinput_destroy(uidev);
    if (dev)
        libevdev_free(dev);
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

void FTNoIR_Protocol::sendHeadposeToGame( double *headpose, double *rawheadpose ) {
    static const int axes[] = {
        /* translation goes first */
        ABS_X, ABS_Y, ABS_Z, ABS_RX, ABS_RY, ABS_RZ
    };
    static const int ranges[] = {
        2,
        2,
        2,
        2, /* | pitch | only goes to 90 */
        1,
        2
    };

    const int max_euler = 90;

    for (int i = 0; i < 6; i++)
        (void) libevdev_uinput_write_event(uidev, EV_ABS, axes[i], ranges[i] * headpose[i] * max_euler / ABS_MAX);
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocol* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Protocol;
}
