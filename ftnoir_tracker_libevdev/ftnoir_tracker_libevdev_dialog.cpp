#include "ftnoir_tracker_libevdev.h"
#include "opentrack/plugin-api.hpp"

#include <QDir>

TrackerControls::TrackerControls()
{
	ui.setupUi(this);

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    
    ui.comboBox->clear();
    
    QDir dir("/dev/input/by-id");
    auto devices = dir.entryList(QStringList { "usb-?*-event-?*"});
    for (QString dev : devices)
        ui.comboBox->addItem(dev);
    tie_setting(s.device_name, ui.comboBox);
}

void TrackerControls::doOK() {
    s.b->save();
	this->close();
}

void TrackerControls::doCancel() {
    s.b->reload();
    this->close();
}

extern "C" OPENTRACK_EXPORT ITrackerDialog* GetDialog()
{
    return new TrackerControls;
}
