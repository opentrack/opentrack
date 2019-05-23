#include "ftnoir_tracker_edtracker.h"
#include "api/plugin-api.hpp"

dialog_edtracker::dialog_edtracker() {

    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    updateDevices();

    tie_setting(s.idx_x, ui.input_x);
    tie_setting(s.idx_y, ui.input_y);
    tie_setting(s.idx_z, ui.input_z);

    tie_setting(s.add_yaw, ui.add_yaw);
    tie_setting(s.add_pitch, ui.add_pitch);
    tie_setting(s.add_roll, ui.add_roll);

}

void dialog_edtracker::doOK() {
    int idx = ui.dev->currentIndex();
    static const joys def { {}, {} };
    auto val = joys_.value(idx, def);
    s.dev = val.device;
    s.b->save();
    close();
}

void dialog_edtracker::doCancel() {
    close();
}

void dialog_edtracker::updateDevices() {
    getJoysticks();

    for (int i = 0; i < joys_.size(); i++) {
        ui.dev->addItem(QString("%1 | %2")
                        .arg(joys_[i].device)
                        .arg(joys_[i].name));
        if (joys_[i].device == s.dev) ui.dev->setCurrentIndex(i);
    }
}

void getJoysticks() {
    QString device;
    char name[128];
    joys j;
    joys_ = {};

    for (int i = 0; i < 64; i++) {
        device = QString("/dev/input/js%1").arg(i);
        int iFile = open(device.toUtf8().data(), O_RDONLY | O_NONBLOCK);
        if (iFile == -1) break;
        if (ioctl(iFile, JSIOCGNAME(sizeof(name)), &name) > 0)
            if (QString(name).contains("EDTracker", Qt::CaseSensitive)) {
                j.name = name;
                j.device = i;
                joys_.append(j);
            }
        close(iFile);
    }
}
