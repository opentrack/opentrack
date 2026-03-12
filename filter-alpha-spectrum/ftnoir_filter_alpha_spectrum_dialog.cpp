#include "ftnoir_filter_alpha_spectrum.h"

#include <QCheckBox>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QSlider>
#include <QSignalBlocker>
#include <QTimer>
#include <QWidget>
#include <algorithm>

namespace {
    static double lerp(double min, double max, double t)
    {
        return min + (max - min) * t;
    }

    static double norm(double value, double min, double max)
    {
        if (max <= min)
            return 0.0;
        return std::clamp((value - min) / (max - min), 0.0, 1.0);
    }
}

dialog_alpha_spectrum::dialog_alpha_spectrum()
{
    ui.setupUi(this);

    detail::alpha_spectrum::shared_calibration_status().ui_open.store(true, std::memory_order_relaxed);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.adaptive_mode, ui.adaptive_mode_check);
    tie_setting(s.advanced_mode_ui, ui.advanced_mode_check);
    tie_setting(s.ema_enabled, ui.ema_enabled_check);
    tie_setting(s.brownian_enabled, ui.brownian_enabled_check);
    tie_setting(s.predictive_enabled, ui.predictive_enabled_check);
    tie_setting(s.mtm_enabled, ui.mtm_enabled_check);

    tie_setting(s.rot_alpha_min, ui.rot_min_slider);
    tie_setting(s.rot_alpha_max, ui.rot_max_slider);
    tie_setting(s.rot_curve, ui.rot_curve_slider);
    tie_setting(s.pos_alpha_min, ui.pos_min_slider);
    tie_setting(s.pos_alpha_max, ui.pos_max_slider);
    tie_setting(s.pos_curve, ui.pos_curve_slider);
    tie_setting(s.rot_deadzone, ui.rot_deadzone_slider);
    tie_setting(s.pos_deadzone, ui.pos_deadzone_slider);
    tie_setting(s.brownian_head_gain, ui.brownian_gain_slider);
    tie_setting(s.adaptive_threshold_lift, ui.adaptive_threshold_slider);
    tie_setting(s.predictive_head_gain, ui.predictive_gain_slider);
    tie_setting(s.mtm_shoulder_base, ui.mtm_shoulder_slider);
    tie_setting(s.ngc_kappa, ui.ngc_kappa_slider);
    tie_setting(s.ngc_nominal_z, ui.ngc_nominal_z_slider);

    tie_setting(s.rot_alpha_min, ui.rot_min_label,
                [](double x) { return tr("%1%").arg(x * 100.0, 0, 'f', 1); });
    tie_setting(s.rot_alpha_max, ui.rot_max_label,
                [](double x) { return tr("%1%").arg(x * 100.0, 0, 'f', 1); });
    tie_setting(s.rot_curve, ui.rot_curve_label,
                [](double x) { return tr("%1").arg(x, 0, 'f', 2); });
    tie_setting(s.pos_alpha_min, ui.pos_min_label,
                [](double x) { return tr("%1%").arg(x * 100.0, 0, 'f', 1); });
    tie_setting(s.pos_alpha_max, ui.pos_max_label,
                [](double x) { return tr("%1%").arg(x * 100.0, 0, 'f', 1); });
    tie_setting(s.pos_curve, ui.pos_curve_label,
                [](double x) { return tr("%1").arg(x, 0, 'f', 2); });
    tie_setting(s.rot_deadzone, ui.rot_deadzone_label,
                [](double x) { return tr("%1°").arg(x, 0, 'f', 3); });
    tie_setting(s.pos_deadzone, ui.pos_deadzone_label,
                [](double x) { return tr("%1mm").arg(x, 0, 'f', 3); });
    tie_setting(s.brownian_head_gain, ui.brownian_gain_label,
                [](double x) { return tr("%1x").arg(x, 0, 'f', 2); });
    tie_setting(s.adaptive_threshold_lift, ui.adaptive_threshold_label,
                [](double x) { return tr("%1%").arg(x * 100.0, 0, 'f', 1); });
    tie_setting(s.predictive_head_gain, ui.predictive_gain_label,
                [](double x) { return tr("%1x").arg(x, 0, 'f', 2); });
    tie_setting(s.mtm_shoulder_base, ui.mtm_shoulder_label,
                [](double x) { return tr("%1%").arg(x * 100.0, 0, 'f', 1); });
    tie_setting(s.ngc_kappa, ui.ngc_kappa_label,
                [](double x) { return tr("%1").arg(x, 0, 'f', 3); });
    tie_setting(s.ngc_nominal_z, ui.ngc_nominal_z_label,
                [](double x) { return tr("%1m").arg(x, 0, 'f', 2); });

    connect(ui.rot_min_slider, &QSlider::valueChanged, this,
            [&](int v) { if (ui.rot_max_slider->value() < v) ui.rot_max_slider->setValue(v); });
    connect(ui.rot_max_slider, &QSlider::valueChanged, this,
            [&](int v) { if (ui.rot_min_slider->value() > v) ui.rot_min_slider->setValue(v); });

    connect(ui.pos_min_slider, &QSlider::valueChanged, this,
            [&](int v) { if (ui.pos_max_slider->value() < v) ui.pos_max_slider->setValue(v); });
    connect(ui.pos_max_slider, &QSlider::valueChanged, this,
            [&](int v) { if (ui.pos_min_slider->value() > v) ui.pos_min_slider->setValue(v); });

    connect(ui.reset_defaults_button, &QPushButton::clicked, this,
            [this] { reset_to_defaults(); });

    auto apply_simple_alpha = [this](int slider_value) {
        const double t = std::clamp(static_cast<double>(slider_value) / ui.simple_alpha_slider->maximum(), 0.0, 1.0);
        const double min_value = lerp(0.005, 0.4, t);
        const double max_value = lerp(0.02, 1.0, t);

        const auto rot_min_cfg = *s.rot_alpha_min;
        const auto rot_max_cfg = *s.rot_alpha_max;
        const auto pos_min_cfg = *s.pos_alpha_min;
        const auto pos_max_cfg = *s.pos_alpha_max;

        s.rot_alpha_min = options::slider_value{min_value, rot_min_cfg.min(), rot_min_cfg.max()};
        s.rot_alpha_max = options::slider_value{max_value, rot_max_cfg.min(), rot_max_cfg.max()};
        s.pos_alpha_min = options::slider_value{min_value, pos_min_cfg.min(), pos_min_cfg.max()};
        s.pos_alpha_max = options::slider_value{max_value, pos_max_cfg.min(), pos_max_cfg.max()};

        ui.simple_alpha_label->setText(tr("Min %1% / Max %2%")
                                           .arg(min_value * 100.0, 0, 'f', 1)
                                           .arg(max_value * 100.0, 0, 'f', 1));
    };

    auto apply_simple_shape = [this](int slider_value) {
        const double t = std::clamp(static_cast<double>(slider_value) / ui.simple_shape_slider->maximum(), 0.0, 1.0);
        const double curve_value = lerp(0.2, 8.0, t);
        const double rot_deadzone_value = lerp(0.0, 0.3, t);
        const double pos_deadzone_value = lerp(0.0, 2.0, t);

        const auto rot_curve_cfg = *s.rot_curve;
        const auto pos_curve_cfg = *s.pos_curve;
        const auto rot_deadzone_cfg = *s.rot_deadzone;
        const auto pos_deadzone_cfg = *s.pos_deadzone;

        s.rot_curve = options::slider_value{curve_value, rot_curve_cfg.min(), rot_curve_cfg.max()};
        s.pos_curve = options::slider_value{curve_value, pos_curve_cfg.min(), pos_curve_cfg.max()};
        s.rot_deadzone = options::slider_value{rot_deadzone_value, rot_deadzone_cfg.min(), rot_deadzone_cfg.max()};
        s.pos_deadzone = options::slider_value{pos_deadzone_value, pos_deadzone_cfg.min(), pos_deadzone_cfg.max()};

        ui.simple_shape_label->setText(tr("Curve %1 / RotDZ %2° / PosDZ %3mm")
                                           .arg(curve_value, 0, 'f', 2)
                                           .arg(rot_deadzone_value, 0, 'f', 3)
                                           .arg(pos_deadzone_value, 0, 'f', 3));
    };

    connect(ui.simple_alpha_slider, &QSlider::valueChanged, this, apply_simple_alpha);
    connect(ui.simple_shape_slider, &QSlider::valueChanged, this, apply_simple_shape);

    auto sync_simple_from_advanced = [this, apply_simple_alpha, apply_simple_shape] {
        const double alpha_t = 0.5 * (norm(*s.rot_alpha_min, 0.005, 0.4) + norm(*s.rot_alpha_max, 0.02, 1.0));
        const double shape_t = 0.25 * (
            norm(*s.rot_curve, 0.2, 8.0) +
            norm(*s.pos_curve, 0.2, 8.0) +
            norm(*s.rot_deadzone, 0.0, 0.3) +
            norm(*s.pos_deadzone, 0.0, 2.0));

        {
            const QSignalBlocker b1(ui.simple_alpha_slider);
            const QSignalBlocker b2(ui.simple_shape_slider);
            ui.simple_alpha_slider->setValue(static_cast<int>(alpha_t * ui.simple_alpha_slider->maximum()));
            ui.simple_shape_slider->setValue(static_cast<int>(shape_t * ui.simple_shape_slider->maximum()));
        }
        apply_simple_alpha(ui.simple_alpha_slider->value());
        apply_simple_shape(ui.simple_shape_slider->value());
    };

    auto update_advanced_visibility = [this] {
        const bool adv = *s.advanced_mode_ui;
        ui.controls_frame->setEnabled(adv);
        ui.ema_enabled_check->setEnabled(adv);
        ui.brownian_enabled_check->setEnabled(adv);
        ui.predictive_enabled_check->setEnabled(adv);
        ui.mtm_enabled_check->setEnabled(adv);
    };

    connect(ui.advanced_mode_check, &QCheckBox::toggled, this, [update_advanced_visibility](bool) {
        update_advanced_visibility();
    });

    sync_simple_from_advanced();
    update_advanced_visibility();

    ui.status_text->setVisible(false);
    ui.status_value->setVisible(false);
    ui.info_rot_text->setVisible(false);
    ui.info_rot_value->setVisible(false);
    ui.info_pos_text->setVisible(false);
    ui.info_pos_value->setVisible(false);
    ui.info_rot_brownian_text->setVisible(false);
    ui.info_rot_brownian_value->setVisible(false);
    ui.info_pos_brownian_text->setVisible(false);
    ui.info_pos_brownian_value->setVisible(false);
    ui.info_predictive_error_text->setVisible(false);
    ui.info_predictive_error_value->setVisible(false);

    const QFont fixed_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui.status_value->setFont(fixed_font);
    ui.info_rot_value->setFont(fixed_font);
    ui.info_pos_value->setFont(fixed_font);
    ui.info_rot_brownian_value->setFont(fixed_font);
    ui.info_pos_brownian_value->setFont(fixed_font);
    ui.info_rot_contrib_value->setFont(fixed_font);
    ui.info_pos_contrib_value->setFont(fixed_font);
    ui.info_predictive_error_value->setFont(fixed_font);

    const QFontMetrics fm(fixed_font);
    const QString status_template = tr("Mon|E1 B1 A1 P1 M1|rE0.000 rP0.000 pE0.000 pP0.000 k0.000");
    ui.status_value->setMinimumWidth(fm.horizontalAdvance(status_template));
    ui.status_value->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto* calib_timer = new QTimer(this);
    calib_timer->setInterval(100);
    connect(calib_timer, &QTimer::timeout, this, [this] {
        pull_status_into_ui(false);
    });
    calib_timer->start();
}

