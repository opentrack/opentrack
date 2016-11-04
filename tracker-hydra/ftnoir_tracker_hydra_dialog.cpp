#include "ftnoir_tracker_hydra.h"
#include "api/plugin-api.hpp"

dialog_hydra::dialog_hydra()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
}

void dialog_hydra::doOK() {
    s.b->save();
    close();
}

void dialog_hydra::doCancel()
{
    close();
}

