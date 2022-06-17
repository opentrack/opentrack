/*
* Copyright (c) 2017-2018 Wei Shuai <cpuwolf@gmail.com>
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*/

// silence #pragma deprecated in bluetoothapis.h
#undef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_VISTA
#undef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_VISTASP1

#include "wii_camera.h"
#include "wii_frame.hpp"

#include "compat/math-imports.hpp"

#include <opencv2/imgproc.hpp>
#include <bluetoothapis.h>

using namespace pt_module;

WIICamera::WIICamera(const QString& module_name) : s { module_name }
{
	cam_info.fps = 70;
	cam_info.res_x = 1024;
	cam_info.res_y = 768;
	cam_info.fov = 42.0f;
	cam_info.name = "Wii";
}

WIICamera::~WIICamera()
{
	stop();
}

QString WIICamera::get_desired_name() const
{
	return QStringLiteral("Wii");
}

QString WIICamera::get_active_name() const
{
	return get_desired_name();
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
	const wii_camera_status new_frame = get_frame(frame);

	//create a fake blank frame
	frame.create(cam_info.res_x, cam_info.res_y, CV_8UC3);
	frame.setTo({0});
	wii.status = new_frame;

	switch (new_frame)
	{
	case wii_cam_data_change:
		get_status(wii);
		get_points(wii);
		break;
	case wii_cam_data_no_change:
		return result(false, cam_info);
	default:
		break;
	}

	return result(true, cam_info);
}

bool WIICamera::start(const pt_settings&)
{
	if (m_pDev)
		return true;
	m_pDev = std::make_unique<wiimote>();
	m_pDev->ChangedCallback = on_state_change;
	m_pDev->CallbackTriggerFlags = (state_change_flags)(CONNECTED |
		EXTENSION_CHANGED |
		MOTIONPLUS_CHANGED);
	return true;
}

void WIICamera::stop()
{
	if (!m_pDev)
		return;

	cam_info = {};
	cam_desired = {};
	pitch_ = 0; roll_ = 0;

	m_pDev->ChangedCallback = nullptr;
	m_pDev->Disconnect();
	m_pDev = nullptr;
}

#ifdef __MINGW32__
extern "C"
    DWORD WINAPI BluetoothAuthenticateDevice(
      HWND                  hwndParent,
      HANDLE                hRadio,
      BLUETOOTH_DEVICE_INFO *pbtbi,
      PWSTR                 pszPasskey,
      ULONG                 ulPasskeyLength
    );

#ifndef BLUETOOTH_SERVICE_ENABLE
#   define BLUETOOTH_SERVICE_DISABLE   0x00
#   define BLUETOOTH_SERVICE_ENABLE    0x01
#endif

#endif

