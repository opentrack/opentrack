#include "vjoystick.h"
#include "api/plugin-api.hpp"

vjoystick_dialog::vjoystick_dialog()
{
    ui.setupUi(this);
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &QWidget::close);
}
