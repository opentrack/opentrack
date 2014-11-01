#include "ftnoir_tracker_libevdev.h"
#include "opentrack/plugin-api.hpp"

#include <algorithm>

#include <QDir>
#include <QDebug>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <string>

static const int ot_libevdev_joystick_axes[6] = { ABS_X, ABS_Y, ABS_Z, ABS_RX, ABS_RY, ABS_RZ };

FTNoIR_Tracker::FTNoIR_Tracker() : node(nullptr), success(false), should_quit(false)
{
}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
    if (success)
    {
        should_quit = true;
        wait();
    }
    if (node)
        libevdev_free(node);
    if (fd != -1)
        close(fd);
}

void FTNoIR_Tracker::start_tracker(QFrame*)
{
    QString node_name = s.device_name;
    std::string str = (QString("/dev/input/by-id/") + node_name).toStdString();
    const char* filename = str.c_str();

    fd = open(filename, 0, O_RDWR);
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

        if (a_min[i] == a_max[i])
            a_max[i]++;

        qDebug() << "axis limits" << i << a_min[i] << "->" << a_max[i];
    }

    success = true;

    QThread::start();
}

void FTNoIR_Tracker::run()
{
    while (!should_quit)
    {
        struct input_event ev;
        int status = libevdev_next_event(node, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (status != LIBEVDEV_READ_STATUS_SUCCESS)
            continue;
        if (ev.type == EV_ABS || ev.type == EV_MSC)
        {
            const int code = ev.code;
            for (int i = 0; i < 6; i++)
            {
                if (ot_libevdev_joystick_axes[i] == code)
                {
                    QMutexLocker l(&mtx);
                    values[i] = ev.value;
                    break;
                }
            }
        }
    }
}

void FTNoIR_Tracker::data(double *data)
{
    if (node)
    {
        QMutexLocker l(&mtx);
        for (int i = 0; i < 6; i++)
        {
            int val = values[i];
            double v = (val - a_min[i])*(i >= Yaw ? 180. : 100.) / a_max[i] - a_min[i];
            data[i] = v;
        }
    }
}

extern "C" OPENTRACK_EXPORT ITracker* GetConstructor()
{
    return new FTNoIR_Tracker;
}
