#include "ftnoir_protocol_wine.h"
#include <QDebug>
#include <QFileDialog>
#include <QDir>

#include "api/plugin-api.hpp"

static const char* lutris_paths[] = {
    "/.local/share/lutris/runners/wine/"
};

static const char* proton_paths[] = {
    "/.steam/steam/steamapps/common",
    "/.steam/root/compatibilitytools.d",
    "/.local/share/Steam/steamapps/common",
};

FTControls::FTControls()
{
    ui.setupUi(this);

    //populate wine select (from lutris)
    for (const char* path : lutris_paths) {
        QDir dir(QDir::homePath() + path);
        dir.setFilter(QDir::Dirs);
        QFileInfoList list = dir.entryInfoList();
        for (int i = 0; i < list.size(); ++i) {
            QFileInfo fileInfo = list.at(i);
            if (fileInfo.fileName() == "." || fileInfo.fileName() == "..") continue;
            ui.wine_path_combo->addItem(fileInfo.fileName(), QVariant{fileInfo.filePath() + "/bin/wine"});
        }
        ui.wine_path_combo->addItem("Custom Selection", QVariant{"CUSTOM"});
    }

    //pupulate proton select
    for (const char* path : proton_paths) {
        QDir dir(QDir::homePath() + path);
        dir.setFilter(QDir::Dirs);
        dir.setNameFilters({ "Proton*" });
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
    tie_setting(s.wine_path, ui.wine_path);
    tie_setting(s.wineprefix, ui.wineprefix);
    tie_setting(s.protocol, ui.protocol_selection);

    connect(ui.wine_path_combo, &QComboBox::currentTextChanged, this, &FTControls::onWinePathComboUpdated);
    connect(ui.browse_wine_path_button, &QPushButton::clicked, this, &FTControls::doBrowseWine);
    connect(ui.browse_wine_prefix_button, &QPushButton::clicked, this, &FTControls::doBrowsePrefix);
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &FTControls::doOK);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &FTControls::doCancel);
}

void FTControls::onWinePathComboUpdated(QString selection) {
    if (selection == "Custom Selection") {
        ui.wine_path->setEnabled(true);
        ui.browse_wine_path_button->setEnabled(true);
    }
    else {
        s.wine_path = QDir::homePath() + "/.local/share/lutris/runners/wine/" + ui.wine_path_combo->currentText() + "/bin/wine";
        ui.wine_path->setEnabled(false);
        ui.browse_wine_path_button->setEnabled(false);
    }
}

void FTControls::doBrowseWine() {
    QFileDialog d(this);
    d.setFileMode(QFileDialog::FileMode::ExistingFile);
    d.setWindowTitle(tr("Select path to Wine Binary"));
    if (s.wine_path->startsWith("~/.local/share/lutris/runners")) {
        d.selectFile(s.wine_path);
    }
    if (d.exec()) {
        s.wine_path = d.selectedFiles()[0];
    }
}
void FTControls::doBrowsePrefix() {
    QFileDialog d(this);
    d.setFileMode(QFileDialog::FileMode::Directory);
    d.setOption(QFileDialog::Option::ShowDirsOnly, true);
    d.setWindowTitle(tr("Select Wine Prefix"));
    if (s.wineprefix->startsWith("/") || s.wineprefix->startsWith("~")) {
        d.selectFile(s.wineprefix);
    }
    if (d.exec()) {
        s.wineprefix = d.selectedFiles()[0];
    }
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
