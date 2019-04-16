/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "api/plugin-api.hpp"
#include "cv/numeric.hpp"
#include "video/video-widget.hpp"
#include "video/camera.hpp"
#include "compat/timer.hpp"

#include "preview.h"
#include "settings.h"
#include "point-extractor.h"

#include <atomic>
#include <memory>
#include <vector>

#include <opencv2/core.hpp>

#include <QThread>
#include <QMutex>
#include <QLayout>

namespace EasyTracker
{

    static const QString KModuleName = "tracker-easy";

    class Dialog;

    using namespace numeric_types;

    struct Tracker : QThread, ITracker
    {
        // We had problem where Qt slots would not get disconnected upon object destruction.
        // Issue seems to be gone now even without Q_OBJECT declaration, go figure.
        //Q_OBJECT
    public:        
        friend class Dialog;

        explicit Tracker();
        ~Tracker() override;

        // From ITracker
        module_status start_tracker(QFrame* parent_window) override;
        void data(double* data) override;
        bool center() override;
       
    private:
        void CreateModelFromSettings();
        void CreateCameraIntrinsicsMatrices();
        void ProcessFrame();

        // From QThread
        void run() override;

        bool maybe_reopen_camera();
        void set_fov(int value);

        QMutex camera_mtx;


        Settings iSettings;

        std::unique_ptr<QLayout> layout;
        std::vector<cv::Point> iPoints;

        int preview_width = 320, preview_height = 240;

        PointExtractor iPointExtractor;

        std::unique_ptr<video::impl::camera> camera;
        video::impl::camera::info iCameraInfo;
        std::unique_ptr<video_widget> widget;

        video::frame iFrame;
        cv::Mat iMatFrame;
        Preview iPreview;

        std::atomic<bool> ever_success = false;
        mutable QMutex center_lock, data_lock;

        //
        Timer iTimer;
        Timer iFpsTimer;
        int iFrameCount = 0;
        int iSkippedFrameCount = 0;
        int iFps = 0;
        int iSkippedFps = 0;

        // Vertices defining the model we are tracking
        std::vector<cv::Point3f> iModel;
        // Bitmap points corresponding to model vertices
        std::vector<cv::Point2f> iTrackedPoints;
        // Intrinsics camera matrix
        cv::Mat iCameraMatrix;
        // Intrinsics distortion coefficients as a matrix
        cv::Mat iDistCoeffsMatrix;
        // Translation solutions
        std::vector<cv::Mat> iTranslations;
        // Rotation solutions
        std::vector<cv::Mat> iRotations;
        // Angle solutions, pitch, yaw, roll, in this order
        std::vector<cv::Vec3d> iAngles;
        // The index of our best solution in the above arrays
        int iBestSolutionIndex = -1;
        // Best translation
        cv::Vec3d iBestTranslation;
        // Best angles
        cv::Vec3d iBestAngles;
    };

}
