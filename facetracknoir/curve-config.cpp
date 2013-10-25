#include "facetracknoir/facetracknoir.h"
#include "facetracknoir/curve-config.h"
#include <QDebug>
#include <QCheckBox>
CurveConfigurationDialog::CurveConfigurationDialog(FaceTrackNoIR *ftnoir, QWidget *parent) :
    QWidget( parent, Qt::Dialog ), mainApp(ftnoir)
{
	ui.setupUi( this );

    QPoint offsetpos(120, 30);
	this->move(parent->pos() + offsetpos);

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
    connect(ui.checkBox, SIGNAL(stateChanged(int)), this, SLOT(curveChanged(int)));

	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
CurveConfigurationDialog::~CurveConfigurationDialog() {
	qDebug() << "~CurveConfigurationDialog() says: started";
}

//
// OK clicked on server-dialog
//
void CurveConfigurationDialog::doOK() {
	save();
	this->close();
}

// override show event
void CurveConfigurationDialog::showEvent ( QShowEvent * ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void CurveConfigurationDialog::doCancel() {
	//
	// Ask if changed Settings should be saved
	//
	if (settingsDirty) {
		int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Discard );

		qDebug() << "doCancel says: answer =" << ret;

		switch (ret) {
			case QMessageBox::Save:
				save();
				this->close();
				break;
			case QMessageBox::Discard:
				this->close();
				break;
			case QMessageBox::Cancel:
				// Cancel was clicked
				break;
			default:
				// should never be reached
			break;
		}
	}
	else {
		this->close();
	}
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void CurveConfigurationDialog::loadSettings() {
    qDebug() << "CurveConfigurationDialog::loadSettings says: Starting ";
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

    qDebug() << "CurveConfigurationDialog::loadSettings says: iniFile = " << currentFile;

    static const char* names[] = {
        "tx_alt",
        "ty_alt",
        "tz_alt",
        "rx_alt",
        "ry_alt",
        "rz_alt"
    };

    iniFile.beginGroup("Tracking");

    ui.checkBox->setChecked(iniFile.value("compensation", true).toBool());

    for (int i = 0; i < 6; i++)
        mainApp->axis(i).altp = iniFile.value(names[i], false).toBool();

    QCheckBox* widgets[] = {
        ui.tx_altp,
        ui.ty_altp,
        ui.tz_altp,
        ui.rx_altp,
        ui.ry_altp,
        ui.rz_altp
    };

    for (int i = 0; i < 6; i++)
        widgets[i]->setChecked(mainApp->axis(i).altp);

    QDoubleSpinBox* widgets2[] = {
        ui.pos_tx,
        ui.pos_ty,
        ui.pos_tz,
        ui.pos_tx,
        ui.pos_ry,
        ui.pos_rz
    };
    
    const char* names2[] = {
        "zero_tx",
        "zero_ty",
        "zero_tz",
        "zero_rx",
        "zero_ry",
        "zero_rz"
    };


    for (int i = 0; i < 6; i++)
        widgets2[i]->setValue(iniFile.value(names2[i], 0).toDouble());
    
    iniFile.endGroup();

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

    QCheckBox* checkboxes[6] = {
        ui.rx_altp,
        ui.ry_altp,
        ui.rz_altp,
        ui.tx_altp,
        ui.ty_altp,
        ui.tz_altp
    };

    QDoubleSpinBox* widgets3[] = {
        ui.pos_tx,
        ui.pos_ty,
        ui.pos_tz,
        ui.pos_tx,
        ui.pos_ry,
        ui.pos_rz
    };

    for (int i = 0; i < 6; i++)
    {
        configs[i]->setConfig(&mainApp->axis(i).curve, currentFile);
        alt_configs[i]->setConfig(&mainApp->axis(i).curveAlt, currentFile);
        configs[i]->loadSettings(currentFile);
        alt_configs[i]->loadSettings(currentFile);
        connect(configs[i], SIGNAL(CurveChanged(bool)), this, SLOT(curveChanged(bool)), Qt::UniqueConnection);
        connect(alt_configs[i], SIGNAL(CurveChanged(bool)), this, SLOT(curveChanged(bool)), Qt::UniqueConnection);
        connect(checkboxes[i], SIGNAL(stateChanged(int)), this, SLOT(curveChanged(int)), Qt::UniqueConnection);
        mainApp->axis(i).zero = widgets3[i]->value();
    }
    
    settingsDirty = false;
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void CurveConfigurationDialog::save() {

	qDebug() << "save() says: started";

    QSettings settings("opentrack");	// Registry settings (in HK_USER)

    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();

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

    QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

    iniFile.beginGroup("Tracking");

    bool compensate = true;

    iniFile.setValue("compensation", compensate = (bool) !!ui.checkBox->isChecked());

    if (mainApp->tracker)
        mainApp->tracker->compensate = compensate;

    iniFile.setValue("rx_alt", ui.rx_altp->checkState() != Qt::Unchecked);
    iniFile.setValue("ry_alt", ui.ry_altp->checkState() != Qt::Unchecked);
    iniFile.setValue("rz_alt", ui.rz_altp->checkState() != Qt::Unchecked);
    iniFile.setValue("tx_alt", ui.tx_altp->checkState() != Qt::Unchecked);
    iniFile.setValue("ty_alt", ui.ty_altp->checkState() != Qt::Unchecked);
    iniFile.setValue("tz_alt", ui.tz_altp->checkState() != Qt::Unchecked);
    
    QDoubleSpinBox* widgets2[] = {
        ui.pos_tx,
        ui.pos_ty,
        ui.pos_tz,
        ui.pos_tx,
        ui.pos_ry,
        ui.pos_rz
    };
    
    const char* names2[] = {
        "zero_tx",
        "zero_ty",
        "zero_tz",
        "zero_rx",
        "zero_ry",
        "zero_rz"
    };
    
    for (int i = 0; i < 6; i++)
    {
        iniFile.setValue(names2[i], widgets2[i]->value());
        mainApp->axis(i).zero = widgets2[i]->value();
    }

    iniFile.endGroup();

	settingsDirty = false;
}
