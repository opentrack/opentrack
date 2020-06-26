#include "ftnoir_tracker_linux_joystick.h"
#include "api/plugin-api.hpp"

dialog_joystick::dialog_joystick() : tracker(nullptr)
{
    ui.setupUi( this );

    // Connect Qt signals to member-functions
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    QList<::linux_joystick> joysticks = getJoysticks();

    for (int i = 0; i < joysticks.size(); i++) {
        ::linux_joystick joy = joysticks[i];
        joys_.push_back(joys { joy.name, joy.device_id});
        ui.joylist->addItem(QString("%1 | %2").arg(joy.dev).arg(joy.name));
        if (joysticks[i].device_id == s.guid) ui.joylist->setCurrentIndex(i);
    }

    tie_setting(s.joy_1, ui.joy_1);
    tie_setting(s.joy_2, ui.joy_2);
    tie_setting(s.joy_3, ui.joy_3);
    tie_setting(s.joy_4, ui.joy_4);
    tie_setting(s.joy_5, ui.joy_5);
    tie_setting(s.joy_6, ui.joy_6);
}

void dialog_joystick::doOK() {
    int idx = ui.joylist->currentIndex();
    static const joys def { {}, {} };
    auto val = joys_.value(idx, def);
    s.guid = val.guid;
    s.b->save();
    close();
}

void dialog_joystick::doCancel() {
    close();
}
