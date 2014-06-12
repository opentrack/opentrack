#include "facetracknoir/facetracknoir.h"
#include "facetracknoir/curve-config.h"
#include <QDebug>
#include <QCheckBox>
CurveConfigurationDialog::CurveConfigurationDialog(FaceTrackNoIR *ftnoir, QWidget *parent) :
    QWidget( parent, Qt::Dialog ), mainApp(ftnoir)
{
	ui.setupUi( this );
    setFont(qApp->font());

    QPoint offsetpos(120, 30);
	this->move(parent->pos() + offsetpos);

	// Connect Qt signals to member-functions
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(mainApp->s.a_x.altp, ui.tx_altp);
    tie_setting(mainApp->s.a_y.altp, ui.ty_altp);
    tie_setting(mainApp->s.a_z.altp, ui.tz_altp);
    tie_setting(mainApp->s.a_yaw.altp, ui.rx_altp);
    tie_setting(mainApp->s.a_pitch.altp, ui.ry_altp);
    tie_setting(mainApp->s.a_roll.altp, ui.rz_altp);

    tie_setting(mainApp->s.tcomp_p, ui.tcomp_enable);
    tie_setting(mainApp->s.tcomp_tz, ui.tcomp_rz);

    tie_setting(mainApp->s.a_x.zero, ui.pos_tx);
    tie_setting(mainApp->s.a_y.zero, ui.pos_ty);
    tie_setting(mainApp->s.a_z.zero, ui.pos_tz);
    tie_setting(mainApp->s.a_yaw.zero, ui.pos_rx);
    tie_setting(mainApp->s.a_pitch.zero, ui.pos_ry);
    tie_setting(mainApp->s.a_roll.zero, ui.pos_rz);

    tie_setting(mainApp->s.a_yaw.invert, ui.chkInvertYaw);
    tie_setting(mainApp->s.a_pitch.invert, ui.chkInvertPitch);
    tie_setting(mainApp->s.a_roll.invert, ui.chkInvertRoll);
    tie_setting(mainApp->s.a_x.invert, ui.chkInvertX);
    tie_setting(mainApp->s.a_y.invert, ui.chkInvertY);
    tie_setting(mainApp->s.a_z.invert, ui.chkInvertZ);

	// Load the settings from the current .INI-file
	loadSettings();
}

void CurveConfigurationDialog::doOK() {
	save();
	this->close();
}

void CurveConfigurationDialog::doCancel() {
    mainApp->b->revert();
    loadSettings();
    close();
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void CurveConfigurationDialog::loadSettings() {
    QFunctionConfigurator* configs[6] = {
        ui.txconfig,
        ui.tyconfig,
        ui.tzconfig,
        ui.rxconfig,
        ui.ryconfig,
        ui.rzconfig
    };

    QFunctionConfigurator* alt_configs[6] = {
        ui.txconfig_alt,
        ui.tyconfig_alt,
        ui.tzconfig_alt,
        ui.rxconfig_alt,
        ui.ryconfig_alt,
        ui.rzconfig_alt
    };

    QSettings settings("opentrack");
    QString currentFile = settings.value("SettingsFile",
                                         QCoreApplication::applicationDirPath() + "/settings/default.ini" )
            .toString();

    for (int i = 0; i < 6; i++)
    {
        configs[i]->setConfig(&mainApp->axis(i).curve);
        alt_configs[i]->setConfig(&mainApp->axis(i).curveAlt);
    }
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void CurveConfigurationDialog::save() {

	qDebug() << "save() says: started";

    QSettings settings("opentrack");
    QString currentFile =
            settings.value("SettingsFile",
                           QCoreApplication::applicationDirPath() + "/settings/default.ini" )
            .toString();

    ui.rxconfig->saveSettings(currentFile);
    ui.ryconfig->saveSettings(currentFile);
    ui.rzconfig->saveSettings(currentFile);
    ui.txconfig->saveSettings(currentFile);
    ui.tyconfig->saveSettings(currentFile);
    ui.tzconfig->saveSettings(currentFile);

    ui.txconfig_alt->saveSettings(currentFile);
    ui.tyconfig_alt->saveSettings(currentFile);
    ui.tzconfig_alt->saveSettings(currentFile);
    ui.rxconfig_alt->saveSettings(currentFile);
    ui.ryconfig_alt->saveSettings(currentFile);
    ui.rzconfig_alt->saveSettings(currentFile);
}
