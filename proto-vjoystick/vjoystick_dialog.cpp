#include "vjoystick.h"
#include "api/plugin-api.hpp"

#include <QDialogButtonBox>

vjoystick_dialog::vjoystick_dialog()
{
    ui.setupUi(this);
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &QWidget::close);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &QWidget::close);
}
