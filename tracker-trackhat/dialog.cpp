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

    ui.exposure_slider->setMinimum((int)t.exposure->min());
    ui.exposure_slider->setMaximum((int)t.exposure->max());

    tie_setting(t.exposure, ui.exposure_slider);
    ui.exposure_label->setValue((int)*t.exposure);

    t.exposure.connect_to(ui.exposure_label, [this] { ui.exposure_label->setValue((int)*t.exposure); });

    // gain

    ui.gain_slider->setMinimum((int)t.gain->min());
    ui.gain_slider->setMaximum((int)t.gain->max());

    tie_setting(t.gain, ui.gain_slider);
    ui.gain_label->setValue((int)*t.gain);

    t.gain.connect_to(ui.gain_label, [this] { ui.gain_label->setValue((int)*t.gain); });

#if 0
    // threshold

    tie_setting(t.threshold, ui.threshold_slider);

    ui.threshold_label->setValue((int)*t.threshold);

    connect(&t.threshold, value_::value_changed<slider_value>(), ui.threshold_label, [=] {
            ui.threshold_label->setValue((int)*t.threshold);
        }, Qt::QueuedConnection);
#endif

    // point filter

    ui.point_filter_limit_label->setValue(*t.point_filter_limit);
    t.point_filter_limit.connect_to(ui.point_filter_limit_label, [this] {
        ui.point_filter_limit_label->setValue(*t.point_filter_limit);
    });

    tie_setting(t.enable_point_filter, ui.enable_point_filter);
    tie_setting(t.point_filter_coefficient, ui.point_filter_slider);
    ui.point_filter_label->setValue(*t.point_filter_coefficient);

    t.point_filter_coefficient.connect_to(ui.point_filter_label, [this] {
        ui.point_filter_label->setValue(*t.point_filter_coefficient);
    });

    tie_setting(t.point_filter_deadzone, ui.point_filter_deadzone);
    ui.point_filter_deadzone_label->setValue(*t.point_filter_deadzone);

    t.point_filter_deadzone.connect_to(ui.point_filter_deadzone_label, [this] {
        ui.point_filter_deadzone_label->setValue(*t.point_filter_deadzone);
    });

    // led

    using trackhat_impl::led_mode;
    ui.led_mode->setItemData(0, (int)led_mode::off);
    ui.led_mode->setItemData(1, (int)led_mode::constant);
    ui.led_mode->setItemData(2, (int)led_mode::dynamic);

    tie_setting(t.led, ui.led_mode);

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
    t.b->reload();
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
