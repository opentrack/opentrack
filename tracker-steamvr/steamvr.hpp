#pragma once

#include "ui_dialog.h"
#include "ui_calibration-dialog.h"
#include "api/plugin-api.hpp"
#include "options/options.hpp"

#include <tuple>
#include <climits>

#include <QString>
#include <QRecursiveMutex>
#include <QList>
#include <QTimer>

#include <openvr.h>

using namespace options;

using vr_error_t = vr::EVRInitError;
using vr_t = vr::IVRSystem*;

using tt = std::tuple<vr_t, vr_error_t>;
using pose_t = vr::TrackedDevicePose_t;
using origin = vr::ETrackingUniverseOrigin;

struct settings : opts
{
    value<QVariant> device_serial;
    value<QVariantList> calibration_matrix;
    value<bool> calibration_enabled;
    settings() :
        opts("valve-steamvr"),
        device_serial(b, "serial", QVariant(QVariant::String)),
        calibration_matrix(b, "calibration-matrix", QVariantList()),
        calibration_enabled(b, "calibration-enabled", false)
    {}
};

struct device_spec
{
    vr::TrackedDevicePose_t pose;
    QString model, serial, type;
    unsigned k;
    QString to_string() const;
    bool is_connected;
};

struct device_list final
{
    using maybe_pose = std::tuple<bool, pose_t>;

    device_list();
    void refresh_device_list();
    const QList<device_spec>& devices() const { return device_specs; }

    static tr_never_inline maybe_pose get_pose(int k);
    static QString error_string(vr_error_t error);
    static constexpr unsigned max_devices = vr::k_unMaxTrackedDeviceCount;

    template<typename F>
    friend auto with_vr_lock(F&& fun) -> decltype(fun(vr_t(), vr_error_t()));

private:
    QList<device_spec> device_specs;
    static QRecursiveMutex mtx;
    static tt vr_init_();
    static void fill_device_specs(QList<device_spec>& list);
    static tt vr_init();
};

class steamvr : public QObject, public ITracker
{
    Q_OBJECT

public:
    steamvr();
    ~steamvr() override;
    module_status start_tracker(QFrame *) override;
    void data(double *data) override;
    bool center() override;

    static void matrix_to_euler(double& yaw, double& pitch, double& roll, const vr::HmdMatrix34_t& result);

    struct mat34
    {
        double m[3][4] {};
    };

    static void matrix_to_euler(double& yaw, double& pitch, double& roll, const mat34& mat);

    static mat34 identity_matrix();
    static mat34 from_vr_matrix(const vr::HmdMatrix34_t& mat);
    static vr::HmdMatrix34_t to_vr_matrix(const mat34& mat);
    static mat34 multiply(const mat34& a, const mat34& b);
    static QVariantList to_variant_list(const mat34& mat);
    static bool from_variant_list(const QVariantList& list, mat34& out);

    settings s;
    unsigned device_index{UINT_MAX};
    mat34 calibration_offset{ identity_matrix() };
    bool calibration_ready{false};
    QRecursiveMutex calibration_mtx;
};

class calibration_recording_dialog : public QDialog
{
    Q_OBJECT
public:
    calibration_recording_dialog(unsigned device_idx, const steamvr::mat34& base_pose, QWidget* parent);

    bool run(steamvr::mat34& out_offset);

private slots:
    void on_startButton_clicked();
    void on_finishButton_clicked();
    void on_cancelButton_clicked();
    void on_sample_timer();

private:
    void update_ui();
    bool check_coverage() const;

    Ui::CalibrationDialog ui;
    QTimer* sample_timer;

    unsigned device_idx;
    steamvr::mat34 base_pose;

    struct sample_data
    {
        steamvr::mat34 pose;
        double yaw;
        double pitch;
    };
    std::vector<sample_data> samples;

    // Tracking coverage
    double min_yaw = 0;
    double max_yaw = 0;
    double min_pitch = 0;
    double max_pitch = 0;

    bool finished_successfully = false;

    // Calibration parameters
    static constexpr int sample_interval_ms = 20; // 50Hz sampling
    static constexpr size_t min_samples = 30;
    static constexpr double min_yaw_range_deg = 60.0;  // ±30° from center
    static constexpr double min_pitch_range_deg = 40.0; // ±20° from center
};

class steamvr_dialog : public ITrackerDialog
{
    Q_OBJECT
public:
    steamvr_dialog();

private:
    Ui::dialog ui;
    settings s;

    void update_calibration_label();
    void run_calibration_wizard();

private slots:
    void doOK();
    void doCancel();
    void doRunCalibration();
    void doClearCalibration();
};

class steamvr_metadata : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("Valve SteamVR"); }
    QIcon icon() override { return QIcon(":/images/rift_tiny.png"); }
};
