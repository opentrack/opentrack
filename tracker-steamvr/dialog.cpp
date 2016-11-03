#include "steamvr.hpp"
#include "api/plugin-api.hpp"

dialog::dialog()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
}

void dialog::doOK()
{
    s.b->save();
    close();
}

void dialog::doCancel()
{
    close();
}

