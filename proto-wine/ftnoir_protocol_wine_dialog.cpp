#include "ftnoir_protocol_wine.h"
#include <QDebug>
#include "api/plugin-api.hpp"

static const char* proton_versions[] = {
    "4.11", "4.2", "3.16", "3.7",
};

FTControls::FTControls()
{
    ui.setupUi(this);

    for (const char* version : proton_versions)
        ui.proton_version->addItem(version, QVariant{version});

    tie_setting(s.proton_version, ui.proton_version);
    tie_setting(s.variant_wine, ui.variant_wine);
    tie_setting(s.variant_proton, ui.variant_proton);
    tie_setting(s.esync, ui.esync);
    tie_setting(s.fsync, ui.fsync);
    tie_setting(s.proton_appid, ui.proton_appid);
    tie_setting(s.wineprefix, ui.wineprefix);
    tie_setting(s.protocol, ui.protocol_selection);

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &FTControls::doOK);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &FTControls::doCancel);
}

void FTControls::doOK()
{
    s.b->save();
    close();
}

void FTControls::doCancel()
{
    s.b->reload();
    close();
}

