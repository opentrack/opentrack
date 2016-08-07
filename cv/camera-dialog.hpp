/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "opentrack-compat/camera-names.hpp"
#include "opentrack-compat/sleep.hpp"

#ifdef __linux
#   include <QProcess>
#endif

#ifdef _WIN32
#   include <QTimer>
#   include <objbase.h>
#   include <winerror.h>
#   include <windows.h>
#endif

#include <opencv2/videoio.hpp>
#include <QMutex>

class camera_dialog
{
    static void maybe_grab_frame(cv::VideoCapture& cap);
#ifdef _WIN32
    static void init_com_threading();
#endif

public:
    virtual ~camera_dialog();
    void open_camera_settings(cv::VideoCapture*, const QString&, QMutex*);
#if defined(_WIN32)
    cv::VideoCapture fake_capture;
    QTimer t;
    void delete_capture();
#endif
};
