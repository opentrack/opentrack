#pragma once
#include "ui_tracker_xreal_one_controls.h"

#include <QTcpSocket>
#include <QThread>

#include "api/plugin-api.hpp"
#include "options/options.hpp"

// Need to include Fusion.h header since it wraps into extern "C" all other includes
#include "Fusion.h"

struct XRealOneIMUFrame;

using namespace options;

struct settings : opts
{
    settings(): opts("xreal-one-tracker")
    {
    }

    // TCP port of the XReal One device in own network
    value<int> port = { b, "port", 52998 };

    // Default IP address of the XReal One device in own network
    value<QString> ip_address = { b, "ip-address", "169.254.2.1" };

    // Gravitational constant in m/s² used to convert accelerometer data from m/s² to g
    value<double> g_constant = { b, "g-constant", 9.80665 };

    // Use Fusion Offset correction algorithm for gyroscope data
    value<bool> use_fusion_offset = { b, "use-fusion-offset", true };

    // Expected packet header in hexadecimal string format, probably this may change in future firmware versions
    value<QString> header_value = { b, "header-value", "283600000080" };

    // Not configurable by user, used to track if any setting changed
    value<bool> changed = { b, "changed", false };
};

class XRealOneTracker : protected QThread, public ITracker
{
    // Average sample rate of the XReal One IMU in Hz (1000 samples per second)
    static constexpr int AvgSampleRateHz = 1000;

    Q_OBJECT

public:
    XRealOneTracker();
    ~XRealOneTracker() override;

public:
    module_status start_tracker(QFrame *) override;

    void data(double *data) override;

protected:
    void run() override;

private:
    // What became of a frame
    enum class FrameResult
    {
        Used,              // a gyro/accel sample that reached Fusion
        BadHeader,
        Magnetometer,
        OtherType,         // flags that mark neither a gyro/accel nor a magnetometer frame
        NotFinite,         // a gyro/accel sample with a value that isn't finite
        RepeatedTimestamp, // a gyro/accel sample with the timestamp of the previous one
    };

    // Frames since the last sample that reached Fusion, by why they didn't
    struct UnusedFrames
    {
        qint64 total = 0;
        qint64 bad_header = 0;
        qint64 magnetometer = 0;
        qint64 other_type = 0;
        qint64 not_finite = 0;
        qint64 repeated_timestamp = 0;
        uint32_t last_other_flags = 0;
        bool warned = false;
    };

    // Frames in a row without a usable gyro/accel sample before warning, about 1 s of the glasses' frames
    static constexpr qint64 WarnAfterFrames = 1400;

    // Parse a received data into XReal IMU Frame data and process it if valid and contains gyroscope and accelerometer data
    void parse_packet(const QByteArray& packet);

    // Process XReal IMU Frame containing gyroscope and accelerometer data
    FrameResult process_ga_packet(const XRealOneIMUFrame& frame);

    // Count a frame, and warn once when WarnAfterFrames frames in a row had no usable gyro/accel sample
    void count_frame(FrameResult result, uint32_t flags);

    // Initialize Fusion AHRS and Offset algorithms and reset the per-connection state (time step, frame counts)
    // Called after each connect
    void initialize_fusion_ahrs();

    // Update internal settings from user-configurable options
    void update_from_settings(bool force = false);

    // Validate the frame header against expected header
    bool is_header_valid(const XRealOneIMUFrame& frame) const;

private:
    QTcpSocket sock;

    settings s;

    // Gyroscope offset correction algorithm
    FusionOffset offset;

    // Attitude and Heading Reference System (AHRS) algorithm
    FusionAhrs ahrs;

    // Last known orientation in Euler angles
    FusionEuler euler_angles;

    // Last received packet timestamp
    uint64_t last_timestamp_ns = 0;

    // Frames since the last usable gyro/accel sample
    UnusedFrames unused;

    // Expected header value, copied from default value in constructor
    // Updated from settings if changed
    uint8_t header[6] = {0};
};


class XRealOneSettingsDialog: public ITrackerDialog
{
    Q_OBJECT
public:
    XRealOneSettingsDialog();

    void register_tracker(ITracker *) override {}

    void unregister_tracker() override {}

private:
    Ui::UIXROneClientControls ui;

    settings s;

private slots:
    void doOK();
    void doCancel();
};


class XRealOneMetadata : public Metadata
{
    Q_OBJECT

    QString name() { return tr("XReal One Tracker (TCP)"); }

    QIcon icon() { return QIcon(":/images/xreal-dev-128.png"); }
};

