#include "ftnoir_protocol_wine.h"
#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QRadioButton>

#include "api/plugin-api.hpp"
#include "options/tie.hpp"

/*
 * 0: path to the directory with wine versions
 * 1: path from a wine version to the exectuable
 * 2: name of the application using the wine versions
 */
static const char* wine_paths[][3] = {
    {"/.local/share/lutris/runners/wine/", "/bin/wine", "Lutris"},
    {"/.var/app/net.lutris.Lutris/data/lutris/runners/wine/", "/bin/wine", "Flatpak Lutris"}
};

static const char* proton_paths[][2] = {
    {"/.steam/steam/steamapps/common", "Proton*"},
    {"/.steam/root/compatibilitytools.d", "*"},
    {"/.local/share/Steam/steamapps/common", "Proton*"},
    {"/.local/share/Steam/compatibilitytools.d", "*"},
};

FTControls::FTControls()
{
    ui.setupUi(this);

    // populate wine select
    ui.wine_path_combo->addItem("System Wine", QVariant{"WINE"});
    for (const char** path : wine_paths) {
        QDir dir(QDir::homePath() + path[0]);
        dir.setFilter(QDir::Dirs);
        QFileInfoList list = dir.entryInfoList();
        for (int i = 0; i < list.size(); ++i) {
            QFileInfo fileInfo = list.at(i);
            if (fileInfo.fileName() == "." || fileInfo.fileName() == "..") continue;

            QString name = fileInfo.fileName() + " (" + path[2] + ")";
            ui.wine_path_combo->addItem(name, QVariant{fileInfo.filePath() + path[1]});
        }
    }
    ui.wine_path_combo->addItem("Custom path to Wine executable", QVariant{"CUSTOM"});

    // populate proton select
    for (const char** path : proton_paths) {
        QDir dir(QDir::homePath() + path[0]);
        dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        dir.setNameFilters({ path[1] });

        QFileInfoList proton_dir_list = dir.entryInfoList();
        for (int i = 0; i < proton_dir_list.size(); ++i) {
            const QFileInfo &proton_dir = proton_dir_list.at(i);

            // check if this Proton Version is already present in any way
            if (ui.proton_version->findText(proton_dir.fileName()) != -1)
                continue;

            QDirIterator proton_executable_it(proton_dir.canonicalFilePath(), QStringList() << "wine", QDir::Files, QDirIterator::Subdirectories);

            if (proton_executable_it.hasNext()) {
                QString proton_executable_path = proton_executable_it.next();
                QDir proton_dist_dir(proton_executable_path);
                proton_dist_dir.cd("../../");

                ui.proton_version->addItem(proton_dir.fileName(), QVariant{proton_dist_dir.canonicalPath()});
            }
        }
    }

    // settings - wine
    tie_setting(s.variant_wine, ui.variant_wine); // radio button
    tie_setting(s.wine_select_path, ui.wine_path_combo); // combo box (dropdown)
    tie_setting(s.wine_custom_path, ui.wine_path); // line edit (enabled via dropdown)
    tie_setting(s.wineprefix, ui.wineprefix); // line edit

    // settings - proton
    tie_setting(s.variant_proton, ui.variant_proton); // radio button
    tie_setting(s.proton_path, ui.proton_version); // combo box (dropdown)
    tie_setting(s.variant_proton_steamplay, ui.subvariant_steamplay); // radio button
    tie_setting(s.proton_appid, ui.proton_appid); // number select
    tie_setting(s.variant_proton_external, ui.subvariant_external); // radio button
    tie_setting(s.protonprefix, ui.protonprefix); // line edit

    // settings - advanced
    tie_setting(s.esync, ui.esync);
    tie_setting(s.fsync, ui.fsync);
    tie_setting(s.protocol, ui.protocol_selection);

    // setup signals and slots for UI
    connect(ui.wine_path_combo, &QComboBox::currentTextChanged, this, &FTControls::onWinePathComboUpdated);
    connect(ui.browse_wine_path_button, &QPushButton::clicked, this, &FTControls::doBrowseWine);
    connect(ui.browse_wine_prefix_button, &QPushButton::clicked, this, &FTControls::doBrowseWinePrefix);
    connect(ui.browse_proton_prefix_button, &QPushButton::clicked, this, &FTControls::doBrowseProtonPrefix);
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &FTControls::doOK);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &FTControls::doCancel);

    // setup signals and slots for UI radio buttons
    connect(ui.variant_wine, &QRadioButton::clicked, this, &FTControls::onRadioButtonsChanged);
    connect(ui.variant_proton, &QRadioButton::clicked, this, &FTControls::onRadioButtonsChanged);
    connect(ui.subvariant_steamplay, &QRadioButton::clicked, this, &FTControls::onRadioButtonsChanged);
    connect(ui.subvariant_external, &QRadioButton::clicked, this, &FTControls::onRadioButtonsChanged);

    // update state of the combo box and associated ui elements
    onWinePathComboUpdated();
    // hide the correct items
    onRadioButtonsChanged();
}

