#pragma once
#include <QWidget>
#include <QPalette>
#include "ui_ftnoir_curves.h"

class FaceTrackNoIR;

class CurveConfigurationDialog: public QWidget
{
    Q_OBJECT
public:
    CurveConfigurationDialog( FaceTrackNoIR *ftnoir, QWidget *parent );
    void loadSettings();
private:
    Ui::UICCurveConfigurationDialog ui;
    void save();

    bool settingsDirty;
    FaceTrackNoIR *mainApp;

private slots:
    void doOK();
    void doCancel();
    void curveChanged( bool ) { settingsDirty = true; }
    void curveChanged( int ) { settingsDirty = true; }
};
