#include "steamvr.hpp"
#include "api/plugin-api.hpp"

steamvr_dialog::steamvr_dialog()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    for (unsigned i = 0; i < 6; i++)
        ui.rotation_order->addItem(QStringLiteral("order #%1").arg(i));

    tie_setting(s.order, ui.rotation_order);
}

void steamvr_dialog::doOK()
{
    s.b->save();
    close();
}

void steamvr_dialog::doCancel()
{
    close();
}

