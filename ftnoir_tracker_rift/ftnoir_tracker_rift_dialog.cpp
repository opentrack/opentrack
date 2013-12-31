#include "ftnoir_tracker_rift.h"
#include "facetracknoir/global-settings.h"

TrackerControls::TrackerControls() :
QWidget()
{
	ui.setupUi( this );

	// Connect Qt signals to member-functions
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.bEnableYaw, ui.chkEnableYaw);
    tie_setting(s.bEnablePitch, ui.chkEnablePitch);
    tie_setting(s.bEnableRoll, ui.chkEnableRoll);

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
	//
	// Ask if changed Settings should be saved
	//
    if (s.b->modifiedp()) {
        int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		switch (ret) {
			case QMessageBox::Save:
                s.b->save();
				this->close();
				break;
			case QMessageBox::Discard:
                s.b->revert();
				this->close();
				break;
			case QMessageBox::Cancel:
			default:
			break;
		}
	}
	else {
		this->close();
	}
}

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITrackerDialog* CALLING_CONVENTION GetDialog( )
{
    return new TrackerControls;
}
