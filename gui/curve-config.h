#pragma once
#include <QWidget>
#include "opentrack-logic/mappings.hpp"
#include "ui_mapping.h"

class MapWidget: public QWidget
{
    Q_OBJECT
public:
    MapWidget(Mappings &m);
    void reload();
private:
    Ui::UICCurveConfigurationDialog ui;
    Mappings& m;
    main_settings s;

    void closeEvent(QCloseEvent *) override { doCancel(); }
private slots:
    void doOK();
    void doCancel();
};
