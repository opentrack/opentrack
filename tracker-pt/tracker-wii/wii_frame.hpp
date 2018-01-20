/* 
* Copyright (c) 2015-2016 Stanislaw Halik <sthalik@misaki.pl>
* Copyright (c) 2017-2018 Wei Shuai <cpuwolf@gmail.com>
*/
#pragma once

#include "pt-api.hpp"

#include <opencv2/core.hpp>
#include <QImage>



namespace pt_module {

enum wii_camera_status : unsigned { wii_cam_wait_for_connect, wii_cam_data_no_change, wii_cam_data_change };

struct wii_info_points {
	unsigned ux;
	unsigned uy;
	int isize;
	bool bvis;
};

struct wii_info {
	struct wii_info_points Points[4];
	bool bBatteryDrained;
	unsigned char BatteryPercent;
	float	 Pitch;
	float	 Roll;
	wii_camera_status status;
};

struct WIIFrame final : pt_frame
{
    cv::Mat mat;
	struct wii_info wii;

    operator const cv::Mat&() const& { return mat; }
    operator cv::Mat&() & { return mat; }
};

struct WIIPreview final : pt_preview
{
    WIIPreview(int w, int h);

    WIIPreview& operator=(const pt_frame& frame) override;
    QImage get_bitmap() override;
    void draw_head_center(double x, double y) override;

    operator cv::Mat&() { return frame_copy; }
    operator cv::Mat const&() const { return frame_copy; }

private:
    static void ensure_size(cv::Mat& frame, int w, int h, int type);

    bool fresh = true;
    cv::Mat frame_copy, frame_color, frame_resize, frame_out;
};

} // ns pt_module
