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

void Camera::set_f(float f)
{
	if (cam_desired.f != f)
	{
		cam_desired.f = f;
		_set_f();
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
		cam_info.fps = 1.0 / dt_mean;
		dt_valid = 0;
	}
	return new_frame;
}

// ----------------------------------------------------------------------------
#ifdef OPENTRACK_API
void CVCamera::start()
{
    cap = new VideoCapture(desired_index);
	// extract camera info
    if (cap->isOpened())
	{
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
        /*
         * XXX some Windows webcams fail to decode first
         *     frames and then some every once in a while
         * -sh
         */
        for (int i = 0; i < 100 && !cap->read(img); i++)
            ;;

		if (img.empty())
			return false;

        *frame = img;
        return true;
	}
	return false;
}

void CVCamera::_set_index()
{
	if (active) restart();
}

void CVCamera::_set_f()
{
	cam_info.f = cam_desired.f;
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

#else
// ----------------------------------------------------------------------------
VICamera::VICamera() : frame_buffer(NULL)
{
	VI.listDevices();
}

void VICamera::start()
{
	if (desired_index >= 0)
	{	
		if (cam_desired.res_x == 0 || cam_desired.res_y == 0)
			VI.setupDevice(desired_index);
		else
			VI.setupDevice(desired_index, cam_desired.res_x, cam_desired.res_y);

		active = true;
		active_index = desired_index;

		cam_info.res_x = VI.getWidth(active_index);
		cam_info.res_y = VI.getHeight(active_index);
		new_frame = cv::Mat(cam_info.res_y, cam_info.res_x, CV_8UC3);
		// If matrix is not continuous we have to copy manually via frame_buffer
		if (!new_frame.isContinuous()) {
			unsigned int size = VI.getSize(active_index);
			frame_buffer = new unsigned char[size];
		}
	}
}

void VICamera::stop()
{
	if (active)
	{
		VI.stopDevice(active_index);
	}
	if (frame_buffer)
	{
		delete[] frame_buffer;
		frame_buffer = NULL;
	}
	active = false;
}

bool VICamera::_get_frame(Mat* frame)
{
	if (active && VI.isFrameNew(active_index))
	{
		if (new_frame.isContinuous())
		{
			VI.getPixels(active_index, new_frame.data, false, true);
		}
		else
		{
			// If matrix is not continuous we have to copy manually via frame_buffer
			VI.getPixels(active_index, frame_buffer, false, true);
			new_frame = cv::Mat(cam_info.res_y, cam_info.res_x, CV_8UC3, frame_buffer).clone();
		}
		*frame = new_frame;
		return true;
	}
	return false;
}

void VICamera::_set_device_index()
{
	if (active) restart();
}

void VICamera::_set_f()
{
	cam_info.f = cam_desired.f;
}

void VICamera::_set_fps()
{
	bool was_active = active;
	if (active) stop();
	VI.setIdealFramerate(desired_index, cam_desired.fps);
	if (was_active) start();
}

void VICamera::_set_res()
{
	if (active) restart();
}
#endif

// ----------------------------------------------------------------------------
Mat FrameRotation::rotate_frame(Mat frame)
{	
	switch (rotation)
	{
	case CLOCKWISE:
		{
			Mat dst;
			transpose(frame, dst);
			flip(dst, dst, 1);
			return dst;
		}

	case COUNTER_CLOCKWISE:
		{
			Mat dst;
			transpose(frame, dst);
			flip(dst, dst, 0);
			return dst;
		}
	
	default:
		return frame;
	}
}
