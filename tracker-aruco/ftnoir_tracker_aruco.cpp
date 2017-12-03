/* Copyright (c) 2013-2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "cv/video-widget.hpp"
#include "ftnoir_tracker_aruco.h"
#include "cv/video-property-page.hpp"
#include "compat/camera-names.hpp"
#include "compat/sleep.hpp"
#include "compat/math-imports.hpp"

#ifdef _MSC_VER
#   pragma warning(disable : 4702)
#endif

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

#ifdef DEBUG_UNSHARP_MASKING
#   include <opencv2/highgui.hpp>
#endif

#include <QMutexLocker>
#include <QDebug>

#include <vector>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <iterator>

constexpr double aruco_tracker::timeout;
constexpr double aruco_tracker::timeout_backoff_c;
constexpr const int aruco_tracker::adaptive_sizes[];

constexpr const aruco_tracker::resolution_tuple aruco_tracker::resolution_choices[];

constexpr const double aruco_tracker::RC;
constexpr const float aruco_tracker::size_min;
constexpr const float aruco_tracker::size_max;

#ifdef DEBUG_UNSHARP_MASKING
constexpr double aruco_tracker::gauss_kernel_size;
#endif

aruco_tracker::aruco_tracker() :
    pose{0,0,0, 0,0,0},
    fps(0),
    no_detection_timeout(0),
    obj_points(4),
    intrinsics(cv::Matx33d::eye()),
    rmat(cv::Matx33d::eye()),
    roi_points(4),
    last_roi(65535, 65535, 0, 0),
    adaptive_size_pos(0),
    use_otsu(false)
{
    cv::setBreakOnError(true);
    // param 2 ignored for Otsu thresholding. it's required to use our fork of Aruco.
    set_detector_params();
}

aruco_tracker::~aruco_tracker()
{
    requestInterruption();
    wait();
    // fast start/stop causes breakage
    portable::sleep(1000);
    camera.release();
}

module_status aruco_tracker::start_tracker(QFrame* videoframe)
{
    videoframe->show();
    videoWidget = std::make_unique<cv_video_widget>(videoframe);
    layout = std::make_unique<QHBoxLayout>();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(videoWidget.get());
    videoframe->setLayout(layout.get());
    videoWidget->show();
    start();

    return status_ok();
}

void aruco_tracker::getRT(cv::Matx33d& r_, cv::Vec3d& t_)
{
    QMutexLocker l(&mtx);

    r_ = r;
    t_ = t;
}

bool aruco_tracker::detect_with_roi()
{
    if (last_roi.width > 1 && last_roi.height > 1)
    {
        detector.setMinMaxSize(clamp(size_min * grayscale.cols / last_roi.width, .01f, 1.f),
                               clamp(size_max * grayscale.cols / last_roi.width, .01f, 1.f));

        detector.detect(grayscale(last_roi), markers, cv::Mat(), cv::Mat(), -1, false);

        if (markers.size() == 1 && markers[0].size() == 4)
        {
            auto& m = markers[0];
            for (unsigned i = 0; i < 4; i++)
            {
                auto& p = m[i];
                p.x += last_roi.x;
                p.y += last_roi.y;
            }
            return true;
        }
    }

    last_roi = cv::Rect(65535, 65535, 0, 0);
    return false;
}

bool aruco_tracker::detect_without_roi()
{
    detector.setMinMaxSize(size_min, size_max);
    detector.detect(grayscale, markers, cv::Mat(), cv::Mat(), -1, false);
    return markers.size() == 1 && markers[0].size() == 4;
}

bool aruco_tracker::open_camera()
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
        fps = 75;
        break;
    case 4:
        fps = 125;
        break;
    case 5:
        fps = 200;
        break;
    }

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
        return false;
    }
    return true;
}

void aruco_tracker::set_intrinsics()
{
    const int w = grayscale.cols, h = grayscale.rows;
    const double diag_fov = static_cast<int>(s.fov) * M_PI / 180.;
    const double fov_w = 2.*atan(tan(diag_fov/2.)/sqrt(1. + h/(double)w * h/(double)w));
    const double fov_h = 2.*atan(tan(diag_fov/2.)/sqrt(1. + w/(double)h * w/(double)h));
    const double focal_length_w = .5 * w / tan(.5 * fov_w);
    const double focal_length_h = .5 * h / tan(.5 * fov_h);

    intrinsics(0, 0) = focal_length_w;
    intrinsics(0, 2) = grayscale.cols/2;
    intrinsics(1, 1) = focal_length_h;
    intrinsics(1, 2) = grayscale.rows/2;
}

void aruco_tracker::update_fps()
{
    const double dt = fps_timer.elapsed_seconds();
    fps_timer.start();
    const double alpha = dt/(dt + RC);

    if (dt > 1e-3)
    {
        fps *= 1 - alpha;
        fps += alpha * (1./dt + .8);
    }
}

void aruco_tracker::draw_ar(bool ok)
{
    if (ok)
    {
        const auto& m = markers[0];
        for (unsigned i = 0; i < 4; i++)
            cv::line(frame, m[i], m[(i+1)%4], cv::Scalar(0, 0, 255), 2, 8);
    }

    char buf[9];
    ::snprintf(buf, sizeof(buf)-1, "Hz: %d", clamp(int(fps), 0, 9999));
    buf[sizeof(buf)-1] = '\0';
    cv::putText(frame, buf, cv::Point(10, 32), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0, 255, 0), 1);
}

void aruco_tracker::clamp_last_roi()
{
    last_roi &= cv::Rect(0, 0, color.cols, color.rows);
}

cv::Point3f aruco_tracker::rotate_model(float x, float y, settings::rot mode)
{
    cv::Point3f pt(x, y, 0);

    if (mode)
    {
        const double theta = int(mode) * 90/4. * M_PI/180;
        pt.x = x * cos(theta) - y * sin(theta);
        pt.y = y * cos(theta) + x * sin(theta);
    }
    return pt;
}

void aruco_tracker::set_points()
{
    using f = float;
    const f hx = f(s.headpos_x), hy = f(s.headpos_y), hz = f(s.headpos_z);

    constexpr float size = 40;

    const int x1=1, x2=2, x3=3, x4=0;

    settings::rot mode = s.model_rotation;

    obj_points[x1] = rotate_model(-size, -size, mode);
    obj_points[x2] = rotate_model(size, -size, mode);
    obj_points[x3] = rotate_model(size, size, mode);
    obj_points[x4] = rotate_model(-size, size, mode);

    for (unsigned i = 0; i < 4; i++)
        obj_points[i] += cv::Point3f(hx, hy, hz);
}

void aruco_tracker::draw_centroid()
{
    repr2.clear();

    static const std::vector<cv::Point3f> centroid { cv::Point3f(0, 0, 0) };

    cv::projectPoints(centroid, rvec, tvec, intrinsics, cv::noArray(), repr2);

    cv::circle(frame, repr2[0], 4, cv::Scalar(255, 0, 255), -1);
}

void aruco_tracker::set_last_roi()
{
    roi_projection.clear();

    using f = float;
    cv::Point3f h(f(s.headpos_x), f(s.headpos_y), f(s.headpos_z));
    for (unsigned i = 0; i < 4; i++)
    {
        cv::Point3f pt(obj_points[i] - h);
        roi_points[i] = pt * c_search_window;
    }

    cv::projectPoints(roi_points, rvec, tvec, intrinsics, cv::noArray(), roi_projection);

    set_roi_from_projection();
}

void aruco_tracker::set_rmat()
{
    cv::Rodrigues(rvec, rmat);

    euler = cv::RQDecomp3x3(rmat, m_r, m_q);

    QMutexLocker lck(&mtx);

    for (int i = 0; i < 3; i++)
        pose[i] = tvec(i) * .1;

    pose[Yaw] = euler[1];
    pose[Pitch] = -euler[0];
    pose[Roll] = euler[2];

    r = rmat;
    t = cv::Vec3d(tvec[0], -tvec[1], tvec[2]);
}

void aruco_tracker::set_roi_from_projection()
{
    last_roi = cv::Rect(color.cols-1, color.rows-1, 0, 0);

    for (unsigned i = 0; i < 4; i++)
    {
        const auto& proj = roi_projection[i];
        int min_x = std::min(int(proj.x), last_roi.x),
            min_y = std::min(int(proj.y), last_roi.y);

        int max_x = std::max(int(proj.x), last_roi.width),
            max_y = std::max(int(proj.y), last_roi.height);

        last_roi.x = min_x;
        last_roi.y = min_y;

        last_roi.width = max_x;
        last_roi.height = max_y;
    }

    clamp_last_roi();
}

void aruco_tracker::set_detector_params()
{
    detector.setDesiredSpeed(3);
    detector.setThresholdParams(adaptive_sizes[adaptive_size_pos], adaptive_thres);
#if !defined USE_EXPERIMENTAL_CANNY
    if (use_otsu)
        detector._thresMethod = aruco::MarkerDetector::FIXED_THRES;
    else
        detector._thresMethod = aruco::MarkerDetector::ADPT_THRES;
#else
        detector._thresMethod = aruco::MarkerDetector::CANNY;
#endif
}

void aruco_tracker::cycle_detection_params()
{
#if !defined USE_EXPERIMENTAL_CANNY
    if (!use_otsu)
        use_otsu = true;
    else
#endif
    {
        use_otsu = false;

        adaptive_size_pos++;
        adaptive_size_pos %= std::size(adaptive_sizes);
    }

    set_detector_params();

    qDebug() << "aruco: switched thresholding params"
#if !defined USE_EXPERIMENTAL_CANNY
             << "otsu:" << use_otsu
#endif
             << "size:" << adaptive_sizes[adaptive_size_pos];
}

void aruco_tracker::run()
{
    cv::setNumThreads(1);

    if (!open_camera())
        return;

    fps_timer.start();
    last_detection_timer.start();

    while (!isInterruptionRequested())
    {
        {
            QMutexLocker l(&camera_mtx);

            if (!camera.read(color))
                continue;
        }

        cv::cvtColor(color, grayscale, cv::COLOR_BGR2GRAY);

#ifdef DEBUG_UNSHARP_MASKING
        {
            constexpr double strength = double(DEBUG_UNSHARP_MASKING);
            cv::GaussianBlur(grayscale, blurred, cv::Size(0, 0), gauss_kernel_size);
            cv::addWeighted(grayscale, 1 + strength, blurred, -strength, 0, grayscale);
            cv::imshow("capture", grayscale);
            cv::waitKey(1);
        }
#endif

        color.copyTo(frame);

        set_intrinsics();

        update_fps();

        markers.clear();

        const bool ok = detect_with_roi() || detect_without_roi();

        if (ok)
        {
            set_points();

            if (!cv::solvePnP(obj_points, markers[0], intrinsics, cv::noArray(), rvec, tvec, false, cv::SOLVEPNP_ITERATIVE))
                goto fail;

            {
                const double dt = last_detection_timer.elapsed_seconds();
                last_detection_timer.start();
                no_detection_timeout -= dt * timeout_backoff_c;
                no_detection_timeout = std::fmax(0., no_detection_timeout);
            }

            set_last_roi();
            draw_centroid();
            set_rmat();
        }
        else
        {
fail:
            // no marker found, reset search region
            last_roi = cv::Rect(65535, 65535, 0, 0);

            const double dt = last_detection_timer.elapsed_seconds();
            last_detection_timer.start();
            no_detection_timeout += dt;
            if (no_detection_timeout > timeout)
            {
                no_detection_timeout = 0;
                cycle_detection_params();
            }
        }

        draw_ar(ok);

        if (frame.rows > 0)
            videoWidget->update_image(frame);
    }
}

void aruco_tracker::data(double *data)
{
    QMutexLocker lck(&mtx);

    data[Yaw] = pose[Yaw];
    data[Pitch] = pose[Pitch];
    data[Roll] = pose[Roll];
    data[TX] = pose[TX];
    data[TY] = pose[TY];
    data[TZ] = pose[TZ];
}

aruco_dialog::aruco_dialog() :
    calibrator(1, 0, 2)
{
    tracker = nullptr;
    calib_timer.setInterval(100);
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

    ui.model_rotation->addItem("0", int(settings::rot_zero));
    ui.model_rotation->addItem("+22.5", int(settings::rot_plus));
    ui.model_rotation->addItem("-22.5", int(settings::rot_neg));
    tie_setting(s.model_rotation, ui.model_rotation);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui.btn_calibrate, SIGNAL(clicked()), this, SLOT(toggleCalibrate()));
    connect(this, SIGNAL(destroyed()), this, SLOT(cleanupCalib()));
    connect(&calib_timer, SIGNAL(timeout()), this, SLOT(update_tracker_calibration()));
    connect(ui.camera_settings, SIGNAL(clicked()), this, SLOT(camera_settings()));

    connect(&s.camera_name, SIGNAL(valueChanged(const QString&)), this, SLOT(update_camera_settings_state(const QString&)));

    update_camera_settings_state(s.camera_name);
}

void aruco_dialog::toggleCalibrate()
{
    if (!calib_timer.isActive())
    {
        s.headpos_x = 0;
        s.headpos_y = 0;
        s.headpos_z = 0;
        calibrator.reset();
        calib_timer.start();
    }
    else
    {
        cleanupCalib();

        cv::Vec3d pos;
        std::tie(pos, std::ignore) = calibrator.get_estimate();
        s.headpos_x = -pos(0);
        s.headpos_y = -pos(1);
        s.headpos_z = -pos(2);
    }
}

void aruco_dialog::cleanupCalib()
{
    if (calib_timer.isActive())
        calib_timer.stop();
}

void aruco_dialog::update_tracker_calibration()
{
    if (calib_timer.isActive() && tracker)
    {
        cv::Matx33d r;
        cv::Vec3d t;
        tracker->getRT(r, t);
        calibrator.update(r, t);
    }
}

void aruco_dialog::doOK()
{
    s.b->save();
    close();
}

void aruco_dialog::doCancel()
{
    close();
}

void aruco_dialog::camera_settings()
{
    if (tracker)
    {
        QMutexLocker l(&tracker->camera_mtx);
        video_property_page::show_from_capture(tracker->camera, camera_name_to_index(s.camera_name));
    }
    else
        video_property_page::show(camera_name_to_index(s.camera_name));
}

void aruco_dialog::update_camera_settings_state(const QString& name)
{
    ui.camera_settings->setEnabled(video_property_page::should_show_dialog(name));
}

OPENTRACK_DECLARE_TRACKER(aruco_tracker, aruco_dialog, aruco_metadata)
