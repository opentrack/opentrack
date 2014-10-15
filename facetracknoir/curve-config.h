#pragma once
#include <QWidget>
#include "./mappings.hpp"
#include "ui_ftnoir_curves.h"

class MapWidget: public QWidget
{
    Q_OBJECT
public:
    MapWidget(Mappings &m, main_settings &s, QWidget *parent );
private:
    Ui::UICCurveConfigurationDialog ui;
    Mappings& m;
private slots:
    void doOK();
    void doCancel();
};
