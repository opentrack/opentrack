#include "dialog.hpp"

using namespace options;

trackhat_dialog::trackhat_dialog()
{
    ui.setupUi(this);
    poll_tracker_info();
    poll_timer.setInterval(100);

    const std::tuple<QString, model_type> model_types[] = {
        { tr("Cap"),                model_cap },
        { tr("Clip (left)"),        model_clip_left },
        { tr("Clip (right)"),       model_clip_right },
        { tr("Mini Clip (left)"),   model_mini_clip_left },
        { tr("Mini Clip (right)"),  model_mini_clip_right },
        { tr("Custom"),             model_mystery_meat },
    };

    ui.model_type->clear();

    for (const auto& [name, type] : model_types)
        ui.model_type->addItem(QIcon{}, name, (QVariant)(int)type);

    // model

    tie_setting(t.model, ui.model_type);
    tie_setting(t.min_pt_size, ui.min_point_size);
    tie_setting(t.max_pt_size, ui.max_point_size);
    tie_setting(t.point_filter_limit, ui.point_filter_limit);

    // exposure

    tie_setting(t.exposure, ui.exposure_slider);
    tie_setting(t.gain, ui.gain_slider);
    ui.exposure_label->setValue((int)*t.exposure);
    ui.gain_label->setValue((int)*t.gain);
    ui.point_filter_limit_label->setValue(*t.point_filter_limit);

    connect(&t.exposure, value_::value_changed<slider_value>(), ui.exposure_label,
        [this] { ui.exposure_label->setValue((int)*t.exposure); }, Qt::QueuedConnection);
    connect(&t.gain, value_::value_changed<slider_value>(), ui.gain_label,
        [this] { ui.gain_label->setValue((int)*t.gain); }, Qt::QueuedConnection);
    connect(&t.point_filter_limit, value_::value_changed<slider_value>(), ui.point_filter_limit_label,
        [this] { ui.point_filter_limit_label->setValue(*t.point_filter_limit); }, Qt::QueuedConnection);

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

void trackhat_dialog::register_tracker(ITracker* tracker_)
{
    tracker = static_cast<Tracker_PT*>(tracker_);
    poll_tracker_info();
    poll_timer.start();
}

void trackhat_dialog::unregister_tracker()
{
    tracker = nullptr;
    poll_tracker_info();
    poll_timer.stop();
    update_raw_data();
}

void trackhat_dialog::save()
{
    s.b->save();
    t.b->save();
}

void trackhat_dialog::reload()
{
    s.b->reload();
    s.b->reload();
}

void trackhat_dialog::doCancel() { reload(); close(); }
void trackhat_dialog::doOK() { save(); close(); }

trackhat_dialog::~trackhat_dialog()
{
}

void trackhat_dialog::poll_tracker_info()
{
    if (!tracker)
        ui.status_label->setText(tr("Status: Tracking stopped."));
    else if (tracker->get_n_points() == 3)
        ui.status_label->setText(tr("Status: %1 points detected. Good!").arg(tracker->get_n_points()));
    else
        ui.status_label->setText(tr("Status: %1 points detected. BAD!").arg(tracker->get_n_points()));
    update_raw_data();
}

void trackhat_dialog::set_buttons_visible(bool x)
{
    ui.buttonBox->setVisible(x);
    adjustSize();
}
void trackhat_dialog::update_raw_data()
{
    QLabel* labels[] = { ui.label_x, ui.label_y, ui.label_z, ui.label_yaw, ui.label_pitch, ui.label_roll };
    if (tracker)
    {
        QString str; str.reserve(16);
        double data[6] {};
        tracker->data(data);
        for (unsigned i = 0; i < std::size(labels); i++)
            labels[i]->setText(str.sprintf("%.2f%s", data[i], i >= 3 ? "°" : " mm"));
    }
    else
        for (QLabel* x : labels)
            x->setText(QStringLiteral("-"));
}
