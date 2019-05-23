#include "ftnoir_tracker_edtracker.h"
#include "api/plugin-api.hpp"
#include "compat/math.hpp"
#include <cinttypes>
#include <algorithm>
#include <cmath>

static const int max_value[] = {
    100,
    100,
    100,
    180,
    90,
    180
};

tracker_edtracker::tracker_edtracker() : pose { 0, 0, 0, 0, 0, 0 } {
}

tracker_edtracker::~tracker_edtracker() {
    requestInterruption();
    wait();
    if (idevice > 0) close(idevice);
}

module_status tracker_edtracker::start_tracker(QFrame *) {
    QString device = QString("/dev/input/js%1").arg(s.dev);
    idevice = open(device.toUtf8().data(), O_RDONLY | O_NONBLOCK);
    if (idevice == -1) error("EDTracker not found");
    notifier = new QSocketNotifier(idevice, QSocketNotifier::Read);
    notifier->setEnabled(true);
    QObject::connect(notifier, SIGNAL(activated(int)), this, SLOT(readyRead(int)));

    return status_ok();
}

void tracker_edtracker::data(double *data) {
    int values[] = {
        0,
        90,
        -90,
        180,
        -180,
    };
    int indices[] = {
        s.add_yaw,
        s.add_pitch,
        s.add_roll,
    };

    data[Yaw] = pose[3 + s.idx_x];
    data[Pitch] = pose[3 + s.idx_y];
    data[Roll] = pose[3 + s.idx_z];

    // add offset (values) for each pose
    for (int i = 0; i < 3; i++) {
        const int k = indices[i];
        if (k >= 0 && k < std::distance(std::begin(values), std::end(values)))
            data[Yaw + i] += values[k];
    }
}

void tracker_edtracker::readyRead(int /*socket*/) {
    struct js_event jev;
    read(idevice, &jev, sizeof (jev));
    if ((jev.type & ~JS_EVENT_INIT) == JS_EVENT_AXIS) {
        pose[3 + jev.number] = double_t(max_value[3 + jev.number] * jev.value / 32678);
    }
}

OPENTRACK_DECLARE_TRACKER(tracker_edtracker, dialog_edtracker, meta_edtracker)
