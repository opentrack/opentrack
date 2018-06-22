/* Copyright (c) 2016 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "video-property-page.hpp"

#ifdef _WIN32

#include "compat/camera-names.hpp"
#include "compat/sleep.hpp"
#include "compat/run-in-thread.hpp"
#include "compat/library-path.hpp"

#include <cstring>

#include <opencv2/videoio.hpp>

#include <QApplication>
#include <QProcess>
#include <QThread>
#include <QMessageBox>

#include <QDebug>

#include <dshow.h>

#define CHECK(expr) if (FAILED(hr = (expr))) { qDebug() << QLatin1String(#expr) << hr; goto done; }
#define CHECK2(expr) if (!(expr)) { qDebug() << QLatin1String(#expr); goto done; }

bool video_property_page::show_from_capture(cv::VideoCapture& cap, int /*index */)
{
    return cap.set(cv::CAP_PROP_SETTINGS, 0);
}

struct prop_settings_worker final : QThread
{
    prop_settings_worker(int idx);
    ~prop_settings_worker() override;
    void _open_prop_page();
    void run() override;

    cv::VideoCapture cap;
    int _idx = -1;
};

prop_settings_worker::prop_settings_worker(int idx)
{
    int ret = cap.get(cv::CAP_PROP_SETTINGS);

    if (ret != 0)
        run_in_thread_async(qApp, [] {
            QMessageBox::warning(nullptr,
                                 "Camera properties",
                                 "Camera dialog already opened",
                                 QMessageBox::Cancel,
                                 QMessageBox::NoButton);
        });
    else
    {
        _idx = idx;
        // DON'T MOVE IT
        // ps3 eye will reset to default settings if done from another thread
        _open_prop_page();
    }
}

void prop_settings_worker::_open_prop_page()
{
    cap.open(_idx);

    if (cap.isOpened())
    {
        cv::Mat tmp;

        for (unsigned k = 0; k < 2000/50; k++)
        {
            if (cap.read(tmp))
            {
                qDebug() << "got frame" << tmp.rows << tmp.cols;
                goto ok;
            }
            portable::sleep(50);
        }
    }

    qDebug() << "property-page: can't open camera";
    _idx = -1;

    return;

ok:
    portable::sleep(100);

    qDebug() << "property-page: opening for" << _idx;

    if (!cap.set(cv::CAP_PROP_SETTINGS, 0))
    {
        run_in_thread_async(qApp, [] {
            QMessageBox::warning(nullptr,
                                 "Camera properties",
                                 "Can't open camera dialog",
                                 QMessageBox::Cancel,
                                 QMessageBox::NoButton);
        });
    }
}

prop_settings_worker::~prop_settings_worker()
{
    if (_idx != -1)
    {
        // ax filter is race condition-prone
        portable::sleep(250);
        cap.release();
        // idem
        portable::sleep(250);

        qDebug() << "property-page: closed" << _idx;
    }
}

void prop_settings_worker::run()
{
    if (_idx != -1)
    {
        while (cap.get(cv::CAP_PROP_SETTINGS) > 0)
            portable::sleep(1000);
    }
}

bool video_property_page::show(int idx)
{
    auto thread = new prop_settings_worker(idx);

    // XXX is this a race condition?
    thread->moveToThread(qApp->thread());
    QObject::connect(thread, &QThread::finished, qApp, [thread] { thread->deleteLater(); }, Qt::DirectConnection);

    thread->start();

    return true;
}

#elif defined(__linux)
#   include <QProcess>
#   include "compat/camera-names.hpp"

bool video_property_page::show(int idx)
{
    const QList<QString> camera_names(get_camera_names());

    if (idx >= 0 && idx < camera_names.size())
        return QProcess::startDetached("qv4l2", QStringList { "-d", QString("/dev/video%1").arg(idx) });
    else
        return false;
}

bool video_property_page::show_from_capture(cv::VideoCapture&, int idx)
{
    return show(idx);
}
#else
bool video_property_page::show(int) { return false; }
bool video_property_page::show_from_capture(cv::VideoCapture&, int) { return false; }
#endif

