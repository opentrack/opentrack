#include "ftnoir_protocol_mouse.h"
#include "api/plugin-api.hpp"

MOUSEControls::MOUSEControls()
{
    ui.setupUi(this);

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &MOUSEControls::doOK);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &MOUSEControls::doCancel);

    tie_setting(s.mouse_x, ui.axis_x);
    tie_setting(s.mouse_y, ui.axis_y);

    tie_setting(s.sensitivity_x, ui.sensitivity_x);
    tie_setting(s.sensitivity_y, ui.sensitivity_y);

    const int data[] = { input_direct, input_legacy };
    for (unsigned k = 0; k < std::size(data); k++)
        ui.input_method->setItemData(k, data[k]);
    tie_setting(s.input_method, ui.input_method);
}

void MOUSEControls::doOK()
{
    s.b->save();
    close();
}

void MOUSEControls::doCancel()
{
    close();
}
