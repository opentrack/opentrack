#pragma once

#include <QMutex>
#include <QMutexLocker>
#include <opencv2/videoio.hpp>
#include "opentrack/camera-names.hpp"

template<typename tracker>
class camera_dialog
{
    cv::VideoCapture fake_capture;
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

        fake_capture = cv::VideoCapture(camera_name_to_index(camera_name));
        fake_capture.set(cv::CAP_PROP_SETTINGS, 1);
        // don't hog the camera capture
        fake_capture.open("");
    }
};

