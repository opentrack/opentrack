#pragma once
#include <QWidget>
#include "ui_ftnoir_curves.h"

class FaceTrackNoIR;

class CurveConfigurationDialog: public QWidget
{
    Q_OBJECT
public:

    explicit CurveConfigurationDialog( FaceTrackNoIR *ftnoir, QWidget *parent=0, Qt::WindowFlags f=0 );
    virtual ~CurveConfigurationDialog();
    void showEvent ( QShowEvent * event );
    void loadSettings();
private:
    Ui::UICCurveConfigurationDialog ui;
    void save();

    bool settingsDirty;
    FaceTrackNoIR *mainApp;

private slots:
    void doOK();
    void doCancel();
    void curveChanged( bool change ) { settingsDirty = true; }
    void curveChanged( int change ) { settingsDirty = true; }
};
