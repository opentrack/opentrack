#include "ftnoir_tracker_joystick.h"
#include "opentrack/plugin-api.hpp"

TrackerControls::TrackerControls() : tracker(nullptr)
{
    ui.setupUi( this );

    // Connect Qt signals to member-functions
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    {
        win32_joy_ctx joy_ctx;
        
        _joys = QList<joys>();
        
        for (auto j : joy_ctx.get_joy_info())
            _joys.push_back(joys { j.name, j.guid });
    }
    
    {
        const QString guid = s.guid;
        int idx = 0;
        for (int i = 0; i < _joys.size(); i++)
        {
            const joys& j = _joys[i];
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

void TrackerControls::doOK() {
    int idx = ui.joylist->currentIndex();
    joys def { "", "" };
    auto val = _joys.value(idx, def);
    s.guid = val.guid;
    s.b->save();
    this->close();
}

void TrackerControls::doCancel() {
    s.b->reload();
    this->close();
}
