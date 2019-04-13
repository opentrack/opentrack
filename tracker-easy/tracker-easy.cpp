/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2016 Stanislaw Halik <sthalik@misaki.pl>
 * Copyright (c) 2019 Stephane Lenclud
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "tracker-easy.h"
#include "video/video-widget.hpp"
#include "compat/math-imports.hpp"
#include "compat/check-visible.hpp"

#include "tracker-easy-api.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>

#include <opencv2/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>

using namespace options;


EasyTracker::EasyTracker(pointer<pt_runtime_traits> const& traits) :
    traits { traits },
    s { traits->get_module_name() },
    point_extractor { traits->make_point_extractor() },
    iPreview{ preview_width, preview_height }
{
    cv::setBreakOnError(true);
    cv::setNumThreads(1);

    connect(s.b.get(), &bundle_::saving, this, &EasyTracker::maybe_reopen_camera, Qt::DirectConnection);
    connect(s.b.get(), &bundle_::reloading, this, &EasyTracker::maybe_reopen_camera, Qt::DirectConnection);

    connect(&s.fov, value_::value_changed<int>(), this, &EasyTracker::set_fov, Qt::DirectConnection);
    set_fov(s.fov);
}

EasyTracker::~EasyTracker()
{
    //
    cv::destroyWindow("Preview");

    requestInterruption();
    wait();

    QMutexLocker l(&camera_mtx);
    camera->stop();    
}


// Compute Euler angles from ratation matrix
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


void EasyTracker::run()
{
    maybe_reopen_camera();

    while(!isInterruptionRequested())
    {        
        bool new_frame = false;

        {
            QMutexLocker l(&camera_mtx);

            if (camera)
                std::tie(iFrame, new_frame) = camera->get_frame();
        }

        if (new_frame)
        {
            //TODO: We should not assume channel size of 1 byte
            iMatFrame = cv::Mat(iFrame.height, iFrame.width, CV_MAKETYPE(CV_8U,iFrame.channels), iFrame.data, iFrame.stride);


            const bool preview_visible = check_is_visible();
            if (preview_visible)
            {
                iPreview = iMatFrame;
            }

            iImagePoints.clear();
            point_extractor->extract_points(iMatFrame, iPreview.iFrameRgb, points, iImagePoints);
            point_count.store(points.size(), std::memory_order_relaxed);

            
            if (preview_visible)
            {
                //iPreview = iMatFrame;
                cv::imshow("Preview", iPreview.iFrameRgb);
                cv::waitKey(1);
            }
            else
            {
                cv::destroyWindow("Preview");
            }

            const bool success = points.size() >= KPointCount || iImagePoints.size() >= KPointCount;

            int topPointIndex = -1;

            {
                QMutexLocker l(&center_lock);

                if (success)
                {
                    ever_success.store(true, std::memory_order_relaxed);

                    // Solve P3P problem with OpenCV

                    // Construct the points defining the object we want to detect based on settings.
                    // We are converting them from millimeters to centimeters.
                    // TODO: Need to support clip too. That's cap only for now.
                    // s.active_model_panel != PointModel::Clip

                    std::vector<cv::Point3f> objectPoints;                    
                    objectPoints.push_back(cv::Point3f(s.cap_x/10.0,  s.cap_z / 10.0,  -s.cap_y / 10.0)); // Right
                    objectPoints.push_back(cv::Point3f(-s.cap_x/10.0,  s.cap_z / 10.0,  -s.cap_y / 10.0)); // Left
                    objectPoints.push_back(cv::Point3f(0, 0, 0)); // Top

                    //Bitmap origin is top left
                    std::vector<cv::Point2f> trackedPoints;
                    // Stuff bitmap point in there making sure they match the order of the object point  
                    // Find top most point, that's the one with min Y as we assume our guy's head is not up side down
                    
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
                    cameraMatrix.at<double>(0, 0) = iCameraInfo.focalLengthX;
                    cameraMatrix.at<double>(1, 1) = iCameraInfo.focalLengthY;
                    cameraMatrix.at<double>(0, 2) = iCameraInfo.principalPointX;
                    cameraMatrix.at<double>(1, 2) = iCameraInfo.principalPointY;
                    cameraMatrix.at<double>(2, 2) = 1;

                    // Create distortion cooefficients
                    cv::Mat distCoeffs = cv::Mat::zeros(8, 1, CV_64FC1);
                    // As per OpenCV docs they should be thus: k1, k2, p1, p2, k3, k4, k5, k6
                    distCoeffs.at<double>(0, 0) = 0; // Radial first order
                    distCoeffs.at<double>(1, 0) = iCameraInfo.radialDistortionSecondOrder; // Radial second order
                    distCoeffs.at<double>(2, 0) = 0; // Tangential first order
                    distCoeffs.at<double>(3, 0) = 0; // Tangential second order
                    distCoeffs.at<double>(4, 0) = 0; // Radial third order
                    distCoeffs.at<double>(5, 0) = iCameraInfo.radialDistortionFourthOrder; // Radial fourth order
                    distCoeffs.at<double>(6, 0) = 0; // Radial fith order
                    distCoeffs.at<double>(7, 0) = iCameraInfo.radialDistortionSixthOrder; // Radial sixth order

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

                }

                // Send solution data back to main thread
                QMutexLocker l2(&data_lock);
                if (iBestSolutionIndex != -1)
                {
                    iBestAngles = iAngles[iBestSolutionIndex];
                    iBestTranslation = iTranslations[iBestSolutionIndex];
                }

            }

            if (preview_visible)
            {
                if (topPointIndex != -1)
                {
                    // Render a cross to indicate which point is the head
                    if (points.size() >= 3)
                    {
                        iPreview.draw_head_center(points[topPointIndex][0], points[topPointIndex][1]);
                    }                    
                }
                
                widget->update_image(iPreview.get_bitmap());

                auto [ w, h ] = widget->preview_size();
                if (w != preview_width || h != preview_height)
                {
                    // Resize preivew if widget size has changed
                    preview_width = w; preview_height = h;
                    iPreview = Preview(w, h);
                }
            }
        }
    }
}

bool EasyTracker::maybe_reopen_camera()
{
    QMutexLocker l(&camera_mtx);

    return camera->start(iCameraInfo);
}

void EasyTracker::set_fov(int value)
{
    QMutexLocker l(&camera_mtx);

}

module_status EasyTracker::start_tracker(QFrame* video_frame)
{
    //video_frame->setAttribute(Qt::WA_NativeWindow);

    widget = std::make_unique<video_widget>(video_frame);
    layout = std::make_unique<QHBoxLayout>(video_frame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(widget.get());
    video_frame->setLayout(layout.get());
    //video_widget->resize(video_frame->width(), video_frame->height());
    video_frame->show();

    // Create our camera
    camera = video::make_camera(s.camera_name);

    start(QThread::HighPriority);

    return {};
}

void EasyTracker::data(double *data)
{
    if (ever_success.load(std::memory_order_relaxed))
    {
        // Get data back from tracker thread
        QMutexLocker l(&data_lock);
        data[Yaw] = iBestAngles[1];
        data[Pitch] = iBestAngles[0];
        data[Roll] = iBestAngles[2];
        data[TX] = iBestTranslation[0];
        data[TY] = iBestTranslation[1];
        data[TZ] = iBestTranslation[2];
    }
}

bool EasyTracker::center()
{
    QMutexLocker l(&center_lock);
    //TODO: Do we need to do anything there?
    return false;
}

int EasyTracker::get_n_points()
{
    return (int)point_count.load(std::memory_order_relaxed);
}

