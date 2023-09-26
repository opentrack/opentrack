#include "tobii.h"

tobii_dialog::tobii_dialog() // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
}

void tobii_dialog::doOK()
{
    // s.b->save();
    close();
}

void tobii_dialog::doCancel()
{
    close();
}
