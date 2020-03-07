#include "shm-layout.hpp"
#include "shm.hpp"

#include "ps3eye-driver/ps3eye.hpp"

#include <thread>
#include <chrono>
#include <cstdlib>

#ifdef __clang__
#   pragma clang diagnostic ignored "-Watomic-implicit-seq-cst"
#endif

#ifdef __GNUG__
#   pragma GCC diagnostic ignored "-Wcast-qual"
#   pragma GCC diagnostic ignored "-Wformat-security"
#   pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif

#ifdef _MSC_VER
#   include <windows.h>
#   define FULL_BARRIER [](){ _ReadWriteBarrier(); MemoryBarrier(); }
#else
#   define FULL_BARRIER __sync_synchronize
#endif

template<int N, typename... xs>
[[noreturn]]
static void error(volatile ps3eye::shm_out& out, const char (&error)[N], const xs&... args)
{
    snprintf((char*)out.error_string, sizeof(ps3eye::shm_out::error_string), error, args...);
    std::quick_exit(2);
}

static void update_settings(ps3eye::camera& camera, const volatile ps3eye::shm_in& in)
{
    // TODO
    //camera.set_framerate(in.framerate);
    camera.set_auto_gain(in.auto_gain);
    camera.set_gain(in.gain);
    camera.set_exposure(in.exposure);
    camera.set_test_pattern_status(in.test_pattern);
}

int main(int argc, char** argv)
{
    (void)argc; (void)argv;
    shm_wrapper mem_("ps3eye-driver-shm", nullptr, sizeof(ps3eye::shm));
    volatile auto& ptr_ = *(ps3eye::shm*)mem_.ptr();
    volatile auto& in = ptr_.in;
    volatile auto& out = ptr_.out;

    auto cameras = ps3eye::list_devices();

    if (cameras.empty())
        error(out, "no camera found");

    auto& camera = cameras[0];
    camera->set_debug(false);
    uint8_t* frame;
    decltype(out.timecode) timecode = 0;

    {
        ps3eye::resolution mode;
        switch (in.resolution)
        {
        case ps3eye::shm_in::mode::qvga:
            mode = ps3eye::res_QVGA;
            frame = (uint8_t*)out.data_320x240;
            break;
        case ps3eye::shm_in::mode::vga:
            mode = ps3eye::res_VGA;
            frame = (uint8_t*)out.data_640x480;
            break;
        default: error(out, "wrong resolution %u", (unsigned)mode);
        }

        int framerate = in.framerate;
        if (!framerate)
            framerate = 60;

        if (!camera->init(mode, framerate))
            error(out, "camera init failed: %s", camera->error_string());

        update_settings(*camera, in);

        if (!camera->start())
            error(out, "can't start camera: %s", camera->error_string());
    }

    out.timecode = 0;
    in.do_exit = false;
    FULL_BARRIER();

    for (;;)
    {
        {
            auto cookie = in.settings_updated;
            if (cookie != out.settings_updated_ack)
            {
                update_settings(*camera, in);
                out.settings_updated_ack = cookie;
            }
        }

        if (!camera->get_frame(frame))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{4});
            continue;
        }

        out.timecode = ++timecode;

        if (in.do_exit)
            break;

        FULL_BARRIER();
    }

    return 0;
}
