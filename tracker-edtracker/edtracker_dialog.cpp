#include "edtracker.h"
#include "api/plugin-api.hpp"

dialog_edtracker::dialog_edtracker() : tracker(nullptr)
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

    {
        ports_ = {};

        for (const QSerialPortInfo& port_info : QSerialPortInfo::availablePorts()) {
            ports_.push_back(ports { port_info.portName(), port_info.serialNumber() });
        }
    }

    {
        const QString com_port_name = s.com_port_name;
        int pidx = 0;
        for (int i = 0; i < ports_.size(); i++)
        {
            const ports& j = ports_[i];
            if (j.port == com_port_name)
                pidx = i;
            ui.portlist->addItem(j.port);
        }
        ui.portlist->setCurrentIndex(pidx);
    }

    tie_setting(s.joy_1, ui.joy_1);
    tie_setting(s.joy_2, ui.joy_2);
    tie_setting(s.joy_3, ui.joy_3);
    tie_setting(s.joy_4, ui.joy_4);
    tie_setting(s.joy_5, ui.joy_5);
    tie_setting(s.joy_6, ui.joy_6);

    tie_setting(s.joy_1_abs, ui.joy_1_abs);
    tie_setting(s.joy_2_abs, ui.joy_2_abs);
    tie_setting(s.joy_3_abs, ui.joy_3_abs);
    tie_setting(s.joy_4_abs, ui.joy_4_abs);
    tie_setting(s.joy_5_abs, ui.joy_5_abs);
    tie_setting(s.joy_6_abs, ui.joy_6_abs);
}

void dialog_edtracker::doOK() {
    int idx = ui.joylist->currentIndex();
    static const joys def { {}, {} };
    auto val = joys_.value(idx, def);
    s.guid = val.guid;

    int pidx = ui.portlist->currentIndex();
    static const ports pdef{ {}, {} };
    auto pval = ports_.value(pidx, pdef);
    s.com_port_name = pval.port;

    s.b->save();
    close();
}

void dialog_edtracker::doCancel() {
    close();
}
