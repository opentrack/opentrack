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
        std::vector<QString> cameras;
        cameras.push_back(KKinectIRSensor);
        return cameras;
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
    iFrame.width = 512;
    iFrame.height = 424;
    iFrame.stride = 0; // Auto step
    iFrame.channels = 3;
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

bool CameraKinectIr::start(const info& args)
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
    }


    if (SUCCEEDED(hr))
    {
        WaitForFirstFrame();
        bool success = iMatFrame.ptr() != nullptr;
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

    SafeRelease(iKinectSensor);

    // Free up our memory buffer if any
    iMatFrame = {};  
}

bool CameraKinectIr::get_frame_(cv::Mat& frame)
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
        INT64 nTime = 0;
        UINT nBufferSize = 0;
        UINT16 *pBuffer = NULL;

        hr = iInfraredFrame->get_RelativeTime(&nTime);

        if (first_frame)
        {
            IFrameDescription* frameDescription = NULL;

            if (SUCCEEDED(hr))
            {
                hr = iInfraredFrame->get_FrameDescription(&frameDescription);
            }

            // TODO: should not request those info for a every frame really
            if (SUCCEEDED(hr))
            {
                hr = frameDescription->get_Width(&width);
            }

            if (SUCCEEDED(hr))
            {
                hr = frameDescription->get_Height(&height);
            }

            if (SUCCEEDED(hr))
            {
                hr = frameDescription->get_DiagonalFieldOfView(&fov);
            }

            if (SUCCEEDED(hr))
                first_frame = false;

            SafeRelease(frameDescription);
        }
        
        if (SUCCEEDED(hr))
        {
            hr = iInfraredFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
        }

        if (SUCCEEDED(hr))
        {
            //ProcessInfrared(nTime, pBuffer, nWidth, nHeight);

            // Create an OpenCV matrix with our 16-bits IR buffer
            cv::Mat raw = cv::Mat(height, width, CV_16UC1, pBuffer, cv::Mat::AUTO_STEP);

            // Convert that OpenCV matrix to an RGB one as this is what is expected by our point extractor
            // TODO: Ideally we should implement a point extractors that works with our native buffer
            // First resample to 8-bits            
            double min = std::numeric_limits<uint16_t>::min();
            double max = std::numeric_limits<uint16_t>::max();
            //cv::minMaxLoc(raw, &min, &max); // Should we use 16bit min and max instead?
            // For scalling to have more precission in the range we are interrested in
            min = max - 255;            
            // See: https://stackoverflow.com/questions/14539498/change-type-of-mat-object-from-cv-32f-to-cv-8u/14539652
            raw.convertTo(raw8, CV_8U, 255.0 / (max - min), -255.0*min / (max - min));
            // Second convert to RGB
            cv::cvtColor(raw8, frame, cv::COLOR_GRAY2BGR);
            //
            success = true;
        }
    }


    return success;
}



}

#endif
