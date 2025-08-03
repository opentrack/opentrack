#pragma once

#include "export.hpp"

#include "logic/mappings.hpp"
#include "gui/ui_mapping-dialog.h"
#include "spline/spline-widget.hpp"

#include <QWidget>
#include <QDialog>
#include <QtEvents>

class OTR_GUI_EXPORT mapping_dialog final : public QDialog
{
    Q_OBJECT
public:
    mapping_dialog(Mappings& m);
    void refresh_tab();
    inline bool embeddable() noexcept { return false; }
private:
    Ui::mapping_dialog ui;
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
    void doAccept();
    void doReject();
};
