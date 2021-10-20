#undef NDEBUG
#include "trackhat.hpp"
#include "compat/sleep.hpp"
#include "compat/timer.hpp"
#include <cassert>

bool camera_handle::ensure_connected()
{
    if (state_ >= st_streaming)
        return true;
    else if (state_ == st_stopped)
        return false;

    Timer t;

    constexpr int max_attempts = 5;

    if (!ensure_device_exists())
        goto error;

    for (int i = 0; i < max_attempts; i++)
    {
        if (!th_check(trackHat_Connect(&device_, TH_FRAME_EXTENDED)))
        {
            state_ = st_streaming;
            if (int ms = (int)t.elapsed_ms(); ms > 1000)
                qDebug() << "tracker/trackhat: connecting took" << ms << "ms";
            return true;
        }

        auto dbg = qDebug();
        dbg << "tracker/trackhat: connect failed, retry";
        dbg.space(); dbg.nospace();
        dbg << (i+1) << "/" << max_attempts;
        portable::sleep(50);
    }

error:
    disconnect();
    return false;
}

bool camera_handle::ensure_device_exists()
{
    switch (state_)
    {
    case st_streaming:
        return true;
    case st_detected:
        disconnect();
        [[fallthrough]];
    case st_stopped:
        assert(!th_check(trackHat_Initialize(&device_)) && device_.m_pInternal);
        if (auto error = th_check(trackHat_DetectDevice(&device_)); error)
        {
            disconnect();
            return false;
        }
        state_ = st_detected;
        return true;
    }
}

void camera_handle::disconnect()
{
    state_ = st_stopped;
    if (device_.m_pInternal)
    {
        (void)!th_check(trackHat_Disconnect(&device_));
        trackHat_Deinitialize(&device_);
    }
}
