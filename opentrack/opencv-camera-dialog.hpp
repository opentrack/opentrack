/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#if !defined(QT_MOC_RUN)

#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <opencv2/videoio.hpp>
#include "opentrack-compat/camera-names.hpp"

#ifdef __linux
#   include <QProcess>
#else
#   include "opentrack-compat/sleep.hpp"
#endif

#ifdef _WIN32
#   include <objbase.h>
#   include <winerror.h>
#   include <windows.h>
#endif

static void init_com_threading(void)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        qDebug() << "failed CoInitializeEx" << hr << "code" << GetLastError();
}

static void maybe_grab_frame(cv::VideoCapture& cap)
{
    for (int i = 0; i < 60; i++)
    {
        if (cap.grab())
            break;
        portable::sleep(50);
    }
}

class camera_dialog
{
public:
    inline virtual ~camera_dialog() {}
#ifdef __linux
    void open_camera_settings(cv::VideoCapture *, const QString &camera_name, QMutex *)
    {
        int idx = camera_name_to_index(camera_name);
        QProcess::startDetached("qv4l2", QStringList() << "-d" << ("/dev/video" + QString::number(idx)));
    }
#else
    void open_camera_settings(cv::VideoCapture* cap, const QString& camera_name, QMutex* camera_mtx)
    {
        if (cap)
        {
            QMutexLocker l(camera_mtx);

            if (cap->isOpened())
            {
                init_com_threading();
                maybe_grab_frame(*cap);
                cap->set(cv::CAP_PROP_SETTINGS, 1);
                return;
            }
        }

        if (t.isActive())
            return;

        // don't hog the camera capture
        if (!t.isSingleShot())
            QObject::connect(&t, &QTimer::timeout, [&]() -> void { delete_capture(); });

        init_com_threading();
        fake_capture = cv::VideoCapture(camera_name_to_index(camera_name));
        maybe_grab_frame(fake_capture);
        fake_capture.set(cv::CAP_PROP_SETTINGS, 1);

        t.setSingleShot(true);
        t.setInterval(5000);

        // HACK: we're not notified when it's safe to close the capture
        t.start();
    }
private:
    cv::VideoCapture fake_capture;
    QTimer t;
    void delete_capture()
    {
        fake_capture.open("");
    }
#endif
};

#endif

