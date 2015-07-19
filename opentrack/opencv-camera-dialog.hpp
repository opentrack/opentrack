#pragma once

#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <opencv2/videoio.hpp>
#include "opentrack/camera-names.hpp"

#ifdef __linux
#include <QProcess>
#endif

template<typename tracker>
class camera_dialog
{
#ifdef __linux
    void open_camera_settings(cv::VideoCapture *, const QString &camera_name, QMutex *)
    {
        int idx = camera_name_to_index(camera_name);
        QProcess::startDetached("qv4l2", QStringList() << "-d" << ("/dev/video" + QString::number(idx)));
    }
#else
    cv::VideoCapture fake_capture;
    QTimer t;

private:
    void delete_capture()
    {
        fake_capture.open("");
    }

public:
    void open_camera_settings(cv::VideoCapture* cap, const QString& camera_name, QMutex* camera_mtx)
    {
        if (cap)
        {
            QMutexLocker l(camera_mtx);

            if (cap->isOpened())
            {
                cap->set(cv::CAP_PROP_SETTINGS, 1);
                return;
            }
        }

        if (t.isActive())
            return;

        // don't hog the camera capture
        if (!t.isSingleShot())
            QObject::connect(&t, &QTimer::timeout, [&]() -> void { delete_capture(); });

        t.setSingleShot(true);
        t.setInterval(3000);

        fake_capture = cv::VideoCapture(camera_name_to_index(camera_name));
        fake_capture.set(cv::CAP_PROP_SETTINGS, 1);
        // HACK: we're not notified when it's safe to close the capture
        t.start();
    }
#endif
};

