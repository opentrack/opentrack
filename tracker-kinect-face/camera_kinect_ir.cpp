/* Copyright (c) 2019, Stephane Lenclud <github@lenclud.com>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "camera_kinect_ir.h"

#ifdef OTR_HAVE_OPENCV

//#include "frame.hpp"

#include "compat/sleep.hpp"
#include "compat/math-imports.hpp"

#include <opencv2/imgproc.hpp>
#include <cstdlib>

namespace Kinect {

    static const char KKinectIRSensor[] = "Kinect V2 IR Sensor";

    CamerasProvider::CamerasProvider() = default;

    std::unique_ptr<video::impl::camera> CamerasProvider::make_camera(const QString& name)
    {
        if (name.compare(KKinectIRSensor) == 0)
        {
            return std::make_unique<CameraKinectIr>();
        }

        return nullptr;
    }

    std::vector<QString> CamerasProvider::camera_names() const
    {
        return { KKinectIRSensor };
    }

    bool CamerasProvider::can_show_dialog(const QString& camera_name)
    {
        return false;
    }

    bool CamerasProvider::show_dialog(const QString& camera_name)
    {
        return false;
    }

// Register our camera provider thus making sure Point Tracker can use Kinect V2 IR Sensor
OTR_REGISTER_CAMERA(CamerasProvider)


CameraKinectIr::CameraKinectIr() 
{
}


CameraKinectIr::~CameraKinectIr()
{
    stop();
}

bool CameraKinectIr::show_dialog()
{
    return false;
}

bool CameraKinectIr::is_open()
{
    return iInfraredFrameReader!=nullptr;
}

///
/// Wait until we get a first frame
///
void CameraKinectIr::WaitForFirstFrame()
{
    bool new_frame = false;
    int attempts = 200; // Kinect cold start can take a while
    while (!new_frame && attempts>0)
    {
        new_frame = get_frame_(iMatFrame);
        portable::sleep(100);
        --attempts;
    }
}



std::tuple<const video::impl::frame&, bool> CameraKinectIr::get_frame()
{
    bool new_frame = false;
    new_frame = get_frame_(iMatFrame);
        
    iFrame.data = iMatFrame.ptr();
    iFrame.width = iWidth;
    iFrame.height = iHeight;
    iFrame.stride = cv::Mat::AUTO_STEP;
    iFrame.channels = iMatFrame.channels();
    iFrame.channelSize = iMatFrame.elemSize1();
    return { iFrame, new_frame };
}

// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
    if (pInterfaceToRelease != NULL)
    {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = NULL;
    }
}

///
///
///
bool CameraKinectIr::start(info& aInfo)
{
    stop();

    HRESULT hr;

    // Get and open Kinect sensor
    hr = GetDefaultKinectSensor(&iKinectSensor);
    if (SUCCEEDED(hr))
    {
        hr = iKinectSensor->Open();
    }

    // Create infrared frame reader	
    if (SUCCEEDED(hr))
    {
        // Initialize the Kinect and get the infrared reader
        IInfraredFrameSource* pInfraredFrameSource = NULL;

        hr = iKinectSensor->Open();

        if (SUCCEEDED(hr))
        {
            hr = iKinectSensor->get_InfraredFrameSource(&pInfraredFrameSource);
        }

        if (SUCCEEDED(hr))
        {
            hr = pInfraredFrameSource->OpenReader(&iInfraredFrameReader);
        }

        SafeRelease(pInfraredFrameSource);

        if (SUCCEEDED(hr))
        {
            iKinectSensor->get_CoordinateMapper(&iCoordinateMapper);
        }
    }


    if (SUCCEEDED(hr))
    {
        WaitForFirstFrame();
        bool success = iMatFrame.ptr() != nullptr;
        if (success)
        {
            // Provide frame info
            aInfo.width = iWidth;
            aInfo.height = iHeight;

            CameraIntrinsics intrinsics;
            hr = iCoordinateMapper->GetDepthCameraIntrinsics(&intrinsics);
            if (SUCCEEDED(hr))
            {
                aInfo.focalLengthX = intrinsics.FocalLengthX;
                aInfo.focalLengthY = intrinsics.FocalLengthY;
                aInfo.principalPointX = intrinsics.PrincipalPointX;
                aInfo.principalPointY = intrinsics.PrincipalPointY;
                aInfo.radialDistortionFourthOrder = intrinsics.RadialDistortionFourthOrder;
                aInfo.radialDistortionSecondOrder = intrinsics.RadialDistortionSecondOrder;
                aInfo.radialDistortionSixthOrder = intrinsics.RadialDistortionSixthOrder;                
            }

        }

        return success;
    }

    stop();
    return false;
}

void CameraKinectIr::stop()
{
    // done with infrared frame reader
    SafeRelease(iInfraredFrame);
    SafeRelease(iInfraredFrameReader);

    // close the Kinect Sensor
    if (iKinectSensor)
    {
        iKinectSensor->Close();
    }

    SafeRelease(iCoordinateMapper);
    SafeRelease(iKinectSensor);

    // Free up our memory buffer if any
    iMatFrame = cv::Mat();
}

bool CameraKinectIr::get_frame_(cv::Mat& aFrame)
{

    if (!iInfraredFrameReader)
    {
        return false;
    }

    bool success = false;

    // Release previous frame if any
    SafeRelease(iInfraredFrame);

    HRESULT hr = iInfraredFrameReader->AcquireLatestFrame(&iInfraredFrame);

    if (SUCCEEDED(hr))
    {
        if (iFirstFrame)
        {
            IFrameDescription* frameDescription = NULL;

            if (SUCCEEDED(hr))
            {
                hr = iInfraredFrame->get_FrameDescription(&frameDescription);
            }

            if (SUCCEEDED(hr))
            {
                hr = frameDescription->get_Width(&iWidth);
            }

            if (SUCCEEDED(hr))
            {
                hr = frameDescription->get_Height(&iHeight);
            }

            if (SUCCEEDED(hr))
            {
                hr = frameDescription->get_DiagonalFieldOfView(&iFov);
            }

            if (SUCCEEDED(hr))
            {
                iFirstFrame = false;
            }
                
            SafeRelease(frameDescription);
        }


        UINT nBufferSize = 0;
        UINT16 *pBuffer = NULL;

        if (SUCCEEDED(hr))
        {
            hr = iInfraredFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
        }

        if (SUCCEEDED(hr))
        {
            // Create an OpenCV matrix with our 16-bits IR buffer
            aFrame = cv::Mat(iHeight, iWidth, CV_16UC1, pBuffer, cv::Mat::AUTO_STEP);
            // Any processing of the frame is left to the user
            success = true;
        }
    }


    return success;
}



}

#endif
