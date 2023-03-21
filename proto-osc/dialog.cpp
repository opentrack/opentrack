#include "dialog.hpp"
#include <QHostAddress>
#include <QPalette>

void osc_dialog::host_address_edited(const QString& str)
{
    bool bad = QHostAddress{str}.isNull();
    auto pal = pal_;
    for (auto role : { QPalette::Highlight, QPalette::Window })
        if (bad)
            pal.setColor(role, Qt::red);
    ui.address->setPalette(pal);
}

osc_dialog::osc_dialog() :
    pal_{palette()}
{
    ui.setupUi( this );

    tie_setting(s.address, ui.address);
    tie_setting(s.port, ui.port);
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &osc_dialog::doOK);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &osc_dialog::doCancel);
    connect(ui.address, &QLineEdit::textChanged, this, &osc_dialog::host_address_edited);
    host_address_edited(ui.address->text());
}

void osc_dialog::save() { s.b->save(); }
void osc_dialog::reload() { s.b->reload(); }

void osc_dialog::doOK() { s.b->save(); close(); }
void osc_dialog::doCancel() { close(); }

void osc_dialog::register_protocol(IProtocol*) {}
void osc_dialog::unregister_protocol() {}

bool osc_dialog::embeddable() noexcept { return true; }
void osc_dialog::set_buttons_visible(bool x) noexcept { ui.buttonBox->setVisible(x); }
