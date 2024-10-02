#pragma once

#include "api/plugin-api.hpp"
#include "compat/shm.h"
#include "wine-shm.h"

#include "ui_ftnoir_winecontrols.h"

#include "options/options.hpp"
using namespace options;

#include <QMutex>
#include <QProcess>
#include <QString>
#include <QVariant>

#include <QDebug>

struct settings : opts
{
    settings() : opts{"proto-wine"} {}
    value<bool> variant_wine{b, "variant-wine", true },
                variant_proton{b, "variant-proton", false },
                variant_proton_steamplay{b, "variant-proton-steamplay", true },
                variant_proton_external{b, "variant-proton-external", false },
                fsync{b, "fsync", true},
                esync{b, "esync", true};

    value<int>     proton_appid{b, "proton-appid", 0};
    value<QVariant> proton_path{b, "proton-version", {} };
    value<QVariant> wine_select_path{b, "wine-select-version", {"WINE"}};
    value<QString> wine_custom_path{b, "wine-custom-version", ""};
    value<QString> wineprefix{b, "wineprefix", "~/.wine/"};
    value<QString> protonprefix{b, "protonprefix", ""};
    value<int>     protocol{b, "protocol", 2};
};

class wine : TR, public IProtocol
{
    Q_OBJECT

public:
    wine();
    ~wine() override;

    module_status initialize() override;
    void pose(const double* headpose, const double*) override;

    QString game_name() override
    {
#ifndef OTR_WINE_NO_WRAPPER
        QMutexLocker foo(&game_name_mutex);
        return connected_game;
#else
        return QStringLiteral("X-Plane");
#endif
    }
private:
    shm_wrapper lck_shm { WINE_SHM_NAME, WINE_MTX_NAME, sizeof(WineSHM) };
    WineSHM* shm = nullptr;
    settings s;

#ifndef OTR_WINE_NO_WRAPPER
    QProcess wrapper;
    int gameid = 0;
    QString connected_game;
    QMutex game_name_mutex;
#endif
};

class FTControls: public IProtocolDialog
{
    Q_OBJECT

public:
    FTControls();
    void register_protocol(IProtocol *) override {}
    void unregister_protocol() override {}

private:
    Ui::UICFTControls ui;
    settings s;

private slots:
    void onWinePathComboUpdated();
    void onRadioButtonsChanged();

    void doBrowseWine();
    void doBrowseWinePrefix();

    void doBrowseProtonPrefix();

    void doOK();
    void doCancel();
};

class wine_metadata : public Metadata
{
    Q_OBJECT

public:
#ifndef OTR_WINE_NO_WRAPPER
    QString name() override { return tr("Wine -- Windows layer for Unix"); }
    QIcon icon() override { return QIcon(":/images/wine.png"); }
#else
    QString name() override { return tr("X-Plane"); }
    QIcon icon() override { return {}; }
#endif
};
