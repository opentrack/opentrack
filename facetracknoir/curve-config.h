#pragma once
#include <QWidget>
#include "./mappings.hpp"
#include "ui_ftnoir_curves.h"

class CurveConfigurationDialog: public QWidget
{
    Q_OBJECT
public:
    CurveConfigurationDialog(Mappings &m, main_settings &s, QWidget *parent );
private:
    Ui::UICCurveConfigurationDialog ui;
    Mappings& m;
    void save();
private slots:
    void doOK();
    void doCancel();
};
