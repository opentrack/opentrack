#include "module.hpp"
#include "compat/library-path.hpp"
#include "compat/sleep.hpp"
#include "compat/run-in-thread.hpp"

#include <cstddef>

#include <QCoreApplication>
#include <QMessageBox>

#include <libusb.h>

#ifdef __GNUG__
#   pragma clang diagnostic ignored "-Wcast-qual"
#endif

int device_count()
{
    libusb_context * ctx = nullptr;
    libusb_device **list = nullptr;
    ssize_t sz = 0;
    int rc = 0, cnt = 0;

    constexpr int vendor_id = 0x1415;
    constexpr int product_id = 0x2000;

    rc = libusb_init(&ctx);

    if (rc)
        goto end;

    sz = libusb_get_device_list(ctx, &list);

    if (sz < 0)
        goto end;

    for (int i = 0; i < sz; ++i) {
        libusb_device *device = list[i];
        libusb_device_descriptor desc = {};

        if (libusb_get_device_descriptor(device, &desc))
            goto end;

        if (desc.idVendor == vendor_id && desc.idProduct == product_id)
            cnt++;
    }

end:
    if (list)
        libusb_free_device_list(list, 1);
    if (ctx)
        libusb_exit(ctx);

    return cnt;
}

bool check_device_exists()
{
    static bool ret = device_count() > 0;
    return ret;
}

static const QString camera_name = QStringLiteral("PS3 Eye open driver");

std::vector<QString> ps3eye_camera_::camera_names() const
{
    if (check_device_exists())
        return { camera_name };
    else
        return {};
}
std::unique_ptr<camera> ps3eye_camera_::make_camera(const QString& name)
{
    if (name == camera_name && check_device_exists())
        return std::make_unique<ps3eye_camera>();
    else
        return {};
}
bool ps3eye_camera_::show_dialog(const QString&)
{
    // TODO
    return false;
}
bool ps3eye_camera_::can_show_dialog(const QString&)
{
    return false;
}

ps3eye_camera::ps3eye_camera()
{
    if (!shm.success())
        return;

    static const QString library_path(OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH);

    wrapper.setWorkingDirectory(library_path);
#ifdef _WIN32
    wrapper.setProgram("\"ps3eye-subprocess.exe\"");
#else
    wrapper.setProgram("ps3eye-subprocess");
#endif
}

ps3eye_camera::~ps3eye_camera()
{
    stop();
}

void ps3eye_camera::stop()
{
    open = false;

    if (wrapper.state() != QProcess::NotRunning)
    {
        if (wrapper.state() != QProcess::NotRunning)
            wrapper.kill();
        wrapper.waitForFinished(1000);
    }
}

bool ps3eye_camera::start(info& args)
{
    if (!shm.success())
        return false;

    volatile auto& ptr = *(ps3eye::shm*)shm.ptr();

    using mode = ps3eye::shm_in::mode;

    open = false;
    fr = {};
    fr.channels = 3;
    fr.channel_size = 1;

    if (!args.width || args.width > 320)
    {
        ptr.in.resolution = mode::vga;
        fr.width = 640; fr.height = 480;
    }
    else
    {
        ptr.in.resolution = mode::qvga;
        fr.width = 320; fr.height = 240;
    }

    ptr.in.framerate = (uint8_t)std::clamp(args.fps, 30, 187);
    ptr.in.gain = (uint8_t)s.gain;
    ptr.in.exposure = (uint8_t)s.exposure;

    wrapper.start();

    constexpr int sleep_ms = 10, max_sleeps = 5000/sleep_ms;

    for (int i = 0; i < max_sleeps; i++)
    {
        if (ptr.out.timecode > 0)
            goto ok;
        portable::sleep(sleep_ms);
    }

    run_in_thread_async(qApp, [&]() {
        QString error;
        if (ptr.out.error_string[0] == '\0')
            error = "Unknown error";
        else
            error = QString::fromLatin1((const char*)ptr.out.error_string,
                                        strnlen((const char*)ptr.out.error_string, sizeof(ptr.out.error_string)));

        QMessageBox::critical(nullptr, "Can't open camera", "PS3 Eye driver error: " + error, QMessageBox::Close);
    });

    return false;

ok:
    open = true;
    return true;
}

std::tuple<const frame&, bool> ps3eye_camera::get_frame()
{
    volatile auto& ptr = *(ps3eye::shm*)shm.ptr();
    constexpr int sleep_ms = 2;
    constexpr int max_sleeps = 2000/sleep_ms;

    if (!open)
        goto fail;

    for (int i = 0; i < max_sleeps; i++)
    {
        unsigned new_timecode = ptr.out.timecode;
        if (timecode != new_timecode)
        {
            timecode = new_timecode;
            goto ok;
        }
        portable::sleep(sleep_ms);
    }

fail:
    stop();
    return { fr, false };

    static_assert(offsetof(decltype(ptr.out), data_640x480) == offsetof(decltype(ptr.out), data_320x240));

ok:
    memcpy(data, (unsigned char*)ptr.out.data_640x480,sizeof(ptr.out.data_640x480));
    fr.data = data;
    return { fr, true};
}

bool ps3eye_camera::show_dialog()
{
    return false;
}

OTR_REGISTER_CAMERA(ps3eye_camera_)
