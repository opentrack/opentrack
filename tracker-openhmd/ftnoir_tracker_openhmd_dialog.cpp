#include "ftnoir_tracker_openhmd.h"
#include "api/plugin-api.hpp"

openhmd_dialog::openhmd_dialog()
{
    ui.setupUi(this);
    
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    
    ui.device_index->setValue(s.device_index);
    ui.invert_yaw->setChecked((bool)s.invert_yaw);
    ui.invert_pitch->setChecked((bool)s.invert_pitch);
    ui.invert_roll->setChecked((bool)s.invert_roll);
}

void openhmd_dialog::register_tracker(ITracker *) {}
void openhmd_dialog::unregister_tracker() {}

void openhmd_dialog::doOK()
{
    s.device_index = ui.device_index->value();
    s.invert_yaw = ui.invert_yaw->isChecked();
    s.invert_pitch = ui.invert_pitch->isChecked();
    s.invert_roll = ui.invert_roll->isChecked();
    s.b->save();
    close();
}

void openhmd_dialog::doCancel()
{
    close();
}
