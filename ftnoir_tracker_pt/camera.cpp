/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "camera.h"
#include <QDebug>

using namespace cv;

// ----------------------------------------------------------------------------
void Camera::set_index(int index)
{
	if (desired_index != index)
	{
		desired_index = index;
		_set_index();

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
/*
void CVCamera::start()
{
	cap = cvCreateCameraCapture(desired_index);
	// extract camera info
	if (cap)
	{
		active = true;
		active_index = desired_index;
		cam_info.res_x = cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_WIDTH);
		cam_info.res_y = cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_HEIGHT);
	}
}

void CVCamera::stop()
{
	if (cap) cvReleaseCapture(&cap);
	active = false;
}

bool CVCamera::_get_frame(Mat* frame)
{
    if (cap && cvGrabFrame(cap) != 0)
	{
		// retrieve frame
		IplImage* _img = cvRetrieveFrame(cap, 0);
		if(_img)
		{
			if(_img->origin == IPL_ORIGIN_TL)
				*frame = Mat(_img);
			else
			{
				Mat temp(_img);
				flip(temp, *frame, 0);
			}
			return true;
		}
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
	if (cap) cvSetCaptureProperty(cap, CV_CAP_PROP_FPS, cam_desired.fps);
}

void CVCamera::_set_res()
{
	if (cap)
	{
		cvSetCaptureProperty(cap, CV_CAP_PROP_FRAME_WIDTH,  cam_desired.res_x);
		cvSetCaptureProperty(cap, CV_CAP_PROP_FRAME_HEIGHT, cam_desired.res_y);
		cam_info.res_x = cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_WIDTH);
		cam_info.res_y = cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_HEIGHT);
	}
}
*/

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

void VICamera::_set_index()
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

