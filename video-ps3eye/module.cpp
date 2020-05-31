#include "module.hpp"
#include "compat/library-path.hpp"
#include "compat/sleep.hpp"
#include "compat/run-in-thread.hpp"

#include <cstddef>
#include <thread>

#include <QCoreApplication>
#include <QMessageBox>

#include <libusb.h>

using namespace options;

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

#ifdef __linux__
#   include <unistd.h>
#endif

bool check_device_exists()
{
#ifdef __linux__
    // don't show when system driver exists
    if (!access("/sys/module/gspca_ov534", R_OK|X_OK))
        return false;
#endif

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

static bool show_dialog_()
{
    (new dialog)->show();
    return true;
}

bool ps3eye_camera_::show_dialog(const QString&)
{
    return show_dialog_();
}

bool ps3eye_camera_::can_show_dialog(const QString& name)
{
    return name == camera_name && check_device_exists();
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

    ptr.in.auto_gain = false;
    ptr.in.framerate = (uint8_t)std::clamp(args.fps, 30, 187);
    ptr.in.gain = (uint8_t)s.gain;
    ptr.in.exposure = (uint8_t)s.exposure;

    sleep_ms = (int)std::ceil(1000./std::max(1, (int)ptr.in.framerate));

    wrapper.start();

    constexpr int sleep_ms = 10, max_sleeps = 5000/sleep_ms;

    for (int i = 0; i < max_sleeps; i++)
    {
        if (ptr.out.timecode > 0)
            goto ok;
        portable::sleep(sleep_ms);
    }

    run_in_thread_sync(qApp, [&]() {
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
    auto volatile* ptr = (ps3eye::shm*)shm.ptr();

    if (shm.success() && open)
    {
        int elapsed = std::min((int)std::ceil(t.elapsed_ms()), 100);
        portable::sleep(sleep_ms - elapsed);

        if (unsigned tc = ptr->out.timecode; tc != timecode)
        {
            timecode = tc;
            goto ok;
        }
    }

    for (int i = 0; i < 2000; i++)
    {
        if (unsigned tc = ptr->out.timecode; tc != timecode)
        {
            timecode = tc;
            goto ok;
        }
        portable::sleep(1);
    }

    stop();
    return { fr, false };

    static_assert(offsetof(decltype(ptr->out), data_640x480) == offsetof(decltype(ptr->out), data_320x240));

ok:
    t.start();
    memcpy(data, (unsigned char*)ptr->out.data_640x480,sizeof(ptr->out.data_640x480));
    fr.data = data;
    return { fr, true };
}

bool ps3eye_camera::show_dialog()
{
    return show_dialog_();
}

OTR_REGISTER_CAMERA(ps3eye_camera_)

dialog::dialog(QWidget* parent) : QWidget(parent)
{
    ui.setupUi(this);
    t.setSingleShot(true);
    t.setInterval(500);
    tie_setting(s.exposure, ui.exposure_slider);
    tie_setting(s.gain, ui.gain_slider);
    ui.exposure_label->setValue((int)*s.exposure);
    ui.gain_label->setValue((int)*s.gain);
    connect(&s.exposure, value_::value_changed<slider_value>(), this, [this](const slider_value&) { s.set_exposure(); t.stop(); t.start(); });
    connect(&s.gain, value_::value_changed<slider_value>(), this, [this](const slider_value&) { s.set_gain(); t.stop(); t.start(); });
    connect(ui.exposure_slider, &QSlider::valueChanged, ui.exposure_label, &QSpinBox::setValue);
    connect(ui.gain_slider, &QSlider::valueChanged, ui.gain_label, &QSpinBox::setValue);
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &dialog::do_ok);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &dialog::do_cancel);
    connect(&t, &QTimer::timeout, this, [this]() { s.apply(); });
}

// XXX copypasta -sh 20200329
void settings::set_gain()
{
    if (!shm.success())
        return;

    auto& ptr = *(ps3eye::shm volatile*)shm.ptr();
    ptr.in.gain = (unsigned char)*gain;
    //++ptr.in.settings_updated;
    std::atomic_thread_fence(std::memory_order_seq_cst);
}

void settings::set_exposure()
{
    if (!shm.success())
        return;

    auto& ptr = *(ps3eye::shm volatile*)shm.ptr();
    ptr.in.exposure = (unsigned char)*exposure;
    //++ptr.in.settings_updated;
    std::atomic_thread_fence(std::memory_order_seq_cst);
}

void settings::apply()
{
    if (!shm.success())
        return;

    auto& ptr = *(ps3eye::shm volatile*)shm.ptr();
    ++ptr.in.settings_updated;
    std::atomic_thread_fence(std::memory_order_seq_cst);
}
