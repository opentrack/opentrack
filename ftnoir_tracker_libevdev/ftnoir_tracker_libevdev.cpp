#include "ftnoir_tracker_libevdev.h"
#include "facetracknoir/plugin-support.h"

#include <algorithm>

#include <QDir>
#include <QDebug>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <string>

static const int ot_libevdev_joystick_axes[6] = { ABS_X, ABS_Y, ABS_Z, ABS_RX, ABS_RY, ABS_RZ };

FTNoIR_Tracker::FTNoIR_Tracker() : node(nullptr), success(false)
{
}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
    if (node)
        libevdev_free(node);
    if (fd != -1)
        close(fd);
}

void FTNoIR_Tracker::StartTracker(QFrame*)
{
    QString pretty_name = s.device_name;
    QString node_name = "usb-" + pretty_name.replace(' ', '_') + "-event-joystick";
    std::string str = (QString("/dev/input/by-id/") + node_name).toStdString();
    const char* filename = str.c_str();
    
    fd = open(filename, O_NONBLOCK, O_RDWR);
    if (fd == -1)
    {
        qDebug() << "error opening" << filename;
        return;
    }
    
    int ret = libevdev_new_from_fd(fd, &node);
    if (ret)
    {
        qDebug() << "libevdev open error" << ret;
        return;
    }
    
    for (int i = 0; i < 6; i++)
    {
        // no error checking here, errors result in SIGFPE
        a_min[i] = libevdev_get_abs_minimum(node, ot_libevdev_joystick_axes[i]);
        a_max[i] = libevdev_get_abs_maximum(node, ot_libevdev_joystick_axes[i]);
        qDebug() << "axis limits" << i << a_min[i] << "->" << a_max[i];
    }
    
    success = true;
}

void FTNoIR_Tracker::GetHeadPoseData(double *data)
{
    if (node)
    {
        int max = 64;
        while (libevdev_has_event_pending(node) == 1 && max-- > 0)
        {
            struct input_event ev;
            int status = libevdev_next_event(node, LIBEVDEV_READ_FLAG_NORMAL, &ev);
            if (status != LIBEVDEV_READ_STATUS_SUCCESS)
                continue;
            if (ev.type == EV_ABS)
            {
                const int val = ev.value, code = ev.code;
                for (int i = 0; i < 6; i++)
                {
                    if (ot_libevdev_joystick_axes[i] == code)
                    {
                        data[i] = (val - a_min[i])*(i >= Yaw ? 180. : 100.) / a_max[i] - a_min[i];
                        break;
                    }
                }
            }
        }
    }
}

extern "C" OPENTRACK_EXPORT ITracker* GetConstructor()
{
    return new FTNoIR_Tracker;
}
