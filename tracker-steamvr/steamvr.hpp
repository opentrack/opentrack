#pragma once
#include "api/plugin-api.hpp"
#include "ui_dialog.h"
#include "compat/util.hpp"
#include "options/options.hpp"

#include "compat/euler.hpp"

#include <openvr.h>

#include <cmath>
#include <memory>
#include <tuple>

#include <QString>
#include <QMutex>
#include <QMutexLocker>
#include <QList>

using namespace options;
using error_t = vr::EVRInitError;
using vr_t = vr::IVRSystem*;

using tt = std::tuple<vr_t, error_t>;
using pose_t = vr::TrackedDevicePose_t;
using origin = vr::ETrackingUniverseOrigin;

struct settings : opts
{
    value<QString> device_serial;
    settings() :
        opts("valve-steamvr"),
        device_serial(b, "serial", "")
    {}
};

struct device_spec
{
    vr::TrackedDevicePose_t pose;
    QString model, serial;
    unsigned k;
};

struct device_list final
{
    using maybe_pose = std::tuple<bool, pose_t>;

    device_list();
    void refresh_device_list();
    const QList<device_spec>& devices() const & { return device_specs; }

    static OTR_NEVER_INLINE maybe_pose get_pose(int k);
    static bool get_all_poses(pose_t*poses);
    static QString strerror(error_t error);
    static constexpr int max_devices = int(vr::k_unMaxTrackedDeviceCount);

    template<typename F>
    friend static auto with_vr_lock(F&& fun) -> decltype(fun(vr_t(), error_t()));

private:
    QList<device_spec> device_specs;
    static QMutex mtx;
    static tt vr_init_();
    static void vr_deleter();
    static void fill_device_specs(QList<device_spec>& list);
    static tt vr_init();
};

class steamvr : public QObject, public ITracker
{
    Q_OBJECT

    using error_t = vr::EVRInitError;
    using vr_t = vr::IVRSystem*;

public:
    steamvr();
    ~steamvr() override;
    void start_tracker(QFrame *) override;
    void data(double *data) override;
    bool center() override;

private:
    void matrix_to_euler(double &yaw, double &pitch, double &roll, const vr::HmdMatrix34_t& result);

    settings s;
    int device_index;

    using rmat = euler::rmat;
    using euler_t = euler::euler_t;
};

class steamvr_dialog : public ITrackerDialog
{
    Q_OBJECT
public:
    steamvr_dialog();

private:
    Ui::dialog ui;
    settings s;
    device_list devices;

private slots:
    void doOK();
    void doCancel();
};

class steamvr_metadata : public Metadata
{
public:
    QString name() { return QString(QCoreApplication::translate("steamvr_metadata", "Valve SteamVR")); }
    QIcon icon() { return QIcon(":/images/rift_tiny.png"); }
};

