#include "camera.h"

using namespace cv;

// ----------------------------------------------------------------------------
Camera::Camera()
	: dt_valid(0), dt_mean(0), cap(NULL)
{}

Camera::~Camera()
{
	if (cap) cvReleaseCapture(&cap);
}

void Camera::set_index(int index)
{
	if (cap) cvReleaseCapture(&cap);

	cap = cvCreateCameraCapture(index);

	// extract camera info
	if (cap)
	{
		cam_info.res_x = cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_WIDTH);
		cam_info.res_y = cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_HEIGHT);
	}

	// reset fps calculation
	dt_mean = 0;
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