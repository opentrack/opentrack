#pragma once
#include <QWidget>
#include "opentrack-logic/mappings.hpp"
#include "ui_mapping.h"

class MapWidget: public QWidget
{
    Q_OBJECT
public:
    MapWidget(Mappings &m);
private:
    Ui::UICCurveConfigurationDialog ui;
    Mappings& m;
    void closeEvent(QCloseEvent *) override { doCancel(); }
private slots:
    void doOK();
    void doCancel();
};