dialog_alpha_spectrum::~dialog_alpha_spectrum()
{
    detail::alpha_spectrum::shared_calibration_status().ui_open.store(false, std::memory_order_relaxed);
}

void dialog_alpha_spectrum::pull_status_into_ui(bool commit_to_settings)
{
    const auto& status = detail::alpha_spectrum::shared_calibration_status();

    if (!status.ui_open.load(std::memory_order_relaxed) && !status.active.load(std::memory_order_relaxed) && !commit_to_settings)
        return;

    const double rot_min = status.rot_alpha_min.load(std::memory_order_relaxed);
    const double rot_max = status.rot_alpha_max.load(std::memory_order_relaxed);
    const double rot_curve = status.rot_curve.load(std::memory_order_relaxed);
    const double rot_deadzone = status.rot_deadzone.load(std::memory_order_relaxed);
    const double pos_min = status.pos_alpha_min.load(std::memory_order_relaxed);
    const double pos_max = status.pos_alpha_max.load(std::memory_order_relaxed);
    const double pos_curve = status.pos_curve.load(std::memory_order_relaxed);
    const double pos_deadzone = status.pos_deadzone.load(std::memory_order_relaxed);
    const double rot_jitter = status.rot_jitter.load(std::memory_order_relaxed);
    const double pos_jitter = status.pos_jitter.load(std::memory_order_relaxed);
    const double rot_objective = status.rot_objective.load(std::memory_order_relaxed);
    const double pos_objective = status.pos_objective.load(std::memory_order_relaxed);
    const double rot_brownian_raw = status.rot_brownian_raw.load(std::memory_order_relaxed);
    const double rot_brownian_filtered = status.rot_brownian_filtered.load(std::memory_order_relaxed);
    const double rot_brownian_delta = status.rot_brownian_delta.load(std::memory_order_relaxed);
    const double rot_brownian_damped = status.rot_brownian_damped.load(std::memory_order_relaxed);
    const double rot_predictive_error = status.rot_predictive_error.load(std::memory_order_relaxed);
    const double pos_brownian_raw = status.pos_brownian_raw.load(std::memory_order_relaxed);
    const double pos_brownian_filtered = status.pos_brownian_filtered.load(std::memory_order_relaxed);
    const double pos_brownian_delta = status.pos_brownian_delta.load(std::memory_order_relaxed);
    const double pos_brownian_damped = status.pos_brownian_damped.load(std::memory_order_relaxed);
    const double pos_predictive_error = status.pos_predictive_error.load(std::memory_order_relaxed);
    const double rot_ema_drive = status.rot_ema_drive.load(std::memory_order_relaxed);
    const double rot_brownian_drive = status.rot_brownian_drive.load(std::memory_order_relaxed);
    const double rot_adaptive_drive = status.rot_adaptive_drive.load(std::memory_order_relaxed);
    const double rot_predictive_drive = status.rot_predictive_drive.load(std::memory_order_relaxed);
    const double rot_mtm_drive = status.rot_mtm_drive.load(std::memory_order_relaxed);
    const double pos_ema_drive = status.pos_ema_drive.load(std::memory_order_relaxed);
    const double pos_brownian_drive = status.pos_brownian_drive.load(std::memory_order_relaxed);
    const double pos_adaptive_drive = status.pos_adaptive_drive.load(std::memory_order_relaxed);
    const double pos_predictive_drive = status.pos_predictive_drive.load(std::memory_order_relaxed);
    const double pos_mtm_drive = status.pos_mtm_drive.load(std::memory_order_relaxed);
    const double rot_mode_expectation = status.rot_mode_expectation.load(std::memory_order_relaxed);
    const double pos_mode_expectation = status.pos_mode_expectation.load(std::memory_order_relaxed);
    const double rot_mode_peak = status.rot_mode_peak.load(std::memory_order_relaxed);
    const double pos_mode_peak = status.pos_mode_peak.load(std::memory_order_relaxed);
    const double ngc_coupling_residual = status.ngc_coupling_residual.load(std::memory_order_relaxed);
    const bool active = status.active.load(std::memory_order_relaxed);

    {
        const QSignalBlocker b1(ui.rot_min_slider);
        const QSignalBlocker b2(ui.rot_max_slider);
        const QSignalBlocker b3(ui.rot_curve_slider);
        const QSignalBlocker b4(ui.rot_deadzone_slider);
        const QSignalBlocker b5(ui.pos_min_slider);
        const QSignalBlocker b6(ui.pos_max_slider);
        const QSignalBlocker b7(ui.pos_curve_slider);
        const QSignalBlocker b8(ui.pos_deadzone_slider);

        const auto rot_min_cfg = *s.rot_alpha_min;
        const auto rot_max_cfg = *s.rot_alpha_max;
        const auto rot_curve_cfg = *s.rot_curve;
        const auto rot_deadzone_cfg = *s.rot_deadzone;
        const auto pos_min_cfg = *s.pos_alpha_min;
        const auto pos_max_cfg = *s.pos_alpha_max;
        const auto pos_curve_cfg = *s.pos_curve;
        const auto pos_deadzone_cfg = *s.pos_deadzone;

        ui.rot_min_slider->setValue(options::slider_value{rot_min, rot_min_cfg.min(), rot_min_cfg.max()}.to_slider_pos(ui.rot_min_slider->minimum(), ui.rot_min_slider->maximum()));
        ui.rot_max_slider->setValue(options::slider_value{rot_max, rot_max_cfg.min(), rot_max_cfg.max()}.to_slider_pos(ui.rot_max_slider->minimum(), ui.rot_max_slider->maximum()));
        ui.rot_curve_slider->setValue(options::slider_value{rot_curve, rot_curve_cfg.min(), rot_curve_cfg.max()}.to_slider_pos(ui.rot_curve_slider->minimum(), ui.rot_curve_slider->maximum()));
        ui.rot_deadzone_slider->setValue(options::slider_value{rot_deadzone, rot_deadzone_cfg.min(), rot_deadzone_cfg.max()}.to_slider_pos(ui.rot_deadzone_slider->minimum(), ui.rot_deadzone_slider->maximum()));
        ui.pos_min_slider->setValue(options::slider_value{pos_min, pos_min_cfg.min(), pos_min_cfg.max()}.to_slider_pos(ui.pos_min_slider->minimum(), ui.pos_min_slider->maximum()));
        ui.pos_max_slider->setValue(options::slider_value{pos_max, pos_max_cfg.min(), pos_max_cfg.max()}.to_slider_pos(ui.pos_max_slider->minimum(), ui.pos_max_slider->maximum()));
        ui.pos_curve_slider->setValue(options::slider_value{pos_curve, pos_curve_cfg.min(), pos_curve_cfg.max()}.to_slider_pos(ui.pos_curve_slider->minimum(), ui.pos_curve_slider->maximum()));
        ui.pos_deadzone_slider->setValue(options::slider_value{pos_deadzone, pos_deadzone_cfg.min(), pos_deadzone_cfg.max()}.to_slider_pos(ui.pos_deadzone_slider->minimum(), ui.pos_deadzone_slider->maximum()));
    }

    ui.rot_min_label->setText(tr("%1%").arg(rot_min * 100.0, 0, 'f', 1));
    ui.rot_max_label->setText(tr("%1%").arg(rot_max * 100.0, 0, 'f', 1));
    ui.rot_curve_label->setText(tr("%1").arg(rot_curve, 0, 'f', 2));
    ui.rot_deadzone_label->setText(tr("%1°").arg(rot_deadzone, 0, 'f', 3));
    ui.pos_min_label->setText(tr("%1%").arg(pos_min * 100.0, 0, 'f', 1));
    ui.pos_max_label->setText(tr("%1%").arg(pos_max * 100.0, 0, 'f', 1));
    ui.pos_curve_label->setText(tr("%1").arg(pos_curve, 0, 'f', 2));
    ui.pos_deadzone_label->setText(tr("%1mm").arg(pos_deadzone, 0, 'f', 3));

    ui.info_rot_value->setText(tr("%1 / %2")
                                        .arg(rot_jitter, 0, 'f', 4)
                                        .arg(rot_objective, 0, 'f', 4));
    ui.info_pos_value->setText(tr("%1 / %2")
                                        .arg(pos_jitter, 0, 'f', 4)
                                        .arg(pos_objective, 0, 'f', 4));
    ui.info_rot_brownian_value->setText(
        tr("%1 / %2 / Δ%3 / %4%")
            .arg(rot_brownian_raw, 0, 'f', 4)
            .arg(rot_brownian_filtered, 0, 'f', 4)
            .arg(rot_brownian_delta, 0, 'f', 4)
            .arg(rot_brownian_damped * 100.0, 0, 'f', 1));
    ui.info_pos_brownian_value->setText(
        tr("%1 / %2 / Δ%3 / %4%")
            .arg(pos_brownian_raw, 0, 'f', 4)
            .arg(pos_brownian_filtered, 0, 'f', 4)
            .arg(pos_brownian_delta, 0, 'f', 4)
            .arg(pos_brownian_damped * 100.0, 0, 'f', 1));
    ui.info_predictive_error_value->setText(
        tr("%1 / %2")
            .arg(rot_predictive_error, 0, 'f', 4)
            .arg(pos_predictive_error, 0, 'f', 4));
    ui.info_rot_contrib_value->setText(
        tr("EMA:%1 Br:%2 Ad:%3 Pr:%4 MTM:%5")
            .arg(rot_ema_drive, 0, 'f', 3)
            .arg(rot_brownian_drive, 0, 'f', 3)
            .arg(rot_adaptive_drive, 0, 'f', 3)
            .arg(rot_predictive_drive, 0, 'f', 3)
            .arg(rot_mtm_drive, 0, 'f', 3));
    ui.info_pos_contrib_value->setText(
        tr("EMA:%1 Br:%2 Ad:%3 Pr:%4 MTM:%5")
            .arg(pos_ema_drive, 0, 'f', 3)
            .arg(pos_brownian_drive, 0, 'f', 3)
            .arg(pos_adaptive_drive, 0, 'f', 3)
            .arg(pos_predictive_drive, 0, 'f', 3)
            .arg(pos_mtm_drive, 0, 'f', 3));
    ui.status_value->setText(
        active ?
            tr("Mon|E%1 B%2 A%3 P%4 M%5|rE%6 rP%7 pE%8 pP%9 k%10")
                .arg(*s.ema_enabled ? 1 : 0)
                .arg(*s.brownian_enabled ? 1 : 0)
                .arg(*s.adaptive_mode ? 1 : 0)
                .arg(*s.predictive_enabled ? 1 : 0)
                .arg(*s.mtm_enabled ? 1 : 0)
                .arg(rot_mode_expectation, 5, 'f', 3)
                .arg(rot_mode_peak, 5, 'f', 3)
                .arg(pos_mode_expectation, 5, 'f', 3)
                .arg(pos_mode_peak, 5, 'f', 3)
                .arg(ngc_coupling_residual, 5, 'f', 3)
            : tr("Idle"));

    if (commit_to_settings)
    {
        const auto rot_min_cfg = *s.rot_alpha_min;
        const auto rot_max_cfg = *s.rot_alpha_max;
        const auto rot_curve_cfg = *s.rot_curve;
        const auto rot_deadzone_cfg = *s.rot_deadzone;
        const auto pos_min_cfg = *s.pos_alpha_min;
        const auto pos_max_cfg = *s.pos_alpha_max;
        const auto pos_curve_cfg = *s.pos_curve;
        const auto pos_deadzone_cfg = *s.pos_deadzone;

        s.rot_alpha_min = options::slider_value{rot_min, rot_min_cfg.min(), rot_min_cfg.max()};
        s.rot_alpha_max = options::slider_value{rot_max, rot_max_cfg.min(), rot_max_cfg.max()};
        s.rot_curve = options::slider_value{rot_curve, rot_curve_cfg.min(), rot_curve_cfg.max()};
        s.rot_deadzone = options::slider_value{rot_deadzone, rot_deadzone_cfg.min(), rot_deadzone_cfg.max()};
        s.pos_alpha_min = options::slider_value{pos_min, pos_min_cfg.min(), pos_min_cfg.max()};
        s.pos_alpha_max = options::slider_value{pos_max, pos_max_cfg.min(), pos_max_cfg.max()};
        s.pos_curve = options::slider_value{pos_curve, pos_curve_cfg.min(), pos_curve_cfg.max()};
        s.pos_deadzone = options::slider_value{pos_deadzone, pos_deadzone_cfg.min(), pos_deadzone_cfg.max()};
    }
}

