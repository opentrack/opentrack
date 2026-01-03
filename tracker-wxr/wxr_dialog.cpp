#include "wxr.h"
#include "api/plugin-api.hpp"
#include <QPushButton>

wxr_dialog::wxr_dialog() // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    ui.setupUi(this);

    //connect(ui.buttonBox, &QDialogButtonBox::clicked, [this](QAbstractButton* btn) {
    //    if (btn == ui.buttonBox->button(QDialogButtonBox::Abort))
    //        *(volatile int*)nullptr /*NOLINT*/ = 0;
    //});

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    
    tie_setting(s.yaw_scale, ui.dblYawScale);
    tie_setting(s.pitch_scale, ui.dblPitchScale);
    tie_setting(s.roll_scale, ui.dblRollScale);

    tie_setting(s.yaw_scale_immersive, ui.dblYawScaleImmersive);
    tie_setting(s.pitch_scale_immersive, ui.dblPitchScaleImmersive);
    tie_setting(s.roll_scale_immersive, ui.dblRollScaleImmersive);
}

void wxr_dialog::doOK()
{
    s.b->save();
    close();
}

void wxr_dialog::doCancel()
{
    close();
}
