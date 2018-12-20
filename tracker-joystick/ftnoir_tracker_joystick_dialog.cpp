#include "ftnoir_tracker_joystick.h"
#include "api/plugin-api.hpp"

dialog_joystick::dialog_joystick() : tracker(nullptr)
{
    ui.setupUi( this );

    // Connect Qt signals to member-functions
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    {
        win32_joy_ctx joy_ctx;

        joys_ = {};

        for (const auto& j : joy_ctx.get_joy_info())
            joys_.push_back(joys { j.name, j.guid });
    }

    {
        const QString guid = s.guid;
        int idx = 0;
        for (int i = 0; i < joys_.size(); i++)
        {
            const joys& j = joys_[i];
            if (j.guid == guid)
                idx = i;
            ui.joylist->addItem(j.name + " " + j.guid);
        }
        ui.joylist->setCurrentIndex(idx);
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
