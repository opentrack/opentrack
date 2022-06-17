/*
 * Copyright (c) 2017-2018 Wei Shuai <cpuwolf@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "pt-api.hpp"

#include "compat/timer.hpp"

#include <functional>
#include <memory>
#include <tuple>

#include <opencv2/core.hpp>

#include <QString>

#include <wiiyourself/wiimote.h>
#include "wii_frame.hpp"

namespace pt_module {

struct WIICamera final : pt_camera
{
	WIICamera(const QString& module_name);
	~WIICamera() override;

	bool start(const pt_settings&) override;
	void stop() override;

	result get_frame(pt_frame& Frame) override;
	result get_info() const override;

	pt_camera_info get_desired() const override { return cam_desired; }
	QString get_desired_name() const override;
	QString get_active_name() const override;

	void set_fov(f x) override { (void) x; }
	void show_camera_settings() override;

private:
	std::unique_ptr<wiimote> m_pDev;
	static void on_state_change(wiimote &remote,
		state_change_flags changed,
		const wiimote_state &new_state);

	wii_camera_status pair();
	wii_camera_status get_frame(cv::Mat& Frame);
	bool get_points(struct wii_info& wii);
	void get_status(struct wii_info& wii);

	pt_camera_info cam_info;
	pt_camera_info cam_desired;
	int pitch_ = 0, roll_ = 0;
	pt_settings s;

	static constexpr inline double dt_eps = 1./384;
};

} // ns pt_module
