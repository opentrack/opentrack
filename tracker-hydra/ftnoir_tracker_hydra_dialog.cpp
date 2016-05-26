#include "ftnoir_tracker_hydra.h"
#include "opentrack/plugin-api.hpp"

TrackerControls::TrackerControls()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
}

void TrackerControls::doOK() {
    s.b->save();
    close();
}

void TrackerControls::doCancel()
{
    close();
}

