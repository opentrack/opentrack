/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "steamvr.hpp"
#include "api/plugin-api.hpp"
#include "compat/util.hpp"
#include "compat/euler.hpp"

#include <cstdlib>
#include <cmath>

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
        using m = float[3][4];
        vr::TrackedDevicePose_t devices[vr::k_unMaxTrackedDeviceCount] = {};

        vr->GetDeviceToAbsoluteTrackingPose(vr::ETrackingUniverseOrigin::TrackingUniverseStanding, 0,
                                            devices, vr::k_unMaxTrackedDeviceCount);

        for (unsigned k = 0; k < vr::k_unMaxTrackedDeviceCount; k++)
        {
            using namespace euler;

            if (!devices[k].bPoseIsValid)
                continue;

            if (vr->GetTrackedDeviceClass(k) != vr::ETrackedDeviceClass::TrackedDeviceClass_HMD)
                continue;

            const m& M = devices[k].mDeviceToAbsoluteTracking.m;

            static constexpr double c[3] { -1, 1, -1 };

            for (unsigned i = 0; i < 3; i++)
                data[i] = double(M[i][3]) * c[i] * 100;

            static constexpr unsigned indices[3] = {Roll, Yaw, Pitch};

            rmat r(M[0][0], M[1][0], M[2][0],
                   M[0][1], M[1][1], M[2][1],
                   M[0][2], M[1][2], M[2][2]);

            euler_t ypr(rmat_to_euler(r));

            for (unsigned i = 0; i < 3; i++)
                data[indices[i]] = 180/M_PI * ypr(i);

            for (unsigned i = 0; i < 3; i++)
                data[i+3] *= c[i];

            goto done;
        }

        qDebug() << "steamvr: no device with pose found";

done:
        (void)0; // expects statement after label
    }
}

void dialog::register_tracker(ITracker*) {}
void dialog::unregister_tracker() {}

OPENTRACK_DECLARE_TRACKER(steamvr, dialog, metadata)
