#include "ftnoir_protocol_wine.h"
#include <QDebug>
#include <QDir>

#include "api/plugin-api.hpp"

static const char* proton_paths[] = {
    "/.steam/steam/steamapps/common",
    "/.steam/root/compatibilitytools.d",
    "/.local/share/Steam/steamapps/common",
};

static const QStringList proton_filter = { "Proton*" };

FTControls::FTControls()
{
    ui.setupUi(this);

    for (const char* path : proton_paths) {
        QDir dir(QDir::homePath() + path);
        dir.setFilter(QDir::Dirs);
        dir.setNameFilters(proton_filter);
        QFileInfoList list = dir.entryInfoList();
        for (int i = 0; i < list.size(); ++i) {
            QFileInfo fileInfo = list.at(i);
            ui.proton_version->addItem(fileInfo.fileName(), QVariant{fileInfo.filePath()});
        }
    }
    tie_setting(s.proton_path, ui.proton_version);
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
