#include "ftnoir_tracker_freepie-udp.h"
#include "facetracknoir/plugin-support.h"

TrackerDialog::TrackerDialog()
{
	ui.setupUi(this);

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.port, ui.port);
    tie_setting(s.enable_yaw, ui.chkEnableYaw);
    tie_setting(s.enable_pitch, ui.chkEnablePitch);
    tie_setting(s.enable_roll, ui.chkEnableRoll);
}

void TrackerDialog::doOK() {
    s.b->save();
	this->close();
}

void TrackerDialog::doCancel() {
    s.b->revert();
    this->close();
}

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITrackerDialog* CALLING_CONVENTION GetDialog()
{
    return new TrackerDialog;
}
