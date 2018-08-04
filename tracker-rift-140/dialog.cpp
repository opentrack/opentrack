#include "rift-140.hpp"
#include "api/plugin-api.hpp"

dialog_rift_140::dialog_rift_140()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
}

void dialog_rift_140::doOK() {
    s.b->save();
    close();
}

void dialog_rift_140::doCancel() {
    close();
}

