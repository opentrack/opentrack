#pragma once
#include <QWidget>
#include "logic/mappings.hpp"
#include "ui_mapping-window.h"

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

    void closeEvent(QCloseEvent *) override { doCancel(); }
private slots:
    void doOK();
    void doCancel();
};
