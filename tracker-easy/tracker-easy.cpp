/* Copyright (c) 2019 Stephane Lenclud
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "tracker-easy.h"
#include "video/video-widget.hpp"
#include "compat/math-imports.hpp"
#include "compat/check-visible.hpp"
#include "compat/sleep.hpp"
#include "point-extractor.h"
#include "cv/init.hpp"

#include <QHBoxLayout>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>

#include <opencv2/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>

#ifdef __GNUC__
#   pragma GCC diagnostic ignored "-Wsign-conversion"
#   pragma GCC diagnostic ignored "-Wfloat-conversion"

#endif

#ifdef __clang__
#   pragma clang diagnostic ignored "-Wimplicit-float-conversion"
#endif

using namespace options;

// Disable debug
#define dbgout if (true) {} else std::cout << "\n" <<std::chrono::system_clock::now().time_since_epoch().count() << ": "
//#define infout if (true) {} else std::cout
// Enable debug
//#define dbgout if (false) {} else std::cout
#define infout if (false) {} else std::cout << "\n" << std::chrono::system_clock::now().time_since_epoch().count() << ": "

// We need at least 3 vertices to be able to do anything
const int KMinVertexCount = 3;



namespace EasyTracker
{

    Tracker::Tracker() :
        iSettings{ KModuleName },
        iPreview{ preview_width, preview_height }
    {
        opencv_init();

        connect(&*iSettings.b, &bundle_::saving, this, &Tracker::CheckCamera, Qt::DirectConnection);
        connect(&*iSettings.b, &bundle_::reloading, this, &Tracker::CheckCamera, Qt::DirectConnection);

        iSettings.fov.connect_to(this, &Tracker::set_fov, Qt::DirectConnection);
        set_fov(iSettings.fov);
        // We could not get this working, nevermind
        //connect(&iSettings.cam_fps, value_::value_changed<int>(), this, &Tracker::SetFps, Qt::DirectConnection);

        // Make sure deadzones are updated whenever the settings are changed
        iSettings.DeadzoneRectHalfEdgeSize.connect_to(this, &Tracker::UpdateSettings, Qt::DirectConnection);

        // Update point extractor whenever some of the settings it needs are changed
        iSettings.iMinBlobSize.connect_to(this, &Tracker::UpdateSettings, Qt::DirectConnection);
        iSettings.iMaxBlobSize.connect_to(this, &Tracker::UpdateSettings, Qt::DirectConnection);

        // Make sure solver is updated whenever the settings are changed
        iSettings.PnpSolver.connect_to(this, &Tracker::UpdateSettings, Qt::DirectConnection);

        // Debug
        iSettings.debug.connect_to(this, &Tracker::UpdateSettings, Qt::DirectConnection);

        // Make sure model is updated whenever it is changed
        iSettings.iCustomModelThree.connect_to(this, &Tracker::UpdateModel, Qt::DirectConnection);
        iSettings.iCustomModelFour .connect_to(this, &Tracker::UpdateModel, Qt::DirectConnection);
        iSettings.iCustomModelFive .connect_to(this, &Tracker::UpdateModel, Qt::DirectConnection);

        // Update model logic
        #define UM(v) iSettings.v.connect_to(this, &Tracker::UpdateModel, Qt::DirectConnection)
        UM(iVertexTopX); UM(iVertexTopY); UM(iVertexTopZ);
        UM(iVertexTopRightX); UM(iVertexTopRightY); UM(iVertexTopRightZ);
        UM(iVertexTopLeftX); UM(iVertexTopLeftY); UM(iVertexTopLeftZ);
        UM(iVertexRightX); UM(iVertexRightY); UM(iVertexRightZ);
        UM(iVertexLeftX); UM(iVertexLeftY); UM(iVertexLeftZ);
        UM(iVertexCenterX); UM(iVertexCenterY); UM(iVertexCenterZ);

        UpdateModel();

        UpdateSettings();

        cv::namedWindow("Preview");
    }

    Tracker::~Tracker()
    {
        iThread.exit();
        iThread.wait();        

        if (iDebug)
            cv::destroyWindow("Preview");
        
        if (camera)
        {
            QMutexLocker l(&camera_mtx);
            camera->stop();
        }        
    }


    // Compute Euler angles from rotation matrix
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

    ///
    void Tracker::CreateCameraIntrinsicsMatrices()
    {
        // Create our camera matrix
        iCameraMatrix(0, 0) = iCameraInfo.fx;
        iCameraMatrix(1, 1) = iCameraInfo.fy;
        iCameraMatrix(0, 2) = iCameraInfo.P_x;
        iCameraMatrix(1, 2) = iCameraInfo.P_y;
        iCameraMatrix(2, 2) = 1;

        // Create distortion cooefficients
        iDistCoeffsMatrix = cv::Matx<double, 8, 1>::zeros();
        // As per OpenCV docs they should be thus: k1, k2, p1, p2, k3, k4, k5, k6
        // 0 - Radial first order
        // 1 - Radial second order
        // 2 - Tangential first order
        // 3 - Tangential second order
        // 4 - Radial third order
        // 5 - Radial fourth order
        // 6 - Radial fifth order
        // 7 - Radial sixth order
        //
        // SL: Using distortion coefficients in this way is breaking our face tracking output.
        // Just disable them for now until we invest time and effort to work it out.
        // For our face tracking use case not having proper distortion coefficients ain't a big deal anyway
        // See issues #1141 and #1020
        //for (unsigned k = 0; k < 8; k++)
        //    iDistCoeffsMatrix(k) = (double)iCameraInfo.dist_c[k];
    }


    void Tracker::MatchVertices(int& aTopIndex, int& aRightIndex, int& aLeftIndex, int& aCenterIndex, int& aTopRight, int& aTopLeft)
    {
        if (iModel.size() == 5)
        {
            MatchFiveVertices(aTopIndex, aRightIndex, aLeftIndex, aTopRight, aTopLeft);
        }
        else if (!iSettings.iClipModelThree)
        {
            MatchThreeOrFourVertices(aTopIndex, aRightIndex, aLeftIndex, aCenterIndex);
        }
        else
        {
            // Clip model
            MatchClipVertices(aTopIndex, aRightIndex, aLeftIndex);
        }
    }


    void Tracker::MatchFiveVertices(int& aTopIndex, int& aRightIndex, int& aLeftIndex, int& aTopRight, int& aTopLeft)
    {
        //Bitmap origin is top left
        iTrackedPoints.clear();

        int vertexIndices[] = { -1,-1,-1,-1,-1 };
        std::vector<int> indices = { 0,1,2,3,4 };

        // Tracked points must match the order of the object model points.
        // Find top most point, that's the one with min Y as we assume our guy's head is not up side down
        int minY = std::numeric_limits<int>::max();
        for (int i = 0; i < (int)iPoints.size(); i++)
        {
            if (iPoints[i].y < minY)
            {
                minY = iPoints[i].y;
                vertexIndices[VertexPosition::Top] = i;                
            }
        }
        indices.erase(std::find(indices.begin(), indices.end(), vertexIndices[VertexPosition::Top]));

        // Find right most point 
        int maxX = 0;
        for (int i = 0; i < (int)iPoints.size(); i++)
        {
            // Excluding top most point
            if (i != vertexIndices[VertexPosition::Top] && iPoints[i].x > maxX)
            {
                maxX = iPoints[i].x;
                vertexIndices[VertexPosition::Right] = i;
            }
        }
        indices.erase(std::find(indices.begin(), indices.end(), vertexIndices[VertexPosition::Right]));

        // Find left most point
        int minX = std::numeric_limits<int>::max();
        for (int i = 0; i < (int)iPoints.size(); i++)
        {
            // Excluding top most point and right most point
            if (i != vertexIndices[VertexPosition::Top] && i != vertexIndices[VertexPosition::Right] && iPoints[i].x < minX)
            {
                minX = iPoints[i].x;
                vertexIndices[VertexPosition::Left] = i;
            }
        }
        indices.erase(std::find(indices.begin(), indices.end(), vertexIndices[VertexPosition::Left]));

        // Check which of our two remaining points is on the left
        int leftIndex = -1;
        int rightIndex = -1;
        if (iPoints[indices[0]].x > iPoints[indices[1]].x)
        {
            leftIndex = indices[1];
            rightIndex = indices[0];
        }
        else
        {
            leftIndex = indices[0];
            rightIndex = indices[1];
        }

        // Check which of the left points is at the top
        if (iPoints[vertexIndices[VertexPosition::Left]].y < iPoints[leftIndex].y)
        {
            vertexIndices[VertexPosition::TopLeft] = vertexIndices[VertexPosition::Left];
            vertexIndices[VertexPosition::Left] = leftIndex;
        }
        else
        {
            vertexIndices[VertexPosition::TopLeft] = leftIndex;
        }

        // Check which of the right points is at the top
        if (iPoints[vertexIndices[VertexPosition::Right]].y < iPoints[rightIndex].y)
        {
            vertexIndices[VertexPosition::TopRight] = vertexIndices[VertexPosition::Right];
            vertexIndices[VertexPosition::Right] = rightIndex;
        }
        else
        {
            vertexIndices[VertexPosition::TopRight] = rightIndex;
        }


        // Order matters, see UpdateModel function 
        iTrackedPoints.push_back(iPoints[vertexIndices[VertexPosition::Top]]);
        iTrackedPoints.push_back(iPoints[vertexIndices[VertexPosition::Right]]);
        iTrackedPoints.push_back(iPoints[vertexIndices[VertexPosition::Left]]);
        iTrackedPoints.push_back(iPoints[vertexIndices[VertexPosition::TopRight]]);
        iTrackedPoints.push_back(iPoints[vertexIndices[VertexPosition::TopLeft]]);

        //
        aTopIndex = vertexIndices[VertexPosition::Top];
        aRightIndex = vertexIndices[VertexPosition::Right];
        aLeftIndex = vertexIndices[VertexPosition::Left];
        aTopRight = vertexIndices[VertexPosition::TopRight];
        aTopLeft = vertexIndices[VertexPosition::TopLeft];


    }


    void Tracker::MatchThreeOrFourVertices(int& aTopIndex, int& aRightIndex, int& aLeftIndex, int& aCenterIndex)
    {
        //Bitmap origin is top left
        iTrackedPoints.clear();
        // Tracked points must match the order of the object model points.
        // Find top most point, that's the one with min Y as we assume our guy's head is not up side down
        int minY = std::numeric_limits<int>::max();
        for (int i = 0; i < (int)iPoints.size(); i++)
        {
            if (iPoints[i].y < minY)
            {
                minY = iPoints[i].y;
                aTopIndex = i;
            }
        }


        int maxX = 0;

        // Find right most point 
        for (int i = 0; i < (int)iPoints.size(); i++)
        {
            // Excluding top most point
            if (i != aTopIndex && iPoints[i].x > maxX)
            {
                maxX = iPoints[i].x;
                aRightIndex = i;
            }
        }

        // Find left most point
        int minX = std::numeric_limits<int>::max();
        for (int i = 0; i < (int)iPoints.size(); i++)
        {
            // Excluding top most point and right most point
            if (i != aTopIndex && i != aRightIndex && iPoints[i].x < minX)
            {
                aLeftIndex = i;
                minX = iPoints[i].x;
            }
        }

        // Find center point, the last one
        for (int i = 0; i < (int)iPoints.size(); i++)
        {
            // Excluding the three points we already have
            if (i != aTopIndex && i != aRightIndex && i != aLeftIndex)
            {
                aCenterIndex = i;
            }
        }

        // Order matters
        iTrackedPoints.push_back(iPoints[aTopIndex]);
        iTrackedPoints.push_back(iPoints[aRightIndex]);
        iTrackedPoints.push_back(iPoints[aLeftIndex]);
        if (iModel.size() > iTrackedPoints.size())
        {
            // We are tracking more than 3 points
            iTrackedPoints.push_back(iPoints[aCenterIndex]);
        }
    }

    /**
    */
    void Tracker::MatchClipVertices(int& aTopIndex, int& aMiddleIndex, int& aBottomIndex)
    {
        //Bitmap origin is top left
        iTrackedPoints.clear();
        // Tracked points must match the order of the object model points.
        // Find top most point, that's the one with min Y as we assume our guy's head is not up side down
        int minY = std::numeric_limits<int>::max();
        for (int i = 0; i < (int)iPoints.size(); i++)
        {
            if (iPoints[i].y < minY)
            {
                minY = iPoints[i].y;
                aTopIndex = i;
            }
        }


        int maxY = 0;

        // Find bottom most point 
        for (int i = 0; i < (int)iPoints.size(); i++)
        {
            // Excluding top most point
            if (i != aTopIndex && iPoints[i].y > maxY)
            {
                maxY = iPoints[i].y;
                aBottomIndex = i;
            }
        }


        // Find center point, the last one
        for (int i = 0; i < (int)iPoints.size(); i++)
        {
            // Excluding the three points we already have
            if (i != aTopIndex && i != aBottomIndex)
            {
                aMiddleIndex = i;
            }
        }

        // Order matters
        iTrackedPoints.push_back(iPoints[aTopIndex]);
        iTrackedPoints.push_back(iPoints[aMiddleIndex]);
        iTrackedPoints.push_back(iPoints[aBottomIndex]);
    }



    ///
    ///
    ///
    void Tracker::ProcessFrame()
    {
        //infout << "ProcessFrame - begin";
        QMutexLocker l(&iProcessLock);

        // Create OpenCV matrix from our frame
        // TODO: Assert channel size is one or two
        iMatFrame = cv::Mat(iFrame.height, iFrame.width, CV_MAKETYPE((iFrame.channel_size == 2 ? CV_16U : CV_8U), iFrame.channels), iFrame.data, iFrame.stride);
        iFrameCount++;

        bool doPreview = check_is_visible();
        if (doPreview)
        {
            iPreview = iMatFrame;
        }

        iPoints.clear();

        // Do not attempt point extraction on a color buffer as it is running real slow and is useless anyway.
        // If we are ever going to support color buffer we will need another implementation.
        if (iFrame.channels == 1)
        {            
            iPointExtractor.ExtractPoints(iMatFrame, (doPreview ? &iPreview.iFrameRgb : nullptr), (int)iModel.size(), iPoints);
        }
        

        const bool success = iPoints.size() >= iModel.size() && iModel.size() >= KMinVertexCount;

        int topPointIndex = -1;
        int rightPointIndex = -1;
        int leftPointIndex = -1;
        int centerPointIndex = -1;
        int topRightPointIndex = -1;
        int topLeftPointIndex = -1;

        if (success)
        {
            // Lets match our 3D vertices with our image 2D points
            MatchVertices(topPointIndex, rightPointIndex, leftPointIndex, centerPointIndex, topRightPointIndex, topLeftPointIndex);

            bool movedEnough = true;
            // Check if we moved enough since last time we were here
            // This is our deadzone management
            if (iDeadzoneHalfEdge != 0 // Check if deazones are enabled
                && iTrackedRects.size() == iTrackedPoints.size())
            {
                movedEnough = false;
                for (size_t i = 0; i < iTrackedPoints.size(); i++)
                {
                    if (!iTrackedRects[i].contains(iTrackedPoints[i]))
                    {
                        movedEnough = true;
                        break;
                    }
                }
            }

            if (!movedEnough)
            {
                // We are in a dead zone
                // However we still have tracking so make sure we don't auto center
                QMutexLocker lock(&iDataLock);
                iBestTime.start();
            }
            else
            {
                // Build deadzone rectangles if needed
                iTrackedRects.clear();
                if (iDeadzoneHalfEdge != 0) // Check if deazones are enabled
                {
                    for (const cv::Point2f& pt : iTrackedPoints)
                    {
                        cv::Rect rect(pt - cv::Point2f(iDeadzoneHalfEdge, iDeadzoneHalfEdge), cv::Size(iDeadzoneEdge, iDeadzoneEdge));
                        iTrackedRects.push_back(rect);
                    }
                }

                dbgout << "Object: " << iModel << "\n";
                dbgout << "Points: " << iTrackedPoints << "\n";

                iAngles.clear();
                iBestSolutionIndex = -1;
                // Solve P3P problem with OpenCV
                int solutionCount = 0;
                if (iModel.size() == 3)
                {
                    solutionCount = cv::solveP3P(iModel, iTrackedPoints, iCameraMatrix, iDistCoeffsMatrix, iRotations, iTranslations, iSolver);
                }
                else
                {
                    //Guess extrinsic boolean is only for ITERATIVE method, it will be set to false for all other method
                    cv::Mat rotation, translation;
                    // Init only needed for iterative, it's also useless as it is
                    rotation = cv::Mat::zeros(3, 1, CV_64FC1);
                    translation = cv::Mat::zeros(3, 1, CV_64FC1);
                    rotation.setTo(cv::Scalar(0));
                    translation.setTo(cv::Scalar(0));
                    /////
                    iRotations.clear();
                    iTranslations.clear();
                    bool solved = cv::solvePnP(iModel, iTrackedPoints, iCameraMatrix, iDistCoeffsMatrix, rotation, translation, true, iSolver );
                    if (solved)
                    {
                        solutionCount = 1;
                        iRotations.push_back(rotation);
                        iTranslations.push_back(translation);
                    }
                }

                // Reset best solution index
                iBestSolutionIndex = -1;

                if (solutionCount > 0)
                {
                    dbgout << "Solution count: " << solutionCount << "\n";
                    int minPitch = std::numeric_limits<int>::max();
                    // Find the solution we want amongst all possible ones
                    for (int i = 0; i < solutionCount; i++)
                    {
                        dbgout << "Translation:\n";
                        dbgout << iTranslations.at(i);
                        dbgout << "\n";
                        dbgout << "Rotation:\n";
                        //dbgout << rvecs.at(i);
                        cv::Mat rotationCameraMatrix;
                        cv::Rodrigues(iRotations[i], rotationCameraMatrix);
                        cv::Vec3d angles;
                        getEulerAngles(rotationCameraMatrix, angles);
                        iAngles.push_back(angles);

                        // Check if pitch is closest to zero
                        int absolutePitch = (int)std::abs(angles[0]);
                        if (minPitch > absolutePitch)
                        {
                            // The solution with pitch closest to zero is the one we want
                            minPitch = absolutePitch;
                            iBestSolutionIndex = i;
                        }

                        dbgout << angles;
                        dbgout << "\n";
                    }

                    dbgout << "\n";
                }

                if (iBestSolutionIndex != -1)
                {
                    // Best translation
                    cv::Vec3d translation = iTranslations[iBestSolutionIndex];
                    // Best angles
                    cv::Vec3d angles = iAngles[iBestSolutionIndex];

                    // Pass solution through our kalman filter
                    iKf.Update(translation[0], translation[1], translation[2], angles[2], angles[0], angles[1]);

                    // Check if our solution makes sense
                    // For now, just discard solutions with extrem pitch 
                    if (std::abs(angles[0]) > 50) //TODO: Put that in settings
                    {
                        infout << "WARNING: discarding solution!";
                        iBadSolutionCount++;
                    }
                    else
                    {
                        iGoodSolutionCount++;
                        // We succeded in finding a solution to our PNP problem
                        ever_success.store(true, std::memory_order_relaxed);

                        // Send solution data back to main thread
                        QMutexLocker l2(&iDataLock);
                        iBestAngles = angles;
                        iBestTranslation = translation;
                        iBestTime.start();
                    }

                }
            }
        }

        if (doPreview)
        {
            double qualityIndex = 1 - (iGoodSolutionCount!=0?(double)iBadSolutionCount / (double)iGoodSolutionCount:0);
            std::ostringstream ss;
            ss << "FPS: " << iFps << "/" << iSkippedFps << "   QI: " << qualityIndex;
            iPreview.DrawInfo(ss.str());

            //Color is BGR
            if (topPointIndex != -1)
            {
                // Render a cross to indicate which point is the head
                static const cv::Scalar color(0, 255, 255); // Yellow
                iPreview.DrawCross(iPoints[topPointIndex],color);
            }

            if (rightPointIndex != -1)
            {
                static const cv::Scalar color(255, 0, 255); // Pink
                iPreview.DrawCross(iPoints[rightPointIndex], color);
            }

            if (leftPointIndex != -1)
            {
                static const cv::Scalar color(255, 0, 0); // Blue                
                iPreview.DrawCross(iPoints[leftPointIndex], color);
            }

            if (centerPointIndex != -1)
            {
                static const cv::Scalar color(0, 255, 0); // Green
                iPreview.DrawCross(iPoints[centerPointIndex], color);
            }

            if (topRightPointIndex != -1)
            {
                static const cv::Scalar color(0, 0, 255); // Red
                iPreview.DrawCross(iPoints[topRightPointIndex], color);
            }

            if (topLeftPointIndex != -1)
            {
                static const cv::Scalar color(255, 255, 0); // Cyan
                iPreview.DrawCross(iPoints[topLeftPointIndex], color);
            }


            // Render our deadzone rects
            for (const cv::Rect& rect : iTrackedRects)
            {
                cv::rectangle(iPreview.iFrameRgb,rect,cv::Scalar(255,0,0));
            }

            // Show full size preview pop-up
            if (iDebug)
            {
                cv::imshow("Preview", iPreview.iFrameRgb);
                cv::waitKey(1);
            }

            // Update preview widget
            widget->update_image(iPreview.get_bitmap());

            auto[w, h] = widget->preview_size();
            if (w != preview_width || h != preview_height)
            {
                // Resize preivew if widget size has changed
                preview_width = w; preview_height = h;
                iPreview = Preview(w, h);
            }
        }
        else
        {
            // No preview, destroy preview pop-up
            if (iDebug)
            {
                cv::destroyWindow("Preview");
            }
        }

        dbgout << "Frame time:" << iTimer.elapsed_seconds();
        //infout << "ProcessFrame - end";
    }

    ///
    ///
    ///
    void Tracker::Tick()
    {
        if (CheckCamera())
        {
            // Camera was just started, skipping that frame as it was causing a deadlock on our process mutex in ProcessFrame
            // In fact it looked like ProcessFrame was called twice without completing.
            // That has something to do with the ticker interval being changed after the camera is started.
            return;
        }
      
        iTimer.start();

        bool new_frame = false;
        {
            QMutexLocker l(&camera_mtx);

            if (camera)
            {
                std::tie(iFrame, new_frame) = camera->get_frame();
            }
                    
        }

        if (new_frame)
        {
            ProcessFrame();
        }
        else
        {
            iSkippedFrameCount++;
        }

        // Compute FPS
        double elapsed = iFpsTimer.elapsed_seconds();
        if (elapsed >= 1.0)
        {
            iFps = iFrameCount / elapsed;
            iSkippedFps = iSkippedFrameCount / elapsed;
            iFrameCount = 0;
            iSkippedFrameCount = 0;
            iFpsTimer.start();
        }
        
    }

    /// @return True if camera was just started, false otherwise.
    bool Tracker::CheckCamera()
    {
        QMutexLocker l(&camera_mtx);

        if (camera->is_open())
        {
            return false;
        }

        iCameraInfo.fps = iSettings.cam_fps;
        iCameraInfo.width = iSettings.cam_res_x;
        iCameraInfo.height = iSettings.cam_res_y;

        bool res = camera->start(iCameraInfo);
        //portable::sleep(5000);

        // We got our camera intrinsics, create corresponding matrices
        CreateCameraIntrinsicsMatrices();

        // If ever the camera implementation provided an FPS now is the time to apply it
        DoSetFps(iCameraInfo.fps);

        return res;
    }

    void Tracker::set_fov(int value)
    {
        (void)value;
        //QMutexLocker l(&camera_mtx);

    }

    // Calling this from another thread than the one it belongs too after it's started somehow breaks our timer
    void Tracker::SetFps(int aFps)
    {
        QMutexLocker l(&camera_mtx);
        DoSetFps(aFps);
    }

    void Tracker::DoSetFps(int aFps)
    {
        // Aplly FPS to timer
        iTicker.setInterval(1000 / aFps + 1);

        // Reset Kalman filter
        //int nStates = 18;            // the number of states
        //int nMeasurements = 6;       // the number of measured states
        //int nInputs = 0;             // the number of control actions
        //double dt = 0.125;           // time between measurements (1/FPS)
        double dt = 1000.0 / aFps;
        iKf.Init(18, 6, 0, dt);
    }


    ///
    /// Create our model from settings specifications
    ///
    void Tracker::UpdateModel()
    {
        infout << "Update model - begin";

        QMutexLocker lock(&iProcessLock);
        // Construct the points defining the object we want to detect based on settings.
        // We are converting them from millimeters to centimeters.
        // TODO: Need to support clip too. That's cap only for now.
        iModel.clear();

        if (!iSettings.iClipModelThree)
        {
            iModel.push_back(cv::Point3f(iSettings.iVertexTopX / 10.0, iSettings.iVertexTopY / 10.0, iSettings.iVertexTopZ / 10.0)); // Top
            iModel.push_back(cv::Point3f(iSettings.iVertexRightX / 10.0, iSettings.iVertexRightY / 10.0, iSettings.iVertexRightZ / 10.0)); // Right
            iModel.push_back(cv::Point3f(iSettings.iVertexLeftX / 10.0, iSettings.iVertexLeftY / 10.0, iSettings.iVertexLeftZ / 10.0)); // Left

            if (iSettings.iCustomModelFour)
            {
                iModel.push_back(cv::Point3f(iSettings.iVertexCenterX / 10.0, iSettings.iVertexCenterY / 10.0, iSettings.iVertexCenterZ / 10.0)); // Center
            }
            else if (iSettings.iCustomModelFive)
            {
                iModel.push_back(cv::Point3f(iSettings.iVertexTopRightX / 10.0, iSettings.iVertexTopRightY / 10.0, iSettings.iVertexTopRightZ / 10.0)); // Top Right
                iModel.push_back(cv::Point3f(iSettings.iVertexTopLeftX / 10.0, iSettings.iVertexTopLeftY / 10.0, iSettings.iVertexTopLeftZ / 10.0)); // Top Left
            }
        }
        else
        {
            // Clip model type
            iModel.push_back(cv::Point3f(iSettings.iVertexClipTopX / 10.0, iSettings.iVertexClipTopY / 10.0, iSettings.iVertexClipTopZ / 10.0)); // Top
            iModel.push_back(cv::Point3f(iSettings.iVertexClipMiddleX / 10.0, iSettings.iVertexClipMiddleY / 10.0, iSettings.iVertexClipMiddleZ / 10.0)); // Middle
            iModel.push_back(cv::Point3f(iSettings.iVertexClipBottomX / 10.0, iSettings.iVertexClipBottomY / 10.0, iSettings.iVertexClipBottomZ / 10.0)); // Bottom
        }

        infout << "Update model - end";
    }

    ///
    /// Take a copy of the settings needed by our thread to avoid deadlocks
    ///
    void Tracker::UpdateSettings()
    {
        infout << "Update Setting - begin";
        QMutexLocker l(&iProcessLock);
        iPointExtractor.UpdateSettings();
        iSolver = iSettings.PnpSolver;
        iDeadzoneHalfEdge = iSettings.DeadzoneRectHalfEdgeSize;
        iDeadzoneEdge = iDeadzoneHalfEdge * 2;
        iTrackedRects.clear();
        iDebug = iSettings.debug;
        infout << "Update Setting - end";
    }

    ///
    module_status Tracker::start_tracker(QFrame* video_frame)
    {
        // Check that we support that solver
        if (iSolver!=cv::SOLVEPNP_P3P && iSolver != cv::SOLVEPNP_AP3P && iModel.size()==3)
        {
            return module_status("Error: Solver not supported use either P3P or AP3P.");
        }

        // Create our camera
        camera = video::make_camera(iSettings.camera_name);

        if (!camera)
            return error(QStringLiteral("Can't open camera %1").arg(iSettings.camera_name));

        //video_frame->setAttribute(Qt::WA_NativeWindow);
        widget = std::make_unique<video_widget>(video_frame);
        layout = std::make_unique<QHBoxLayout>(video_frame);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(&*widget);
        video_frame->setLayout(&*layout);
        //video_widget->resize(video_frame->width(), video_frame->height());
        video_frame->show();

        // Precise timer is needed otherwise the interval is not really respected
        iTicker.setTimerType(Qt::PreciseTimer);
        SetFps(iSettings.cam_fps);
        iTicker.moveToThread(&iThread);
        // Connect timer timeout signal to our tick slot
        connect(&iTicker, SIGNAL(timeout()), SLOT(Tick()), Qt::DirectConnection);
        // Start our timer once our thread is started
        iTicker.connect(&iThread, SIGNAL(started()), SLOT(start()));
        iFpsTimer.start(); // Kick off our FPS counter
        iThread.setObjectName("EasyTrackerThread");
        iThread.setPriority(QThread::HighPriority); // Do we really want that?
        iThread.start();

        return {};
    }

    //
    void FeedData(double* aData, const cv::Vec3d& aAngles, const cv::Vec3d& aTranslation)
    {
        aData[Yaw] = aAngles[1];
        aData[Pitch] = aAngles[0];
        aData[Roll] = aAngles[2];
        aData[TX] = aTranslation[0];
        aData[TY] = aTranslation[1];
        aData[TZ] = aTranslation[2];
    }

    //
    // That's called around 250 times per second.
    // Therefore we better not do anything here other than provide current data.
    //
    void Tracker::data(double* aData)
    {
        if (ever_success.load(std::memory_order_relaxed))
        {
            // Get data back from tracker thread
            QMutexLocker l(&iDataLock);
            // If there was no new data recently then we provide center data.
            // Basically, if our user remove her hat, we will go back to center position until she puts it back on.
            if (iSettings.iAutoCenter && iBestTime.elapsed_ms() > iSettings.iAutoCenterTimeout)
            {
                // Reset to center until we get new data
                FeedData(aData, iCenterAngles, iCenterTranslation);
            }
            else
            {
                // We got valid data, provide it
                FeedData(aData, iBestAngles, iBestTranslation);
            }
        }
    }

    bool Tracker::center()
    {
        QMutexLocker l(&iDataLock);
        iCenterTranslation = iBestTranslation;
        iCenterAngles = iBestAngles;
        // Returning false tells the pipeline we want to use the default center behaviour
        return false;
    }


}
