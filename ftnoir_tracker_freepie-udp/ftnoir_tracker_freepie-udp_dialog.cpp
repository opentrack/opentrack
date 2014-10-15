#include "ftnoir_tracker_freepie-udp.h"
#include "facetracknoir/plugin-support.h"

TrackerDialog::TrackerDialog()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.port, ui.port);
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
