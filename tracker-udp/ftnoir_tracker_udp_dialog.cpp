#include "ftnoir_tracker_udp.h"
#include "opentrack/plugin-api.hpp"

TrackerControls::TrackerControls()
{
	ui.setupUi( this );

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.port, ui.spinPortNumber);
    tie_setting(s.add_yaw, ui.add_yaw);
    tie_setting(s.add_pitch, ui.add_pitch);
    tie_setting(s.add_roll, ui.add_roll);
}

void TrackerControls::doOK() {
    s.b->save();
	this->close();
}

void TrackerControls::doCancel() {
    s.b->reload();
    this->close();
}

