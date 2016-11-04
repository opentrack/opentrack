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

    connect(ui.powCurve, &QSlider::valueChanged, this, &dialog_ewma::update_labels);
    connect(ui.minSmooth, &QSlider::valueChanged, this, &dialog_ewma::update_labels);
    connect(ui.maxSmooth, &QSlider::valueChanged, this, &dialog_ewma::update_labels);

    using std::min;
    using std::max;

    connect(ui.minSmooth, &QSlider::valueChanged, this,
            [&](int v) -> void { if (ui.maxSmooth->value() < v) ui.maxSmooth->setValue(v); });

    connect(ui.maxSmooth, &QSlider::valueChanged, this,
            [&](int v) -> void { if (ui.minSmooth->value() > v) ui.minSmooth->setValue(v); });

    update_labels(0);
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

void dialog_ewma::update_labels(int)
{
    ui.curve_label->setText(QString::number(static_cast<slider_value>(s.kSmoothingScaleCurve).cur() * 100, 'f', 2) + "%");
    ui.min_label->setText(QString::number(static_cast<slider_value>(s.kMinSmoothing).cur() * 100, 'f', 2) + "%");
    ui.max_label->setText(QString::number(static_cast<slider_value>(s.kMaxSmoothing).cur() * 100, 'f', 2) + "%");
}
