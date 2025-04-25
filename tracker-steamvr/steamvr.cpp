/*
 * Copyright (c) 2017, Benjamin Flegel
 * Copyright (c) 2017, Stanislaw Halik
 * Copyright (c) 2017, Anthony Coddington
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "steamvr.hpp"

#include "api/plugin-api.hpp"

#include <cmath>
#include <cstdlib>

#include <QMessageBox>
#include <QDebug>

QRecursiveMutex device_list::mtx;

template<typename F>
auto with_vr_lock(F&& fun) -> decltype(fun(vr_t(), vr_error_t()))
{
    QMutexLocker l(&device_list::mtx);
    auto [v, e] = device_list::vr_init();
    return fun(v, e);
}

void device_list::fill_device_specs(QList<device_spec>& list)
{
    with_vr_lock([&](vr_t v, vr_error_t)
    {
        list.clear();

        pose_t device_states[max_devices];

        if (!v)
            return;

        v->GetDeviceToAbsoluteTrackingPose(origin::TrackingUniverseSeated, 0,
                                           device_states, vr::k_unMaxTrackedDeviceCount);

        constexpr unsigned bufsiz = vr::k_unMaxPropertyStringSize;
        static char str[bufsiz+1] {}; // vr_lock prevents reentrancy

        for (unsigned k = 0; k < vr::k_unMaxTrackedDeviceCount; k++)
        {
            if (v->GetTrackedDeviceClass(k) == vr::ETrackedDeviceClass::TrackedDeviceClass_Invalid)
            {
                qDebug() << "steamvr: no device with index";
                continue;
            }

            if (!device_states[k].bDeviceIsConnected)
            {
                qDebug() << "steamvr: device not connected but proceeding";
                continue;
            }

            unsigned len;

            len = v->GetStringTrackedDeviceProperty(k, vr::ETrackedDeviceProperty::Prop_SerialNumber_String, str, bufsiz);
            if (!len)
            {
                qDebug() << "steamvr: getting serial number failed for" << k;
                continue;
            }

            device_spec dev;

            dev.serial = str;

            len = v->GetStringTrackedDeviceProperty(k, vr::ETrackedDeviceProperty::Prop_ModelNumber_String, str, bufsiz);
            if (!len)
            {
                qDebug() << "steamvr: getting model number failed for" << k;
                continue;
            }

            switch (v->GetTrackedDeviceClass(k))
            {
            using enum vr::ETrackedDeviceClass;
            case TrackedDeviceClass_HMD:
                dev.type = "HMD"; break;
            case TrackedDeviceClass_Controller:
                dev.type = "Controller"; break;
            case TrackedDeviceClass_TrackingReference:
                dev.type = "Tracking reference"; break;
            case TrackedDeviceClass_DisplayRedirect:
                dev.type = "Display redirect"; break;
            case TrackedDeviceClass_GenericTracker:
                dev.type = "Generic"; break;
            default:
                dev.type = "Unknown"; break;
            }

            dev.model = str;
            dev.pose = device_states[k];
            dev.k = k;
            dev.is_connected = device_states[k].bDeviceIsConnected;

            list.push_back(dev);
        }
    });
}

device_list::device_list()
{
    refresh_device_list();
}

void device_list::refresh_device_list()
{
    device_specs.clear();
    device_specs.reserve(max_devices);
    fill_device_specs(device_specs);
}

device_list::maybe_pose device_list::get_pose(int k)
{
    if (!(unsigned(k) < max_devices))
        return maybe_pose(false, pose_t{});

    return with_vr_lock([k](vr_t v, vr_error_t)
    {
        static pose_t poses[max_devices] {}; // vr_lock removes reentrancy

        v->GetDeviceToAbsoluteTrackingPose(origin::TrackingUniverseSeated, 0,
                                           poses, max_devices);

        const pose_t& pose = poses[k];

        if (pose.bPoseIsValid && pose.bDeviceIsConnected)
            return maybe_pose{ true, poses[k] };
        else
            eval_once(qDebug() << "steamvr:"
                               << "no valid pose from device" << k
                               << "valid" << pose.bPoseIsValid
                               << "connected" << pose.bDeviceIsConnected);

        return maybe_pose{ false, {} };
    });
}

tt device_list::vr_init()
{
    static tt t = vr_init_();
    return t;
}

tt device_list::vr_init_()
{
    vr_error_t error = vr_error_t::VRInitError_Unknown;
    vr_t v = vr::VR_Init(&error, vr::EVRApplicationType::VRApplication_Other);

    if (v)
        std::atexit(vr::VR_Shutdown);
    else
        qDebug() << "steamvr: init failure" << error << device_list::error_string(error);

    return { v, error };
}

QString device_list::error_string(vr_error_t err)
{
    const char* str = vr::VR_GetVRInitErrorAsSymbol(err);
    const char* desc = vr::VR_GetVRInitErrorAsEnglishDescription(err);

    if (!desc)
        desc = "No description";

    if (str)
        return QStringLiteral("%1: %2").arg(str, desc);
    else
        return { "Unknown error" };
}

steamvr::steamvr() = default;
steamvr::~steamvr() = default;

module_status steamvr::start_tracker(QFrame*)
{
    return with_vr_lock([this](vr_t v, vr_error_t e)
    {
        QString err;

        if (!v)
        {
            err = device_list::error_string(e);
            return error(err);
        }

        const QString serial = s.device_serial().toString();
        device_list d;
        const QList<device_spec>& specs = d.devices();
        const int sz = specs.count();

        if (sz == 0)
            err = tr("No HMD connected");

        for (const device_spec& spec : specs)
        {
            if (serial == "" || serial == spec.to_string())
            {
                device_index = spec.k;
                break;
            }
        }

        if (device_index == UINT_MAX && err.isEmpty())
            err = tr("Can't find device with that serial");

        if (err.isEmpty())
        {
            if (auto* c = vr::VRCompositor(); c != nullptr)
            {
                c->SetTrackingSpace(origin::TrackingUniverseSeated);
                return status_ok();
            }
            else
                return error("vr::VRCompositor == NULL");
        }

      return error(err);
    });
}

void steamvr::data(double* data)
{
    if (device_index != UINT_MAX)
    {
        auto [ok, pose] = device_list::get_pose(device_index);
        if (ok)
        {
            constexpr int c = 10;

            const auto& result = pose.mDeviceToAbsoluteTracking;

            data[TX] = (double)(-result.m[0][3] * c);
            data[TY] = (double)(result.m[1][3] * c);
            data[TZ] = (double)(result.m[2][3] * c);

            matrix_to_euler(data[Yaw], data[Pitch], data[Roll], result);

            constexpr double r2d = 180 / M_PI;
            data[Yaw] *= r2d; data[Pitch] *= r2d; data[Roll] *= r2d;
        }
    }
}

bool steamvr::center()
{
    return with_vr_lock([&](vr_t v, vr_error_t)
    {
        if (v)
        {
            if (v->GetTrackedDeviceClass(device_index) == vr::ETrackedDeviceClass::TrackedDeviceClass_HMD)
            {
                auto* c = vr::VRChaperone();
                if (!c)
                {
                    eval_once(qDebug() << "steamvr: vr::VRChaperone == NULL");
                    return false;
                }
                else
                {
                    c->ResetZeroPose(origin::TrackingUniverseSeated);
                    // Use chaperone universe real world up instead of opentrack's initial pose centering
                    // Note: Controllers will be centered based on initial headset position.
                    return true;
                }
            }
            else
                // with controllers, resetting the seated pose does nothing
                return false;
        }
        return false;
    });
}

void steamvr::matrix_to_euler(double& yaw, double& pitch, double& roll, const vr::HmdMatrix34_t& result)
{
    using d = double;

    yaw = std::atan2(d(result.m[2][0]), d(result.m[0][0]));
    pitch = std::atan2(-d(result.m[1][2]), d(result.m[1][1]));
    roll = std::asin(d(result.m[1][0]));
}

steamvr_dialog::steamvr_dialog()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    ui.device->clear();
    ui.device->addItem("First available", QVariant(QVariant::String));

    device_list list;
    for (const device_spec& spec : list.devices())
    {
        QString text = spec.to_string();
        if (!spec.is_connected)
            text = QStringLiteral("%1 [disconnected]").arg(text);
        ui.device->addItem(text, spec.to_string());
    }

    tie_setting(s.device_serial, ui.device);
}

void steamvr_dialog::doOK()
{
    s.b->save();
    close();
}

void steamvr_dialog::doCancel()
{
    close();
}



QString device_spec::to_string() const
{
    return QStringLiteral("<%1> %2 [%3]").arg(type, model, serial);
}

OPENTRACK_DECLARE_TRACKER(steamvr, steamvr_dialog, steamvr_metadata)