void dialog_alpha_spectrum::reset_to_defaults()
{
    s.rot_alpha_min.set_to_default();
    s.rot_alpha_max.set_to_default();
    s.rot_curve.set_to_default();
    s.rot_deadzone.set_to_default();
    s.pos_alpha_min.set_to_default();
    s.pos_alpha_max.set_to_default();
    s.pos_curve.set_to_default();
    s.pos_deadzone.set_to_default();
    s.brownian_head_gain.set_to_default();
    s.adaptive_threshold_lift.set_to_default();
    s.predictive_head_gain.set_to_default();
    s.mtm_shoulder_base.set_to_default();
    s.ngc_kappa.set_to_default();
    s.ngc_nominal_z.set_to_default();
    s.adaptive_mode.set_to_default();
    s.ema_enabled.set_to_default();
    s.brownian_enabled.set_to_default();
    s.predictive_enabled.set_to_default();
    s.mtm_enabled.set_to_default();
    s.advanced_mode_ui.set_to_default();
    s.b->save();
    s.b->reload();

    const double alpha_t = 0.5 * (norm(*s.rot_alpha_min, 0.005, 0.4) + norm(*s.rot_alpha_max, 0.02, 1.0));
    const double shape_t = 0.25 * (
        norm(*s.rot_curve, 0.2, 8.0) +
        norm(*s.pos_curve, 0.2, 8.0) +
        norm(*s.rot_deadzone, 0.0, 0.3) +
        norm(*s.pos_deadzone, 0.0, 2.0));

    {
        const QSignalBlocker b1(ui.simple_alpha_slider);
        const QSignalBlocker b2(ui.simple_shape_slider);
        ui.simple_alpha_slider->setValue(static_cast<int>(alpha_t * ui.simple_alpha_slider->maximum()));
        ui.simple_shape_slider->setValue(static_cast<int>(shape_t * ui.simple_shape_slider->maximum()));
    }

    ui.simple_alpha_label->setText(tr("Min %1% / Max %2%")
                                       .arg((*s.rot_alpha_min) * 100.0, 0, 'f', 1)
                                       .arg((*s.rot_alpha_max) * 100.0, 0, 'f', 1));
    ui.simple_shape_label->setText(tr("Curve %1 / RotDZ %2° / PosDZ %3mm")
                                       .arg(static_cast<double>(*s.rot_curve), 0, 'f', 2)
                                       .arg(static_cast<double>(*s.rot_deadzone), 0, 'f', 3)
                                       .arg(static_cast<double>(*s.pos_deadzone), 0, 'f', 3));

    ui.advanced_mode_check->setChecked(*s.advanced_mode_ui);
    ui.controls_frame->setEnabled(*s.advanced_mode_ui);
    ui.ema_enabled_check->setEnabled(*s.advanced_mode_ui);
    ui.brownian_enabled_check->setEnabled(*s.advanced_mode_ui);
    ui.predictive_enabled_check->setEnabled(*s.advanced_mode_ui);
    ui.mtm_enabled_check->setEnabled(*s.advanced_mode_ui);
}

void dialog_alpha_spectrum::doOK()
{
    save();
    close();
}

void dialog_alpha_spectrum::doCancel()
{
    close();
}

void dialog_alpha_spectrum::save()
{
    s.b->save();
}

void dialog_alpha_spectrum::reload()
{
    s.b->reload();
}

void dialog_alpha_spectrum::set_buttons_visible(bool x)
{
    ui.buttonBox->setVisible(x);
}
