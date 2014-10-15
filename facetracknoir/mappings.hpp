#pragma once

#include <QSettings>
#include "options.h"
using namespace options;
#include "../qfunctionconfigurator/functionconfig.h"
#include "main-settings.hpp"

class Mapping {
public:
    Mapping(QString primary,
            QString secondary,
            int maxInput1,
            int maxOutput1,
            int maxInput2,
            int maxOutput2,
            axis_opts& opts) :
        curve(maxInput1, maxOutput1),
        curveAlt(maxInput2, maxOutput2),
        opts(opts),
        name1(primary),
        name2(secondary)
    {
        // XXX TODO move all this qsettings boilerplate into a single header -sh 20141004
        QSettings settings("opentrack");
        QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
        QSettings iniFile(currentFile, QSettings::IniFormat);
        curve.loadSettings(iniFile, primary);
        curveAlt.loadSettings(iniFile, secondary);
    }
    Map curve;
    Map curveAlt;
    axis_opts& opts;
    QString name1, name2;
};

class Mappings {
private:
    Mapping axes[6];
public:
    Mappings(std::vector<axis_opts*> opts) :
        axes {
            Mapping("tx","tx_alt", 100, 100, 100, 100, *opts[TX]),
            Mapping("ty","ty_alt", 100, 100, 100, 100, *opts[TY]),
            Mapping("tz","tz_alt", 100, 100, 100, 100, *opts[TZ]),
            Mapping("rx", "rx_alt", 180, 180, 180, 180, *opts[Yaw]),
            Mapping("ry", "ry_alt", 180, 180, 180, 180, *opts[Pitch]),
            Mapping("rz", "rz_alt", 180, 180, 180, 180, *opts[Roll])
        }
    {}

    inline Mapping& operator()(int i) { return axes[i]; }
    inline const Mapping& operator()(int i) const { return axes[i]; }

    void load_mappings()
    {
        QSettings settings("opentrack");
        QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
        QSettings iniFile( currentFile, QSettings::IniFormat );

        for (int i = 0; i < 6; i++)
        {
            axes[i].curve.loadSettings(iniFile, axes[i].name1);
            axes[i].curveAlt.loadSettings(iniFile, axes[i].name2);
        }
    }
    void save_mappings()
    {
        QSettings settings("opentrack");
        QString currentFile = settings.value("SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini").toString();
        QSettings iniFile(currentFile, QSettings::IniFormat);

        for (int i = 0; i < 6; i++)
        {
            axes[i].curve.saveSettings(iniFile, axes[i].name1);
            axes[i].curveAlt.saveSettings(iniFile, axes[i].name2);
        }
    }
    
    void invalidate_unsaved()
    {
        for (int i = 0; i < 6; i++)
        {
            axes[i].curve.invalidate_unsaved_settings();
            axes[i].curveAlt.invalidate_unsaved_settings();
        }
    }
};
