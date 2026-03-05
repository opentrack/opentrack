#pragma once
#include "api/plugin-api.hpp"
#include "options/options.hpp"
#include "ui_openhmd-dialog.h"

#include <openhmd.h>
#include <QMutex>
#include <QThread>

using namespace options;

struct openhmd_settings : opts {
    value<int> device_index;
    value<bool> invert_yaw;
    value<bool> invert_pitch;
    value<bool> invert_roll;
    openhmd_settings() :
        opts("openhmd-tracker"),
        device_index(b, "device-index", 0),
        invert_yaw(b, "invert-yaw", false),
        invert_pitch(b, "invert-pitch", false),
        invert_roll(b, "invert-roll", false)
    {}
};

class openhmd_tracker : protected QThread, public ITracker
{
    Q_OBJECT
public:
    openhmd_tracker();
    ~openhmd_tracker() override;
    module_status start_tracker(QFrame *frame) override;
    void data(double *data) override;
    
protected:
    void run() override;
    
private:
    // Deterministic conversion: OpenHMD quat -> OpenGL matrix -> OpenTrack frame -> Euler
    static void quat_to_euler(const float q[4], double &yaw, double &pitch, double &roll);
    
    ohmd_context* ctx = nullptr;
    ohmd_device* device = nullptr;
    double last_pose[6] = {0, 0, 0, 0, 0, 0};
    QMutex mutex;
    openhmd_settings s;
    volatile bool should_quit = false;
};

class openhmd_dialog : public ITrackerDialog
{
    Q_OBJECT
public:
    openhmd_dialog();
    void register_tracker(ITracker *tracker) override;
    void unregister_tracker() override;
    
private:
    Ui::openhmd_dialog ui;
    openhmd_settings s;
    
private slots:
    void doOK();
    void doCancel();
};

class openhmd_metadata : public Metadata
{
    Q_OBJECT
    QString name() { return tr("OpenHMD head tracker"); }
    QIcon icon() { return QIcon(":/images/rift_tiny.png"); }
};
