#pragma once

#include "ui_dialog.h"
#include "api/plugin-api.hpp"
#include "options/options.hpp"

#include <tuple>
#include <climits>

#include <QString>
#include <QRecursiveMutex>
#include <QList>

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
    settings() :
        opts("valve-steamvr"),
        device_serial(b, "serial", QVariant(QVariant::String))
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

    static void matrix_to_euler(double& yaw, double& pitch, double& roll, const vr::HmdMatrix34_t& result);

    settings s;
    unsigned device_index{UINT_MAX};

public:
    steamvr();
    ~steamvr() override;
    module_status start_tracker(QFrame *) override;
    void data(double *data) override;
    bool center() override;
};

class steamvr_dialog : public ITrackerDialog
{
    Q_OBJECT
public:
    steamvr_dialog();

private:
    Ui::dialog ui;
    settings s;

private slots:
    void doOK();
    void doCancel();
};

class steamvr_metadata : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("Valve SteamVR"); }
    QIcon icon() override { return QIcon(":/images/rift_tiny.png"); }
};
