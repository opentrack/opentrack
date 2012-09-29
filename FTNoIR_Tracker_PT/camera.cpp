#include "camera.h"

using namespace cv;

// ----------------------------------------------------------------------------
Camera::Camera()
	: dt_valid(0), dt_mean(0), cap(NULL), active_index(-1)
{}

Camera::~Camera()
{
	if (cap) cvReleaseCapture(&cap);
}

void Camera::set_index(int index)
{
	if (index == active_index) return;
	if (cap) cvReleaseCapture(&cap);

	cap = cvCreateCameraCapture(index);

	// extract camera info
	if (cap)
	{
		cam_info.res_x = cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_WIDTH);
		cam_info.res_y = cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_HEIGHT);
	}

	active_index = index;
	dt_mean = 0; // reset fps calculation
}

bool Camera::set_fps(int fps)
{
	return cap && cvSetCaptureProperty(cap, CV_CAP_PROP_FPS, fps);
}

bool Camera::set_res(int x_res, int y_res)
{
	if (cap)
	{
		if (x_res == cam_info.res_x && y_res == cam_info.res_y) return true;
		cvSetCaptureProperty(cap, CV_CAP_PROP_FRAME_WIDTH,  x_res);
		cvSetCaptureProperty(cap, CV_CAP_PROP_FRAME_HEIGHT, y_res);
		cam_info.res_x = cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_WIDTH);
		cam_info.res_y = cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_HEIGHT);
		if (x_res == cam_info.res_x && y_res == cam_info.res_y) return true;
	}
	return false;
}

cv::Mat Camera::get_frame(float dt)
{
	cv::Mat frame;
    if (cap && cvGrabFrame(cap) != 0)
	{
		// retrieve frame
		IplImage* _img = cvRetrieveFrame(cap, 0);
		if(_img)
		{
			if(_img->origin == IPL_ORIGIN_TL)
				frame = Mat(_img);
			else
			{
				Mat temp(_img);
				flip(temp, frame, 0);
			}	
		}
	}

	// measure fps of valid frames
	const float dt_smoothing_const = 0.9;
	dt_valid += dt;
	if (!frame.empty())
	{
		dt_mean = dt_smoothing_const * dt_mean + (1.0 - dt_smoothing_const) * dt_valid;
		cam_info.fps = 1.0 / dt_mean;
		dt_valid = 0;
	}

	return frame;
}