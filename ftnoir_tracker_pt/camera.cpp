/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

 #if defined(OPENTRACK_API) && defined(_WIN32)
#include <windows.h>
#include <dshow.h>
#endif
 
#include "camera.h"
#include <string>
#include <QDebug>

using namespace cv;

#if defined(OPENTRACK_API) && (defined(__unix) || defined(__linux) || defined(__APPLE__))
#include <unistd.h>
#endif

#ifdef OPENTRACK_API
void get_camera_device_names(std::vector<std::string>& device_names) {
#   if defined(_WIN32)
    // Create the System Device Enumerator.
    HRESULT hr;
    ICreateDevEnum *pSysDevEnum = NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
    if (FAILED(hr))
    {
        return;
    }
    // Obtain a class enumerator for the video compressor category.
    IEnumMoniker *pEnumCat = NULL;
    hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

    if (hr == S_OK) {
        // Enumerate the monikers.
        IMoniker *pMoniker = NULL;
        ULONG cFetched;
        while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) {
            IPropertyBag *pPropBag;
            hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
            if (SUCCEEDED(hr))	{
                // To retrieve the filter's friendly name, do the following:
                VARIANT varName;
                VariantInit(&varName);
                hr = pPropBag->Read(L"FriendlyName", &varName, 0);
                if (SUCCEEDED(hr))
                {
					auto wstr = std::wstring(varName.bstrVal);
					auto str = std::string(wstr.begin(), wstr.end());
                    device_names.push_back(str);
                }
                VariantClear(&varName);

                ////// To create an instance of the filter, do the following:
                ////IBaseFilter *pFilter;
                ////hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
                ////	(void**)&pFilter);
                // Now add the filter to the graph.
                //Remember to release pFilter later.
                pPropBag->Release();
            }
            pMoniker->Release();
        }
        pEnumCat->Release();
    }
    pSysDevEnum->Release();
#   else
    for (int i = 0; i < 16; i++) {
        char buf[128];
        sprintf(buf, "/dev/video%d", i);
        if (access(buf, R_OK | W_OK) == 0) {
            device_names.push_back(std::string(buf));
        }
    }
#   endif
}
#else
// ----------------------------------------------------------------------------
void get_camera_device_names(std::vector<std::string>& device_names)
{
	videoInput VI;
	VI.listDevices();
	std::string device_name;
	for(int index = 0; ; ++index) {
		device_name = VI.getDeviceName(index);
		if (device_name.empty()) break;
		device_names.push_back(device_name);
	}
}
#endif

// ----------------------------------------------------------------------------
void Camera::set_device_index(int index)
{
	if (desired_index != index)
	{
		desired_index = index;
		_set_device_index();

		// reset fps
		dt_valid = 0;
		dt_mean = 0;
		active_index = index;
	}
}

void Camera::set_fps(int fps)
{
	if (cam_desired.fps != fps)
	{
		cam_desired.fps = fps;
		_set_fps();
	}
}

void Camera::set_res(int x_res, int y_res)
{
	if (cam_desired.res_x != x_res || cam_desired.res_y != y_res)
	{
		cam_desired.res_x = x_res;
		cam_desired.res_y = y_res;
		_set_res();
        _set_fps();
	}
}

bool Camera::get_frame(float dt, cv::Mat* frame)
{
	bool new_frame = _get_frame(frame);
	// measure fps of valid frames
	const float dt_smoothing_const = 0.9;
	dt_valid += dt;
	if (new_frame)
	{
		dt_mean = dt_smoothing_const * dt_mean + (1.0 - dt_smoothing_const) * dt_valid;
		cam_info.fps = dt_mean > 1e-3 ? 1.0 / dt_mean : 0;
		dt_valid = 0;
	}
	return new_frame;
}

void CVCamera::start()
{
    cap = new VideoCapture(desired_index);
	// extract camera info
    if (cap->isOpened())
	{
        _set_fps();
        _set_res();
		active = true;
		active_index = desired_index;
        cam_info.res_x = cap->get(CV_CAP_PROP_FRAME_WIDTH);
        cam_info.res_y = cap->get(CV_CAP_PROP_FRAME_HEIGHT);
    } else {
        delete cap;
        cap = nullptr;
    }
}

void CVCamera::stop()
{
    if (cap)
    {
        cap->release();
        delete cap;
    }
	active = false;
}

bool CVCamera::_get_frame(Mat* frame)
{
    if (cap && cap->isOpened())
	{
        Mat img;
        for (int i = 0; i < 100 && !cap->read(img); i++)
            ;;

		if (img.empty())
			return false;

        *frame = img;
        return true;
	}
	return false;
}

void CVCamera::_set_fps()
{
    if (cap) cap->set(CV_CAP_PROP_FPS, cam_desired.fps);
}

void CVCamera::_set_res()
{
	if (cap)
	{
        cap->set(CV_CAP_PROP_FRAME_WIDTH,  cam_desired.res_x);
        cap->set(CV_CAP_PROP_FRAME_HEIGHT, cam_desired.res_y);
        cam_info.res_x = cap->get(CV_CAP_PROP_FRAME_WIDTH);
        cam_info.res_y = cap->get(CV_CAP_PROP_FRAME_HEIGHT);
	}
}
void CVCamera::_set_device_index()
{
    if (cap)
    {
        cap->release();
        delete cap;
    }
    cap = new VideoCapture(desired_index);
}
