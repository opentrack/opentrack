/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2015-2017 Stanislaw Halik <sthalik@misaki.pl>
 * Copyright (c) 2017-2018 Wei Shuai <cpuwolf@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "wii_point_extractor.h"

#include "point_tracker.h"
#include "wii_frame.hpp"

#include "cv/numeric.hpp"
#include "compat/math.hpp"

#include <opencv2/videoio.hpp>

#undef PREVIEW
//#define PREVIEW

#if defined PREVIEW
#   include <opencv2/highgui.hpp>
#endif

#include <cmath>
#include <algorithm>
#include <cinttypes>
#include <memory>

#include <QDebug>

using namespace types;
using namespace pt_module;


WIIPointExtractor::WIIPointExtractor(const QString& module_name) : s(module_name)
{

}

//define a temp draw function
void WIIPointExtractor::_draw_point(cv::Mat& preview_frame, const vec2& p, const cv::Scalar& color, int thinkness)
{
	static constexpr int len = 9;

	cv::Point p2(iround(p[0] * preview_frame.cols + preview_frame.cols / 2),
		iround(-p[1] * preview_frame.cols + preview_frame.rows / 2));

	cv::line(preview_frame,
		cv::Point(p2.x - len, p2.y),
		cv::Point(p2.x + len, p2.y),
		color,
		thinkness);
	cv::line(preview_frame,
		cv::Point(p2.x, p2.y - len),
		cv::Point(p2.x, p2.y + len),
		color,
		thinkness);
};

bool WIIPointExtractor::_draw_points(cv::Mat& preview_frame, const struct wii_info &wii, std::vector<vec2>& points)
{
	points.reserve(4);
	points.clear();

	for (unsigned index = 0; index < 4; index++)
	{
		const struct wii_info_points &dot = wii.Points[index];
		if (dot.bvis) {
			//qDebug() << "wii:" << dot.RawX << "+" << dot.RawY;

			const float W = 1024.0f;
			const float H = 768.0f;
			const float RX = W - dot.ux;
			const float RY = H - dot.uy;
			//vec2 dt((dot.RawX - W / 2.0f) / W, -(dot.RawY - H / 2.0f) / W);
			//anti-clockwise rotate 2D point
			vec2 dt((RX - W / 2.0f) / W, -(RY - H / 2.0f) / W);

			points.push_back(dt);
			_draw_point(preview_frame, dt, cv::Scalar(0, 255, 0), dot.isize);
		}
	}
	const bool success = points.size() >= PointModel::N_POINTS;

	return success;
}

void WIIPointExtractor::_draw_bg(cv::Mat& preview_frame, const struct wii_info &wii)
{
	//draw battery status
	cv::line(preview_frame,
		cv::Point(0, 0),
		cv::Point(preview_frame.cols*wii.BatteryPercent / 100, 0),
		(wii.bBatteryDrained ? cv::Scalar(255, 0, 0) : cv::Scalar(0, 140, 0)),
		2);

	//draw horizon
	int pdelta = iround((preview_frame.rows / 2) * tan((wii.Pitch)* M_PI / 180.0f));
	int rdelta = iround((preview_frame.cols / 4) * tan((wii.Roll)* M_PI / 180.0f));

	cv::line(preview_frame,
		cv::Point(0, preview_frame.rows / 2 + rdelta - pdelta),
		cv::Point(preview_frame.cols, preview_frame.rows / 2 - rdelta - pdelta),
		cv::Scalar(80, 80, 80),
		1);
}


void WIIPointExtractor::extract_points(const pt_frame& frame_, pt_preview& preview_frame_, std::vector<vec2>& points)
{
	const cv::Mat& frame = frame_.as_const<WIIFrame>()->mat;
	const struct wii_info& wii = frame_.as_const<WIIFrame>()->wii;
	cv::Mat& preview_frame = *preview_frame_.as<WIIPreview>();

	//create a blank frame
	//cv::Mat blank_frame(preview_frame.cols, preview_frame.rows, CV_8UC3, cv::Scalar(0, 0, 0));
	//cv::cvtColor(_frame, _frame2, cv::COLOR_BGR2BGRA);
	//cv::resize(blank_frame, preview_frame, cv::Size(preview_frame.cols, preview_frame.rows), 0, 0, cv::INTER_NEAREST);

	switch (wii.status) {
	case wii_cam_data_change:
		_draw_bg(preview_frame, wii);
		_draw_points(preview_frame, wii, points);
		break;
	case wii_cam_wait_for_connect:
		char txtbuf[64];
		sprintf(txtbuf, "%s", "wait for WIImote");
		//draw wait text
		cv::putText(preview_frame,
			txtbuf,
			cv::Point(preview_frame.cols / 10, preview_frame.rows / 2),
			cv::FONT_HERSHEY_SIMPLEX,
			1,
			cv::Scalar(255, 255, 255),
			1);
		break;
	}
}

