#pragma once

#include "settings.hpp"
#include "ui_dialog.h"
#include "api/plugin-api.hpp"

class osc_dialog: public IProtocolDialog
{
    Q_OBJECT

public:
    osc_dialog();
    void register_protocol(IProtocol*) override;
    void unregister_protocol() override;
private:
    void set_buttons_visible(bool x) noexcept override;
    bool embeddable() noexcept override;
    void save() override;
    void reload() override;

    Ui_OSC_Dialog ui;
    osc_settings s;
    const QPalette pal_;

private slots:
    void doOK();
    void doCancel();
    void host_address_edited(const QString& str);
};
