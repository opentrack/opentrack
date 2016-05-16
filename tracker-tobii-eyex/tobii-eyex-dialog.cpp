#include "tobii-eyex.hpp"

tobii_eyex_dialog::tobii_eyex_dialog()
{
    ui.setupUi(this);

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &tobii_eyex_dialog::do_ok);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &tobii_eyex_dialog::do_cancel);

    ui.tracking_mode->addItem("Relative", tobii_relative);
    ui.tracking_mode->addItem("Absolute", tobii_absolute);

    tie_setting(s.mode, ui.tracking_mode);
}

void tobii_eyex_dialog::do_ok()
{
    s.b->save();
    rs.b->save();
    close();
}

void tobii_eyex_dialog::do_cancel()
{
    close();
}
