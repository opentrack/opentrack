/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "steamvr.hpp"
#include "api/plugin-api.hpp"
#include "compat/util.hpp"

#include <cstdlib>
#include <cmath>
#include <algorithm>

#include <QMessageBox>
#include <QDebug>

void steamvr::vr_deleter()
{
    static std::atomic_flag atexit_done = ATOMIC_FLAG_INIT;

    if (atexit_done.test_and_set(std::memory_order_seq_cst))
    {
        std::atexit(vr::VR_Shutdown);
    }
}

steamvr::vr_t steamvr::vr_init(error_t& error)
{
    error = error_t::VRInitError_Unknown;

    // the background type would surely confuse users
    vr_t vr = vr::VR_Init(&error, vr::EVRApplicationType::VRApplication_Other);

    if (vr)
        vr_deleter();

    return vr;
}

QString steamvr::strerror(error_t error)
{
    const char* str(vr::VR_GetVRInitErrorAsSymbol(error));
    return QString(str ? str : "No description");
}

steamvr::steamvr() : vr(nullptr)
{
}

steamvr::~steamvr()
{
}

void steamvr::start_tracker(QFrame*)
{
    error_t e(error_t::VRInitError_Unknown);
    vr = vr_init(e);

    if (!vr)
    {
        QMessageBox::warning(nullptr,
                             QCoreApplication::translate("steamvr", "Valve SteamVR init error"), strerror(e),
                             QMessageBox::Close, QMessageBox::NoButton);
        return;
    }

    bool ok = false;

    for (unsigned k = 0; k < vr::k_unMaxTrackedDeviceCount; k++)
        if (vr->GetTrackedDeviceClass(k) == vr::ETrackedDeviceClass::TrackedDeviceClass_HMD)
        {
            ok = true;
            break;
        }

    if (!ok)
    {
        QMessageBox::warning(nullptr,
                             QCoreApplication::translate("steamvr", "Valve SteamVR init warning"),
                             QCoreApplication::translate("steamvr", "No HMD connected"),
                             QMessageBox::Close, QMessageBox::NoButton);
        return;
    }
}

quat steamvr::get_quaternion(const vr::HmdMatrix34_t& r)
{
    using std::max;
    using std::sqrt;
    using std::acos;

    const auto& m = r.m;

    float qw = sqrt(max(0.f, 1 + m[0][0] + m[1][1] + m[2][2])) / 2;
    float qx = sqrt(max(0.f, 1 + m[0][0] - m[1][1] - m[2][2])) / 2;
    float qy = sqrt(max(0.f, 1 - m[0][0] + m[1][1] - m[2][2])) / 2;
    float qz = sqrt(max(0.f, 1 - m[0][0] - m[1][1] + m[2][2])) / 2;
    qx = (m[1][2] - m[2][1]) < 0 ? -qx : qx;
    qy = (m[2][0] - m[0][2]) < 0 ? -qy : qy;
    qz = (m[0][1] - m[1][0]) < 0 ? -qz : qz;

    float s = sqrt(1 - qw * qw);
    s = s < .001 ? 1 : s;

    return quat(2 * acos(qw), qx/s, qy/s, qz/s);
}

void steamvr::data(double* data)
{
    if (vr)
    {
        vr::TrackedDevicePose_t devices[vr::k_unMaxTrackedDeviceCount] = {};

        vr->GetDeviceToAbsoluteTrackingPose(vr::ETrackingUniverseOrigin::TrackingUniverseStanding, 0,
                                            devices, vr::k_unMaxTrackedDeviceCount);

        bool done = false;

        for (unsigned k = 0; k < vr::k_unMaxTrackedDeviceCount; k++)
        {
            using namespace euler;

            if (!devices[k].bPoseIsValid)
                continue;

            if (vr->GetTrackedDeviceClass(k) != vr::ETrackedDeviceClass::TrackedDeviceClass_HMD)
                continue;

            const auto& result = devices[k].mDeviceToAbsoluteTracking;

            static constexpr double c[3] { -1, 1, -1 };

            for (unsigned i = 0; i < 3; i++)
                data[i] = double(result.m[i][3]) * c[i] * 100;

            const quat q = get_quaternion(result);
            static constexpr double r2d = 180/M_PI;

            using std::atan2;
            using std::asin;

            data[Roll]  = r2d * atan2(2*(q(0)*q(1) + q(2)*q(3)), 1 - 2*(q(1)*q(1) + q(2)*q(2)));
            data[Pitch] = r2d * asin(2*(q(0)*q(2) - q(3)*q(1)));
            data[Yaw]   = r2d * atan2(2*(q(0)*q(3) + q(1)*q(2)), 1 - 2*(q(2)*q(2) + q(3)*q(3)));

            done = true;
            break;
        }

        if (!done)
            qDebug() << "steamvr: no device with pose found";
    }
}

void steamvr_dialog::register_tracker(ITracker*) {}
void steamvr_dialog::unregister_tracker() {}

OPENTRACK_DECLARE_TRACKER(steamvr, steamvr_dialog, steamvr_metadata)
