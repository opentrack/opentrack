#include "steamvr.hpp"
#include "api/plugin-api.hpp"
#include "compat/util.hpp"

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
                             tr("Valve SteamVR init error"), strerror(e),
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
                             tr("Valve SteamVR init warning"),
                             tr("No HMD connected"),
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

        for (unsigned k = 0; k < vr::k_unMaxTrackedDeviceCount; k++)
        {
            if (!devices[k].bPoseIsValid)
                continue;

            if (vr->GetTrackedDeviceClass(k) != vr::ETrackedDeviceClass::TrackedDeviceClass_HMD)
                continue;

            const vr::HmdMatrix34_t& result = devices[k].mDeviceToAbsoluteTracking;

            int c = 10;

            data[TX] = -result.m[0][3] * c;
            data[TY] = result.m[1][3] * c;
            data[TZ] = result.m[2][3] * c;

            using std::atan2;
            using std::asin;

            // transformation matrix to euler angles
            if (result.m[0][0] == 1.0f || result.m[0][0] == -1.0f)
            {
                data[Yaw] = -atan2(result.m[0][2], result.m[2][3]);
                data[Pitch] = 0;
                data[Roll] = 0;
            }
            else
            {
                data[Yaw] = -atan2(-result.m[2][0], result.m[0][0]);
                data[Pitch] = atan2(-result.m[1][2], result.m[1][1]);
                data[Roll] = asin(result.m[1][0]);
            }

            static constexpr double r2d = 180 / M_PI;

            data[Yaw] *= r2d;
            data[Pitch] *= r2d;
            data[Roll] *= r2d;

            break;
        }
    }
}

void steamvr_dialog::register_tracker(ITracker*) {}
void steamvr_dialog::unregister_tracker() {}

OPENTRACK_DECLARE_TRACKER(steamvr, steamvr_dialog, steamvr_metadata)
