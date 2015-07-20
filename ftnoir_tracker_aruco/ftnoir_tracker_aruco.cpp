/* Copyright (c) 2013-2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include <vector>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <QMutexLocker>
#include "./include/markerdetector.h"
#include "ftnoir_tracker_aruco.h"
#include "opentrack/plugin-api.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "opentrack/camera-names.hpp"
#include "opentrack/opencv-calibration.hpp"

typedef struct {
    int width;
    int height;
} resolution_tuple;

static resolution_tuple resolution_choices[] = {
    { 640, 480 },
    { 320, 240 },
    { 320, 200 },
    { 0, 0 }
};

Tracker::Tracker() : stop(false), layout(nullptr), videoWidget(nullptr)
{
}

Tracker::~Tracker()
{
    stop = true;
    wait();
    if (videoWidget)
        delete videoWidget;
    if(layout)
        delete layout;
    camera.release();
}

void Tracker::start_tracker(QFrame* videoframe)
{
    videoframe->show();
    videoWidget = new ArucoVideoWidget(videoframe);
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(videoWidget);
    if (videoframe->layout())
        delete videoframe->layout();
    videoframe->setLayout(layout);
    videoWidget->show();
    start();
    for (int i = 0; i < 6; i++)
        pose[i] = 0;
    this->layout = layout;
}

#define HT_PI 3.1415926535

void Tracker::getRT(cv::Matx33d& r_, cv::Vec3d& t_)
{
    QMutexLocker l(&mtx);

    r_ = r;
    t_ = t;
}

void Tracker::run()
{
    int rint = s.resolution;
    if (rint < 0 || rint >= (int)(sizeof(resolution_choices) / sizeof(resolution_tuple)))
        rint = 0;
    resolution_tuple res = resolution_choices[rint];
    int fps;
    switch (static_cast<int>(s.force_fps))
    {
    default:
    case 0:
        fps = 0;
        break;
    case 1:
        fps = 30;
        break;
    case 2:
        fps = 60;
        break;
    case 3:
        fps = 125;
        break;
    case 4:
        fps = 200;
        break;
    }

    {
        QMutexLocker l(&camera_mtx);

        camera = cv::VideoCapture(camera_name_to_index(s.camera_name));
        if (res.width)
        {
            camera.set(cv::CAP_PROP_FRAME_WIDTH, res.width);
            camera.set(cv::CAP_PROP_FRAME_HEIGHT, res.height);
        }
        if (fps)
            camera.set(cv::CAP_PROP_FPS, fps);

        if (!camera.isOpened())
        {
            qDebug() << "aruco tracker: can't open camera";
            return;
        }
    }

    aruco::MarkerDetector detector;
    detector.setDesiredSpeed(3);

    cv::Rect last_roi(65535, 65535, 0, 0);

    auto freq = cv::getTickFrequency();
    auto last_time = cv::getTickCount();
    double cur_fps = 0;
    std::vector<int> box_sizes { 5, 7, 9, 11 };
    int box_idx = 0;
    double failed = 0;
    const double max_failed = 1.25;
    cv::Vec3d rvec, tvec;
    double last_fov = -1;
    cv::Mat intrinsics = cv::Mat::eye(3, 3, CV_32FC1);
    cv::Mat dist_coeffs = cv::Mat::zeros(5, 1, CV_32FC1);

    while (!stop)
    {
        cv::Mat color;
        {
            QMutexLocker l(&camera_mtx);

            if (!camera.read(color))
                continue;
        }
        cv::Mat grayscale;
        cv::cvtColor(color, grayscale, cv::COLOR_RGB2GRAY);
        
        const int scale = grayscale.cols > 480 ? 2 : 1;
        detector.setThresholdParams(box_sizes[box_idx], 5);

        static constexpr double pi = 3.1415926f;
        const int w = grayscale.cols, h = grayscale.rows;
        const double diag = sqrt(w * w + h * h)/w, diag_fov = static_cast<int>(s.fov) * pi / 180.;
        const double fov = 2.*atan(tan(diag_fov/2.)/sqrt(1. + diag*diag));
        const float focal_length_w = .5 * w / tan(.5 * fov);
        const float focal_length_h = focal_length_w;

        if (last_fov != s.fov)
        {
            last_fov = s.fov;
            if (!get_camera_calibration(static_cast<QString>(s.camera_name), intrinsics, dist_coeffs, grayscale.cols, grayscale.rows))
            {
                intrinsics.at<float> (0, 0) = focal_length_w;
                intrinsics.at<float> (1, 1) = focal_length_h;
                intrinsics.at<float> (0, 2) = grayscale.cols/2;
                intrinsics.at<float> (1, 2) = grayscale.rows/2;
            }
            else
            {
                qDebug() << "got calibration";
            }
        }

        std::vector< aruco::Marker > markers;

        const double size_min = 0.02;
        const double size_max = 0.4;

        bool roi_valid = false;
        
        auto time = cv::getTickCount();
        
        const double dt = (time - last_time) / freq;
        last_time = time;
        cur_fps = cur_fps * 0.97 + 0.03 * (dt == 0 ? 0 : 1./dt);
        
        if (last_roi.width > 0 && last_roi.height)
        {
            detector.setThresholdParams(box_sizes[box_idx], 5);
            detector.setMinMaxSize(std::max(0.01, size_min * grayscale.cols / last_roi.width),
                                   std::min(1.0, size_max * grayscale.cols / last_roi.width));

            if (detector.detect(grayscale(last_roi), markers, cv::Mat(), cv::Mat(), -1, false),
                markers.size() == 1 && markers[0].size() == 4)
            {
                failed = std::max(0., failed - dt);
                auto& m = markers.at(0);
                for (int i = 0; i < 4; i++)
                {
                    auto& p = m.at(i);
                    p.x += last_roi.x;
                    p.y += last_roi.y;
                }
                roi_valid = true;
            }
        }

        if (!roi_valid)
        {
            failed += dt;
            if (failed > max_failed)
            {
                box_idx++;
                box_idx %= box_sizes.size();
                qDebug() << "aruco: box size now" << box_sizes[box_idx];
                failed = 0;
            }
            detector.setThresholdParams(box_sizes[box_idx], 5);
            detector.setMinMaxSize(size_min, size_max);
            detector.detect(grayscale, markers, cv::Mat(), cv::Mat(), -1, false);
        }

        if (markers.size() == 1 && markers[0].size() == 4) {
            const auto& m = markers.at(0);
            for (int i = 0; i < 4; i++)
                cv::line(color, m[i], m[(i+1)%4], cv::Scalar(0, 0, 255), scale, 8);
        }

        char buf[128];

        frame = color.clone();

        ::sprintf(buf, "Hz: %d", (int)cur_fps);
        cv::putText(frame, buf, cv::Point(10, 32), cv::FONT_HERSHEY_PLAIN, scale, cv::Scalar(0, 255, 0), scale*2);

        if (markers.size() == 1 && markers[0].size() == 4) {
            const auto& m = markers.at(0);
            const float size = 40;

            cv::Mat obj_points(4,3,CV_32FC1);
            const int x1=1, x2=2, x3=3, x4=0;
            obj_points.at<float>(x1,0)=-size + s.headpos_x;
            obj_points.at<float>(x1,1)=-size + s.headpos_y;
            obj_points.at<float>(x1,2)= 0 + s.headpos_z;

            obj_points.at<float>(x2,0)=size + s.headpos_x;
            obj_points.at<float>(x2,1)=-size + s.headpos_y;
            obj_points.at<float>(x2,2)= 0 + s.headpos_z;

            obj_points.at<float>(x3,0)=size + s.headpos_x;
            obj_points.at<float>(x3,1)=size + s.headpos_y;
            obj_points.at<float>(x3,2)= 0 + s.headpos_z;

            obj_points.at<float>(x4,0)= -size + s.headpos_x;
            obj_points.at<float>(x4,1)= size + s.headpos_y;
            obj_points.at<float>(x4,2)= 0 + s.headpos_z;
            
            std::vector<cv::Point2f> img_points = m;
            if (!cv::solvePnP(obj_points, img_points, intrinsics, dist_coeffs, rvec, tvec, false, cv::SOLVEPNP_ITERATIVE))
                goto fail;

            {
                std::vector<cv::Point2f> repr2;
                std::vector<cv::Point3f> centroid;
                centroid.push_back(cv::Point3f(0, 0, 0));
                cv::projectPoints(centroid, rvec, tvec, intrinsics, dist_coeffs, repr2);

                {
                    auto s = cv::Scalar(255, 0, 255);
                    cv::circle(frame, repr2.at(0), 4, s, -1);
                }
            }

            for (int i = 0; i < 4; i++)
            {
                obj_points.at<float>(i, 0) -= s.headpos_x;
                obj_points.at<float>(i, 1) -= s.headpos_y;
                obj_points.at<float>(i, 2) -= s.headpos_z;
            }

            cv::Mat rvec_, tvec_;
            cv::solvePnP(obj_points, m, intrinsics, dist_coeffs, rvec_, tvec_, false, cv::SOLVEPNP_ITERATIVE);

            cv::Mat roi_points = obj_points * c_search_window;
            std::vector<cv::Point2f> roi_projection(4);
            cv::projectPoints(roi_points, rvec_, tvec_, intrinsics, dist_coeffs, roi_projection);

            last_roi = cv::Rect(color.cols-1, color.rows-1, 0, 0);

            for (int i = 0; i < 4; i++)
            {
                auto proj = roi_projection[i];
                int min_x = std::min<int>(proj.x, last_roi.x),
                    min_y = std::min<int>(proj.y, last_roi.y);

                int max_x = std::max<int>(proj.x, last_roi.width),
                    max_y = std::max<int>(proj.y, last_roi.height);

                last_roi.x = min_x;
                last_roi.y = min_y;

                last_roi.width = max_x;
                last_roi.height = max_y;
            }

            if (last_roi.x < 0)
                last_roi.x = 0;
            if (last_roi.y < 0)
                last_roi.y = 0;
            if (last_roi.width < 0)
                last_roi.width = 0;
            if (last_roi.height < 0)
                last_roi.height = 0;
            if (last_roi.x >= color.cols-1)
                last_roi.x = color.cols-1;
            if (last_roi.width >= color.cols-1)
                last_roi.width = color.cols-1;
            if (last_roi.y >= color.rows-1)
                last_roi.y= color.rows-1;
            if (last_roi.height >= color.rows-1)
                last_roi.height = color.rows-1;

            last_roi.width -= last_roi.x;
            last_roi.height -= last_roi.y;

            auto rmat = cv::Matx33d::zeros();
            cv::Matx33d m_r(3, 3, CV_64FC1), m_q(3, 3, CV_64FC1);
            cv::Rodrigues(rvec, rmat);

            {
                cv::Vec3d euler = cv::RQDecomp3x3(rmat, m_r, m_q);

                QMutexLocker lck(&mtx);

                for (int i = 0; i < 3; i++)
                    pose[i] = tvec(i);
                pose[Yaw] = euler[1];
                pose[Pitch] = -euler[0];
                pose[Roll] = euler[2];

                r = rmat;
                t = cv::Vec3d(tvec[0], -tvec[1], tvec[2]);
            }

            if (roi_valid)
                cv::rectangle(frame, last_roi, cv::Scalar(255, 0, 255), 1);
        }
        else
fail:
            last_roi = cv::Rect(65535, 65535, 0, 0);

        if (frame.rows > 0)
            videoWidget->update_image(frame);
    }
}

void Tracker::data(double *data)
{
    QMutexLocker lck(&mtx);

    data[Yaw] = pose[Yaw];
    data[Pitch] = pose[Pitch];
    data[Roll] = pose[Roll];
    data[TX] = pose[TX] * .1;
    data[TY] = pose[TY] * .1;
    data[TZ] = pose[TZ] * .1;
}

TrackerControls::TrackerControls()
{
    tracker = nullptr;
    calib_timer.setInterval(200);
    ui.setupUi(this);
    setAttribute(Qt::WA_NativeWindow, true);
    ui.cameraName->addItems(get_camera_names());
    tie_setting(s.camera_name, ui.cameraName);
    tie_setting(s.resolution, ui.resolution);
    tie_setting(s.force_fps, ui.cameraFPS);
    tie_setting(s.fov, ui.cameraFOV);
    tie_setting(s.headpos_x, ui.cx);
    tie_setting(s.headpos_y, ui.cy);
    tie_setting(s.headpos_z, ui.cz);
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui.btn_calibrate, SIGNAL(clicked()), this, SLOT(toggleCalibrate()));
    connect(this, SIGNAL(destroyed()), this, SLOT(cleanupCalib()));
    connect(&calib_timer, SIGNAL(timeout()), this, SLOT(update_tracker_calibration()));
    connect(ui.camera_settings, SIGNAL(pressed()), this, SLOT(camera_settings()));
}

void TrackerControls::toggleCalibrate()
{
    if (!calib_timer.isActive())
    {
        s.headpos_x = 0;
        s.headpos_y = 0;
        s.headpos_z = 0;
        calibrator.reset();
        calib_timer.start();
    } else {
        cleanupCalib();
        
        auto pos = calibrator.get_estimate();
        s.headpos_x = pos(0);
        s.headpos_y = pos(1);
        s.headpos_z = pos(2);
    }
}

void TrackerControls::cleanupCalib()
{
    if (calib_timer.isActive())
        calib_timer.stop();
}

void TrackerControls::update_tracker_calibration()
{
    if (calib_timer.isActive() && tracker)
    {
        cv::Matx33d r;
        cv::Vec3d t;
        tracker->getRT(r, t);
        calibrator.update(r, t);
    }
}

void TrackerControls::doOK()
{
    s.b->save();
    this->close();
}

void TrackerControls::doCancel()
{
    s.b->reload();
    this->close();
}

void TrackerControls::camera_settings()
{
    open_camera_settings(tracker ? &tracker->camera : nullptr, s.camera_name, tracker ? &tracker->camera_mtx : nullptr);
}

OPENTRACK_DECLARE_TRACKER(Tracker, TrackerControls, TrackerDll)
