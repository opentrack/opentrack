#ifndef __TRACKER_H__
#define __TRACKER_H__

#include <QThread>
#include <QMessageBox>
#include <QLineEdit>
#include <QPoint>
#include <QWaitCondition>
#include <QList>
#include <QPainterPath>
#include <QDebug>
#include <QMutex>
#include "plugin-support.h"
#include "mappings.hpp"

#include <vector>
#include <atomic>

#include <qfunctionconfigurator/functionconfig.h>
#include "tracker_types.h"
#include "facetracknoir/main-settings.hpp"
#include "facetracknoir/options.h"
#include "facetracknoir/timer.hpp"



class Tracker : protected QThread {
    Q_OBJECT
private:
    QMutex mtx;
    main_settings& s;
    // XXX can be const-cast when functionconfig const-correct -sh 20141004
    Mappings& m;
    Timer t;
    T6DOF output_pose, raw_6dof;
    std::atomic<bool> centerp;
    std::atomic<bool> enabledp;
    std::atomic<bool> should_quit;

    static void get_curve(double pos, double& out, Mapping& axis);
protected:
    void run() override;
public:
    Tracker(main_settings& s, Mappings& m);
    ~Tracker();

    void get_raw_and_mapped_poses(double* mapped, double* raw) const;
    void start() { QThread::start(); }
    void center() { centerp.store(true); }
    void toggle_enabled() { enabledp.store(!enabledp.load()); }
};
#endif