void FTControls::onWinePathComboUpdated() {
    // enable the custom text field if required
    if (ui.wine_path_combo->currentData() == "CUSTOM") {
        ui.wine_path->setEnabled(true);
        ui.browse_wine_path_button->setEnabled(true);
    }
    else {
        ui.wine_path->setEnabled(false);
        ui.browse_wine_path_button->setEnabled(false);
    }
}

void FTControls::onRadioButtonsChanged() {
    if (ui.variant_wine->isChecked()) {
        // wine settings selected

        // enable wine settings
        ui.wine_path_combo->setEnabled(true);
        ui.wineprefix->setEnabled(true);
        ui.browse_wine_prefix_button->setEnabled(true);
        if (ui.wine_path_combo->currentData() == "CUSTOM") {
            ui.wine_path->setEnabled(true);
            ui.browse_wine_path_button->setEnabled(true);
        }

        // disable proton settings
        ui.proton_version->setEnabled(false);
        ui.proton_subgroup->setEnabled(false);
    }
    else if (ui.variant_proton->isChecked()) {
        // proton settings selected

        // disable wine settings
        ui.wine_path_combo->setEnabled(false);
        ui.wine_path->setEnabled(false);
        ui.browse_wine_path_button->setEnabled(false);
        ui.wineprefix->setEnabled(false);
        ui.browse_wine_prefix_button->setEnabled(false);

        // enable proton settings
        ui.proton_version->setEnabled(true);
        ui.proton_subgroup->setEnabled(true);

        // run proton radio buttons logic
        if (ui.subvariant_steamplay->isChecked()) {
            // enable steamplay settings
            ui.proton_appid->setEnabled(true);

            // disable external settings
            ui.protonprefix->setEnabled(false);
            ui.browse_proton_prefix_button->setEnabled(false);
        }
        else if (ui.subvariant_external->isChecked()) {
            // disable steamplay settings
            ui.proton_appid->setEnabled(false);

            // enable external settinsg
            ui.protonprefix->setEnabled(true);
            ui.browse_proton_prefix_button->setEnabled(true);
        }
    }
    else {
        // for some reason QTs auto exclusive feature is not always correctly working
        // this is a somewhat hacky solution
        ui.variant_wine->setChecked(ui.wine_path_combo->isEnabled());
        ui.variant_proton->setChecked(ui.proton_version->isEnabled());
    }
}

void FTControls::doBrowseWine() {
    QFileDialog d(this);
    d.setFileMode(QFileDialog::FileMode::ExistingFile);
    d.setWindowTitle(tr("Select path to Wine Binary"));
    if (s.wine_custom_path->startsWith("~/.local/share/lutris/runners")) {
        d.selectFile(s.wine_custom_path);
    }
    if (d.exec()) {
        s.wine_custom_path = d.selectedFiles()[0];
    }
}
void FTControls::doBrowseWinePrefix() {
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

void FTControls::doBrowseProtonPrefix() {
    QFileDialog d(this);
    d.setFileMode(QFileDialog::FileMode::Directory);
    d.setOption(QFileDialog::Option::ShowDirsOnly, true);
    d.setWindowTitle(tr("Select Proton Prefix"));
    if (s.protonprefix->startsWith("/") || s.protonprefix->startsWith("~")) {
        d.selectFile(s.protonprefix);
    }
    if (d.exec()) {
        s.protonprefix = d.selectedFiles()[0];
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
