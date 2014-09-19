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
private:
    Ui::UICCurveConfigurationDialog ui;
    void save();
    FaceTrackNoIR *mainApp;
private slots:
    void doOK();
    void doCancel();
};
