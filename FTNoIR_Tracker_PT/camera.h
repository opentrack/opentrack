/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>

// ----------------------------------------------------------------------------
struct CamInfo
{
	int res_x;
	int res_y;
	int fps;
	float f;	// (focal length) / (sensor width)
};

// ----------------------------------------------------------------------------
class Camera
{
public:
	
	Camera();
	~Camera();

	void set_index(int index);
	void set_f(float f) { cam_info.f = f; }
	bool set_fps(int fps);
	bool set_res(int x_res, int y_res);

	// gets a frame from the camera, dt: time since last call in seconds
	cv::Mat get_frame(float dt);

	// WARNING: returned reference is valid as long as object
	const CamInfo& get_info() const { return cam_info; }

protected:
	int active_index;
	CvCapture* cap;
	CamInfo cam_info;
	float dt_valid;
	float dt_mean;
};

#endif //CAMERA_H
