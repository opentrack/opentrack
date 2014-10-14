#include "ftnoir_tracker_rift.h"
#include "facetracknoir/plugin-support.h"

TrackerControls::TrackerControls() :
QWidget()
{
    ui.setupUi( this );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.constant_drift, ui.constantDrift);
    tie_setting(s.deadzone, ui.deadzone);
    tie_setting(s.persistence, ui.persistence);
    tie_setting(s.useYawSpring, ui.yawSpring);
}

void TrackerControls::doOK() {
    s.b->save();
	this->close();
}

void TrackerControls::doCancel() {
    s.b->reload();
    close();
}

extern "C" OPENTRACK_EXPORT ITrackerDialog* GetDialog()
{
    return new TrackerControls;
}
