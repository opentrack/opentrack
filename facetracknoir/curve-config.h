#pragma once
#include <QWidget>
#include "opentrack/mappings.hpp"
#include "ui_mapping.h"

class MapWidget: public QWidget
{
    Q_OBJECT
public:
    MapWidget(Mappings &m, main_settings &s);
private:
    Ui::UICCurveConfigurationDialog ui;
    Mappings& m;
    void closeEvent(QCloseEvent *) override { doCancel(); }
private slots:
    void doOK();
    void doCancel();
};
