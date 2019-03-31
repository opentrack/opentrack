/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_pt.h"
#include "video/video-widget.hpp"
#include "compat/math-imports.hpp"
#include "compat/check-visible.hpp"

#include "pt-api.hpp"

#include <QHBoxLayout>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>

#include <opencv2\calib3d.hpp>

#include <iostream>

using namespace options;

namespace pt_impl {

Tracker_PT::Tracker_PT(pointer<pt_runtime_traits> const& traits) :
    traits { traits },
    s { traits->get_module_name() },
    point_extractor { traits->make_point_extractor() },
    camera { traits->make_camera() },
    frame { traits->make_frame() },
    preview_frame { traits->make_preview(preview_width, preview_height) }
{
    cv::setBreakOnError(true);
    cv::setNumThreads(1);

    connect(s.b.get(), &bundle_::saving, this, &Tracker_PT::maybe_reopen_camera, Qt::DirectConnection);
    connect(s.b.get(), &bundle_::reloading, this, &Tracker_PT::maybe_reopen_camera, Qt::DirectConnection);

    connect(&s.fov, value_::value_changed<int>(), this, &Tracker_PT::set_fov, Qt::DirectConnection);
    set_fov(s.fov);
}

Tracker_PT::~Tracker_PT()
{
    requestInterruption();
    wait();

    QMutexLocker l(&camera_mtx);
    camera->stop();
}


// Calculates rotation matrix to euler angles
cv::Vec3f EulerAngles(cv::Mat &R)
{

    float sy = sqrt(R.at<double>(0, 0) * R.at<double>(0, 0) + R.at<double>(1, 0) * R.at<double>(1, 0));

    bool singular = sy < 1e-6; // If

    float x, y, z;
    if (!singular)
    {
        x = atan2(R.at<double>(2, 1), R.at<double>(2, 2));
        y = atan2(-R.at<double>(2, 0), sy);
        z = atan2(R.at<double>(1, 0), R.at<double>(0, 0));
    }
    else
    {
        x = atan2(-R.at<double>(1, 2), R.at<double>(1, 1));
        y = atan2(-R.at<double>(2, 0), sy);
        z = 0;
    }

    // Convert to degrees
    return cv::Vec3f(x* 180 / CV_PI, y* 180 / CV_PI, z* 180 / CV_PI);
}


void getEulerAngles(cv::Mat &rotCamerMatrix, cv::Vec3d &eulerAngles)
{

    cv::Mat cameraMatrix, rotMatrix, transVect, rotMatrixX, rotMatrixY, rotMatrixZ;
    double* _r = rotCamerMatrix.ptr<double>();
    double projMatrix[12] = { _r[0],_r[1],_r[2],0,
                          _r[3],_r[4],_r[5],0,
                          _r[6],_r[7],_r[8],0 };

    cv::decomposeProjectionMatrix(cv::Mat(3, 4, CV_64FC1, projMatrix),
        cameraMatrix,
        rotMatrix,
        transVect,
        rotMatrixX,
        rotMatrixY,
        rotMatrixZ,
        eulerAngles);
}


void Tracker_PT::run()
{
    maybe_reopen_camera();

    while(!isInterruptionRequested())
    {
        pt_camera_info info;
        bool new_frame = false;

        {
            QMutexLocker l(&camera_mtx);

            if (camera)
                std::tie(new_frame, info) = camera->get_frame(*frame);
        }

        if (new_frame)
        {
            const bool preview_visible = check_is_visible();

            if (preview_visible)
                *preview_frame = *frame;

            iImagePoints.clear();
            point_extractor->extract_points(*frame, *preview_frame, points, iImagePoints);
            point_count.store(points.size(), std::memory_order_relaxed);

            const bool success = points.size() >= PointModel::N_POINTS;

            Affine X_CM;

            {
                QMutexLocker l(&center_lock);

                if (success)
                {
                    int dynamic_pose_ms = s.dynamic_pose && s.active_model_panel != PointModel::Clip
                                          ? s.init_phase_timeout
                                          : 0;

                    point_tracker.track(points,
                                        PointModel(s),
                                        info,
                                        dynamic_pose_ms);
                    ever_success.store(true, std::memory_order_relaxed);

                    // Solve P3P problem with OpenCV

                    // Construct the points defining the object we want to detect based on settings.
                    // We are converting them from millimeters to meters.
                    // TODO: Need to support clip too. That's cap only for now. 
                    std::vector<cv::Point3f> objectPoints;                    
                    objectPoints.push_back(cv::Point3f(s.cap_x/10.0,  s.cap_z / 10.0,  -s.cap_y / 10.0)); // Right
                    objectPoints.push_back(cv::Point3f(-s.cap_x/10.0,  s.cap_z / 10.0,  -s.cap_y / 10.0)); // Left
                    objectPoints.push_back(cv::Point3f(0, 0, 0)); // Top

                    //Bitmap origin is top left
                    std::vector<cv::Point2f> trackedPoints;
                    // Stuff bitmap point in there making sure they match the order of the object point  
                    // Find top most point, that's the one with min Y as we assume our guy's head is not up side down
                    int topPointIndex = -1;
                    int minY = std::numeric_limits<int>::max();
                    for (int i = 0; i < 3; i++)
                    {
                        if (iImagePoints[i][1]<minY)
                        {
                            minY = iImagePoints[i][1];
                            topPointIndex = i;
                        }
                    }

                    int rightPointIndex = -1;
                    int maxX = 0;

                    // Find right most point 
                    for (int i = 0; i < 3; i++)
                    {
                        // Excluding top most point
                        if (i!=topPointIndex && iImagePoints[i][0] > maxX)
                        {
                            maxX = iImagePoints[i][0];
                            rightPointIndex = i;
                        }
                    }

                    // Find left most point
                    int leftPointIndex = -1;
                    for (int i = 0; i < 3; i++)
                    {
                        // Excluding top most point
                        if (i != topPointIndex && i != rightPointIndex)
                        {
                            leftPointIndex = i;
                            break;
                        }
                    }

                    //
                    trackedPoints.push_back(cv::Point2f(iImagePoints[rightPointIndex][0], iImagePoints[rightPointIndex][1]));
                    trackedPoints.push_back(cv::Point2f(iImagePoints[leftPointIndex][0], iImagePoints[leftPointIndex][1]));                    
                    trackedPoints.push_back(cv::Point2f(iImagePoints[topPointIndex][0], iImagePoints[topPointIndex][1]));

                    std::cout << "Object: " << objectPoints << "\n";
                    std::cout << "Points: " << trackedPoints << "\n";


                    // Create our camera matrix
                    // TODO: Just do that once, use data member instead
                    // Double or Float?
                    cv::Mat cameraMatrix;
                    cameraMatrix.create(3, 3, CV_64FC1);
                    cameraMatrix.setTo(cv::Scalar(0));
                    cameraMatrix.at<double>(0, 0) = camera->info.focalLengthX;
                    cameraMatrix.at<double>(1, 1) = camera->info.focalLengthY;
                    cameraMatrix.at<double>(0, 2) = camera->info.principalPointX;
                    cameraMatrix.at<double>(1, 2) = camera->info.principalPointY;
                    cameraMatrix.at<double>(2, 2) = 1;

                    // Create distortion cooefficients
                    cv::Mat distCoeffs = cv::Mat::zeros(8, 1, CV_64FC1);
                    // As per OpenCV docs they should be thus: k1, k2, p1, p2, k3, k4, k5, k6
                    distCoeffs.at<double>(0, 0) = 0; // Radial first order
                    distCoeffs.at<double>(1, 0) = camera->info.radialDistortionSecondOrder; // Radial second order
                    distCoeffs.at<double>(2, 0) = 0; // Tangential first order
                    distCoeffs.at<double>(3, 0) = 0; // Tangential second order
                    distCoeffs.at<double>(4, 0) = 0; // Radial third order
                    distCoeffs.at<double>(5, 0) = camera->info.radialDistortionFourthOrder; // Radial fourth order
                    distCoeffs.at<double>(6, 0) = 0; // Radial fith order
                    distCoeffs.at<double>(7, 0) = camera->info.radialDistortionSixthOrder; // Radial sixth order

                    // Define our solution arrays
                    // They will receive up to 4 solutions for our P3P problem
                    

                    // TODO: try SOLVEPNP_AP3P too
                    iAngles.clear();
                    iBestSolutionIndex = -1;
                    int solutionCount = cv::solveP3P(objectPoints, trackedPoints, cameraMatrix, distCoeffs, iRotations, iTranslations, cv::SOLVEPNP_P3P);

                    if (solutionCount > 0)
                    {
                        std::cout << "Solution count: " << solutionCount << "\n";
                        int minPitch = std::numeric_limits<int>::max();
                        // Find the solution we want
                        for (int i = 0; i < solutionCount; i++)
                        {
                            std::cout << "Translation:\n";
                            std::cout << iTranslations.at(i);
                            std::cout << "\n";
                            std::cout << "Rotation:\n";
                            //std::cout << rvecs.at(i);
                            cv::Mat rotationCameraMatrix;
                            cv::Rodrigues(iRotations[i], rotationCameraMatrix);
                            cv::Vec3d angles;
                            getEulerAngles(rotationCameraMatrix,angles);
                            iAngles.push_back(angles);

                            // Check if pitch is closest to zero
                            int absolutePitch = std::abs(angles[0]);
                            if (minPitch > absolutePitch)
                            {
                                minPitch = absolutePitch;
                                iBestSolutionIndex = i;
                            }

                            //cv::Vec3f angles=EulerAngles(quaternion);
                            std::cout << angles;
                            std::cout << "\n";
                        }

                        std::cout << "\n";
                        
                    }

                    // TODO: Work out rotation angles
                    // TODO: Choose the one solution that makes sense for us



                }

                QMutexLocker l2(&data_lock);
                X_CM = point_tracker.pose();
                if (iBestSolutionIndex != -1)
                {
                    iBestAngles = iAngles[iBestSolutionIndex];
                    iBestTranslation = iTranslations[iBestSolutionIndex];
                }

            }

            if (preview_visible)
            {
                const f fx = pt_camera_info::get_focal_length(info.fov, info.res_x, info.res_y);
                Affine X_MH(mat33::eye(), vec3(s.t_MH_x, s.t_MH_y, s.t_MH_z));
                Affine X_GH = X_CM * X_MH;
                vec3 p = X_GH.t; // head (center?) position in global space

                if (p[2] > f(.1))
                    preview_frame->draw_head_center((p[0] * fx) / p[2], (p[1] * fx) / p[2]);

                widget->update_image(preview_frame->get_bitmap());

                auto [ w, h ] = widget->preview_size();
                if (w != preview_width || h != preview_height)
                {
                    preview_width = w; preview_height = h;
                    preview_frame = traits->make_preview(w, h);
                }
            }
        }
    }
}

bool Tracker_PT::maybe_reopen_camera()
{
    QMutexLocker l(&camera_mtx);

    return camera->start(s.camera_name,
                         s.cam_fps, s.cam_res_x, s.cam_res_y);
}

void Tracker_PT::set_fov(int value)
{
    QMutexLocker l(&camera_mtx);
    camera->set_fov(value);
}

module_status Tracker_PT::start_tracker(QFrame* video_frame)
{
    //video_frame->setAttribute(Qt::WA_NativeWindow);

    widget = std::make_unique<video_widget>(video_frame);
    layout = std::make_unique<QHBoxLayout>(video_frame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(widget.get());
    video_frame->setLayout(layout.get());
    //video_widget->resize(video_frame->width(), video_frame->height());
    video_frame->show();

    start(QThread::HighPriority);

    return {};
}

void Tracker_PT::data(double *data)
{
    if (ever_success.load(std::memory_order_relaxed))
    {
        Affine X_CM;
        {
            QMutexLocker l(&data_lock);
            X_CM = point_tracker.pose();
        }

        Affine X_MH(mat33::eye(), vec3(s.t_MH_x, s.t_MH_y, s.t_MH_z));
        Affine X_GH(X_CM * X_MH);

        // translate rotation matrix from opengl (G) to roll-pitch-yaw (E) frame
        // -z -> x, y -> z, x -> -y
        mat33 R_EG(0, 0,-1,
                   -1, 0, 0,
                   0, 1, 0);
        mat33 R(R_EG *  X_GH.R * R_EG.t());

        // get translation(s)
        const vec3& t = X_GH.t;

        // extract rotation angles
        auto r00 = (double)R(0, 0);
        auto r10 = (double)R(1,0), r20 = (double)R(2,0);
        auto r21 = (double)R(2,1), r22 = (double)R(2,2);

        double beta  = atan2(-r20, sqrt(r21*r21 + r22*r22));
        double alpha = atan2(r10, r00);
        double gamma = atan2(r21, r22);

        constexpr double rad2deg = 180/M_PI;

        data[Yaw]   = rad2deg * alpha;
        data[Pitch] = -rad2deg * beta;
        data[Roll]  = rad2deg * gamma;

        // convert to cm
        data[TX] = (double)t[0] / 10;
        data[TY] = (double)t[1] / 10;
        data[TZ] = (double)t[2] / 10;


        QMutexLocker l(&data_lock);
        data[Yaw] = iBestAngles[1];
        data[Pitch] = iBestAngles[0];
        data[Roll] = iBestAngles[2];
        data[TX] = iBestTranslation[0];
        data[TY] = iBestTranslation[1];
        data[TZ] = iBestTranslation[2];

    }
}

bool Tracker_PT::center()
{
    QMutexLocker l(&center_lock);

    point_tracker.reset_state();
    return false;
}

int Tracker_PT::get_n_points()
{
    return (int)point_count.load(std::memory_order_relaxed);
}

bool Tracker_PT::get_cam_info(pt_camera_info& info)
{
    QMutexLocker l(&camera_mtx);
    bool ret;

    std::tie(ret, info) = camera->get_info();
    return ret;
}

Affine Tracker_PT::pose() const
{
    QMutexLocker l(&data_lock);
    return point_tracker.pose();
}

} // ns pt_impl
