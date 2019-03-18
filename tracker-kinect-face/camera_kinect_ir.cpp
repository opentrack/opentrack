/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "camera_kinect_ir.h"

#if __has_include(<opencv2/core.hpp>)

//#include "frame.hpp"

#include "compat/sleep.hpp"
//#include "compat/camera-names.hpp"
#include "compat/math-imports.hpp"

#include <opencv2/imgproc.hpp>

//#include "cv/video-property-page.hpp"

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


OTR_REGISTER_CAMERA(CamerasProvider)


CameraKinectIr::CameraKinectIr() 
{
}



bool CameraKinectIr::show_dialog()
{
    return false;
}

bool CameraKinectIr::is_open()
{
    return iInfraredFrameReader!=nullptr;
}




std::tuple<const video::impl::frame&, bool> CameraKinectIr::get_frame()
{
    

    bool new_frame = false;
    while (!new_frame)
    {
        new_frame = get_frame_(iMatFrame);
    }
        
    iFrame.data = iMatFrame.ptr();
    iFrame.width = 512;
    iFrame.height = 424;
    iFrame.stride = 0; // Auto step
    iFrame.channels = 3;
    return { iFrame, true };
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
        return true;
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
        IFrameDescription* frameDescription = NULL;
        int nWidth = 0;
        int nHeight = 0;
        float diagonalFieldOfView = 0.0f;
        UINT nBufferSize = 0;
        UINT16 *pBuffer = NULL;

        hr = iInfraredFrame->get_RelativeTime(&nTime);

        if (SUCCEEDED(hr))
        {
            hr = iInfraredFrame->get_FrameDescription(&frameDescription);
        }

        if (SUCCEEDED(hr))
        {
            hr = frameDescription->get_Width(&nWidth);
        }

        if (SUCCEEDED(hr))
        {
            hr = frameDescription->get_Height(&nHeight);
        }

        if (SUCCEEDED(hr))
        {
            hr = frameDescription->get_DiagonalFieldOfView(&diagonalFieldOfView);
        }
        
        if (SUCCEEDED(hr))
        {
            hr = iInfraredFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
        }

        if (SUCCEEDED(hr))
        {
            //ProcessInfrared(nTime, pBuffer, nWidth, nHeight);

            // Create an OpenCV matrix with our 16-bits IR buffer
            cv::Mat raw = cv::Mat(nHeight, nWidth, CV_16UC1, pBuffer); // .clone();

            // Convert that OpenCV matrix to an RGB one as this is what is expected by our point extractor
            // TODO: Ideally we should implement a point extractors that works with our native buffer
            // First resample to 8-bits
            double min = std::numeric_limits<uint16_t>::min();
            double max = std::numeric_limits<uint16_t>::max();
            // For scalling to have more precission in the range we are interrested in
            min = max - 255;
            //cv::minMaxLoc(raw, &min, &max); // Should we use 16bit min and max instead?

            cv::Mat raw8;
            raw.convertTo(raw8, CV_8U, 255.0 / (max - min), -255.0*min / (max - min));
            // Second convert to RGB
            cv::cvtColor(raw8, frame, cv::COLOR_GRAY2BGR);
            int newType = frame.type();


            success = true;
        }

        SafeRelease(frameDescription);
    }


    return success;
}



} // ns pt_module

#endif
