#include "dialog.hpp"

using namespace options;

trackhat_dialog::trackhat_dialog()
{
    ui.setupUi(this);
    poll_tracker_info();
    poll_timer.setInterval(250);

    struct {
        const char* name;
        model_type t;
    } model_types[] = {
        { "Cap", model_cap },
        { "Clip (left)", model_clip_left },
        { "Clip (right)", model_clip_right },
        { "Mini Clip (left)", model_mini_clip_left },
        { "Mini Clip (right)", model_mini_clip_right },
        { "Mystery Meat", model_mystery_meat },
    };

    ui.model_type->clear();

    for (auto x : model_types)
        ui.model_type->addItem(QIcon{}, tr(x.name), (QVariant)(int)x.t);

    // model

    tie_setting(t.model, ui.model_type);
    tie_setting(t.min_pt_size, ui.min_point_size);
    tie_setting(t.max_pt_size, ui.max_point_size);

    // exposure

    tie_setting(t.exposure, ui.exposure_slider);
    tie_setting(t.gain, ui.gain_slider);
    ui.exposure_label->setValue((int)*t.exposure);
    ui.gain_label->setValue((int)*t.gain);

    connect(&t.exposure, value_::value_changed<slider_value>(), ui.exposure_label,
        [this] { ui.exposure_label->setValue((int)*t.exposure); }, Qt::QueuedConnection);
    connect(&t.gain, value_::value_changed<slider_value>(), ui.gain_label,
        [this] { ui.gain_label->setValue((int)*t.gain); }, Qt::QueuedConnection);

    // threshold

    connect(ui.threshold_slider, &QSlider::valueChanged, this, [this] (int value) {
            if (value <= ui.threshold_2_slider->value())
                ui.threshold_2_slider->setValue(value-1);
        }, Qt::DirectConnection);

    connect(ui.threshold_2_slider, &QSlider::valueChanged, this, [this] (int value) {
            if (value >= ui.threshold_slider->value())
                ui.threshold_slider->setValue(value+1);
        }, Qt::DirectConnection);

    tie_setting(t.threshold, ui.threshold_slider);
    tie_setting(t.threshold_2, ui.threshold_2_slider);

    ui.threshold_label->setValue((int)*t.threshold);
    ui.threshold_2_label->setValue((int)*t.threshold_2);

    connect(&t.threshold, value_::value_changed<slider_value>(), ui.threshold_label, [=] {
            ui.threshold_label->setValue((int)*t.threshold);
        }, Qt::QueuedConnection);

    connect(&t.threshold_2, value_::value_changed<slider_value>(), ui.threshold_2_label, [=] {
            ui.threshold_2_label->setValue((int)*t.threshold_2);
        }, Qt::QueuedConnection);

    // point filter

    tie_setting(t.enable_point_filter, ui.enable_point_filter);
    tie_setting(t.point_filter_coefficient, ui.point_filter_slider);
    ui.point_filter_label->setValue(*t.point_filter_coefficient);

    connect(&t.point_filter_coefficient, value_::value_changed<slider_value>(), ui.point_filter_label,
        [this] { ui.point_filter_label->setValue(*t.point_filter_coefficient); }, Qt::QueuedConnection);

    // stuff

    connect(&poll_timer, &QTimer::timeout, this, &trackhat_dialog::poll_tracker_info);
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &trackhat_dialog::doOK);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &trackhat_dialog::doCancel);
}

void trackhat_dialog::register_tracker(ITracker* tracker)
{
    tracker = static_cast<Tracker_PT*>(tracker);
    poll_timer.start();
}

void trackhat_dialog::unregister_tracker()
{
    tracker = nullptr;
    poll_timer.stop();
}

void trackhat_dialog::doOK()
{
    s.b->save();
    t.b->save();
    close();
}

void trackhat_dialog::doCancel()
{
    close();
}

void trackhat_dialog::poll_tracker_info()
{
    if (!tracker)
    {
        poll_timer.stop();
        ui.status_label->setText(tr("Tracking stopped."));
        return;
    }

    ui.status_label->setText(tr("Tracking. %1 points detected.").arg(tracker->get_n_points()));
}

trackhat_dialog::~trackhat_dialog() = default;
