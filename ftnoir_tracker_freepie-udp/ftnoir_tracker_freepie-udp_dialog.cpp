#include "ftnoir_tracker_freepie-udp.h"
#include "opentrack/plugin-api.hpp"

TrackerDialog::TrackerDialog()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.port, ui.port);
    tie_setting(s.idx_x, ui.input_x);
    tie_setting(s.idx_y, ui.input_y);
    tie_setting(s.idx_z, ui.input_z);
}

void TrackerDialog::doOK() {
    s.b->save();
    this->close();
}

void TrackerDialog::doCancel() {
    s.b->reload();
    this->close();
}

extern "C" OPENTRACK_EXPORT ITrackerDialog* GetDialog()
{
    return new TrackerDialog;
}
