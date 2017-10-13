#include "ftnoir_filter_ewma2.h"

#include <cmath>

#include <QDebug>
#include <QString>

#include "api/plugin-api.hpp"
#include "ui_ftnoir_ewma_filtercontrols.h"

dialog_ewma::dialog_ewma()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.kMaxSmoothing, ui.maxSmooth);
    tie_setting(s.kMinSmoothing, ui.minSmooth);
    tie_setting(s.kSmoothingScaleCurve, ui.powCurve);

    tie_setting(s.kSmoothingScaleCurve, ui.curve_label,
                [](auto& x) { return QStringLiteral("%1%").arg(x * 100, 0, 'f', 2); });

    tie_setting(s.kMinSmoothing, ui.min_label,
                [](auto& x) { return QStringLiteral("%1%").arg(x * 100, 0, 'f', 2);});

    tie_setting(s.kMaxSmoothing, ui.max_label,
                [](auto& x) { return QStringLiteral("%1%").arg(x * 100, 0, 'f', 2);});

    connect(ui.minSmooth, &QSlider::valueChanged, this,
            [&](int v) -> void { if (ui.maxSmooth->value() < v) ui.maxSmooth->setValue(v); });

    connect(ui.maxSmooth, &QSlider::valueChanged, this,
            [&](int v) -> void { if (ui.minSmooth->value() > v) ui.minSmooth->setValue(v); });
}

void dialog_ewma::doOK()
{
    s.b->save();
    close();
}

void dialog_ewma::doCancel()
{
    close();
}
