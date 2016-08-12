/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "cv/camera-dialog.hpp"
#include <QDebug>
#include <QMutexLocker>

void camera_dialog::maybe_grab_frame(cv::VideoCapture& cap)
{
    for (int i = 0; i < 60; i++)
    {
        if (cap.grab())
            break;
        portable::sleep(50);
    }
}

camera_dialog::~camera_dialog() {}

void camera_dialog::open_camera_settings(cv::VideoCapture* cap, const QString& camera_name, QMutex* camera_mtx)
{
#ifdef _WIN32
    init_com_threading(com_apartment);

    if (cap)
    {
        QMutexLocker l(camera_mtx);

        if (cap->isOpened())
        {
            maybe_grab_frame(*cap);
            cap->set(cv::CAP_PROP_SETTINGS, 1);
            return;
        }
    }

    if (t.isActive())
        t.stop();

    // don't hog the camera capture
    if (!t.isSingleShot())
        QObject::connect(&t, &QTimer::timeout, [&]() -> void { delete_capture(); });

    fake_capture = cv::VideoCapture(camera_name_to_index(camera_name));
    maybe_grab_frame(fake_capture);
    fake_capture.set(cv::CAP_PROP_SETTINGS, 1);

    t.setSingleShot(true);
    t.setInterval(5000);

    // HACK: we're not notified when it's safe to close the capture
    t.start();
#elif defined(__linux)
    int idx = camera_name_to_index(camera_name);
    QProcess::startDetached("qv4l2", QStringList() << "-d" << ("/dev/video" + QString::number(idx)));
#else
    // nothing
#endif
}

#ifdef _WIN32
void camera_dialog::delete_capture()
{
    fake_capture.open("");
}
#endif
