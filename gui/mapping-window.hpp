#pragma once

#include "logic/mappings.hpp"
#include "ui_mapping-window.h"

#include <QWidget>
#include <QShowEvent>
#include <QCloseEvent>
#include <QCheckBox>

class MapWidget final : public QWidget
{
    Q_OBJECT
public:
    MapWidget(Mappings& m);
    void reload();
private:
    Ui::mapping_window ui;
    Mappings& m;
    main_settings s;


    void closeEvent(QCloseEvent*) override;

    void save_dialog();
    void invalidate_dialog();

private slots:
    void doOK();
    void doCancel();
};
