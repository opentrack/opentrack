/*
* Copyright (c) 2017-2018 Wei Shuai <cpuwolf@gmail.com>
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*/
#pragma once

#include "pt-api.hpp"

#include <opencv2/core.hpp>
#include <QImage>



namespace pt_module {

enum wii_camera_status : unsigned { wii_cam_wait_for_dongle, wii_cam_wait_for_sync, wii_cam_wait_for_connect, wii_cam_data_no_change, wii_cam_data_change };

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

    void set_last_frame(const pt_frame& frame) override;
    QImage get_bitmap() override;
    void draw_head_center(f x, f y) override;

    operator cv::Mat&() { return frame_copy; }
    operator cv::Mat const&() const { return frame_copy; }

private:
    static void ensure_size(cv::Mat& frame, int w, int h, int type);

    cv::Mat frame_copy, frame_out;
	wii_camera_status status = wii_cam_wait_for_connect;
};

} // ns pt_module
