/*
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

#include <cmath>
#include <algorithm>
#include <cinttypes>
#include <memory>

#include <QDebug>

using namespace numeric_types;
using namespace pt_module;


WIIPointExtractor::WIIPointExtractor(const QString& module_name) : s(module_name)
{
}

//define a temp draw function
void WIIPointExtractor::draw_point(cv::Mat& preview_frame, const vec2& p, const cv::Scalar& color, int thickness)
{
	static constexpr int len = 9;

	cv::Point p2(iround(p[0] * preview_frame.cols + preview_frame.cols / 2.f),
		 iround(-p[1] * preview_frame.cols + preview_frame.rows / 2.f));

	cv::line(preview_frame,
		cv::Point(p2.x - len, p2.y),
		cv::Point(p2.x + len, p2.y),
		color,
		thickness);
	cv::line(preview_frame,
		cv::Point(p2.x, p2.y - len),
		cv::Point(p2.x, p2.y + len),
		color,
		thickness);
}

void WIIPointExtractor::draw_points(cv::Mat& preview_frame, const struct wii_info& wii)
{
	constexpr int W = 1024;
	constexpr int H = 768;

	for (const wii_info_points& dot : wii.Points)
	{
		if (dot.bvis) {
			//qDebug() << "wii:" << dot.RawX << "+" << dot.RawY;
			//anti-clockwise rotate the 2D point
			const float RX = W - dot.ux;
			const float RY = H - dot.uy;
			//vec2 dt((dot.RawX - W / 2.0f) / W, -(dot.RawY - H / 2.0f) / W);
			//vec2 dt((RX - W / 2.0f) / W, -(RY - H / 2.0f) / W);
			//vec2 dt((2.0f*RX - W) / W, -(2.0f*RY - H ) / W);
			vec2 dt;
			std::tie(dt[0], dt[1]) = to_screen_pos(RX, RY, W, H);
            draw_point(preview_frame, dt, cv::Scalar(0, 255, 0), std::clamp(dot.isize, 1, 32));
		}
	}
}

void WIIPointExtractor::draw_bg(cv::Mat& preview_frame, const struct wii_info& wii)
{
	//draw battery status
	cv::line(preview_frame,
		cv::Point(0, 0),
		cv::Point(preview_frame.cols*wii.BatteryPercent / 100, 0),
		(wii.bBatteryDrained ? cv::Scalar(255, 0, 0) : cv::Scalar(0, 140, 0)),
		2);

	//draw horizon
	int pdelta = iround(preview_frame.rows / 4.f * tan(wii.Pitch * pi / 180.f));
	int rdelta = iround(preview_frame.cols / 4.f * tan(wii.Roll* pi / 180.f));

	cv::line(preview_frame,
		cv::Point(0, preview_frame.rows / 2 + rdelta - pdelta),
		cv::Point(preview_frame.cols, preview_frame.rows / 2 - rdelta - pdelta),
		cv::Scalar(80, 80, 80),
		1);
}

void WIIPointExtractor::extract_points(const pt_frame& frame_,
                                       pt_preview& preview_frame_,
                                       bool preview_visible,
                                       std::vector<vec2>& points)
{
	const struct wii_info& wii = frame_.as_const<WIIFrame>()->wii;
	cv::Mat& preview_frame = *preview_frame_.as<WIIPreview>();

	map_points(wii, points);
	if (preview_visible && wii.status == wii_cam_data_change)
	{
		draw_bg(preview_frame, wii);
		draw_points(preview_frame, wii);
	}
}

bool WIIPointExtractor::map_points(const struct wii_info& wii, std::vector<vec2>& points)
{
	constexpr int W = 1024;
	constexpr int H = 768;
	points.reserve(4);
	points.clear();

	for (unsigned index = 0; index < 4; index++) // NOLINT(modernize-loop-convert)
	{
		const struct wii_info_points &dot = wii.Points[index];
		if (dot.bvis) {
			//qDebug() << "wii:" << dot.RawX << "+" << dot.RawY;
			//anti-clockwise rotate the 2D point
			const float RX = W - dot.ux;
			const float RY = H - dot.uy;
			//vec2 dt((dot.RawX - W / 2.0f) / W, -(dot.RawY - H / 2.0f) / W);
			//vec2 dt((RX - W / 2.0f) / W, -(RY - H / 2.0f) / W);
			//vec2 dt((2.0f*RX - W) / W, -(2.0f*RY - H ) / W);
			vec2 dt;
			std::tie(dt[0], dt[1]) = to_screen_pos(RX, RY, W, H);

			points.push_back(dt);
		}
	}

	return points.size() >= PointModel::N_POINTS;
}
