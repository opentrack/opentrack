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

void steamvr::data(double* data)
{
    if (vr)
    {
        vr::TrackedDevicePose_t devices[vr::k_unMaxTrackedDeviceCount] = {};

        vr->GetDeviceToAbsoluteTrackingPose(vr::ETrackingUniverseOrigin::TrackingUniverseSeated, 0,
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

            using std::atan2;
            using std::asin;

            //const euler_t rot = get_ypr(s.order);

            static constexpr double r2d = 180/M_PI;

            //for (unsigned i = 3; i < 6; i++)
            //    data[i] = r2d * rot[i];

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
