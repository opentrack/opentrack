#pragma once

#include "logic/mappings.hpp"
#include "ui_mapping-dialog.h"

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
    void refresh_tab();
private:
    Ui::mapping_window ui;
    Mappings& m;
    main_settings s;

    spline_widget* widgets[6][2];

    void closeEvent(QCloseEvent*) override;

    void load();

    void save_dialog();
    void invalidate_dialog();

private slots:
    void doOK();
    void doCancel();
};
