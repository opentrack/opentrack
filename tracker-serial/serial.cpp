/* Copyright (c) 2014, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "serial.h"
#include "api/plugin-api.hpp"
#include "compat/math-imports.hpp"

#include <QPushButton>

#include <cmath>
#include <QDebug>

static const double incr[3] =
{
    10, 5, 3
};

static const double max_values[3] = {
    180, 2, 3,
};

serial_tracker::serial_tracker() = default;
serial_tracker::~serial_tracker() = default;

module_status serial_tracker::start_tracker(QFrame*)
{
    t.start();
    return {};
}

void serial_tracker::data(double *data)
{
    const double dt = t.elapsed_seconds();
    t.start();

    for (int i = 0; i < 3; i++)
    {
        double last_ = last[i];
        double max = max_values[i] * 2;
        double incr_ = incr[i];
        double x = fmod(last_ + incr_ * dt, max);
        last[i] = x;
        if (x > max_values[i])
            x = -max + x;
        data[i+3] = x;
    }
}

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

OPENTRACK_DECLARE_TRACKER(serial_tracker, test_dialog, test_metadata)
