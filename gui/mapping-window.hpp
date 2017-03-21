#pragma once

#if 0

#include "logic/mappings.hpp"
#include "ui_mapping-window.h"

#include <QWidget>
#include <QDialog>
#include <QShowEvent>
#include <QCloseEvent>
#include <QCheckBox>

class MapWidget final : public QDialog
{
    Q_OBJECT
public:
    MapWidget(Mappings& m);
private:
    Ui::mapping_window ui;
    Mappings& m;
    main_settings s;

    void closeEvent(QCloseEvent*) override;

    void load();

    void save_dialog();
    void invalidate_dialog();

private slots:
    void doOK();
    void doCancel();
};

#endif
