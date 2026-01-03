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
    value<QString> ip_address = { b, "ip_address", "169.254.2.1" };

    // Gravitational constant in m/s² used to convert accelerometer data from m/s² to g
    value<double> g_constant = { b, "g_constant", 9.80665 };

    // Use Fusion Offset correction algorithm for gyroscope data
    value<bool> use_fusion_offset = { b, "use_fusion_offset", true };

    // Expected packet header in hexadecimal string format, probably this may change in future firmware versions
    value<QString> header_value = { b, "header_value", "283600000080" };

    // Gyroscope and Accelerometer data type marker, probably this may change in future firmware versions
    value<QString> ga_marker_value = { b, "ga_marker_value", "00401f000040" };

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
    // Parse a received data into XReal IMU Frame data and process it if valid and contains gyroscope and accelerometer data
    void parse_packet(const QByteArray& packet);

    // Process XReal IMU Frame containing gyroscope and accelerometer data
    void process_ga_packet(const XRealOneIMUFrame& frame);

    // Initialize Fusion AHRS and Offset algorithms, reset state to default values
    // Called on before connect
    void initialize_fusion_ahrs();

    // Update internal settings from user-configurable options
    void update_from_settings(bool force = false);

    // Validate the frame header against expected header
    bool is_header_valid(const XRealOneIMUFrame& frame) const;

    // Determine if the frame contains gyroscope and accelerometer data
    bool is_gyro_accel_data(const XRealOneIMUFrame& frame) const;

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

    // Expected header and data type values, copied from default values in constructor
    // Updated from settings if changed
    uint8_t header[6] = {0};

    // Expected gyro and accelerometer data type marker, copied from default value in constructor
    // Updated from settings if changed
    uint8_t ga_marker[6] = {0};
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