wii_camera_status WIICamera::pair()
{
	wii_camera_status ret = wii_cam_wait_for_sync;
	HBLUETOOTH_RADIO_FIND hbt;
	BLUETOOTH_FIND_RADIO_PARAMS bt_param;
	constexpr int max_devices = 64;
	HANDLE hbtlist[max_devices];
	int ibtidx = 0;
	bool wiifound = false;

	bt_param.dwSize = sizeof(bt_param);
	hbt = BluetoothFindFirstRadio(&bt_param, hbtlist + ibtidx);
	if (!hbt) { ret = wii_cam_wait_for_dongle; return ret; }
	do
		ibtidx++;
	while (ibtidx < max_devices && BluetoothFindNextRadio(&bt_param, hbtlist + ibtidx));
	BluetoothFindRadioClose(hbt);

	int i;
	bool error = false;
	for (i = 0; i < ibtidx; i++)
	{
		BLUETOOTH_RADIO_INFO btinfo;
		btinfo.dwSize = sizeof(btinfo);

		if (ERROR_SUCCESS != BluetoothGetRadioInfo(hbtlist[i], &btinfo)) {break;}

		HBLUETOOTH_DEVICE_FIND hbtdevfd;
		BLUETOOTH_DEVICE_SEARCH_PARAMS btdevparam {};
		BLUETOOTH_DEVICE_INFO btdevinfo;

		btdevinfo.dwSize = sizeof(btdevinfo);
		btdevparam.dwSize = sizeof(btdevparam);
		btdevparam.fReturnUnknown = TRUE;
		btdevparam.fReturnAuthenticated = TRUE;
		btdevparam.fReturnConnected = TRUE;
		btdevparam.fReturnRemembered = TRUE;
		btdevparam.fIssueInquiry = TRUE;
		btdevparam.cTimeoutMultiplier = 1;
		btdevparam.hRadio = hbtlist[i];
		hbtdevfd=BluetoothFindFirstDevice(&btdevparam, &btdevinfo);
		if (!hbtdevfd) {
			int error= GetLastError();
			qDebug() << error;
			break;
		}
		do
		{
			if (!!wcscmp(btdevinfo.szName, L"Nintendo RVL-CNT-01-TR") &&
			    !!wcscmp(btdevinfo.szName, L"Nintendo RVL-CNT-01"))
			{
				continue;
			}

			if ((btdevinfo.fRemembered)&&error) {
				BluetoothRemoveDevice(&btdevinfo.Address);
			}

			wiifound = true;
			if (btdevinfo.fConnected) {
				break;
			}
			WCHAR pwd[6];
			pwd[0] = btinfo.address.rgBytes[0];
			pwd[1] = btinfo.address.rgBytes[1];
			pwd[2] = btinfo.address.rgBytes[2];
			pwd[3] = btinfo.address.rgBytes[3];
			pwd[4] = btinfo.address.rgBytes[4];
			pwd[5] = btinfo.address.rgBytes[5];

			if (ERROR_SUCCESS != BluetoothAuthenticateDevice(nullptr, hbtlist[i], &btdevinfo, pwd, 6)) { error = true; continue; }
			DWORD servicecount = 32;
			GUID guids[32];
			if (ERROR_SUCCESS != BluetoothEnumerateInstalledServices(hbtlist[i], &btdevinfo, &servicecount, guids)) { error = true; continue; }
			if (ERROR_SUCCESS != BluetoothSetServiceState(hbtlist[i], &btdevinfo, (GUID*)&HumanInterfaceDeviceServiceClass_UUID, BLUETOOTH_SERVICE_ENABLE)) { error = true; continue; }
			break;
		} while (BluetoothFindNextDevice(hbtdevfd, &btdevinfo));
		BluetoothFindDeviceClose(hbtdevfd);
	}

	for (i = 0; i < ibtidx; i++)
	{
		CloseHandle(hbtlist[i]);
	}
	if (wiifound) { ret = wii_cam_wait_for_connect; }
	return ret;
}

wii_camera_status WIICamera::get_frame(cv::Mat&)
{
	wii_camera_status ret = wii_cam_wait_for_connect;

	if (!m_pDev->IsConnected()) {
		qDebug() << "wii wait";
		ret = pair();
		switch(ret)
		{
		case wii_cam_wait_for_sync:
			m_pDev->Disconnect();
			goto goodbye;
		case wii_cam_wait_for_connect:
			break;
		default:
		    break;
		}
		if (!m_pDev->Connect(wiimote::FIRST_AVAILABLE)) {
			Beep(500, 30); Sleep(1000);
			goto goodbye;
		}
	}

	if (m_pDev->RefreshState() == NO_CHANGE) {
		Sleep(14); // don't hog the CPU if nothing changed
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

bool WIICamera::get_points(struct wii_info& wii)
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
	m_pDev->SetLEDs(0);
	return ret;
}

void WIICamera::get_status(struct wii_info& wii)
{
	//draw battery status
	wii.BatteryPercent = m_pDev->BatteryPercent;
	wii.bBatteryDrained = m_pDev->bBatteryDrained;

	//draw horizon
	if (m_pDev->Nunchuk.Acceleration.Orientation.UpdateAge < 10)
	{
		pitch_ = (int)m_pDev->Acceleration.Orientation.Pitch;
		roll_ = (int)m_pDev->Acceleration.Orientation.Roll;
	}

	wii.Pitch = pitch_;
	wii.Roll = roll_;
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
