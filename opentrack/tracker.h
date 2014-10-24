#pragma once

#include <atomic>
#include <vector>

#include "./timer.hpp"
#include "./plugin-support.h"
#include "./mappings.hpp"
#include "./pose.hpp"

#include "../qfunctionconfigurator/functionconfig.h"
#include "./main-settings.hpp"
#include "./options.hpp"

#include <QMutex>
#include <QThread>

class Tracker : private QThread {
    Q_OBJECT
private:
    QMutex mtx;
    main_settings& s;
    Mappings& m;

    Timer t;
    Pose output_pose, raw_6dof, final_raw;

    double newpose[6];
    std::atomic<bool> centerp;
    std::atomic<bool> enabledp;
    std::atomic<bool> should_quit;
    SelectedLibraries const& libs;

    Quat r_b;
    double t_b[3];

    double map(double pos, Mapping& axis);
    void logic();

    static void t_compensate(const double* input, double* output, bool rz);
    void run() override;
public:
    Tracker(main_settings& s, Mappings& m, SelectedLibraries& libs);
    ~Tracker();

    void get_raw_and_mapped_poses(double* mapped, double* raw) const;
    void start() { QThread::start(); }
    void toggle_enabled() { enabledp.store(!enabledp.load()); }
    void center() { centerp.store(!centerp.load()); }
};
