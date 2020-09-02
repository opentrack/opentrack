/* Copyright (c) 2019 Stephane Lenclud
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
#include "kalman-filter-pose.h"

#include <atomic>
#include <memory>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/video/tracking.hpp>

#include <QThread>
#include <QMutex>
#include <QLayout>

namespace EasyTracker
{

    namespace VertexPosition
    {
        enum Type
        {
            Top = 0,
            Right,
            Left,
            TopRight,
            TopLeft,
            Center
        };
    }

    static const QString KModuleName = "tracker-easy";

    class Dialog;

    using namespace numeric_types;

    struct Tracker : public QObject, ITracker
    {
        Q_OBJECT
    public:        
        friend class Dialog;

        explicit Tracker();
        ~Tracker() override;

        // From ITracker
        module_status start_tracker(QFrame* parent_window) override;
        void data(double* data) override;
        bool center() override;

    private slots:
        void Tick();


    private:
        void UpdateModel();
        void CreateCameraIntrinsicsMatrices();
        void ProcessFrame();        
        void MatchVertices(int& aTopIndex, int& aRightIndex, int& aLeftIndex, int& aCenterIndex, int& aTopRight, int& aTopLeft);
        void MatchThreeOrFourVertices(int& aTopIndex, int& aRightIndex, int& aLeftIndex, int& aCenterIndex);
        void MatchFiveVertices(int& aTopIndex, int& aRightIndex, int& aLeftIndex, int& aTopRight, int& aTopLeft);
        void MatchClipVertices(int& aTopIndex, int& aMiddleIndex, int& aBottomIndex);
        
        
        //

        bool CheckCamera();
        void set_fov(int value);
        void SetFps(int aFps);
        void DoSetFps(int aFps);
        void UpdateSettings();

        QMutex camera_mtx;
        QThread iThread;
        QTimer iTicker;

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
        mutable QMutex iProcessLock, iDataLock;

        //// Copy the settings need by our thread to avoid dead locks
        // Deadzone
        int iDeadzoneEdge=0;
        int iDeadzoneHalfEdge=0;
        // Solver
        int iSolver = cv::SOLVEPNP_P3P;
        bool iDebug = false;
        ////

        // Statistics
        Timer iTimer;
        Timer iFpsTimer;        
        int iFrameCount = 0;
        int iSkippedFrameCount = 0;
        int iFps = 0;
        int iSkippedFps = 0;
        uint iBadSolutionCount = 0;
        uint iGoodSolutionCount = 0;

        //
        KalmanFilterPose iKf;

        // Vertices defining the model we are tracking
        std::vector<cv::Point3f> iModel;
        // Bitmap points corresponding to model vertices
        std::vector<cv::Point2f> iTrackedPoints;

        std::vector<cv::Rect> iTrackedRects;

        // Intrinsics camera matrix
        cv::Matx33d iCameraMatrix { cv::Matx33d::zeros() };
        // Intrinsics distortion coefficients as a matrix
        cv::Matx<double, 8, 1> iDistCoeffsMatrix;
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
        // Time at which we found our last best solution
        Timer iBestTime;
        // Center translation
        cv::Vec3d iCenterTranslation = {0,0,0};
        // Center angles
        cv::Vec3d iCenterAngles = { 0,0,0 };
    };

}
