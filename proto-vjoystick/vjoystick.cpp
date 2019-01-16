/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */
#include "vjoystick.h"
#include "api/plugin-api.hpp"
#include "compat/math.hpp"

#include <cstring>
#include <QDebug>

#include <QPushButton>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

// required for api headers
#include <windows.h>

#undef PPJOY_MODE
#include <public.h>
#include <vjoyinterface.h>

#define OPENTRACK_VJOYSTICK_ID 1

const unsigned char handle::axis_ids[6] =
{
    HID_USAGE_X,
    HID_USAGE_Y,
    HID_USAGE_Z,
    HID_USAGE_RX,
    HID_USAGE_RY,
    HID_USAGE_RZ,
//    HID_USAGE_SL0,
//    HID_USAGE_SL1,
//    HID_USAGE_WHL,
};

static constexpr double val_minmax[6] =
{
    50,
    50,
    50,
    180,
    180,
    180
};

bool handle::init()
{
    if (!AcquireVJD(OPENTRACK_VJOYSTICK_ID))
        return false;

    unsigned cnt = 0;
    bool status = true;

    for (unsigned i = 0; i < axis_count; i++)
    {
        if (!GetVJDAxisExist(OPENTRACK_VJOYSTICK_ID, axis_ids[i]))
            continue;
        cnt++;
        status &= !!GetVJDAxisMin(OPENTRACK_VJOYSTICK_ID, axis_ids[i], &axis_min[i]);
        status &= !!GetVJDAxisMax(OPENTRACK_VJOYSTICK_ID, axis_ids[i], &axis_max[i]);
    }
    //(void)ResetVJD(OPENTRACK_VJOYSTICK_ID);

    return status && cnt;
}

handle::handle()
{
    if (!isVJDExists(OPENTRACK_VJOYSTICK_ID))
        joy_state = state::notent;
    else if (init())
        joy_state = state::success;
    else
        joy_state = state::fail;
}

handle::~handle()
{
    if (joy_state == state::success)
        RelinquishVJD(OPENTRACK_VJOYSTICK_ID);
}

bool handle::to_axis_value(unsigned axis_id, double val, int& ret) const
{
    if (!axis_min[axis_id] && !axis_max[axis_id])
        return false;

    const double minmax = val_minmax[axis_id];
    const double min = axis_min[axis_id];
    const double max = axis_max[axis_id];

    ret = int(clamp((val+minmax) * max / (2*minmax) - min, min, max));
    return true;
}

vjoystick_proto::vjoystick_proto() = default;
vjoystick_proto::~vjoystick_proto() = default;

module_status vjoystick_proto::initialize()
{
    h = handle{};

    if (h->get_state() != state::success)
    {
        QMessageBox msgbox;
        msgbox.setIcon(QMessageBox::Critical);
        msgbox.setText(tr("vjoystick driver missing"));
        msgbox.setInformativeText(tr("vjoystick won't work without the driver installed."));

        QPushButton* driver_button = msgbox.addButton(tr("Download the driver"), QMessageBox::ActionRole);
        QPushButton* project_site_button = msgbox.addButton(tr("Visit project site"), QMessageBox::ActionRole);
        msgbox.addButton(QMessageBox::Close);

        (void) msgbox.exec();

        if (msgbox.clickedButton() == driver_button)
        {
            static const char* download_driver_url = "https://sourceforge.net/projects/vjoystick/files/latest/download";
            QDesktopServices::openUrl(QUrl(download_driver_url, QUrl::StrictMode));
        }
        else if (msgbox.clickedButton() == project_site_button)
        {
            static const char* project_site_url = "http://vjoystick.sourceforge.net/site/";
            QDesktopServices::openUrl(QUrl(project_site_url, QUrl::StrictMode));
        }
    }

    switch (h->get_state())
    {
    default:
    case state::notent:
        return error(tr("vjoystick not installed or disabled"));
    case state::fail:
        return error(tr("can't initialize vjoystick"));
    case state::success:
        return status_ok();
    }
}

void vjoystick_proto::pose(const double *pose)
{
    if (h->get_state() != state::success)
        return;

    for (unsigned i = 0; i < handle::axis_count; i++)
    {
        int val;
        if (!h->to_axis_value(i, pose[i], val))
            continue;
        SetAxis(val, OPENTRACK_VJOYSTICK_ID, handle::axis_ids[i]);
    }
}

OPENTRACK_DECLARE_PROTOCOL(vjoystick_proto, vjoystick_dialog, vjoystick_metadata)
