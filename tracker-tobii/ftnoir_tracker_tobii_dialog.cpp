#include "ftnoir_tracker_tobii.h"
#include "api/plugin-api.hpp"

dialog_tobii::dialog_tobii() : tracker(nullptr)
{
    ui.setupUi( this );

    // Connect Qt signals to member-functions
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
}

void dialog_tobii::doOK() {
    close();
}

void dialog_tobii::doCancel() {
    close();
}
