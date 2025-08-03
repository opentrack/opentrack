#include "test.h"
#include "api/plugin-api.hpp"
#include <QPushButton>

test_dialog::test_dialog() // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    ui.setupUi(this);

    connect(ui.buttonBox, &QDialogButtonBox::clicked, [this](QAbstractButton* btn) {
        if (btn == ui.buttonBox->button(QDialogButtonBox::Abort))
            *(volatile int*)nullptr /*NOLINT*/ = 0;
    });

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
}

void test_dialog::doOK()
{
    //s.b->save();
    close();
}

void test_dialog::doCancel()
{
    close();
}
