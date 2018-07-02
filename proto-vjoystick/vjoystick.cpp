/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */
#include "vjoystick.h"

#include <cmath>
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

static constexpr inline unsigned axis_count = 6;

static constexpr unsigned char axis_ids[axis_count] =
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

static constexpr double val_minmax[axis_count] =
{
    50,
    50,
    50,
    180,
    180,
    180
};

void handle::init()
{
    for (unsigned i = 0; i < axis_count; i++)
    {
        if (!GetVJDAxisExist(OPENTRACK_VJOYSTICK_ID, axis_ids[i]))
        {
            // avoid floating point division by zero
            axis_min[i] = 0;
            axis_max[i] = 1;
            continue;
        }
        GetVJDAxisMin(OPENTRACK_VJOYSTICK_ID, axis_ids[i], &axis_min[i]);
        GetVJDAxisMax(OPENTRACK_VJOYSTICK_ID, axis_ids[i], &axis_max[i]);
    }
    (void) ResetVJD(OPENTRACK_VJOYSTICK_ID);
}

handle::handle()
{
    const bool ret = AcquireVJD(OPENTRACK_VJOYSTICK_ID);
    if (!ret)
    {
        if (!isVJDExists(OPENTRACK_VJOYSTICK_ID))
            joy_state = state_notent;
        else
            joy_state = state_fail;
    }
    else
    {
        joy_state = state_success;
        init();
    }
}

handle::~handle()
{
    if (joy_state == state_success)
    {
        (void) RelinquishVJD(OPENTRACK_VJOYSTICK_ID);
        joy_state = state_fail;
    }
}

LONG handle::to_axis_value(unsigned axis_id, double val)
{
    const double minmax = val_minmax[axis_id];
    const double min = axis_min[axis_id];
    const double max = axis_max[axis_id];

    const double x = (val+minmax) * max / (2*minmax) - min;
    return std::fmax(min, std::fmin(x, max));
}

vjoystick_proto::vjoystick_proto()
{
}

vjoystick_proto::~vjoystick_proto()
{
}

bool vjoystick_proto::correct()
{
    if (h.get_state() != state_success)
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

    return h.get_state() == state_success;
}

void vjoystick_proto::pose(const double *pose)
{
    if (h.get_state() != state_success)
        return;

    for (unsigned i = 0; i < axis_count; i++)
        SetAxis(h.to_axis_value(i, pose[i]), OPENTRACK_VJOYSTICK_ID, axis_ids[i]);
}

OPENTRACK_DECLARE_PROTOCOL(vjoystick_proto, vjoystick_dialog, vjoystick_metadata)
