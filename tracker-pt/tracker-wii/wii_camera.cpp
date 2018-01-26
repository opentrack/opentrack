/*
* Copyright (c) 2017-2018 Wei Shuai <cpuwolf@gmail.com>
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*/


#include "wii_camera.h"
#include "wii_frame.hpp"

#include "compat/sleep.hpp"
#include "compat/camera-names.hpp"
#include "compat/math-imports.hpp"

#include <opencv2/imgproc.hpp>

#include "cv/video-property-page.hpp"


using namespace pt_module;

WIICamera::WIICamera(const QString& module_name) : s { module_name }
{
	cam_info.fps = 70;
	cam_info.res_x = 1024;
	cam_info.res_y = 768;
	cam_info.fov = 42.0f;
	cam_info.idx = 0;
}

QString WIICamera::get_desired_name() const
{
    return desired_name;
}

QString WIICamera::get_active_name() const
{
    return active_name;
}

void WIICamera::show_camera_settings()
{

}

WIICamera::result WIICamera::get_info() const
{
    if (cam_info.res_x == 0 || cam_info.res_y == 0)
        return result(false, pt_camera_info());
    return result(true, cam_info);
}

WIICamera::result WIICamera::get_frame(pt_frame& frame_)
{
    cv::Mat& frame = frame_.as<WIIFrame>()->mat;
	struct wii_info& wii = frame_.as<WIIFrame>()->wii;

    const wii_camera_status new_frame = _get_frame(frame);
	//create a fake blank frame
	frame = cv::Mat(cam_info.res_x, cam_info.res_y, CV_8UC3, cv::Scalar(0, 0, 0));
	wii.status = new_frame;

    switch (new_frame)
    {
	case wii_cam_data_change:
		_get_status(wii);
		_get_points(wii);
		break;
	case wii_cam_data_no_change:
		return result(false, cam_info);
	}

	return result(true, cam_info);
}

pt_camera_open_status WIICamera::start(int idx, int fps, int res_x, int res_y)
{
	m_pDev = std::make_unique<wiimote>();
	if (!m_pDev)
	{
		stop();
		return cam_open_error;
	}
	m_pDev->ChangedCallback = on_state_change;
	m_pDev->CallbackTriggerFlags = (state_change_flags)(CONNECTED |
		EXTENSION_CHANGED |
		MOTIONPLUS_CHANGED);
    return cam_open_ok_no_change;
}

void WIICamera::stop()
{
	onExit = true;
	m_pDev->ChangedCallback = NULL;
	m_pDev->Disconnect();
	Beep(1000, 200);
	if (m_pDev) {
		m_pDev=nullptr;
		m_pDev = NULL;
	}

    desired_name = QString();
    active_name = QString();
    cam_info = pt_camera_info();
    cam_desired = pt_camera_info();
}

wii_camera_status WIICamera::_get_frame(cv::Mat& frame)
{
	wii_camera_status ret = wii_cam_wait_for_connect;

	if (!m_pDev->IsConnected()) {
		qDebug() << "wii wait";
		if (!m_pDev->Connect(wiimote::FIRST_AVAILABLE)) {
			Beep(500, 30); Sleep(1000);
			goto goodbye;
		}
	}

	if (m_pDev->RefreshState() == NO_CHANGE) {
		Sleep(1); // don't hog the CPU if nothing changed
		ret = wii_cam_data_no_change;
		goto goodbye;
	}

	// did we loose the connection?
	if (m_pDev->ConnectionLost())
	{
		goto goodbye;
	}

	ret = wii_cam_data_change;
goodbye:
	return ret;
}

bool WIICamera::_get_points(struct wii_info& wii)
{
	bool dot_sizes = (m_pDev->IR.Mode == wiimote_state::ir::EXTENDED);
	bool ret = false;
	int point_count = 0;

	for (unsigned index = 0; index < 4; index++)
	{
		wiimote_state::ir::dot &dot = m_pDev->IR.Dot[index];
		if (dot.bVisible) {
			wii.Points[index].ux = dot.RawX;
			wii.Points[index].uy = dot.RawY;
			if (dot_sizes) {
				wii.Points[index].isize = dot.Size;
			} else {
				wii.Points[index].isize = 1;
			}
			wii.Points[index].bvis = dot.bVisible;
			point_count++;
			ret = true;
		} else {
			wii.Points[index].ux = 0;
			wii.Points[index].uy = 0;
			wii.Points[index].isize = 0;
			wii.Points[index].bvis = dot.bVisible;
		}
	}
	m_pDev->SetLEDs(3 - point_count);
	return ret;
}

void WIICamera::_get_status(struct wii_info& wii)
{
	//draw battery status
	wii.BatteryPercent = m_pDev->BatteryPercent;
	wii.bBatteryDrained = m_pDev->bBatteryDrained;

	//draw horizon
	static int p = 0;
	static int r = 0;
	if (m_pDev->Nunchuk.Acceleration.Orientation.UpdateAge < 10)
	{
		p = m_pDev->Acceleration.Orientation.Pitch;
		r = m_pDev->Acceleration.Orientation.Roll;
	}

	wii.Pitch = p;
	wii.Roll = r;
}

void WIICamera::on_state_change(wiimote &remote,
	state_change_flags  changed,
	const wiimote_state &new_state)
{
	// the wiimote just connected
	if (changed & CONNECTED)
	{
		/* wiimote connected */
		remote.SetLEDs(0x0f);
		Beep(1000, 300); Sleep(500);

		qDebug() << "wii connected";

		if (new_state.ExtensionType != wiimote::BALANCE_BOARD)
		{
			if (new_state.bExtension)
				remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR_EXT); // no IR dots
			else
				remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR);		//    IR dots
		}
	}
	// another extension was just connected:
	else if (changed & EXTENSION_CONNECTED)
	{

		Beep(1000, 200);

		// switch to a report mode that includes the extension data (we will
		//  loose the IR dot sizes)
		// note: there is no need to set report types for a Balance Board.
		if (!remote.IsBalanceBoard())
			remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR_EXT);
	}
	else if (changed & EXTENSION_DISCONNECTED)
	{

		Beep(200, 300);

		// use a non-extension report mode (this gives us back the IR dot sizes)
		remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR);
	}
}
